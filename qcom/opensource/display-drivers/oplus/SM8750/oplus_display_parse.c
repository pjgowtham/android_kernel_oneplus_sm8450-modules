/***************************************************************
** Copyright (C), 2024, OPLUS Mobile Comm Corp., Ltd
**
** File : oplus_display_parse.c
** Description : oplus display dts parse implement
** Version : 1.1
** Date : 2024/05/09
** Author : Display
******************************************************************/
#include "oplus_display_parse.h"
#include "oplus_display_esd.h"
#include "oplus_display_bl.h"
#include "oplus_display_power.h"
#include "oplus_display_pwm.h"
#include "oplus_display_interface.h"
#include "oplus_display_device_ioctl.h"
#include "oplus_display_ffl.h"
#include "oplus_debug.h"
#include "oplus_display_ext.h"
#include "oplus_display_dfte.h"
#ifdef OPLUS_FEATURE_AP_UIR_DIMMING
#include "oplus_apuirdim.h"
#endif

extern int dynamic_osc_clock;
bool oplus_enhance_mipi_strength = false;
bool apollo_backlight_enable = false;

static int oplus_panel_parse_common_config(struct dsi_panel *panel)
{
	int ret = 0;

	struct dsi_parser_utils *utils = &panel->utils;

		panel->oplus_panel.vendor_name = utils->get_property(utils->data,
			"oplus,mdss-dsi-vendor-name", NULL);

	if (!panel->oplus_panel.vendor_name) {
		OPLUS_DSI_ERR("Failed to found panel name, using dumming name\n");
		panel->oplus_panel.vendor_name = DSI_PANEL_OPLUS_DUMMY_VENDOR_NAME;
	}

	panel->oplus_panel.manufacture_name = utils->get_property(utils->data,
			"oplus,mdss-dsi-manufacture", NULL);

	if (!panel->oplus_panel.manufacture_name) {
		OPLUS_DSI_ERR("Failed to found panel name, using dumming name\n");
		panel->oplus_panel.manufacture_name = DSI_PANEL_OPLUS_DUMMY_MANUFACTURE_NAME;
	}

	panel->oplus_panel.gpio_pre_on = utils->read_bool(utils->data,
			"oplus,gpio-pre-on");
	OPLUS_DSI_INFO("oplus,gpio-pre-on: %s\n",
		panel->oplus_panel.gpio_pre_on ? "true" : "false");

	panel->oplus_panel.panel_id_switch_page = utils->read_bool(utils->data,
			"oplus,dsi-panel-id-switch-page");
	OPLUS_DSI_INFO("oplus,dsi-panel-id-switch-page: %s\n",
		panel->oplus_panel.panel_id_switch_page ? "true" : "false");

	panel->oplus_panel.is_osc_support = utils->read_bool(utils->data, "oplus,osc-support");
	OPLUS_DSI_INFO("osc mode support: %s\n", panel->oplus_panel.is_osc_support ? "Yes" : "Not");

	panel->oplus_panel.is_apl_read_support = utils->read_bool(utils->data, "oplus,apl-read-support");
	OPLUS_DSI_INFO("apl read support: %s\n", panel->oplus_panel.is_apl_read_support ? "Yes" : "Not");

	if (panel->oplus_panel.is_osc_support) {
		ret = utils->read_u32(utils->data, "oplus,mdss-dsi-osc-clk-mode0-rate",
				&panel->oplus_panel.osc_clk_mode0_rate);
		if (ret) {
			OPLUS_DSI_ERR("failed get panel parameter: oplus,mdss-dsi-osc-clk-mode0-rate\n");
			panel->oplus_panel.osc_clk_mode0_rate = 0;
		}
		dynamic_osc_clock = panel->oplus_panel.osc_clk_mode0_rate;

		ret = utils->read_u32(utils->data, "oplus,mdss-dsi-osc-clk-mode1-rate",
				&panel->oplus_panel.osc_clk_mode1_rate);
		if (ret) {
			OPLUS_DSI_ERR("failed get panel parameter: oplus,mdss-dsi-osc-clk-mode1-rate\n");
			panel->oplus_panel.osc_clk_mode1_rate = 0;
		}
	}

	panel->oplus_panel.fpga_support = utils->read_bool(utils->data, "oplus,fpga-support");
	OPLUS_DSI_INFO("fpga support: %s\n", panel->oplus_panel.fpga_support ? "Yes" : "Not");
	if (panel->oplus_panel.fpga_support) {
		oplus_panel_register_fpga_notifier();
	}

	panel->oplus_panel.gamma_compensation_support = utils->read_bool(utils->data, "oplus,gamma-compensation-support");
	OPLUS_DSI_INFO("panel gamma compensation support: %s\n", panel->oplus_panel.gamma_compensation_support ? "Yes" : "Not");

	panel->oplus_panel.pl_check_enable = utils->read_bool(utils->data, "oplus,pcd-lvd-check-enable");
	OPLUS_DSI_INFO("oplus,pcd-lvd-check-enable: %s\n", panel->oplus_panel.pl_check_enable ? "Yes" : "Not");
	if (panel->oplus_panel.pl_check_enable) {
		panel->oplus_panel.pl_check_flag = true;
	}

	ret = utils->read_u32(utils->data, "oplus,pcd-lvd-check-time-gap", &panel->oplus_panel.pl_check_time_gap);
	if (ret) {
		OPLUS_DSI_INFO("oplus,pcd-lvd-check-time-gap is not config, default 0\n");
	}

	panel->oplus_panel.all_timing_switch_frame_delay = utils->read_bool(utils->data,
			"oplus,panel-all-timing-switch-frame-delay");
	OPLUS_DSI_INFO("oplus,panel-all-timing-switch-frame-delay: %s\n",
			panel->oplus_panel.all_timing_switch_frame_delay ? "true" : "false");

	return 0;
}

