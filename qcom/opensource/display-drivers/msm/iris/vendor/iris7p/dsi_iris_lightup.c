// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2020, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */
#include <drm/drm_mipi_dsi.h>
#include <video/mipi_display.h>
#include <dsi_drm.h>
#include <sde_encoder_phys.h>
#include "dsi_parser.h"
#include "dsi_iris_api.h"
#include "dsi_iris_lightup.h"
#include "pw_iris_lightup_ocp.h"
#include "dsi_iris_lp.h"
#include "pw_iris_lp.h"
#include "pw_iris_pq.h"
#include "pw_iris_ioctl.h"
#include "pw_iris_lut.h"
#include "pw_iris_gpio.h"
#include "pw_iris_timing_switch.h"
#include "pw_iris_timing_switch.h"
#include "pw_iris_loop_back.h"
#include "pw_iris_log.h"
#include "pw_iris_memc.h"
#include "pw_iris_i3c.h"
#include "pw_iris_i2c.h"
#include "pw_iris_memc_helper.h"
#include "pw_iris_dts_fw.h"
#include "dsi_iris_memc.h"
#include "dsi_iris_cmpt.h"

void iris_init_i7p(struct dsi_display *display, struct dsi_panel *panel)
{
	struct iris_cfg *pcfg = iris_get_cfg();
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();
#ifdef IRIS_EXT_CLK
	struct platform_device *pdev = pcfg->pdev;
#endif

	IRIS_LOGI("%s(), for dispaly: %s, panel: %s",
			__func__,
			display->display_type, panel->name);

	if (iris_virtual_display(display)) {
		pcfg_ven->display2 = display;
		pcfg_ven->panel2 = panel;
		return;
	}

	iris_driver_ops_init_i7p();
	pcfg_ven->display = display;
	pcfg_ven->panel = panel;
	pcfg->panel_name = panel->name;
	pcfg->iris_i2c_read = iris_pure_i2c_single_read;
	pcfg->iris_i2c_write = iris_pure_i2c_single_write;
	pcfg->iris_i2c_burst_write = iris_pure_i2c_burst_write;
	pcfg->iris_i2c_conver_ocp_write = NULL;
	pcfg->lightup_ops.acquire_panel_lock = dsi_iris_acquire_panel_lock;
	pcfg->lightup_ops.release_panel_lock = dsi_iris_release_panel_lock;
	pcfg->lightup_ops.transfer = iris_dsi_send_cmds;
	pcfg->lightup_ops.obtain_cur_timing_info = dsi_iris_obtain_cur_timing_info;
	pcfg->lightup_ops.get_panel2_power_refcount = dsi_iris_get_panel2_power_refcount;
	pcfg->lightup_ops.get_display_info = iris_debug_display_info_get;
	pcfg->lightup_ops.wait_vsync = iris_wait_vsync;
	pcfg->lightup_ops.send_pwil_cmd = iris_send_pwil_cmd;
	pcfg->lightup_ops.change_header = NULL;
	pcfg->get_panel_mode = dsi_iris_get_panel_mode;
	pcfg->ioctl_ops.get_selected_configure = NULL;
	pcfg->iris_memc_ops.iris_memc_get_main_panel_timing_info = iris_get_main_panel_timing_info;
	pcfg->iris_memc_ops.iris_memc_get_main_panel_dsc_en_info = iris_get_main_panel_curr_mode_dsc_en;
	pcfg->iris_memc_ops.iris_set_idle_check_interval = iris_set_idle_check_interval;
	pcfg->wait_pre_framedone = _iris_wait_prev_frame_done;
	pcfg->iris_core_lightup = iris_core_lightup;
#ifdef IRIS_EXT_CLK
	pcfg->iris_clk_set = iris_core_clk_set;
#endif
	pcfg->platform_ops.fill_desc_para = NULL;
	pcfg->set_esd_status = iris_set_esd_status;
	pcfg->iris_is_read_cmd = iris_is_read_cmd;
	pcfg->iris_is_last_cmd = iris_is_last_cmd;
	pcfg->iris_is_curmode_cmd_mode = iris_is_curmode_cmd_mode;
	pcfg->iris_is_curmode_vid_mode = iris_is_curmode_vid_mode;
	pcfg->iris_set_msg_flags = iris_set_msg_flags;
	pcfg->iris_switch_cmd_type = iris_switch_cmd_type;
	pcfg->iris_set_msg_ctrl = iris_set_msg_ctrl;
	pcfg->iris_set_cmdq_handle_in_switch = NULL;
	pcfg->iris_vdo_mode_send_cmd_with_handle = NULL;
	pcfg->iris_vdo_mode_send_short_cmd_with_handle = NULL;
	pcfg->iris_is_cmdq_empty = NULL;

	pcfg->aod = false;
	pcfg->fod = false;
	pcfg->fod_pending = false;
	pcfg->abyp_ctrl.abypass_mode = ANALOG_BYPASS_MODE; //default abyp
	pcfg->n2m_ratio = 1;
	pcfg->dtg_ctrl_pt = 0;
	pcfg->iris_pwil_mode_state = 2;

	pcfg->frc_label = 0;
	pcfg->frc_demo_window = 0;
	pcfg->dev = &(display->pdev->dev);
	pcfg->bl_max_level = panel->bl_config.bl_max_level;
	pcfg->crtc0_old_interval = 0;
	pcfg->dsi_dev = NULL;
	pcfg->panel_backlight_in_pt = 0;

	atomic_set(&pcfg->fod_cnt, 0);

	iris_init_memc();
	iris_init_timing_switch();
	iris_lp_init();
	pcfg->lp_ctrl.force_exit_ulps_during_switching = panel->ulps_feature_enabled;
	pcfg->iris_mipi1_power_on_pending_en = true;
	pcfg->memc_chain_en = false;

#ifdef IRIS_EXT_CLK // skip ext clk
	if (pdev && iris_clk_parse(pdev->dev.of_node)) {
		IRIS_LOGI("Get clk from iris node");
		pcfg->ext_clk = devm_clk_get(&pdev->dev, IRIS_EXT_CLK_NAME);
	} else if (iris_clk_parse(pcfg->dev->of_node)) {
		IRIS_LOGI("Get clk from dsi node");
		pcfg->ext_clk = devm_clk_get(&display->pdev->dev, IRIS_EXT_CLK_NAME);
	} else {
		pcfg->ext_clk = NULL;
	}
	if (IS_ERR_OR_NULL(pcfg->ext_clk)) {
		IRIS_LOGE("%s(%d) --- failed to get ext_clk", __func__, __LINE__);
        }
#endif

	if (!iris_virtual_display(display)) {
		pw_iris_dbgfs_lp_init(display);
		iris_dbgfs_pq_init();
		iris_dbgfs_cont_splash_init(display);
		iris_dbgfs_memc_init();
		iris_dbgfs_loop_back_init(display);
		iris_dbgfs_adb_type_init(display);
		iris_dbgfs_fw_calibrate_status_init();
		iris_dbgfs_status_init(display);
		iris_dbgfs_scl_init();
		iris_dbg_gpio_init();
	}
	iris_get_vreg();
	mutex_init(&pcfg->gs_mutex);
	mutex_init(&pcfg->ioctl_mutex);
	mutex_init(&pcfg->i2c_read_mutex);
	//iris_driver_register();
}

