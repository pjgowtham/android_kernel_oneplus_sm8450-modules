// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2020, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */
//#include <drm/drm_mipi_dsi.h>
#include <video/mipi_display.h>
#include <dsi_drm.h>
#include <sde_encoder_phys.h>
#include "oplus_display_ext.h"
#include "dsi_parser.h"
#include "dsi_iris_api.h"
#include "dsi_iris_lightup.h"
#if defined(CONFIG_PXLW_IRIS5)
#include "pw_iris5_frc.h"
#include "pw_iris5_pq.h"
#include "pw_iris5_lp.h"
#endif
#include "pw_iris_lightup_ocp.h"
#include "pw_iris_loop_back.h"
#include "dsi_iris_lp.h"
#include "pw_iris_lp.h"
#include "pw_iris_pq.h"
#include "pw_iris_ioctl.h"
#include "pw_iris_lut.h"
#include "pw_iris_gpio.h"
#include "pw_iris_timing_switch.h"
#include "pw_iris_log.h"
#include "pw_iris_memc.h"
#include "pw_iris_memc_helper.h"
#include "pw_iris_i3c.h"
#include "pw_iris_i2c.h"
#include "pw_iris_memc_helper.h"
#include "pw_iris_dts_fw.h"
#include "pw_iris_def.h"

#define to_dsi_display(x) container_of(x, struct dsi_display, host)


static struct iris_vendor_cfg gcfg_ext = {
	.display = NULL,
	.panel = NULL,
	.display2 = NULL,
	.panel2 = NULL,
};

static void _iris_send_cont_splash_pkt(uint32_t type);

static void _iris_i2c_preload_work(struct work_struct *work)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	iris_get_cfg()->iris_i2c_preload = true;
	iris_enable(pcfg_ven->panel, NULL);
	iris_mipi_domain_power_down();
	iris_get_cfg()->iris_i2c_preload = false;
}

static void _iris_i2c_preload_work_init(void)
{
	struct iris_cfg *pcfg = iris_get_cfg();
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	pcfg->iris_i2c_preload = false;

	if (pcfg_ven->panel && pcfg->valid >= PARAM_PARSED
			&& (pcfg_ven->panel->panel_mode == DSI_OP_VIDEO_MODE))
		INIT_WORK(&iris_get_cfg()->iris_i2c_preload_work, _iris_i2c_preload_work);
}

void dsi_iris_acquire_panel_lock(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (pcfg_ven->panel)
		mutex_lock(&pcfg_ven->panel->panel_lock);
}

void dsi_iris_release_panel_lock(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (pcfg_ven->panel)
		mutex_unlock(&pcfg_ven->panel->panel_lock);
}

int dsi_iris_obtain_cur_timing_info(struct iris_mode_info *timing_info)
{
	int ret = -EINVAL;
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (timing_info && pcfg_ven->panel && pcfg_ven->panel->cur_mode) {
		dsi_mode_to_iris_mode(timing_info, &pcfg_ven->panel->cur_mode->timing);
		ret = 0;
	}
	return ret;
}

u32 dsi_iris_get_panel2_power_refcount(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();
	if (pcfg_ven && pcfg_ven->panel2->power_info.refcount)
		return pcfg_ven->panel2->power_info.refcount;

	return 0;
}

int dsi_iris_get_panel_mode(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (pcfg_ven->panel)
		return dsi_op_mode_to_iris_op_mode(pcfg_ven->panel->panel_mode);

	return IRIS_CMD_MODE;
}

void iris_send_pwil_cmd(struct iris_cmd_set *pcmdset, u32 addr, u32 meta)
{
	struct iris_cfg *pcfg = iris_get_cfg();

	if (!pcmdset)
		return;

	iris_dsi_send_cmds(pcmdset->cmds,
				pcmdset->count, pcmdset->state, pcfg->vc_ctrl.to_iris_vc_id);
}

int iris_get_vreg(void)
{
	int rc = 0;
	int i;
	struct regulator *vreg = NULL;
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();
	struct dsi_panel *panel = pcfg_ven->panel;

	for (i = 0; i < pcfg_ven->iris_power_info.count; i++) {
		vreg = devm_regulator_get(panel->parent,
				pcfg_ven->iris_power_info.vregs[i].vreg_name);
		rc = IS_ERR(vreg);
		if (rc) {
			IRIS_LOGE("failed to get %s regulator",
					pcfg_ven->iris_power_info.vregs[i].vreg_name);
			goto error_put;
		}
		pcfg_ven->iris_power_info.vregs[i].vreg = vreg;
	}

	return rc;
error_put:
	for (i = i - 1; i >= 0; i--) {
		devm_regulator_put(pcfg_ven->iris_power_info.vregs[i].vreg);
		pcfg_ven->iris_power_info.vregs[i].vreg = NULL;
	}
	return rc;
}

static int _iris_put_vreg(void)
{
	int i;
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	for (i = pcfg_ven->iris_power_info.count - 1; i >= 0; i--)
		devm_regulator_put(pcfg_ven->iris_power_info.vregs[i].vreg);

	return 0;
}

void iris_init(struct dsi_display *display, struct dsi_panel *panel)
{
	struct iris_cfg *pcfg = iris_get_cfg();

	switch (pcfg->iris_chip_type) {
#if defined(CONFIG_PXLW_IRIS5)
	case CHIP_IRIS5:
		iris_init_i5(display, panel);
		break;
#endif
#if defined(CONFIG_PXLW_IRIS7)
	case CHIP_IRIS7:
		iris_init_i7(display, panel);
		break;
#endif
#if defined(CONFIG_PXLW_IRIS7P)
	case CHIP_IRIS7P:
		iris_init_i7p(display, panel);
		break;
#endif
#if defined(CONFIG_PXLW_IRIS8)
	case CHIP_IRIS8:
		iris_init_i8(display, panel);
		break;
#endif
	default:
		IRIS_LOGE("%s(): unsupported chip type %d", __func__, pcfg->iris_chip_type);
		break;
	}
	_iris_i2c_preload_work_init();
}

void iris_deinit(struct dsi_display *display)
{
	struct iris_cfg *pcfg = NULL;
	int i;

	pcfg = iris_get_cfg();

	if (!iris_is_chip_supported())
		return;

	if (iris_virtual_display(display))
		return;

#ifdef IRIS_EXT_CLK // skip ext clk
	if (pcfg->ext_clk) {
		devm_clk_put(&display->pdev->dev, pcfg->ext_clk);
		pcfg->ext_clk = NULL;
	}
#endif

	for (i = 0; i < iris_get_cmd_list_cnt(); i++)
		iris_free_ipopt_buf(i);
	iris_free_ipopt_buf(IRIS_LUT_PIP_IDX);

	if (pcfg->pq_update_cmd.update_ipopt_array) {
		switch (pcfg->iris_chip_type) {
		case CHIP_IRIS7:
			kfree(pcfg->pq_update_cmd.update_ipopt_array);
			break;
		case CHIP_IRIS5:
		case CHIP_IRIS7P:
		case CHIP_IRIS8:
			kvfree(pcfg->pq_update_cmd.update_ipopt_array);
			break;
		default:
		    kvfree(pcfg->pq_update_cmd.update_ipopt_array);
			break;
		}
		pcfg->pq_update_cmd.update_ipopt_array = NULL;
		pcfg->pq_update_cmd.array_index = 0;
	}

	iris_free_seq_space();

	_iris_put_vreg();
	iris_sysfs_status_deinit();
	iris_deinit_timing_switch();
	//iris_driver_unregister();
}