static int oplus_panel_parse_sequence_config(struct dsi_panel *panel)
{
	int ret = 0;
	struct dsi_parser_utils *utils = &panel->utils;

	panel->oplus_panel.enhance_mipi_strength = utils->read_bool(utils->data, "oplus,enhance_mipi_strength");
	oplus_enhance_mipi_strength = panel->oplus_panel.enhance_mipi_strength;
	OPLUS_DSI_INFO("lcm enhance_mipi_strength: %s\n", panel->oplus_panel.enhance_mipi_strength ? "true" : "false");

	ret = utils->read_u32(utils->data, "oplus,wait-te-config", &panel->oplus_panel.wait_te_config);
	if (ret) {
		OPLUS_DSI_INFO("failed to get panel parameter: oplus,wait-te-config\n");
		panel->oplus_panel.wait_te_config = 0;
	}

	panel->oplus_panel.oplus_bl_demura_dbv_support = utils->read_bool(utils->data,
			"oplus,bl_denura-dbv-switch-support");
	OPLUS_DSI_INFO("oplus,bl_denura-dbv-switch-support: %s\n",
		panel->oplus_panel.oplus_bl_demura_dbv_support ? "true" : "false");
	panel->oplus_panel.bl_demura_mode = 0;

	panel->oplus_panel.cmdq_sync_support = utils->read_bool(utils->data,
			"oplus,cmdq-sync-support");
	OPLUS_DSI_INFO("oplus,cmdq-sync-support: %s\n",
		panel->oplus_panel.cmdq_sync_support ? "true" : "false");
	panel->oplus_panel.cmdq_sync_count = 0;

	return 0;
}