int iris_lightoff_i7p(struct dsi_panel *panel, bool dead,
		struct iris_cmd_set *off_cmds)
{
	struct iris_cfg *pcfg = iris_get_cfg();
	int lightup_opt = iris_lightup_opt_get();

	if (pcfg->valid < PARAM_PREPARED) {
		if (panel && !panel->is_secondary && off_cmds)
			iris_abyp_send_panel_cmd(off_cmds);
		return 0;
	}

	pcfg->metadata = 0; // clean metadata
	pcfg->dtg_ctrl_pt = 0;

	if (!panel || panel->is_secondary) {
		IRIS_LOGD("no need to light off for 2nd panel.");
		return 0;
	}

	if ((lightup_opt & 0x10) == 0)
		pcfg->abyp_ctrl.abypass_mode = ANALOG_BYPASS_MODE; //clear to ABYP mode

	IRIS_LOGI("%s(%d), panel %s, mode: %s(%d) ---", __func__, __LINE__,
			dead ? "dead" : "alive",
			pcfg->abyp_ctrl.abypass_mode == PASS_THROUGH_MODE ? "PT" : "ABYP",
			pcfg->abyp_ctrl.abypass_mode);
	if (off_cmds && (!dead)) {
		if (pcfg->abyp_ctrl.abypass_mode == PASS_THROUGH_MODE)
			iris_pt_send_panel_cmd(off_cmds);
		else
			iris_abyp_send_panel_cmd(off_cmds);
	}
	iris_lightoff_memc();
	iris_quality_setting_off();
	iris_lp_setting_off();
	iris_memc_setting_off();
	iris_dtg_update_reset();
	iris_clear_aod_state();
	pcfg->panel_pending = 0;
	iris_set_pinctrl_state(false);

	if (pcfg->crtc0_old_interval != 0
		&& pcfg->iris_memc_ops.iris_set_idle_check_interval)
		pcfg->iris_memc_ops.iris_set_idle_check_interval(0, pcfg->crtc0_old_interval);

	IRIS_LOGI("%s(%d) ---", __func__, __LINE__);

	return 0;
}

//module_platform_driver(iris_driver);
