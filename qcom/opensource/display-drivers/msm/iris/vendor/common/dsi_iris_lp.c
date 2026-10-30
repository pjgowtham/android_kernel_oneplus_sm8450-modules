// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2020, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */
#include <video/mipi_display.h>
#include <drm/drm_bridge.h>
#include <drm/drm_encoder.h>
#include "dsi_drm.h"
#include <sde_encoder.h>
#include <sde_encoder_phys.h>
#include <sde_connector.h>
#include "dsi_iris_api.h"
#include "dsi_iris_lightup.h"
#include "pw_iris_lightup_ocp.h"
#include "dsi_iris_lp.h"
#include "pw_iris_lp.h"
#include "pw_iris_timing_switch.h"
#include "pw_iris_log.h"

static struct drm_encoder *iris_get_drm_encoder_handle(void)
{
	struct iris_vendor_cfg *pcfg_ven;

	pcfg_ven = iris_get_vendor_cfg();

	if (pcfg_ven->display->bridge == NULL || pcfg_ven->display->bridge->base.encoder == NULL) {
		IRIS_LOGE("Can not get drm encoder");
		return NULL;
	}

	return pcfg_ven->display->bridge->base.encoder;
}

void _iris_wait_prev_frame_done(void)
{
	int i = 0;
	struct drm_encoder *drm_enc = iris_get_drm_encoder_handle();
	struct sde_encoder_virt *sde_enc = NULL;

	if (!drm_enc) {
		IRIS_LOGE("invalid encoder\n");
		return;
	}

	sde_enc = to_sde_encoder_virt(drm_enc);
	for (i = 0; i < sde_enc->num_phys_encs; i++) {
		struct sde_encoder_phys *phys = sde_enc->phys_encs[i];
		int pending_cnt = 0;

		if (phys->split_role != ENC_ROLE_SLAVE) {
			int j = 0;

			pending_cnt = atomic_read(&phys->pending_kickoff_cnt);
			for (j = 0; j < pending_cnt; j++)
				sde_encoder_wait_for_event(phys->parent, MSM_ENC_TX_COMPLETE);

			break;
		}
	}
}

int iris_prepare_for_kickoff(void *phys_enc)
{
	struct sde_encoder_phys *phys_encoder = phys_enc;
	struct sde_connector *c_conn = NULL;
	struct dsi_display *display = NULL;
	struct iris_cfg *pcfg;
	struct iris_vendor_cfg *pcfg_ven;
	int mode;

	if (phys_encoder == NULL)
		return -EFAULT;
	if (phys_encoder->connector == NULL)
		return -EFAULT;

	c_conn = to_sde_connector(phys_encoder->connector);
	if (c_conn == NULL)
		return -EFAULT;

	if (c_conn->connector_type != DRM_MODE_CONNECTOR_DSI)
		return 0;

	display = c_conn->display;
	if (display == NULL)
		return -EFAULT;

	pcfg = iris_get_cfg();
	if (pcfg->valid < PARAM_PREPARED)
		return 0;

	pcfg_ven = iris_get_vendor_cfg();
#if defined(CHECK_KICKOFF_FPS_CADNENCE)
	if (iris_virtual_display(display)) {
		pcfg->ktime_kickoff_aux_last = pcfg->ktime_kickoff_aux;
		pcfg->ktime_kickoff_aux = ktime_get();
		iris_check_kickoff_fps_cadence_2nd();
		//return 0;
	} else {
		pcfg->ktime_kickoff_main_last = pcfg->ktime_kickoff_main;
		pcfg->ktime_kickoff_main = ktime_get();
		iris_check_kickoff_fps_cadence();
	}
#endif
	if (pcfg->rx_mode == IRIS_CMD_MODE) {
		mutex_lock(&pcfg->abyp_ctrl.abypass_mutex);
		mutex_unlock(&pcfg->abyp_ctrl.abypass_mutex);
	}
	if (pcfg->abyp_ctrl.pending_mode != MAX_MODE) {
		mutex_lock(&pcfg->abyp_ctrl.abypass_mutex);
		mode = pcfg->abyp_ctrl.pending_mode;
		pcfg->abyp_ctrl.pending_mode = MAX_MODE;
		mutex_unlock(&pcfg->abyp_ctrl.abypass_mutex);

		if (pcfg->lightup_ops.acquire_panel_lock)
			pcfg->lightup_ops.acquire_panel_lock();
		iris_abyp_switch_proc(mode);
		if (pcfg->lightup_ops.release_panel_lock)
			pcfg->lightup_ops.release_panel_lock();
	}

	iris_set_metadata(true);
	iris_restore_capen();

	return 0;
}