static int oplus_panel_parse_apollo_config(struct dsi_panel *panel)
{
	int ret = 0;
	struct dsi_parser_utils *utils = &panel->utils;

	/* Add for apollo */
	panel->oplus_panel.is_apollo_support = utils->read_bool(utils->data, "oplus,apollo_backlight_enable");
	apollo_backlight_enable = panel->oplus_panel.is_apollo_support;
	OPLUS_DSI_INFO("apollo_backlight_enable: %s\n", panel->oplus_panel.is_apollo_support ? "true" : "false");

	if (panel->oplus_panel.is_apollo_support) {
		ret = utils->read_u32(utils->data, "oplus,apollo-sync-brightness-level",
				&panel->oplus_panel.sync_brightness_level);

		if (ret) {
			OPLUS_DSI_INFO("failed to get panel parameter: oplus,apollo-sync-brightness-level\n");
			/* Default sync brightness level is set to 200 */
			panel->oplus_panel.sync_brightness_level = 200;
		}
		panel->oplus_panel.dc_apollo_sync_enable = utils->read_bool(utils->data, "oplus,dc_apollo_sync_enable");
		if (panel->oplus_panel.dc_apollo_sync_enable) {
			ret = utils->read_u32(utils->data, "oplus,dc-apollo-backlight-sync-level",
					&panel->oplus_panel.dc_apollo_sync_brightness_level);
			if (ret) {
				OPLUS_DSI_INFO("failed to get panel parameter: oplus,dc-apollo-backlight-sync-level\n");
				panel->oplus_panel.dc_apollo_sync_brightness_level = 397;
			}
			ret = utils->read_u32(utils->data, "oplus,dc-apollo-backlight-sync-level-pcc-max",
					&panel->oplus_panel.dc_apollo_sync_brightness_level_pcc);
			if (ret) {
				OPLUS_DSI_INFO("failed to get panel parameter: oplus,dc-apollo-backlight-sync-level-pcc-max\n");
				panel->oplus_panel.dc_apollo_sync_brightness_level_pcc = 30000;
			}
			ret = utils->read_u32(utils->data, "oplus,dc-apollo-backlight-sync-level-pcc-min",
					&panel->oplus_panel.dc_apollo_sync_brightness_level_pcc_min);
			if (ret) {
				OPLUS_DSI_INFO("failed to get panel parameter: oplus,dc-apollo-backlight-sync-level-pcc-min\n");
				panel->oplus_panel.dc_apollo_sync_brightness_level_pcc_min = 29608;
			}
			OPLUS_DSI_INFO("dc apollo sync enable(%d,%d,%d)\n", panel->oplus_panel.dc_apollo_sync_brightness_level,
					panel->oplus_panel.dc_apollo_sync_brightness_level_pcc, panel->oplus_panel.dc_apollo_sync_brightness_level_pcc_min);
		}
	}

	return 0;
}

static int oplus_panel_parse_serial_number_info(struct dsi_panel *panel)
{
	struct dsi_parser_utils *utils = NULL;
	int ret = 0;

	if (!panel) {
		OPLUS_DSI_ERR("Oplus Features config No panel device\n");
		return -ENODEV;
	}
	utils = &panel->utils;

	panel->oplus_panel.serial_number.serial_number_support = utils->read_bool(utils->data,
			"oplus,dsi-serial-number-enabled");
	OPLUS_DSI_INFO("oplus,dsi-serial-number-enabled: %s\n", panel->oplus_panel.serial_number.serial_number_support ? "true" : "false");

	if (panel->oplus_panel.serial_number.serial_number_support) {
		ret = utils->read_u32(utils->data, "oplus,dsi-serial-number-reg",
				&panel->oplus_panel.serial_number.serial_number_reg);
		if (ret) {
			OPLUS_DSI_INFO("failed to get oplus,dsi-serial-number-reg\n");
			panel->oplus_panel.serial_number.serial_number_reg = 0xA1;
		}

		ret = utils->read_u32(utils->data, "oplus,dsi-serial-number-index",
				&panel->oplus_panel.serial_number.serial_number_index);
		if (ret) {
			OPLUS_DSI_INFO("failed to get oplus,dsi-serial-number-index\n");
			/* Default sync start index is set 5 */
			panel->oplus_panel.serial_number.serial_number_index = 7;
		}

		ret = utils->read_u32(utils->data, "oplus,dsi-serial-number-read-count",
				&panel->oplus_panel.serial_number.serial_number_conut);
		if (ret) {
			OPLUS_DSI_INFO("failed to get oplus,dsi-serial-number-read-count\n");
			/* Default  read conut 5 */
			panel->oplus_panel.serial_number.serial_number_conut = 5;
		}

		ret = utils->read_u32(utils->data, "oplus,dsi-serial-number-base-year",
				&panel->oplus_panel.serial_number.base_year);
		if (ret) {
			OPLUS_DSI_INFO("failed to oplus,dsi-serial-number-base-year\n");
			/* Default oplus,dsi-serial-number-base-year 0 */
			panel->oplus_panel.serial_number.base_year = 0;
		}

		panel->oplus_panel.serial_number.is_switch_page = utils->read_bool(utils->data,
			"oplus,dsi-serial-number-switch-page");
		OPLUS_DSI_INFO("oplus,dsi-serial-number-switch-page: %s", panel->oplus_panel.serial_number.is_switch_page ? "true" : "false");
	}

	return 0;
}

