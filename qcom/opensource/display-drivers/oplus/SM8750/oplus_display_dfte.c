/***************************************************************
** Copyright (C),  2025,  OPLUS Mobile Comm Corp.,  Ltd
** File : oplus_display_dfte.c
** Description : Oplus Dynamic Te Feature, Epic 8906208
** Version : 1.0
** Date : 2025/07/03
**
** ------------------------------- Revision History: -----------
**  <author>          <data>        <version >        <desc>
**  Li.Ping        2025/07/03        1.0           Build this moudle
******************************************************************/
#include <linux/notifier.h>
#include <linux/soc/qcom/panel_event_notifier.h>
#include "oplus_display_sysfs_attrs.h"
#include "oplus_display_interface.h"
#include "oplus_display_device_ioctl.h"
#include "oplus_display_dfte.h"
#include "oplus_display_pwm.h"

/* -------------------- macro -------------------- */
#define OPLUS_DYNAMIC_FLOAT_TE_MODE							2
/* dfte feature bit setting */
#define OPLUS_DYNAMIC_FLOAT_TE_SUPPORT									(BIT(0))
#define OPLUS_DYNAMIC_FLOAT_TE_SWITCH_SUPPORT							(BIT(1))

#define OPLUS_DYNAMIC_FLOAT_TE_MODE0					(BIT(24))  /* 61hz */
#define OPLUS_DYNAMIC_FLOAT_TE_MODE1					(BIT(25))  /* 91hz */
#define OPLUS_DYNAMIC_FLOAT_TE_MODE2					(BIT(26))  /* 122hz */

/* -------------------- extern ---------------------------------- */


bool oplus_panel_dynamic_float_te_support(struct dsi_panel *panel)
{
	if (!panel) {
		OPLUS_DFTE_ERR("Invalid dsi_panel params\n");
		return false;
	}

	if (!panel->oplus_panel.dfte_params.support) {
		OPLUS_DFTE_ERR("Invalid dynamic float te params\n");
		return false;
	}
	return (bool)((panel->oplus_panel.dfte_params.support) & OPLUS_DYNAMIC_FLOAT_TE_SUPPORT);
}

int oplus_panel_dynamic_float_te_config(struct dsi_panel *panel)
{
	int rc = 0;
	int val = 0;
	struct dsi_parser_utils *utils = NULL;

	if (!panel) {
		OPLUS_DFTE_ERR("dynamic_float_te Invalid panel params\n");
		return -EINVAL;
	}

	utils = &panel->utils;

	rc = utils->read_u32(utils->data, "oplus,dynamic-float-te-support", &val);
	if (rc) {
		OPLUS_DFTE_ERR("failed to read oplus,dynamic-float-te-support, rc=%d\n", rc);
		/* set default value to disable */
		panel->oplus_panel.dfte_params.support = 0x0;
		panel->oplus_panel.dfte_params.dynamic_float_te_en = false;
		goto end;
	} else {
		panel->oplus_panel.dfte_params.support = val;
		panel->oplus_panel.dfte_params.dynamic_float_te_en = false;
	}

	if (oplus_panel_dynamic_float_te_support(panel)) {
		/* Reserve interface for later development */
		OPLUS_DFTE_INFO("dynamic float te support\n");
	}

	OPLUS_DFTE_INFO("dynamic float te probe successful\n");

end:
	return rc;
}
EXPORT_SYMBOL(oplus_panel_dynamic_float_te_config);

enum dsi_cmd_set_type get_panel_cmd_type(u32 dynamic_float_te_en)
{
	struct dsi_display *display = get_main_display();
	enum dsi_cmd_set_type type;

	if (!display || !display->panel) {
		OPLUS_DFTE_ERR("Invalid display or panel\n");
		return DSI_CMD_SET_MAX;
	}

	if (dynamic_float_te_en) {
		if (oplus_panel_pwm_get_switch_state(display->panel) == PWM_SWITCH_MODE2)
			type = DSI_CMD_SET_DYNAMIC_FLOAT_DC_TE;
		else
			type = DSI_CMD_SET_DYNAMIC_FLOAT_TE;
	} else {
		type = DSI_CMD_SET_DYNAMIC_FLOAT_DEFAULT_TE;
	}

	return type;
}

