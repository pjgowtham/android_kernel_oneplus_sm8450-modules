/***************************************************************
** Copyright (C),  2025,  OPLUS Mobile Comm Corp.,  Ltd
** File : oplus_display_dfte.h
** Description : Oplus Dynamic FLOAT Te Feature, Epic 8906208
** Version : 1.0
** Date : 2025/07/03
**
** ------------------------------- Revision History: -----------
**  <author>          <data>        <version >        <desc>
**  Li.Ping        2025/07/03        1.0           Build this moudle
******************************************************************/

#ifndef _OPLUS_DISPLAY_DYNAMIC_FLOAT_TE_H_
#define _OPLUS_DISPLAY_DYNAMIC_FLOAT_TE_H_

/* please just only include linux common head file  */
#include <linux/err.h>
#include "dsi_panel.h"
#include "oplus_panel.h"

enum panel_dfte_mode_state {
	TIMING_NO_SET = 0,
	TIMING_60HZ,
	TIMING_61HZ,
	TIMING_DC_61HZ,
	TIMING_90HZ,
	TIMING_91HZ,
	TIMING_DC_91HZ,
	TIMING_120HZ,
	TIMING_122HZ,
	TIMING_DC_122HZ,
	TIMING_144HZ,
	TIMING_145HZ,
	TIMING_DC_145HZ,
	TIMING_165HZ,
	TIMING_166HZ,
	TIMING_DC_166HZ,
	TIMING_NOT_SUPPORT,
	TIMING_MODE_MAX,
};

/* -------------------- extern ---------------------------------- */


/* -------------------- pwm turbo debug log-------------------------------------------  */
#define OPLUS_DFTE_ERR(fmt, arg...)	\
	do {	\
		if (oplus_display_log_level >= OPLUS_LOG_LEVEL_ERR)	\
			pr_err("[DFTE_SWITCH][ERR][%s:%d]"pr_fmt(fmt), __func__, __LINE__, ##arg);	\
	} while (0)

#define OPLUS_DFTE_WARN(fmt, arg...)	\
	do {	\
		pr_warn("[DFTE_SWITCH][WARN][%s:%d]"pr_fmt(fmt), __func__, __LINE__, ##arg);	\
	} while (0)

#define OPLUS_DFTE_INFO(fmt, arg...)	\
	do { \
		if(1) \
			pr_info("[DFTE_SWITCH][INFO][%s:%d]"pr_fmt(fmt), __func__, __LINE__, ##arg);	\
	} while (0)

#define OPLUS_DFTE_DEBUG(fmt, arg...)	\
	do {	\
		if (oplus_display_log_level >= OPLUS_LOG_LEVEL_DEBUG || (oplus_display_log_type & OPLUS_DEBUG_LOG_DSI))	 \
			pr_info("[DFTE_SWITCH][DEBUG][%s:%d]"pr_fmt(fmt), __func__, __LINE__, ##arg);	\
	} while (0)

/* -------------------- function declaration ---------------------------------------- */
bool oplus_panel_dynamic_float_te_support(struct dsi_panel *panel);
int oplus_panel_dynamic_float_te_config(struct dsi_panel *panel);
/* config */
#endif /* _OPLUS_DISPLAY_DYNAMIC_FLOAT_TE_H_ */

int oplus_display_panel_set_dynamic_float_te(void *data);
int oplus_display_panel_get_dynamic_float_te_state(void *data);
void power_off_restore_dynamic_float_te_state(void);
/* -------------------- oplus api nodes ----------------------------------------------- */
ssize_t oplus_get_dynamic_float_te_debug_attr(struct kobject *obj, struct kobj_attribute *attr, char *buf);
ssize_t oplus_set_dynamic_float_te_debug_attr(struct kobject *obj, struct kobj_attribute *attr, const char *buf, size_t count);