static int oplus_panel_parse_btb_sn_info(struct dsi_panel *panel)
{
	struct dsi_parser_utils *utils = NULL;
	int ret = 0;

	if (!panel) {
		OPLUS_DSI_ERR("Oplus Features config No panel device\n");
		return -ENODEV;
	}
	utils = &panel->utils;

	panel->oplus_panel.btb_sn.btb_sn_support = utils->read_bool(utils->data,
			"oplus,dsi-btb-sn-enabled");
	OPLUS_DSI_INFO("oplus,dsi-btb-sn-enabled: %s\n", panel->oplus_panel.btb_sn.btb_sn_support ? "true" : "false");

	if (panel->oplus_panel.btb_sn.btb_sn_support) {
		panel->oplus_panel.btb_sn.is_reg_lock = utils->read_bool(utils->data, "oplus,dsi-btb-sn-lock");
		OPLUS_DSI_INFO("oplus,dsi-btb-sn-lock: %s\n", panel->oplus_panel.btb_sn.is_reg_lock ? "true" : "false");

		ret = utils->read_u32(utils->data, "oplus,dsi-btb-sn-reg",
				&panel->oplus_panel.btb_sn.btb_sn_reg);
		if (ret) {
			OPLUS_DSI_INFO("failed to get oplus,dsi-btb-sn-reg\n");
			panel->oplus_panel.btb_sn.btb_sn_reg = 0xA1;
		}

		ret = utils->read_u32(utils->data, "oplus,dsi-btb-sn-index",
				&panel->oplus_panel.btb_sn.btb_sn_index);
		if (ret) {
			OPLUS_DSI_INFO("failed to get oplus,dsi-btb-sn-index\n");
			/* Default sync start index is set 1 */
			panel->oplus_panel.btb_sn.btb_sn_index = 1;
		}

		ret = utils->read_u32(utils->data, "oplus,dsi-btb-sn-read-count",
				&panel->oplus_panel.btb_sn.btb_sn_conut);
		if (ret) {
			OPLUS_DSI_INFO("failed to get oplus,dsi-btb-sn-read-count\n");
			/* Default  read conut 32 */
			panel->oplus_panel.btb_sn.btb_sn_conut = 32;
		}

		panel->oplus_panel.btb_sn.is_switch_page = utils->read_bool(utils->data,
			"oplus,dsi-btb-sn-switch-page");
		OPLUS_DSI_INFO("oplus,dsi-btb-sn-switch-page: %s", panel->oplus_panel.btb_sn.is_switch_page ? "true" : "false");
	}

	return 0;
}