void update_dynamic_float_te_state(u32 dynamic_float_te_en, u32 refresh_rate)
{
	struct dsi_panel *panel = NULL;
	struct dsi_display *display = get_main_display();

	if (!display || !display->panel) {
		OPLUS_DFTE_ERR("Invalid display or panel\n");
		return;
	}

	panel = display->panel;

	switch(refresh_rate) {
		case 60 :
			if (dynamic_float_te_en) {
				panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_61HZ;
				if (oplus_panel_pwm_get_switch_state(display->panel) == PWM_SWITCH_MODE2)
					panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_DC_61HZ;
			} else {
				panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_60HZ;
			}
			break;
		case 90 :
			if (dynamic_float_te_en) {
				if (oplus_panel_pwm_get_switch_state(display->panel) == PWM_SWITCH_MODE2)
					panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_DC_91HZ;
				else
					panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_91HZ;
			} else {
				panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_90HZ;
			}
			break;
		case 120 :
			if (dynamic_float_te_en) {
				if (oplus_panel_pwm_get_switch_state(display->panel) == PWM_SWITCH_MODE2)
					panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_DC_122HZ;
				else
					panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_122HZ;
			} else {
				panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_120HZ;
			}
			break;
		default :
			OPLUS_DFTE_WARN("refresh_rate is not as expected timming, refresh_rate: %d\n", refresh_rate);
			panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_NOT_SUPPORT;
			break;
	}

	return;
}

void power_off_restore_dynamic_float_te_state(void)
{
	struct dsi_panel *panel = NULL;
	u32 dynamic_float_te_support = 0;
	struct dsi_display *display = get_main_display();

	if (!display || !display->panel) {
		OPLUS_DFTE_ERR("Invalid display or panel\n");
		return;
	}

	panel = display->panel;

	dynamic_float_te_support = oplus_panel_dynamic_float_te_support(panel);

	if (!dynamic_float_te_support){
		OPLUS_DFTE_INFO("dynamic float te not support\n");
		return;
	}

	panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_NO_SET;

	return;
}
EXPORT_SYMBOL(power_off_restore_dynamic_float_te_state);

int oplus_display_panel_set_dfte(u32 refresh_rate, u32 dynamic_float_te_en)
{
	int rc = 0;
	struct dsi_panel *panel = NULL;
	struct dsi_display *display = get_main_display();
	enum dsi_cmd_set_type type;

	if (!display || !display->panel) {
		OPLUS_DFTE_ERR("Invalid display or panel\n");
		rc = -EINVAL;
		return rc;
	}

	panel = display->panel;

	mutex_lock(&display->display_lock);
	panel->oplus_panel.dfte_params.dynamic_float_te_en = dynamic_float_te_en;
	type = get_panel_cmd_type(dynamic_float_te_en);

	switch(refresh_rate) {
		case 60 :
		case 120 :
			mutex_lock(&panel->panel_lock);
			rc = dsi_panel_tx_cmd_set(panel, type, false);
			update_dynamic_float_te_state(panel->oplus_panel.dfte_params.dynamic_float_te_en, refresh_rate);
			mutex_unlock(&panel->panel_lock);
			break;
		default :
			OPLUS_DFTE_WARN("refresh_rate is not as expected timming, refresh_rate: %d\n", refresh_rate);
			panel->oplus_panel.dfte_params.dynamic_float_te_state = TIMING_NOT_SUPPORT;
			break;
	}
	mutex_unlock(&display->display_lock);

	return rc;
}


u32 oplus_display_panel_get_dynamic_float_te_st(void)
{
	int rc = 0;
	u32 dynamic_float_te_support = 0;
	u32 dynamic_float_te_st = 0;
	struct dsi_panel *panel = NULL;
	struct dsi_display *display = get_main_display();

	if (!display || !display->panel) {
		OPLUS_DFTE_ERR("Invalid display or panel\n");
		rc = -EINVAL;
		return rc;
	}

	panel = display->panel;

	dynamic_float_te_support = oplus_panel_dynamic_float_te_support(panel);

	if (!dynamic_float_te_support){
		OPLUS_DFTE_INFO("dynamic float te not support\n");
		return dynamic_float_te_support;
	}

	dynamic_float_te_st = dynamic_float_te_support << 8 | panel->oplus_panel.dfte_params.dynamic_float_te_state;

	return dynamic_float_te_st;
}

