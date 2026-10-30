/***************************************************************
** Copyright (C), 2025, OPLUS Mobile Comm Corp., Ltd
** File : oplus_apuirdim.h
** Description : oplus_apuirdim header
** Version : 2.0
** Date : 2025/08/13
** Author : Display
***************************************************************/
#ifndef _OPLUS_APUIR_H_
#define _OPLUS_APUIR_H_
#include <linux/kobject.h>
#include "oplus_display_utils.h"
/* log level config */
extern unsigned int oplus_apuir_log_level;
extern unsigned int oplus_adfr_display_id;
/* debug log */
#define APUIR_ERR(fmt, arg...)	\
	do {	\
		if (oplus_apuir_log_level >= OPLUS_APUIR_LOG_LEVEL_ERR)	\
			pr_err("[APUIR][%u][ERR][%s:%d]"pr_fmt(fmt), oplus_apuir_display_id, __func__, __LINE__, ##arg);	\
	} while (0)

#define APUIR_WARN(fmt, arg...)	\
	do {	\
		if (oplus_apuir_log_level >= OPLUS_APUIR_LOG_LEVEL_WARN)	\
			pr_warn("[APUIR][%u][WARN][%s:%d]"pr_fmt(fmt), oplus_apuir_display_id, __func__, __LINE__, ##arg);	\
	} while (0)

#define APUIR_INFO(fmt, arg...)	\
	do {	\
		if (oplus_apuir_log_level >= OPLUS_APUIR_LOG_LEVEL_INFO)	\
			pr_info("[APUIR][%u][INFO][%s:%d]"pr_fmt(fmt), oplus_apuir_display_id, __func__, __LINE__, ##arg);	\
	} while (0)

#define APUIR_DEBUG(fmt, arg...)	\
	do {	\
		if ((oplus_apuir_log_level >= OPLUS_APUIR_LOG_LEVEL_DEBUG) || (oplus_display_log_type & OPLUS_DEBUG_LOG_APUIR))	\
			pr_info("[APUIR][%u][DEBUG][%s:%d]"pr_fmt(fmt), oplus_apuir_display_id, __func__, __LINE__, ##arg);	\
	} while (0)

/* debug trace */
#define OPLUS_APUIR_TRACE_BEGIN(name)	\
	do {	\
		if (oplus_display_trace_enable & OPLUS_DISPLAY_APUIR_TRACE_ENABLE)	\
			SDE_ATRACE_BEGIN(name);	\
	} while (0)

#define OPLUS_APUIR_TRACE_END(name)	\
	do {	\
		if (oplus_display_trace_enable & OPLUS_DISPLAY_APUIR_TRACE_ENABLE)	\
			SDE_ATRACE_END(name);	\
	} while (0)

#define OPLUS_APUIR_TRACE_INT(name, value)	\
	do {	\
		if (oplus_display_trace_enable & OPLUS_DISPLAY_APUIR_TRACE_ENABLE)	\
			SDE_ATRACE_INT(name, value);	\
	} while (0)
void oplus_apuir_init(void *dsi_panel);
bool oplus_apuir_get_uir_state(void);
int oplus_apuir_set_ds(void *sde_enc_v);
void oplus_apuir_set_cmd(void *dsi_display, unsigned int ds);
void oplus_apuir_setcmd_work_handler(struct work_struct *work_item);
void oplus_apuir_set_up800nit_ds_list(int count, u32* list);
void oplus_apuir_set_less800nit_ds_list(int count, u32* list);
void oplus_apuir_setenable(int enable);
int oplus_get_apuir_enable(void);
#endif /* _OPLUS_APUIR_H_ */