int oplus_panel_parse_features_config(struct dsi_panel *panel)
{
	struct dsi_parser_utils *utils = NULL;
	if (!panel) {
		OPLUS_DSI_ERR("Oplus Features config No panel device\n");
		return -ENODEV;
	}

	utils = &panel->utils;
	panel->oplus_panel.dp_support = utils->get_property(utils->data,
			"oplus,dp-enabled", NULL);

	if (!panel->oplus_panel.dp_support) {
		OPLUS_DSI_INFO("Failed to found panel dp support, using null dp config\n");
		panel->oplus_panel.dp_support = false;
	}

	panel->oplus_panel.cabc_enabled = utils->read_bool(utils->data,
			"oplus,dsi-cabc-enabled");
	OPLUS_DSI_INFO("oplus,dsi-cabc-enabled: %s\n", panel->oplus_panel.cabc_enabled ? "true" : "false");

	panel->oplus_panel.dre_enabled = utils->read_bool(utils->data,
			"oplus,dsi-dre-enabled");
	OPLUS_DSI_INFO("oplus,dsi-dre-enabled: %s\n", panel->oplus_panel.dre_enabled ? "true" : "false");

	panel->oplus_panel.panel_init_compatibility_enable = utils->read_bool(utils->data,
			"oplus,panel_init_compatibility_enable");
	OPLUS_DSI_INFO("oplus,panel_init_compatibility_enable: %s\n",
			panel->oplus_panel.panel_init_compatibility_enable ? "true" : "false");

	panel->oplus_panel.vid_timming_switch_enabled = utils->read_bool(utils->data,
			"oplus,dsi-vid-timming-switch_enable");
	OPLUS_DSI_INFO("oplus,panel_init_compatibility_enable: %s\n",
			panel->oplus_panel.vid_timming_switch_enabled ? "true" : "false");

	panel->oplus_panel.vid_timming_switch_post_enabled = utils->read_bool(utils->data,
			"oplus,dsi_vid_timming_switch_post_enable");
	OPLUS_DSI_INFO("oplus,dsi_vid_timming_switch_post_enable: %s\n",
			panel->oplus_panel.vid_timming_switch_post_enabled ? "true" : "false");

	panel->oplus_panel.change_voltage_before_panel_bl_0 = utils->read_bool(utils->data,
			"oplus,change-voltage-before-panel-bl-0-enable");
	OPLUS_DSI_INFO("oplus,change-voltage-before-panel-bl-0-enable: %s\n",
			panel->oplus_panel.change_voltage_before_panel_bl_0 ? "true" : "false");

	panel->oplus_panel.interval_time_nolp_pre = utils->read_bool(utils->data,
			"oplus,interval-time-nolp-pre");
	OPLUS_DSI_INFO("oplus,interval-time-nolp-pre: %s\n",
			panel->oplus_panel.interval_time_nolp_pre ? "true" : "false");

	panel->oplus_panel.bl_ic_ktz8868_used = utils->read_bool(utils->data,
		"oplus,bl-use-ktz8868-ic-ctrl");
	OPLUS_DSI_INFO("oplus,bl-use-ktz8868-ic-ctrl: %s\n",
		panel->oplus_panel.bl_ic_ktz8868_used ? "true" : "false");

	panel->oplus_panel.mipi_reset_enable = utils->read_bool(utils->data,
		"oplus,mipi-reset-enable");
	OPLUS_DSI_INFO("oplus,mipi-reset-enable: %s\n",
		panel->oplus_panel.mipi_reset_enable ? "true" : "false");

	panel->oplus_panel.enable_dsi_cmd_package = utils->read_bool(utils->data,
		"oplus,enable_dsi_cmd_package");
	OPLUS_DSI_INFO("oplus,enable_dsi_cmd_package: %s\n",
		panel->oplus_panel.enable_dsi_cmd_package ? "true" : "false");

	panel->oplus_panel.vid_fps_switch_compenstate_enable = utils->read_bool(utils->data,
		"oplus,video-mode-fps-switch-compenstate-enable");
	OPLUS_DSI_INFO("oplus,video-mode-fps-switch-compenstate-enable: %s\n",
		panel->oplus_panel.vid_fps_switch_compenstate_enable ? "true" : "false");

	panel->oplus_panel.ofp_configuration_enable_for_ili7838e = utils->read_bool(utils->data,
		"oplus,ofp-configuration-enable-for-ili7838E");
	OPLUS_DSI_INFO("ofp-configuration-enable-for-ili7838E: %s\n",
		panel->oplus_panel.ofp_configuration_enable_for_ili7838e ? "true" : "false");

	panel->oplus_panel.doze_disable_esdcheck = utils->read_bool(utils->data,
		"oplus,doze_disable_esdcheck");
	OPLUS_DSI_INFO("oplus,doze_disable_esdcheck: %s\n",
		panel->oplus_panel.doze_disable_esdcheck ? "true" : "false");

	panel->oplus_panel.ramless_aod_mode_cmd_switch_support = utils->read_bool(utils->data,
			"oplus,ramless_aod_mode_cmd_switch_support");
	OPLUS_DSI_INFO("oplus,ramless_aod_mode_cmd_switch_support: %s\n",
			panel->oplus_panel.ramless_aod_mode_cmd_switch_support ? "true" : "false");

	panel->oplus_panel.disalbe_aod_recovery = utils->read_bool(utils->data,
		"oplus,disalbe-aod-recovery");

	panel->oplus_panel.interval_time_fps_to_esd_flag = utils->read_bool(utils->data,
			"oplus,interval-time-switch-fps-to-esd");
	OPLUS_DSI_INFO("oplus,interval-time-switch-fps-to-esd: %s\n",
			panel->oplus_panel.interval_time_fps_to_esd_flag ? "true" : "false");

	return 0;
}