void iris_control_pwr_regulator(bool on)
{
	int rc = 0;
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (!iris_is_chip_supported())
		return;

	rc = dsi_pwr_enable_regulator(&pcfg_ven->iris_power_info, on);
	if (rc)
		IRIS_LOGE("failed to power %s iris", on ? "on" : "off");
}

void iris_power_on(struct dsi_panel *panel)
{
	if (!iris_is_chip_supported())
		return;

	IRIS_LOGI("%s(), for [%s] %s, secondary: %s",
			__func__,
			panel->name, panel->type,
			panel->is_secondary ? "true" : "false");

	if (panel->is_secondary)
		return;

	iris_control_pwr_regulator(true);

	if (iris_vdd_valid()) {
		iris_enable_vdd();
	} else { // No need to control vdd and clk
//		IRIS_LOGW("%s(), vdd does not valid, use pmic", __func__);
//		iris_control_pwr_regulator(true);
	}

	usleep_range(5000, 5000);
}

void iris_power_off(struct dsi_panel *panel)
{
	struct iris_cfg *pcfg = iris_get_cfg();

	if (!iris_is_chip_supported())
		return;

	IRIS_LOGI("%s(), for [%s] %s, secondary: %s",
			__func__,
			panel->name, panel->type,
			panel->is_secondary ? "true" : "false");

	if (panel->is_secondary) {
		pcfg->ap_mipi1_power_st = false;
		IRIS_LOGI("ap_mipi1_power_st: %d", pcfg->ap_mipi1_power_st);
		return;
	}
	iris_reset_off(NULL);
#ifdef IRIS_EXT_CLK
	iris_clk_disable(false);
#endif
	if (iris_vdd_valid())
		iris_disable_vdd();
//	else
//		iris_control_pwr_regulator(false);

	iris_control_pwr_regulator(false);
}

bool iris_virtual_display(const struct dsi_display *display)
{
	if (display && display->panel && display->panel->is_secondary)
		return true;

	return false;
}

bool iris_is_virtual_encoder_phys(void *phys_enc)
{
	struct sde_encoder_phys *phys_encoder = phys_enc;
	struct sde_connector *c_conn = NULL;
	struct dsi_display *display = NULL;

	if (phys_encoder == NULL)
		return false;

	if (phys_encoder->connector == NULL)
		return false;

	c_conn = to_sde_connector(phys_encoder->connector);
	if (c_conn == NULL)
		return false;

	display = c_conn->display;
	if (display == NULL)
		return false;

	if (!iris_virtual_display(display))
		return false;

	return true;
}

struct iris_vendor_cfg *iris_get_vendor_cfg(void)
{
	return &gcfg_ext;
}

static int32_t _iris_parse_tx_mode(
		struct device_node *np,
		struct dsi_panel *panel,
		struct iris_cfg *pcfg)
{
	int32_t rc = 0;
	u8 tx_mode;
	struct iris_dts_ops *p_dts_ops = iris_get_dts_ops();

	if (!p_dts_ops)
		return rc;

	pcfg->rx_mode = dsi_op_mode_to_iris_op_mode(panel->panel_mode);
	pcfg->tx_mode = dsi_op_mode_to_iris_op_mode(panel->panel_mode);
	IRIS_LOGI("%s, panel_mode = %d", __func__, pcfg->rx_mode);
	rc = p_dts_ops->read_u8(np, "pxlw,iris-tx-mode", &tx_mode);
	if (!rc) {
		IRIS_LOGI("get property: pxlw, iris-tx-mode: %d", tx_mode);
		//pcfg->tx_mode = tx_mode;
	}
	if (pcfg->rx_mode == pcfg->tx_mode)
		pcfg->pwil_mode = PT_MODE;
	else
		pcfg->pwil_mode = RFB_MODE;

	IRIS_LOGI("%s(), pwil mode: %d", __func__, pcfg->pwil_mode);
	return 0;
}

static int _iris_parse_pwr_entries(struct dsi_display *display)
{
	int32_t rc = 0;
	char *supply_name = NULL;
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (!display || !display->panel)
		return -EINVAL;

	if (!strcmp(display->panel->type, "primary")) {
		supply_name = "qcom,iris-supply-entries";

		rc = dsi_pwr_of_get_vreg_data(&display->panel->utils,
				&pcfg_ven->iris_power_info, supply_name);
		if (rc) {
			rc = -EINVAL;
			IRIS_LOGE("%s pwr enters error", __func__);
		}
	}
	return rc;
}

static void _iris_parse_bl_endian(struct dsi_panel *panel, struct iris_cfg *pcfg)
{
	if (!panel || !pcfg)
		return;

	pcfg->switch_bl_endian = panel->utils.read_bool(panel->utils.data,
			"pxlw,switch-bl-endian");
	IRIS_LOGI("%s(), switch backlight endian: %s",
			__func__,
			pcfg->switch_bl_endian ? "true" : "false");
}

static int _iris_parse_subnode(struct dsi_display *display, void *node)
{
	int32_t rc = -EINVAL;
	struct iris_cfg *pcfg = iris_get_cfg();
	struct device_node *lightup_node = node;

	rc = _iris_parse_tx_mode(lightup_node, display->panel, pcfg);
	if (rc)
		IRIS_LOGE("no set iris tx mode!");
	rc = _iris_parse_pwr_entries(display);
	if (rc)
		IRIS_LOGE("pwr entries error\n");

	_iris_parse_bl_endian(display->panel, pcfg);
	return 0;
}

