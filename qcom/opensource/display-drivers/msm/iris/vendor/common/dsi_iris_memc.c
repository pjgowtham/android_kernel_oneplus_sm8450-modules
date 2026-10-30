// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2020, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */

#include <linux/types.h>
#include <dsi_drm.h>
#include "dsi_display.h"
#include "dsi_panel.h"
#include "dsi_iris_api.h"
#include "dsi_iris_lightup.h"
#include "pw_iris_log.h"
#include "dsi_iris_memc.h"

int iris_get_main_panel_timing_info(struct iris_panel_timing_info *timing_info)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (!pcfg_ven || !pcfg_ven->panel || !pcfg_ven->panel->cur_mode || !timing_info) {
		IRIS_LOGE("%s(), cannot get timing info of curr mode!", __func__);
		return -EINVAL;
	}

	timing_info->flag = timing_info->flag; //not change
	timing_info->width = pcfg_ven->panel->cur_mode->timing.h_active;
	timing_info->height = pcfg_ven->panel->cur_mode->timing.v_active;
	timing_info->fps = pcfg_ven->panel->cur_mode->timing.refresh_rate;
	timing_info->dsc = pcfg_ven->panel->cur_mode->timing.dsc_enabled;
	timing_info->h_back_porch = pcfg_ven->panel->cur_mode->timing.h_back_porch;
	timing_info->h_sync_width = pcfg_ven->panel->cur_mode->timing.h_sync_width;
	timing_info->h_front_porch = pcfg_ven->panel->cur_mode->timing.h_front_porch;
	timing_info->v_back_porch = pcfg_ven->panel->cur_mode->timing.v_back_porch;
	timing_info->v_sync_width = pcfg_ven->panel->cur_mode->timing.v_sync_width;
	timing_info->v_front_porch = pcfg_ven->panel->cur_mode->timing.v_front_porch;
	timing_info->dsi_xfer_ms = (pcfg_ven->panel->cur_mode->priv_info) ?
		pcfg_ven->panel->cur_mode->priv_info->dsi_transfer_time_us/1000 : 0;

	IRIS_LOGD("%s(), flag: %d, width: %d, height: %d, fps: %d, dsc_en: %d,"
		"h_back_porch: %d, h_sync_width: %d, h_front_porch: %d, v_back_porch: %d,"
		"v_sync_width: %d, v_front_porch: %d", __func__, timing_info->flag, timing_info->width,
		timing_info->height, timing_info->fps, timing_info->dsc, timing_info->h_back_porch,
		timing_info->h_sync_width, timing_info->h_front_porch, timing_info->v_back_porch,
		timing_info->v_sync_width, timing_info->v_front_porch);

	return 0;
}

int iris_get_main_panel_curr_mode_dsc_en(bool *dsc_en)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (!pcfg_ven || !pcfg_ven->panel || !pcfg_ven->panel->cur_mode
			|| !pcfg_ven->panel->cur_mode->priv_info || !dsc_en) {
		IRIS_LOGE("%s(), cannot get dsc_en info of curr mode!", __func__);
		return -EINVAL;
	}

	*dsc_en = pcfg_ven->panel->cur_mode->priv_info->dsc_enabled;
	IRIS_LOGD("%s(), dsc_en: %d", __func__, *dsc_en);

	return 0;
}

int iris_get_aux_panel_timing_info(struct iris_panel_timing_info *timing_info)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (!pcfg_ven || !pcfg_ven->panel2 || !pcfg_ven->panel2->cur_mode || !timing_info) {
		return -EINVAL;
	}

	timing_info->flag = timing_info->flag; //not change
	timing_info->width = pcfg_ven->panel2->cur_mode->timing.h_active;
	timing_info->height = pcfg_ven->panel2->cur_mode->timing.v_active;
	timing_info->fps = pcfg_ven->panel2->cur_mode->timing.refresh_rate;
	timing_info->dsc = pcfg_ven->panel2->cur_mode->timing.dsc_enabled;
	timing_info->h_back_porch = pcfg_ven->panel2->cur_mode->timing.h_back_porch;
	timing_info->h_sync_width = pcfg_ven->panel2->cur_mode->timing.h_sync_width;
	timing_info->h_front_porch = pcfg_ven->panel2->cur_mode->timing.h_front_porch;
	timing_info->v_back_porch = pcfg_ven->panel2->cur_mode->timing.v_back_porch;
	timing_info->v_sync_width = pcfg_ven->panel2->cur_mode->timing.v_sync_width;
	timing_info->v_front_porch = pcfg_ven->panel2->cur_mode->timing.v_front_porch;
	timing_info->dsi_xfer_ms = (pcfg_ven->panel2->cur_mode->priv_info) ?
		pcfg_ven->panel2->cur_mode->priv_info->dsi_transfer_time_us/1000 : 0;

	IRIS_LOGD("%s(), flag: %d, width: %d, height: %d, fps: %d, dsc_en: %d,"
		"h_back_porch: %d, h_sync_width: %d, h_front_porch: %d, v_back_porch: %d,"
		"v_sync_width: %d, v_front_porch: %d", __func__, timing_info->flag, timing_info->width,
		timing_info->height, timing_info->fps, timing_info->dsc, timing_info->h_back_porch,
		timing_info->h_sync_width, timing_info->h_front_porch, timing_info->v_back_porch,
		timing_info->v_sync_width, timing_info->v_front_porch);

	return 0;
}

int iris_get_aux_panel_curr_mode_dsc_en(bool *dsc_en)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (!pcfg_ven || !pcfg_ven->panel2 || !pcfg_ven->panel2->cur_mode
			|| !pcfg_ven->panel2->cur_mode->priv_info || !dsc_en) {
		return -EINVAL;
	}

	*dsc_en = pcfg_ven->panel2->cur_mode->priv_info->dsc_enabled;
	IRIS_LOGD("%s(), dsc_en: %d", __func__, *dsc_en);

	return 0;
}

int iris_get_aux_panel_curr_mode_dsc_size(uint32_t *slice_width, uint32_t *slice_height)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (!pcfg_ven || !pcfg_ven->panel2 || !pcfg_ven->panel2->cur_mode
			|| !pcfg_ven->panel2->cur_mode->priv_info) {
		return -EINVAL;
	}

	*slice_width = pcfg_ven->panel2->cur_mode->priv_info->dsc.config.slice_width;
	*slice_height = pcfg_ven->panel2->cur_mode->priv_info->dsc.config.slice_height;
	IRIS_LOGD("%s(), slice_width: %d, slice_height:%d", __func__, *slice_width, *slice_height);

	return 0;
}

int iris_try_panel_lock(void) {
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (!pcfg_ven || !pcfg_ven->panel) {
		IRIS_LOGE("%s(), invalid params!", __func__);
		return -EINVAL;
	}

	return mutex_trylock(&pcfg_ven->panel->panel_lock);
}

unsigned long long iris_set_idle_check_interval(unsigned int crtc_id, unsigned long long new_interval)
{
	return 0;
}