int oplus_panel_parse_vsync_config(
				struct dsi_display_mode *mode,
				struct dsi_parser_utils *utils)
{
	int rc;
	struct dsi_display_mode_priv_info *priv_info;

	priv_info = mode->priv_info;

	rc = utils->read_u32(utils->data, "oplus,apollo-panel-vsync-period",
				  &priv_info->oplus_priv_info.vsync_period);
	if (rc) {
		OPLUS_DSI_DEBUG("panel prefill lines are not defined rc=%d\n", rc);
		priv_info->oplus_priv_info.vsync_period = 1000000 / mode->timing.refresh_rate;
	}

	rc = utils->read_u32(utils->data, "oplus,apollo-panel-vsync-width",
				  &priv_info->oplus_priv_info.vsync_width);
	if (rc) {
		OPLUS_DSI_DEBUG("panel vsync width not defined rc=%d\n", rc);
		priv_info->oplus_priv_info.vsync_width = priv_info->oplus_priv_info.vsync_period >> 1;
	}

	rc = utils->read_u32(utils->data, "oplus,apollo-panel-async-bl-delay",
				  &priv_info->oplus_priv_info.async_bl_delay);
	if (rc) {
		OPLUS_DSI_DEBUG("panel async backlight delay to bottom of frame was disabled rc=%d\n", rc);
		priv_info->oplus_priv_info.async_bl_delay = 0;
	} else {
		if (priv_info->oplus_priv_info.async_bl_delay >= priv_info->oplus_priv_info.vsync_period) {
			OPLUS_DSI_ERR("async backlight delay value was out of vsync period\n");
			priv_info->oplus_priv_info.async_bl_delay = priv_info->oplus_priv_info.vsync_width;
		}
	}

	priv_info->oplus_priv_info.refresh_rate = mode->timing.refresh_rate;

	OPLUS_DSI_INFO("vsync width = %d, vsync period = %d, refresh rate = %d\n", \
			priv_info->oplus_priv_info.vsync_width, priv_info->oplus_priv_info.vsync_period, priv_info->oplus_priv_info.refresh_rate);

	return 0;
}

#ifdef OPLUS_FEATURE_AP_UIR_DIMMING
void oplus_panel_parse_apuir_ds_list(struct dsi_panel *panel) {
	int rc = 0;
	struct dsi_parser_utils *utils = &panel->utils;
	char payload[128] = "";
	u32 cnt = 0;
	int up800nit_ds_count = 0;
	u32 *up800nit_ds_list = NULL;
	int less800nit_ds_count = 0;
	u32 *less800nit_ds_list = NULL;

	/* get up800nit_ds_list */
	up800nit_ds_count = utils->count_u32_elems(utils->data,
		"oplus,apuir-up800nit-ds-list");
	if (up800nit_ds_count < 1) {
		OPLUS_DSI_INFO("aapuir puir-up800nit-ds-list is NULL! oplus_apuir_setenable 0\n");
		up800nit_ds_count = 0;
		oplus_apuir_setenable(0);
		return;
	} else {
		oplus_apuir_setenable(1);
	}

	up800nit_ds_list = kcalloc(up800nit_ds_count,
			sizeof(u32), GFP_KERNEL);
	if (!up800nit_ds_list) {
		kfree(up800nit_ds_list);
		OPLUS_DSI_ERR("apuir oplus,apuir-less800nit-ds-list alloc failed!\n");
		return;
	}

	rc = utils->read_u32_array(utils->data,
		"oplus,apuir-up800nit-ds-list",
		up800nit_ds_list,
		up800nit_ds_count);

	if (rc) {
		kfree(up800nit_ds_list);
		OPLUS_DSI_ERR("apuir up800nit_ds_list parse failed!\n");
		return;
	}
	oplus_apuir_set_up800nit_ds_list(up800nit_ds_count, up800nit_ds_list);

	cnt = 0;
	for (int i = 0; i < up800nit_ds_count; i++) {
		cnt += scnprintf(payload + cnt, sizeof(payload) - cnt, "[%u]", up800nit_ds_list[i]);
	}
	OPLUS_DSI_INFO("apuir up800nit_ds_list count: %d, mode_list: %s\n", up800nit_ds_count, payload);

	/* get less800nit_ds_list */
	less800nit_ds_count = utils->count_u32_elems(utils->data,
		"oplus,apuir-less800nit-ds-list");
	if (less800nit_ds_count < 1) {
		OPLUS_DSI_INFO("apuir oplus,apuir-less800nit-ds-list is NULL!\n");
		less800nit_ds_count = 0;
		return;
	}

	less800nit_ds_list = kcalloc(less800nit_ds_count,
			sizeof(u32), GFP_KERNEL);
	if (!less800nit_ds_list) {
		kfree(less800nit_ds_list);
		OPLUS_DSI_ERR("apuir oplus,apuir-less800nit-ds-list alloc failed!\n");
		return;
	}

	rc = utils->read_u32_array(utils->data,
			"oplus,apuir-less800nit-ds-list",
			less800nit_ds_list,
			less800nit_ds_count);

	if (rc) {
		kfree(less800nit_ds_list);
		OPLUS_DSI_ERR("apuir less800nit_ds_list parse failed!\n");
		return;
	}
	oplus_apuir_set_less800nit_ds_list(less800nit_ds_count, less800nit_ds_list);

	cnt = 0;
	for (int i = 0; i < less800nit_ds_count; i++) {
		cnt += scnprintf(payload + cnt, sizeof(payload) - cnt, "[%u]", less800nit_ds_list[i]);
	}
	OPLUS_DSI_INFO("apuir less800nit_ds_list count: %d, mode_list: %s\n", less800nit_ds_count, payload);
	return;
}
#endif