int iris_parse_param(struct dsi_display *display)
{
	int32_t ret = 0;
	struct device_node *lightup_node = NULL;
	struct iris_cfg *pcfg = iris_get_cfg();

	IRIS_LOGI("%s(%d), enter.", __func__, __LINE__);
	if (!display || !display->pdev->dev.of_node || !display->panel_node) {
		IRIS_LOGE("the param is null");
		return -EINVAL;
	}
	if (display->panel->is_secondary)
		return 0;

	pcfg->valid = PARAM_EMPTY;	/* empty */
	spin_lock_init(&pcfg->iris_1w_lock);
	init_completion(&pcfg->frame_ready_completion);
	switch (pcfg->iris_chip_type) {
	case CHIP_IRIS5:
		iris_parse_iris_golden_fw(display->panel_node, "iris5");
		break;
	case CHIP_IRIS7:
		iris_parse_iris_golden_fw(display->panel_node, "iris7");
		break;
	case CHIP_IRIS7P:
		iris_parse_iris_golden_fw(display->panel_node, "iris7p");
		break;
	case CHIP_IRIS8:
		iris_parse_iris_golden_fw(display->panel_node, "iris8");
		break;
	default:
		IRIS_LOGI("unknow godlen fw type");
		break;
	}

	lightup_node = of_parse_phandle(display->panel_node, "pxlw,iris-lightup-config", 0);
	if (lightup_node) {
		iris_set_dts_ops(DTS_CTX_FROM_IMG);
		_iris_parse_subnode(display, lightup_node);

		ret = pw_iris_parse_param(lightup_node);
	}
	return ret;
}

static uint32_t _iris_calculate_delay_us(uint32_t payload_size, uint32_t cmd_num)
{
	uint32_t delay_us = 0;
	uint32_t panel_mbit_clk = 0;
	uint32_t lane_num = 0;
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	IRIS_LOGV("%s(%d), clk_rate_hz is %llu, num_data_lanes is %d", __func__, __LINE__,
		pcfg_ven->panel->cur_mode->timing.clk_rate_hz,
		pcfg_ven->panel->host_config.num_data_lanes);

	if (pcfg_ven->panel->cur_mode->timing.clk_rate_hz)
		panel_mbit_clk = pcfg_ven->panel->cur_mode->timing.clk_rate_hz / 1000000;

	if (!panel_mbit_clk) {
		IRIS_LOGE("%s(%d), panel_mbit_clk is 0, default set to 50MHz.",
			__func__, __LINE__);
		panel_mbit_clk = 50;
	}

	lane_num = pcfg_ven->panel->host_config.num_data_lanes;
	if (!lane_num) {
		/*default set to 4 lanes*/
		lane_num = 4;
	}

	/* follow:
	 *	8*(total_payload_size + total_command_num*6)*(1+inclk/pclk)/(lane_num*bitclk)
	 *	assume inclk/pclk = 2, this is the max value
	 */
	delay_us = 8 * (payload_size + cmd_num * 6) * (1+2) / (lane_num * panel_mbit_clk);

	return delay_us;
}

void iris_insert_delay_us(uint32_t payload_size, uint32_t cmd_num)
{
	uint32_t delay_us = 0;

	IRIS_LOGD("%s, payload_size is %d, cmd_num is %d",
		__func__, payload_size, cmd_num);

	if ((!payload_size) || (!cmd_num))
		return;

	if ((payload_size > 4096) || (cmd_num > 128))
		IRIS_LOGE("%s, it is risky to send such packets, payload_size %d, cmd_num %d",
			__func__, payload_size, cmd_num);
	/*embedded size is 240, non-embedded size is 256, use 240 default*/
	delay_us = _iris_calculate_delay_us(payload_size, payload_size/240 + 1);

	IRIS_LOGD("%s(%d): delay_us is %d", __func__, __LINE__, delay_us);

	if (delay_us)
		udelay(delay_us);
}

int iris_lightup(struct dsi_panel *panel)
{
	ktime_t ktime0;
	ktime_t ktime1;
	uint32_t timeus0 = 0;
	uint32_t timeus1 = 0;
	uint8_t type = 0;
	struct dsi_display *display = to_dsi_display(panel->host);
	int rc;
	struct iris_cfg *pcfg = iris_get_cfg();
#if defined(CONFIG_PXLW_IRIS7)
	struct iris_mode_info iris_timing;
#endif

	IRIS_LOGI("%s(), start +++, cmd list index: %u",
			__func__,
			iris_get_cmd_list_index());

	IRIS_ATRACE_BEGIN("iris_lightup");
	ktime0 = ktime_get();

	if (!pcfg->iris_i2c_preload) {
		rc = dsi_display_clk_ctrl(display->dsi_clk_handle,
				DSI_CORE_CLK | DSI_LINK_CLK, DSI_CLK_ON);
		if (rc) {
			IRIS_LOGE("%s(), failed to enable all DSI clocks for display: %s, return: %d",
					__func__, display->name, rc);
		}

		rc = iris_display_engine_enable(display);
		if (rc) {
			IRIS_LOGE("%s(), failed to enable cmd engine for display: %s, return: %d",
					__func__, display->name, rc);
		}
	}
	if (pcfg->iris_chip_type == CHIP_IRIS5) {
#if defined(CONFIG_PXLW_IRIS5)
		_iris_pre_lightup_i5();
#endif
	} else {
		_iris_pre_lightup();
	}
	_iris_load_mcu();

	type = iris_get_cont_splash_type();

	/*use to debug cont splash*/
	if (type == IRIS_CONT_SPLASH_LK) {
		IRIS_LOGI("%s(%d), enter cont splash", __func__, __LINE__);
		_iris_send_cont_splash_pkt(IRIS_CONT_SPLASH_LK);
	} else {
		_iris_send_lightup_pkt();
		if (pcfg->iris_chip_type == CHIP_IRIS5) {
#if defined(CONFIG_PXLW_IRIS5)
			iris_update_gamma_i5();
			if (!(pcfg->dual_test & 0x20))
				iris_dual_setting_switch(pcfg->dual_setting);
			else
				iris_scaler_filter_ratio_get();
#endif
		} else {
			iris_update_gamma();
			iris_ioinc_filter_ratio_send();
		}
	}


	if (panel->cur_mode->priv_info->cmd_sets[DSI_CMD_SET_PRE_ON].cmds != NULL) {
		struct iris_cmd_set cmdset;

		memset(&cmdset, 0x00, sizeof(cmdset));
		dsi_cmdset_to_iris_cmdset(&cmdset, &(panel->cur_mode->priv_info->cmd_sets[DSI_CMD_SET_PRE_ON]));
		IRIS_ATRACE_BEGIN("iris_pt_send_panel_cmd");
		rc = iris_pt_send_panel_cmd(&cmdset);
		IRIS_ATRACE_END("iris_pt_send_panel_cmd");
		IRIS_LOGI("%s(%d), pre_on_cmds", __func__, __LINE__);
	}

	ktime1 = ktime_get();
	if (type == IRIS_CONT_SPLASH_LK)
		IRIS_LOGI("%s(), exit cont splash", __func__);
	else {
		/*continuous splash should not use dma setting low power*/
		if (pcfg->iris_chip_type == CHIP_IRIS5) {
			if (pcfg->pw_chip_func_ops.iris_lp_init_i5_)
				pcfg->pw_chip_func_ops.iris_lp_init_i5_();
		} else
			iris_lp_enable_post();
	}

	iris_tx_buf_to_vc_set(pcfg->vc_ctrl.vc_arr[VC_PT]);

	if (!pcfg->iris_i2c_preload) {
		rc = iris_display_engine_disable(display);
		if (rc) {
			IRIS_LOGE("%s(), failed to disable cmd engine for display: %s, return: %d",
					__func__, display->name, rc);
		}
		rc = dsi_display_clk_ctrl(display->dsi_clk_handle,
				DSI_CORE_CLK | DSI_LINK_CLK, DSI_CLK_OFF);
		if (rc) {
			IRIS_LOGE("%s(), failed to disable all DSI clocks for display: %s, return: %d",
					__func__, display->name, rc);
		}
	}

	iris_update_last_pt_timing();

#if defined(CONFIG_PXLW_IRIS7)
	if (pcfg->iris_chip_type == CHIP_IRIS7)
		iris_sdr2hdr_set_img_size(iris_timing.h_active,
				iris_timing.v_active);
#endif

	IRIS_ATRACE_END("iris_lightup");

	timeus0 = (u32) ktime_to_us(ktime1) - (u32)ktime_to_us(ktime0);
	timeus1 = (u32) ktime_to_us(ktime_get()) - (u32)ktime_to_us(ktime1);
	IRIS_LOGI("%s() spend time0 %d us, time1 %d us.",
			__func__, timeus0, timeus1);

#ifdef IRIS_MIPI_TEST
	_iris_read_power_mode();
#endif
	pcfg->abyp_ctrl.preloaded = true;
	IRIS_LOGI("%s(), end +++", __func__);

	return 0;
}

