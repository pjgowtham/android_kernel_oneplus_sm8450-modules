// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2020, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */

#include "dsi_display.h"
#include "dsi_iris_lightup.h"
#include "pw_iris_log.h"

int iris_debug_display_info_get(char *kbuf, int size)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();
	struct dsi_panel *panel;
	int len = 0;

	panel = pcfg_ven->panel;
	if (panel) {
		len += snprintf(kbuf, size,
				"%-20s:\t%s\n", "panel name", panel->name);
		len += snprintf(kbuf + len, size - len,
			"%-20s:\t%d\n", "panel state", panel->panel_initialized);

		if (panel->cur_mode && panel->cur_mode->priv_info) {
			len += snprintf(kbuf + len, size - len,
					"%-20s:\t%dx%d@%d\n", "panel timing", panel->cur_mode->timing.h_active,
					panel->cur_mode->timing.v_active, panel->cur_mode->timing.refresh_rate);

			len += snprintf(kbuf + len, size - len,
					"%-20s:\t%d/%d\n", "panel transfer time",
					panel->cur_mode->priv_info->mdp_transfer_time_us,
					panel->cur_mode->priv_info->dsi_transfer_time_us);

			len += snprintf(kbuf + len, size - len,
					"%-20s:\t%llu\n", "panel clock", panel->cur_mode->priv_info->clk_rate_hz);

			len += snprintf(kbuf + len, size - len,
					"%-20s:\t%d\n", "panel dsc", panel->cur_mode->priv_info->dsc_enabled);

			if (panel->cur_mode->priv_info->dsc_enabled) {
				len += snprintf(kbuf + len, size - len,
						"%-20s:\t%d\n", "panel dsc bpc",
						panel->cur_mode->priv_info->dsc.config.bits_per_component);
				len += snprintf(kbuf + len, size - len,
						"%-20s:\t%d\n", "panel dsc bpp",
						panel->cur_mode->priv_info->dsc.config.bits_per_pixel >> 4);
			}
		}
	}

	panel = pcfg_ven->panel2;
	if (panel) {
		len += snprintf(kbuf + len, size - len,
				"%-20s:\t%s\n", "panel name", panel->name);
		len += snprintf(kbuf + len, size - len,
			"%-20s:\t%d\n", "panel state", panel->panel_initialized);

		if (panel->cur_mode && panel->cur_mode->priv_info) {
			len += snprintf(kbuf + len, size - len,
					"%-20s:\t%dx%d@%d\n", "panel timing", panel->cur_mode->timing.h_active,
					panel->cur_mode->timing.v_active, panel->cur_mode->timing.refresh_rate);

			len += snprintf(kbuf + len, size - len,
					"%-20s:\t%d/%d\n", "panel transfer time",
					panel->cur_mode->priv_info->mdp_transfer_time_us,
					panel->cur_mode->priv_info->dsi_transfer_time_us);

			len += snprintf(kbuf + len, size - len,
					"%-20s:\t%llu\n", "panel clock", panel->cur_mode->priv_info->clk_rate_hz);

			len += snprintf(kbuf + len, size - len,
					"%-20s:\t%d\n", "panel dsc", panel->cur_mode->priv_info->dsc_enabled);

			if (panel->cur_mode->priv_info->dsc_enabled) {
				len += snprintf(kbuf + len, size - len,
						"%-20s:\t%d\n", "panel dsc bpc",
						panel->cur_mode->priv_info->dsc.config.bits_per_component);
				len += snprintf(kbuf + len, size - len,
						"%-20s:\t%d\n", "panel dsc bpp",
						panel->cur_mode->priv_info->dsc.config.bits_per_pixel >> 4);
			}
		}
	}

	return len;
}

static ssize_t iris_dbg_display_info_show(struct file *file, char __user *ubuf,
		size_t count, loff_t *ppos)
{
	char *kbuf = NULL;
	int size = count < PAGE_SIZE ? PAGE_SIZE : count;

	if (*ppos)
		return 0;

	kbuf = vzalloc(size);
	if (kbuf == NULL) {
		IRIS_LOGE("Fatal erorr: No mem!\n");
		return -ENOMEM;
	}

	size = iris_debug_display_info_get(kbuf, size);
	if (size >= count)
		size = count - 1;

	if (copy_to_user(ubuf, kbuf, size)) {
		vfree(kbuf);
		return -EFAULT;
	}

	vfree(kbuf);

	*ppos += size;

	return size;
}

static const struct file_operations iris_dbg_dislay_info_fops = {
	.open = simple_open,
	.read = iris_dbg_display_info_show,
};

int iris_dbgfs_status_init(void *display)
{
	struct iris_cfg *pcfg = iris_get_cfg();

#if 0
	if (pcfg->iris_kobj) {
		retval = sysfs_create_group(pcfg->iris_kobj, &iris_dbg_attr_group);
		if (retval) {
			kobject_put(pcfg->iris_kobj);
			IRIS_LOGE("sysfs create display_mode node fail");
		} else{
			IRIS_LOGI("sysfs create display_mode node successfully");
		}
	}
#endif
	if (pcfg->dbg_root == NULL) {
		pcfg->dbg_root = debugfs_create_dir(IRIS_DBG_TOP_DIR, NULL);
		if (IS_ERR_OR_NULL(pcfg->dbg_root)) {
			IRIS_LOGE("create dir for iris failed, error %ld",
					PTR_ERR(pcfg->dbg_root));
			return -ENODEV;
		}
	}

	if (debugfs_create_file("display_info", 0644,
				pcfg->dbg_root, display,
				&iris_dbg_dislay_info_fops) == NULL)
		IRIS_LOGE("create file display_info failed");
	return  pw_dbgfs_status_init(display);
}