void oplus_panel_parse_ignore_mode_config(struct dsi_panel *panel)
{
	int rc = 0;
	struct dsi_parser_utils *utils = &panel->utils;
	char payload[128] = "";
	u32 cnt = 0;

	panel->oplus_panel.ignore_mode_count = utils->count_u32_elems(utils->data,
			"oplus,factory-ignore-mode");
	if (panel->oplus_panel.ignore_mode_count < 1) {
		OPLUS_DSI_INFO("factory ignore mode is NULL!\n");
		panel->oplus_panel.ignore_mode_count = 0;
		return;
	}

	panel->oplus_panel.ignore_mode = kcalloc(panel->oplus_panel.ignore_mode_count,
			sizeof(u32), GFP_KERNEL);
	if (!panel->oplus_panel.ignore_mode) {
		kfree(panel->oplus_panel.ignore_mode);
		OPLUS_DSI_ERR("factory ignore mode list alloc failed!\n");
		return;
	}

	rc = utils->read_u32_array(utils->data,
			"oplus,factory-ignore-mode",
			panel->oplus_panel.ignore_mode,
			panel->oplus_panel.ignore_mode_count);

	if (rc) {
		OPLUS_DSI_ERR("factory ignore mode list parse failed!\n");
		return;
	}

	for (int i = 0; i < panel->oplus_panel.ignore_mode_count; i++) {
		cnt += scnprintf(payload + cnt, sizeof(payload) - cnt, "[%u]", panel->oplus_panel.ignore_mode[i]);
	}
	OPLUS_DSI_INFO("parse ignore mode count: %d, mode_list: %s\n", panel->oplus_panel.ignore_mode_count, payload);

	return;
}

int oplus_panel_parse_config(struct dsi_panel *panel)
{
	if (!panel) {
		OPLUS_DSI_ERR("Oplus features config no panel device\n");
		return -ENODEV;
	}

	/* parse common config */
	oplus_panel_parse_common_config(panel);

	/* parse feature config */
	oplus_panel_parse_power_config(panel);
	oplus_panel_parse_sequence_config(panel);
	oplus_panel_parse_apollo_config(panel);
	oplus_panel_parse_ffc_config(panel);
	oplus_panel_parse_pwm_config(panel);
	oplus_panel_parse_serial_number_info(panel);
	oplus_panel_parse_btb_sn_info(panel);
	oplus_panel_parse_power_sequence_config(panel);
	oplus_dsi_panel_parse_mipi_err(panel);
	oplus_dsi_panel_parse_pcd(panel);
	oplus_dsi_panel_parse_lvd(panel);
	oplus_panel_parse_ignore_mode_config(panel);
	oplus_dsi_panel_parse_lut(panel);
#ifdef OPLUS_FEATURE_AP_UIR_DIMMING
	oplus_panel_parse_apuir_ds_list(panel);
#endif
	oplus_panel_dynamic_float_te_config(panel);

	return 0;
}