void iris_core_lightup(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	iris_lightup(pcfg_ven->panel);

}

int iris_enable(struct dsi_panel *panel, struct iris_cmd_set *on_cmds)
{
	int rc = 0;
	int abyp_status_gpio;
	int prev_mode;
	int lightup_opt = iris_lightup_opt_get();
	struct iris_cfg *pcfg = iris_get_cfg();
	ktime_t ktime0 = 0;
	ktime_t ktime1 = 0;
	ktime_t ktime2 = 0;
	ktime_t ktime3 = 0;
	ktime_t ktime4 = 0;
	uint32_t timeus0 = 0;
	uint32_t timeus1 = 0;
	uint32_t timeus2 = 0;
	uint32_t timeus3 = 0;
	uint32_t timeus4 = 0;
	struct iris_mode_info iris_timing;

#ifdef IRIS_EXT_CLK
	iris_clk_enable(panel->is_secondary);
#endif

	if (pcfg->valid < PARAM_PREPARED) {
		if (on_cmds != NULL)
			rc = iris_abyp_send_panel_cmd(on_cmds);
		goto end;
	}
	// for Iris debug only, please don't port to customer
	// begin ...
	//IRIS_LOGI("%s(), compile time: %s, commit id: %s + %s",
			//__func__, COMPILE_TIME, DISP_COMMIT_ID, DTBO_COMMIT_ID);
	// ... end
	IRIS_LOGI_IF(panel != NULL,
			"%s(), for %s %s, secondary: %s",
			__func__,
			panel->name, panel->type,
			panel->is_secondary ? "true" : "false");

	if (IRIS_IF_LOGI())
		ktime0 = ktime_get();

	iris_enable_memc(panel->is_secondary);
	if (panel->is_secondary) {
		goto end;
	}

	IRIS_ATRACE_BEGIN("iris_enable");
	memset(&iris_timing, 0x00, sizeof(iris_timing));
	dsi_mode_to_iris_mode(&iris_timing, &panel->cur_mode->timing);
	iris_update_panel_timing(&iris_timing);
	iris_lp_enable_pre();
	pcfg->iris_initialized = false;

	/* Force Iris work in ABYP mode */
	if (iris_is_abyp_timing(&iris_timing))
		pcfg->abyp_ctrl.abypass_mode = ANALOG_BYPASS_MODE;

	if (iris_platform_get() == IRIS_FPGA)
		pcfg->abyp_ctrl.abypass_mode = PASS_THROUGH_MODE;

	IRIS_LOGI("%s(), mode:%d, rate: %d, v: %d, on_opt:0x%x",
			__func__,
			pcfg->abyp_ctrl.abypass_mode,
			iris_timing.refresh_rate,
			iris_timing.v_active,
			lightup_opt);

	/* support lightup_opt */
	if (lightup_opt & 0x1) {
		if (on_cmds != NULL)
			rc = iris_abyp_send_panel_cmd(on_cmds);
		IRIS_LOGI("%s(), force ABYP lightup.", __func__);
		IRIS_ATRACE_END("iris_enable");
		goto end;
	}

	if (IRIS_IF_LOGI())
		ktime1 = ktime_get();

	switch (pcfg->iris_chip_type) {
#if defined(CONFIG_PXLW_IRIS7)
	case CHIP_IRIS7:
		iris_bulksram_power_domain_proc_i7();
		break;
#endif
#if defined(CONFIG_PXLW_IRIS7P)
	case CHIP_IRIS7P:
		iris_bulksram_power_domain_proc_i7p();
		break;
#endif
	default:
		break;
	}

	if (iris_is_sleep_abyp_mode()) {
#if defined(CONFIG_PXLW_IRIS7)
		if (pcfg->iris_chip_type == CHIP_IRIS7)
			iris_disable_temp_sensor();
#endif
		iris_sleep_abyp_power_down();

		if (IRIS_IF_LOGI()) {
			ktime2 = ktime_get();
			ktime3 = ktime_get();
			ktime4 = ktime_get();
		}
	} else {
#ifdef IRIS_WA_FOR_IRIS8_A0
#if defined(CONFIG_PXLW_IRIS8)
		if (pcfg->iris_chip_type == CHIP_IRIS8)
			iris_sys_pll_tx_phy_wa();//only for iris8 A0 stand by light up
#endif
#endif
		prev_mode = pcfg->abyp_ctrl.abypass_mode;
		if (pcfg->iris_i2c_preload)
			goto _iris_lightup;
		abyp_status_gpio = iris_exit_abyp(true);
		if (abyp_status_gpio == 1) {
			IRIS_LOGE("%s(), failed to exit abyp!", __func__);
			IRIS_ATRACE_END("iris_enable");
			goto end;
		}

		if (IRIS_IF_LOGI())
			ktime2 = ktime_get();

		if (iris_platform_get() == IRIS_FPGA)
			iris_fpga_type_get();
_iris_lightup:
		rc = iris_lightup(panel);
		pcfg->iris_initialized = true;
		if (pcfg->iris_i2c_preload) {
			if (IRIS_IF_LOGI()) {
				ktime3 = ktime_get();
				ktime4 = ktime_get();
			}
			goto iris_enable_exit;
		}
		pcfg->abyp_ctrl.abypass_mode = PASS_THROUGH_MODE;
		if (IRIS_IF_LOGI())
			ktime3 = ktime_get();

		if (on_cmds != NULL) {
			IRIS_ATRACE_BEGIN("iris_pt_send_panel_cmd");
			rc = iris_pt_send_panel_cmd(on_cmds);
			IRIS_ATRACE_END("iris_pt_send_panel_cmd");
		}

		if (IRIS_IF_LOGI())
			ktime4 = ktime_get();

		//Switch back to ABYP mode if need
		if ((iris_platform_get() != IRIS_FPGA) && !(iris_lightup_opt_get() & 0x2)) {
			if (prev_mode == ANALOG_BYPASS_MODE)
				iris_abyp_switch_proc(ANALOG_BYPASS_MODE);
		}
	}

iris_enable_exit:
	if (IRIS_IF_LOGI()) {
		timeus0 = (u32) ktime_to_us(ktime1) - (u32)ktime_to_us(ktime0);
		timeus1 = (u32) ktime_to_us(ktime2) - (u32)ktime_to_us(ktime1);
		timeus2 = (u32) ktime_to_us(ktime3) - (u32)ktime_to_us(ktime2);
		timeus3 = (u32) ktime_to_us(ktime4) - (u32)ktime_to_us(ktime3);
		timeus4 = (u32) ktime_to_us(ktime_get()) - (u32)ktime_to_us(ktime4);
	}
	IRIS_LOGI("%s(), iris takes total %d us, prepare %d us, enter PT %d us,"
			" light up %d us, exit PT %d us.",
			__func__,
			timeus0 + timeus1 + timeus2 + timeus4,
			timeus0, timeus1, timeus2, timeus4);

	if (on_cmds != NULL)
		IRIS_LOGI("Send panel cmd takes %d us.", timeus3);

	IRIS_ATRACE_END("iris_enable");

end:
#ifdef IRIS_EXT_CLK
	iris_clk_disable(panel->is_secondary);
#endif
	return rc;
}