bool iris_check_reg_read(struct dsi_panel *panel)
{
	int i, j = 0;
	int len = 0, *lenp;
	int group = 0, count = 0;
	struct drm_panel_esd_config *config;
	struct iris_cfg *pcfg;

	pcfg = iris_get_cfg();

	if (!panel)
		return false;

	if (!(iris_esd_ctrl_get() & 0x8) && !(IRIS_IF_LOGD()))
		return false;

	config = &(panel->esd_config);

	lenp = config->status_valid_params ?: config->status_cmds_rlen;
	count = config->status_cmd.count;

	for (i = 0; i < count; i++)
		len += lenp[i];

	for (j = 0; j < config->groups; ++j) {
		for (i = 0; i < len; ++i) {
			IRIS_LOGI("panel esd [%d] - [%d] : 0x%x", j, i, config->return_buf[i]);

			if (config->return_buf[i] != config->status_value[group + i]) {
				pcfg->lp_ctrl.esd_cnt_panel++;
				IRIS_LOGI("mismatch: 0x%x != 0x%x. Cnt:%d", config->return_buf[i],
					config->status_value[group + i], pcfg->lp_ctrl.esd_cnt_panel);
				break;
			}
		}

		if (i == len)
			return true;
		group += len;
	}

	return false;
}

void iris_set_esd_status(bool enable)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();
	struct dsi_display *display = pcfg_ven->display;

	if (!display)
		return;

	if (!pcfg_ven->panel || !display->drm_conn)
		return;

	if (!enable) {
		if (display->panel->esd_config.esd_enabled) {
			sde_connector_schedule_status_work(display->drm_conn, false);
			display->panel->esd_config.esd_enabled = false;
			IRIS_LOGD("disable esd work");
		}
	} else {
		if (!display->panel->esd_config.esd_enabled) {
			sde_connector_schedule_status_work(display->drm_conn, true);
			display->panel->esd_config.esd_enabled = true;
			IRIS_LOGD("enabled esd work");
		}
	}
}

u8 dsi_op_mode_to_iris_op_mode(u8 dsi_mode)
{
	u8 iris_mode;

	switch (dsi_mode) {
	case DSI_OP_VIDEO_MODE:
		iris_mode = IRIS_VIDEO_MODE;
		break;
	case DSI_OP_CMD_MODE:
		iris_mode = IRIS_CMD_MODE;
		break;
	case DSI_OP_MODE_MAX:
		iris_mode = IRIS_MODE_MAX;
		break;
	default:
		iris_mode = IRIS_MODE_MAX;
		break;
	}
	return iris_mode;
}

// prepare_commit before prepare_for_kickoff
int iris_prepare_commit(void *phys_enc)
{
	struct sde_encoder_phys *phys_encoder = phys_enc;
	struct sde_connector *c_conn = NULL;
	struct dsi_display *display = NULL;
	struct iris_cfg *pcfg = iris_get_cfg();

	if (phys_encoder == NULL)
		return -EFAULT;
	if (phys_encoder->connector == NULL)
		return -EFAULT;

	c_conn = to_sde_connector(phys_encoder->connector);
	if (c_conn == NULL)
		return -EFAULT;

	if (c_conn->connector_type != DRM_MODE_CONNECTOR_DSI)
		return 0;

	display = c_conn->display;
	if (display == NULL)
		return -EFAULT;

	if (iris_virtual_display(display)) {
		// retain iris_osd_autorefresh
		pcfg->iris_cur_osd_autorefresh = pcfg->iris_osd_autorefresh_enabled;
	}
	return 0;
}