int oplus_display_panel_set_dynamic_float_te(void *data)
{
	int rc = 0;
	u32 refresh_rate = 0;
	struct dsi_panel *panel = NULL;
	u32 *buf = data;
	u32 dynamic_float_te_en = *buf & 0xF;
	struct dsi_display *display = get_main_display();

	if (!display || !display->panel) {
		OPLUS_DFTE_ERR("Invalid display or panel\n");
		rc = -EINVAL;
		return rc;
	}

	panel = display->panel;

	if (!panel || !panel->cur_mode) {
		OPLUS_DFTE_ERR("Invalid display or panel\n");
		rc = -EINVAL;
		return rc;
	}

	if (display->panel->power_mode != SDE_MODE_DPMS_ON) {
		OPLUS_DSI_WARN("display panel is not power on\n");
		rc = -EFAULT;
		return rc;
	}

	if (!oplus_panel_dynamic_float_te_support(panel)) {
		OPLUS_DSI_INFO("panel is not support dynamic float te feature\n");
		return rc;
	}

	if (display->panel->cur_mode)
		refresh_rate = display->panel->cur_mode->timing.refresh_rate;

	OPLUS_DFTE_INFO("Set dynamic float te en=%d, panel->cur_mode->timing.refresh_rate=%d\n", dynamic_float_te_en, refresh_rate);

	oplus_display_panel_set_dfte(refresh_rate, dynamic_float_te_en);

	return rc;
}
EXPORT_SYMBOL(oplus_display_panel_set_dynamic_float_te);

int oplus_display_panel_get_dynamic_float_te_state(void *data)
{
	int rc = 0;
	u32 *dynamic_float_te_state = data;
	struct dsi_panel *panel = NULL;
	struct dsi_display *display = get_main_display();

	if (!display || !display->panel) {
		OPLUS_DFTE_ERR("Invalid display or panel\n");
		rc = -EINVAL;
		return rc;
	}

	panel = display->panel;

	mutex_lock(&display->display_lock);
	mutex_lock(&panel->panel_lock);

	*dynamic_float_te_state = oplus_display_panel_get_dynamic_float_te_st();

	mutex_unlock(&panel->panel_lock);
	mutex_unlock(&display->display_lock);
	OPLUS_DFTE_INFO("Get dynamic te state: %d\n", *dynamic_float_te_state);

	return rc;
}
EXPORT_SYMBOL(oplus_display_panel_get_dynamic_float_te_state);

ssize_t oplus_get_dynamic_float_te_debug_attr(struct kobject *obj, struct kobj_attribute *attr, char *buf)
{
	u32 dynamic_float_te_st = 0;
	struct dsi_display *display = get_main_display();

	if (!display || !display->panel) {
		OPLUS_DFTE_ERR("display is null\n");
		return -EINVAL;
	}

	if (!buf) {
		OPLUS_DFTE_ERR("Invalid params\n");
		return -EINVAL;
	}

	if (!oplus_panel_dynamic_float_te_support(display->panel)) {
		OPLUS_DSI_INFO("panel is not support dynamic float te feature\n");
		dynamic_float_te_st = 0;
		return sysfs_emit(buf, "0x%x\n", dynamic_float_te_st);
	}

	dynamic_float_te_st = oplus_display_panel_get_dynamic_float_te_st();

	return sysfs_emit(buf, "0x%x\n", dynamic_float_te_st);
}
EXPORT_SYMBOL(oplus_get_dynamic_float_te_debug_attr);

ssize_t oplus_set_dynamic_float_te_debug_attr(struct kobject *obj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int rc = 0;
	u32 dynamic_float_te_en = 0;
	u32 refresh_rate = 0;
	struct dsi_panel *panel = NULL;
	struct dsi_display *display = get_main_display();

	if (!display || !display->panel) {
		OPLUS_DFTE_ERR("Invalid display or panel\n");
		rc = -EINVAL;
		return count;
	}

	panel = display->panel;

	if (!panel || !panel->cur_mode) {
		OPLUS_DFTE_ERR("Invalid display or panel\n");
		rc = -EINVAL;
		return count;
	}

	if (!buf) {
		OPLUS_DFTE_ERR("Invalid params\n");
		return count;
	}

	sscanf(buf, "%u", &dynamic_float_te_en);

	if (display->panel->power_mode != SDE_MODE_DPMS_ON) {
		OPLUS_DFTE_WARN("display panel is not on\n");
		rc = -EFAULT;
		return count;
	}

	if (!oplus_panel_dynamic_float_te_support(panel)) {
		OPLUS_DSI_INFO("panel is not support dynamic float te feature\n");
		return count;
	}

	if (display->panel->cur_mode)
		refresh_rate = display->panel->cur_mode->timing.refresh_rate;

	OPLUS_DFTE_INFO("Set dynamic float te debug en=%d, panel->cur_mode->timing.refresh_rate=%d\n", dynamic_float_te_en, refresh_rate);

	oplus_display_panel_set_dfte(refresh_rate, dynamic_float_te_en);

	return count;
}
EXPORT_SYMBOL(oplus_set_dynamic_float_te_debug_attr);