int iris_set_aod(struct dsi_panel *panel, bool aod)
{
	int rc = 0;
	struct iris_cfg *pcfg = iris_get_cfg();

	if (!pcfg)
		return rc;

	if (panel->is_secondary)
		return rc;

	IRIS_LOGI("%s(%d), aod: %d", __func__, __LINE__, aod);
	if (pcfg->aod == aod) {
		IRIS_LOGI("[%s:%d] aod: %d no change", __func__, __LINE__, aod);
		return rc;
	}

	if (aod) {
		if (!pcfg->fod) {
			pcfg->abyp_prev_mode = pcfg->abyp_ctrl.abypass_mode;
			if (iris_get_abyp_mode() == PASS_THROUGH_MODE)
				iris_abyp_switch_proc(ANALOG_BYPASS_MODE);
		}
	} else {
		if (!pcfg->fod) {
			if (iris_get_abyp_mode() == ANALOG_BYPASS_MODE &&
					pcfg->abyp_prev_mode == PASS_THROUGH_MODE &&
					!pcfg->fod) {
				iris_abyp_switch_proc(PASS_THROUGH_MODE);
			}
		}
	}

	if (pcfg->fod_pending)
		pcfg->fod_pending = false;
	pcfg->aod = aod;

	return rc;
}

int iris_set_fod(struct dsi_panel *panel, bool fod)
{
	int rc = 0;
	struct iris_cfg *pcfg = iris_get_cfg();

	if (!pcfg)
		return rc;

	if (panel->is_secondary)
		return rc;

	IRIS_LOGD("%s(%d), fod: %d", __func__, __LINE__, fod);
	if (pcfg->fod == fod) {
		IRIS_LOGD("%s(%d), fod: %d no change", __func__, __LINE__, fod);
		return rc;
	}

	if (!dsi_panel_initialized(panel)) {
		IRIS_LOGD("%s(%d), panel is not initialized fod: %d", __func__, __LINE__, fod);
		pcfg->fod_pending = true;
		atomic_set(&pcfg->fod_cnt, 1);
		pcfg->fod = fod;
		return rc;
	}

	if (fod) {
		if (!pcfg->aod) {
			pcfg->abyp_prev_mode = pcfg->abyp_ctrl.abypass_mode;
			if (iris_get_abyp_mode() == PASS_THROUGH_MODE)
				iris_abyp_switch_proc(ANALOG_BYPASS_MODE);
		}
	} else {
		/* pending until hbm off cmds sent in update_hbm 1->0 */
		pcfg->fod_pending = true;
		atomic_set(&pcfg->fod_cnt, 1);
	}

	pcfg->fod = fod;

	return rc;
}

bool iris_get_aod(struct dsi_panel *panel)
{
	struct iris_cfg *pcfg = iris_get_cfg();

	if (panel && panel->is_secondary)
		return false;

	return pcfg->aod;
}

static void _iris_send_cont_splash_pkt(uint32_t type)
{
	int seq_cnt = 0;
	uint32_t size = 0;
	const int iris_max_opt_cnt = 30;
	struct iris_ctrl_opt *opt_arr = NULL;
	struct iris_cfg *pcfg = iris_get_cfg();
	struct iris_ctrl_seq *pseq_cs = NULL;
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	size = IRIS_IP_CNT * iris_max_opt_cnt * sizeof(struct iris_ctrl_opt);

	if (type == IRIS_CONT_SPLASH_LK) {
		pseq_cs = _iris_get_ctrl_seq_cs(pcfg);
		if (!pseq_cs) {
			IRIS_LOGE("%s(), invalid pseq_cs", __func__);
			return;
		}

		iris_send_assembled_pkt(pseq_cs->ctrl_opt, pseq_cs->cnt);
	} else if (type == IRIS_CONT_SPLASH_KERNEL) {
		opt_arr = vmalloc(size);
		if (opt_arr == NULL) {
			IRIS_LOGE("%s(), failed to malloc buffer!", __func__);
			return;
		}
		memset(opt_arr, 0xff, size);

		iris_lp_enable_pre();
		seq_cnt = _iris_select_cont_splash_ipopt(type, opt_arr);
		iris_send_assembled_pkt(opt_arr, seq_cnt);
		iris_lp_enable_post();
		_iris_read_chip_id();

		vfree(opt_arr);
	} else if (type == IRIS_CONT_SPLASH_BYPASS_PRELOAD) {
		if (pcfg->rx_mode == IRIS_VIDEO_MODE) {
			iris_reset_sys_domain();
			schedule_work(&pcfg->iris_i2c_preload_work);
		} else {
			if (pcfg_ven && pcfg_ven->panel)
				iris_enable(pcfg_ven->panel, NULL);
		}
	}
}

void iris_send_cont_splash(struct dsi_display *display)
{
	struct dsi_panel *panel = display->panel;
	struct iris_cfg *pcfg = iris_get_cfg();
	int lightup_opt = iris_lightup_opt_get();
	uint32_t type;
	int rc = 0;
	struct iris_mode_info iris_timing;

	if (!iris_is_chip_supported())
		return;

	if (panel->is_secondary)
		return;
#ifdef IRIS_EXT_CLK //skip ext clk
	iris_clk_enable(panel->is_secondary);
#endif
	rc = iris_set_pinctrl_state(true);
	if (rc) {
		IRIS_LOGE("%s() failed to set iris pinctrl, rc=%d\n", __func__, rc);
		return;
	}

	memset(&iris_timing, 0x00, sizeof(iris_timing));
	dsi_mode_to_iris_mode(&iris_timing, &panel->cur_mode->timing);
	type = iris_get_cont_type_with_timing_switch(&iris_timing);

	if (lightup_opt & 0x1)
		type = IRIS_CONT_SPLASH_NONE;

#if defined(CONFIG_PXLW_IRIS5)
	if ((panel->panel_mode == DSI_OP_VIDEO_MODE)
			&& (iris_get_default_work_mode() == ANALOG_BYPASS_MODE)
				&& (type == IRIS_CONT_SPLASH_BYPASS_PRELOAD)
					&& (pcfg->valid >= PARAM_PARSED)) {
		schedule_work(&pcfg->iris_i2c_preload_work);
		return;
	}
#endif

	pcfg->lightup_ops.acquire_panel_lock();
	_iris_send_cont_splash_pkt(type);
	pcfg->lightup_ops.release_panel_lock();
}

int iris_lightoff(struct dsi_panel *panel, bool dead,
		struct iris_cmd_set *off_cmds)
{
	int rc = 0;
	struct iris_cfg *pcfg;

	pcfg = iris_get_cfg();

	switch (pcfg->iris_chip_type) {
#if defined(CONFIG_PXLW_IRIS5)
	case CHIP_IRIS5:
		iris_lightoff_i5(panel, off_cmds);
		break;
#endif
#if defined(CONFIG_PXLW_IRIS7)
	case CHIP_IRIS7:
		rc = iris_lightoff_i7(panel, dead, off_cmds);
		break;
#endif
#if defined(CONFIG_PXLW_IRIS7P)
	case CHIP_IRIS7P:
		rc = iris_lightoff_i7p(panel, dead, off_cmds);
		break;
#endif
#if defined(CONFIG_PXLW_IRIS8)
	case CHIP_IRIS8:
		rc = iris_lightoff_i8(panel, dead, off_cmds);
		break;
#endif
	default:
		IRIS_LOGE("%s(): unsupported chip type %d", __func__, pcfg->iris_chip_type);
		break;
	}

	pcfg->iris_pq_disable = 0;

	return rc;
}

int iris_disable(struct dsi_panel *panel, bool dead, struct iris_cmd_set *off_cmds)
{
	return iris_lightoff(panel, dead, off_cmds);
}

uint32_t iris_schedule_line_no_get(void)
{
	struct iris_cfg *pcfg = iris_get_cfg();
	uint32_t schedule_line_no, panel_vsw_vbp;
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if ((pcfg->frc_enabled) || (pcfg->pwil_mode == FRC_MODE))
		schedule_line_no = pcfg->ovs_delay_frc + 1;
	else
		schedule_line_no = pcfg->ovs_delay + 1;

	panel_vsw_vbp = pcfg_ven->panel->cur_mode->timing.v_back_porch +
			pcfg_ven->panel->cur_mode->timing.v_sync_width;

	if (pcfg->vsw_vbp_delay > panel_vsw_vbp)
		schedule_line_no += pcfg->vsw_vbp_delay - panel_vsw_vbp;

	if (pcfg->dtg_eco_enabled)
		schedule_line_no += pcfg->vsw_vbp_delay;

	return schedule_line_no;
}

#ifdef IRIS_EXT_CLK
bool iris_clk_parse(struct device_node *np)
{
	const char *clk_dtsi_name = NULL;
	int count, i = 0;

	if (!of_get_property(np, "clocks", NULL) || !of_get_property(np, "clock-names", NULL)) {
		IRIS_LOGI("ext clk not exist in node: %s", np->name);
	} else {
		count = of_property_count_strings(np, "clock-names");
		if (count < 0) {
			IRIS_LOGE("parse clock error");
		} else {
			for (i = (count-1);  i >= 0; i--) {
				of_property_read_string_index(np, "clock-names", i, &clk_dtsi_name);
				if (!strncmp(clk_dtsi_name, IRIS_EXT_CLK_NAME, strlen(IRIS_EXT_CLK_NAME))) {
					return true;
				}
			}
		}
	}
	return false;
}

void iris_clk_enable(bool is_secondary)
{
	int rc = 0;
	struct iris_cfg *pcfg = iris_get_cfg();

	if (is_secondary) {
		IRIS_LOGD("%s(), %d, skip enable clk in virtual channel", __func__, __LINE__);
		return;
	}

	if (IS_ERR_OR_NULL(pcfg->ext_clk)) {
		rc = PTR_ERR(pcfg->ext_clk);
		IRIS_LOGE("%s(), %d, failed to get iris ext clk, rc = %d", __func__, __LINE__, rc);
		return;
	}

	if (pcfg->ext_clk && !pcfg->clk_enable_flag) {
		IRIS_LOGI("%s(), %d, enable ext clk", __func__, __LINE__);
		clk_prepare_enable(pcfg->ext_clk);
		pcfg->clk_enable_flag = true;
		usleep_range(5000, 5001);
	} else {
		if (!pcfg->ext_clk)
			IRIS_LOGE("%s(), %d, ext clk not exist!", __func__, __LINE__);
		if (pcfg->clk_enable_flag)
			IRIS_LOGI("%s(), %d, ext clk has enabled", __func__, __LINE__);
	}
}

void iris_clk_disable(bool is_secondary)
{
	int rc = 0;
	struct iris_cfg *pcfg = iris_get_cfg();

	if (is_secondary) {
		IRIS_LOGD("%s(), %d, skip disable clk in virtual channel", __func__, __LINE__);
		return;
	}

	if (IS_ERR_OR_NULL(pcfg->ext_clk)) {
		rc = PTR_ERR(pcfg->ext_clk);
		IRIS_LOGE("%s(), %d, failed to get iris ext clk, rc = %d", __func__, __LINE__, rc);
		return;
	}

	if (pcfg->ext_clk && pcfg->clk_enable_flag) {
		IRIS_LOGI("%s(), %d, disable ext clk", __func__, __LINE__);
		clk_disable_unprepare(pcfg->ext_clk);
		pcfg->clk_enable_flag = false;
	} else {
		if (!pcfg->ext_clk)
			IRIS_LOGE("%s(), %d, ext clk not exist!", __func__, __LINE__);
		if (!pcfg->clk_enable_flag)
			IRIS_LOGI("%s(), %d, ext clk not enabled", __func__, __LINE__);
	}
}

void iris_core_clk_set(bool enable, bool is_secondary)
{
	if (enable)
		iris_clk_enable(is_secondary);
	else
		iris_clk_disable(is_secondary);

}

#endif

static ssize_t _iris_cont_splash_write(
		struct file *file, const char __user *buff,
		size_t count, loff_t *ppos)
{
	unsigned long val;
	char buf[64] = {0};

	if (count >= sizeof(buf))
		return -EFAULT;

	if (copy_from_user(buf, buff, count))
		return -EFAULT;

	if (kstrtoul(buf, 0, &val))
		return -EFAULT;


	_iris_set_cont_splash_type(val);

	if (val == IRIS_CONT_SPLASH_KERNEL) {
		struct iris_cfg *pcfg = iris_get_cfg();

		pcfg->lightup_ops.acquire_panel_lock();
		_iris_send_cont_splash_pkt(val);
		pcfg->lightup_ops.release_panel_lock();
	} else if (val != IRIS_CONT_SPLASH_LK &&
			val != IRIS_CONT_SPLASH_NONE) {
		IRIS_LOGE("the value is %zu, need to be 1 or 2 3", val);
	}

	return count;
}

static ssize_t _iris_cont_splash_read(
		struct file *file, char __user *buff,
		size_t count, loff_t *ppos)
{
	uint8_t type;
	int len, tot = 0;
	char bp[512];

	if (*ppos)
		return 0;

	type = iris_get_cont_splash_type();
	len = sizeof(bp);
	tot = scnprintf(bp, len, "%u\n", type);

	if (copy_to_user(buff, bp, tot))
		return -EFAULT;

	*ppos += tot;

	return tot;
}

static const struct file_operations iris_cont_splash_fops = {
	.open = simple_open,
	.write = _iris_cont_splash_write,
	.read = _iris_cont_splash_read,
};

int iris_wait_vsync(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();
	struct drm_encoder *drm_enc;

	if (pcfg_ven->display == NULL || pcfg_ven->display->bridge == NULL)
		return -ENOLINK;
	drm_enc = pcfg_ven->display->bridge->base.encoder;
	if (!drm_enc || !drm_enc->crtc)
		return -ENOLINK;
	if (sde_encoder_is_disabled(drm_enc))
		return -EIO;

	sde_encoder_wait_for_event(drm_enc, MSM_ENC_VBLANK);

	return 0;
}

int iris_sync_panel_brightness(int32_t step, void *phys_enc)
{
	struct sde_encoder_phys *phys_encoder = phys_enc;
	struct sde_connector *c_conn = NULL;
	struct dsi_display *display = NULL;
	struct iris_cfg *pcfg;
	int rc = 0;

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

	if (pcfg->panel_pending == step) {
		IRIS_LOGI("sync pending panel %d %d,%d,%d",
				step, pcfg->panel_pending, pcfg->panel_delay,
				pcfg->panel_level);
		IRIS_ATRACE_BEGIN("sync_panel_brightness");
		if (step <= 2) {
			rc = c_conn->ops.set_backlight(&c_conn->base,
					display, pcfg->panel_level);
			if (pcfg->panel_delay > 0)
				usleep_range(pcfg->panel_delay, pcfg->panel_delay + 1);
		} else {
			if (pcfg->panel_delay > 0)
				usleep_range(pcfg->panel_delay, pcfg->panel_delay + 1);
			rc = c_conn->ops.set_backlight(&c_conn->base,
					display, pcfg->panel_level);
		}
		if (c_conn->bl_device)
			c_conn->bl_device->props.brightness = pcfg->panel_level;
		pcfg->panel_pending = 0;
		IRIS_ATRACE_END("sync_panel_brightness");
	}

	return rc;
}

int iris_dbgfs_cont_splash_init(void *display)
{
	int ret = 0;
	struct iris_cfg *pcfg = iris_get_cfg();

	if (pcfg->dbg_root == NULL) {
		pcfg->dbg_root = debugfs_create_dir("iris", NULL);
		if (IS_ERR_OR_NULL(pcfg->dbg_root)) {
			IRIS_LOGE("debugfs_create_dir for iris_debug failed, error %ld",
					PTR_ERR(pcfg->dbg_root));
			return -ENODEV;
		}
	}
	if (debugfs_create_file("iris_cont_splash", 0644, pcfg->dbg_root, display,
				&iris_cont_splash_fops) == NULL) {
		IRIS_LOGE("%s(%d): debugfs_create_file: index fail",
				__FILE__, __LINE__);
		return -EFAULT;
	}

	ret = pw_dbgfs_cont_splash_init(display);

	return ret;
}

void iris_update_rd_ptr_time(void)
{
	unsigned long flags;
	struct iris_cfg *pcfg = iris_get_cfg();

	spin_lock_irqsave(&pcfg->backlight_v2.bl_spinlock, flags);
	pcfg->backlight_v2.rd_ptr_ktime = ktime_get();
	spin_unlock_irqrestore(&pcfg->backlight_v2.bl_spinlock, flags);
}

int iris_get_wait_vsync_count(void)
{
	unsigned long flags;
	int wait_vsync_count = 0;
	struct iris_cfg *pcfg = iris_get_cfg();

	spin_lock_irqsave(&pcfg->backlight_v2.bl_spinlock, flags);
	wait_vsync_count = pcfg->backlight_v2.wait_vsync_count;
	spin_unlock_irqrestore(&pcfg->backlight_v2.bl_spinlock, flags);

	return wait_vsync_count;
}

void iris_update_backlight_v2(struct sde_connector *c_conn)
{
	u32 delta_us;
	int level, delay;
	ktime_t now_ktime;
	unsigned long flags;
	ktime_t rd_ptr_ktime;
	struct dsi_display *display;
	struct iris_cfg *pcfg = iris_get_cfg();

	if (!c_conn) {
		IRIS_LOGE("%s: invalid sde connector", __func__);
		return;
	}

	display = c_conn->display;

	spin_lock_irqsave(&pcfg->backlight_v2.bl_spinlock, flags);
	level = pcfg->backlight_v2.level;
	delay = pcfg->backlight_v2.delay;
	rd_ptr_ktime = pcfg->backlight_v2.rd_ptr_ktime;
	pcfg->backlight_v2.level = -1;
	pcfg->backlight_v2.delay = -1;
	spin_unlock_irqrestore(&pcfg->backlight_v2.bl_spinlock, flags);

	if (level < 0)
		return;

	IRIS_LOGI("%s: %d %d", __func__, level, delay);

	now_ktime = ktime_get();
	delta_us = ktime_to_us(ktime_sub(now_ktime, rd_ptr_ktime));
	if (delay > delta_us) {
		delay -= delta_us;
		usleep_range(delay, delay + 1);
	}

	c_conn->ops.set_backlight(&c_conn->base, display, level);
}

void iris_prepare(struct dsi_display *display)
{
	struct iris_cfg *pcfg = iris_get_cfg();
	char *key = "qcom,mdss_dsi_";
	static bool is_boot = true;
	u32 *payload = NULL;

	if (!iris_is_chip_supported())
		return;

	if (pcfg->pw_chip_func_ops.iris_frc_prepare_i5_)
		pcfg->pw_chip_func_ops.iris_frc_prepare_i5_(pcfg);

	if (!display || display->panel->is_secondary)
		return;

	if (is_boot) {
		is_boot = false;

		if (pcfg->valid == PARAM_PARSED) {
			if (display->name && (strlen(display->name) > strlen(key))) {
				if (_iris_fw_parse_dts(display->name + strlen(key))) {
					pcfg->valid = PARAM_EMPTY;
					pcfg->abyp_ctrl.abypass_mode = ANALOG_BYPASS_MODE;
					return;
				}
			}
			iris_parse_memc_param();
			iris_frc_setting_init();
			iris_parse_lut_cmds(LOAD_GOLDEN_ONLY);
			iris_alloc_seq_space();
			iris_alloc_update_ipopt_space();
			payload = iris_get_ipopt_payload_data(IRIS_IP_SYS, pcfg->id_sys_dma_gen_ctrl, 4);
			if (payload)
				pcfg->default_dma_gen_ctrl = payload[0];
			payload = iris_get_ipopt_payload_data(IRIS_IP_SYS, ID_SYS_DMA_GEN_CTRL2, 4);
			if (payload)
				pcfg->default_dma_gen_ctrl_2 = payload[0];
			pcfg->valid = PARAM_PREPARED;	/* prepare ok */
		}
	}
}

#if 0
static int _iris_dev_probe(struct platform_device *pdev)
{
	struct iris_cfg *pcfg;
	int rc;

	IRIS_LOGI("%s()", __func__);
	if (!pdev || !pdev->dev.of_node) {
		IRIS_LOGE("%s(), pdev not found", __func__);
		return -ENODEV;
	}

	pcfg = iris_get_cfg();
	pcfg->pdev = pdev;
	dev_set_drvdata(&pdev->dev, pcfg);

	rc = iris_enable_pinctrl(pdev, pcfg);
	if (rc) {
		IRIS_LOGE("%s(), failed to enable pinctrl, return: %d",
			__func__, rc);
	}

	rc = iris_parse_gpio(pdev, pcfg);
	if (rc) {
		IRIS_LOGE("%s(), failed to parse gpio, return: %d",
				__func__, rc);
		return rc;
	}

	iris_request_gpio();

	return 0;
}

static int _iris_dev_remove(struct platform_device *pdev)
{
	struct iris_cfg *pcfg = dev_get_drvdata(&pdev->dev);

	IRIS_LOGI("%s()", __func__);

	iris_release_gpio(pcfg);

	return 0;
}

static const struct of_device_id iris_dt_match[] = {
	{.compatible = "pxlw,iris"},
	{}
};

static struct platform_driver iris_driver = {
	.probe = _iris_dev_probe,
	.remove = _iris_dev_remove,
	.driver = {
		.name = "pxlw-iris",
		.of_match_table = iris_dt_match,
	},
};

int iris_driver_register(void)
{
	if (iris_driver_registered)
		return 0;
	else
		iris_driver_registered = true;

	return platform_driver_register(&iris_driver);
}

void iris_driver_unregister(void)
{
	if (iris_driver_unregistered || !iris_driver_registered)
		return;
	else
		iris_driver_unregistered = true;

	platform_driver_unregister(&iris_driver);
}
#endif
//module_platform_driver(iris_driver);

int iris_dsi_send_cmds(struct iris_cmd_desc *cmds,
		u32 count, enum iris_cmd_set_state state, u8 vc_id)
{
	int rc = 0;
	int i = 0;
	int cont_payload_size = 0;
	int cont_cmd_num = 0;
	ssize_t len;
	const struct mipi_dsi_host_ops *ops;
	struct iris_cfg *pcfg = iris_get_cfg();
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();
	struct dsi_display *display = NULL;
	struct dsi_panel *panel = pcfg_ven->panel;
	u8 vc_id_bak;

	if (!panel || !panel->cur_mode)
		return -EINVAL;

	if (count == 0 || cmds == NULL) {
		IRIS_LOGD("%s(), panel %s no commands to be sent for state %d",
				__func__,
				panel->name, state);
		if (pcfg->iris_chip_type == CHIP_IRIS7)
			return -EINVAL;
		else if (pcfg->iris_chip_type == CHIP_IRIS7P)
			goto error;
	}

	IRIS_ATRACE_BEGIN(__func__);

	display = pcfg_ven->display;

	ops = panel->host->ops;

	for (i = 0; i < count; i++) {
		vc_id_bak = cmds->msg.channel;
		cmds->msg.channel = vc_id;

		if (state == IRIS_CMD_SET_STATE_LP)
			cmds->msg.flags |= MIPI_DSI_MSG_USE_LPM;
		else
			cmds->msg.flags &= ~MIPI_DSI_MSG_USE_LPM;

		cmds->last_command ? pcfg->iris_set_msg_flags(cmds, LAST_FLAG)
			: pcfg->iris_set_msg_flags(cmds, BATCH_FLAG);

		dsi_display_clk_ctrl(display->dsi_clk_handle,
				DSI_CORE_CLK | DSI_LINK_CLK, DSI_CLK_ON);
		WARN_ON(!mutex_is_locked(&panel->panel_lock));
		len = ops->transfer(panel->host, &cmds->msg);
		dsi_display_clk_ctrl(display->dsi_clk_handle,
				DSI_CORE_CLK | DSI_LINK_CLK, DSI_CLK_OFF);

		if (IRIS_IF_LOGVV())
			_iris_dump_packet((u8 *)cmds->msg.tx_buf, cmds->msg.tx_len);

		if (len < 0) {
			rc = len;
			IRIS_LOGE("%s(), failed to set cmds: %d, return: %d",
					__func__,
					cmds->msg.type, rc);
			dump_stack();
			goto error;
		}
		if (cmds->post_wait_ms)
			usleep_range(cmds->post_wait_ms * 1000,
					((cmds->post_wait_ms * 1000) + 10));

		cont_payload_size += cmds->msg.tx_len;
		cont_cmd_num++;
		if (cmds->last_command) {
			if (pcfg->vc_ctrl.vc_enable)
				if (pcfg->vc_ctrl.to_iris_vc_id == vc_id)
					iris_insert_delay_us(cont_payload_size, cont_cmd_num);
			cont_payload_size = 0;
			cont_cmd_num = 0;
		}
		cmds->msg.channel = vc_id_bak;
		cmds++;
	}
error:
	IRIS_ATRACE_END(__func__);
	return rc;
}
