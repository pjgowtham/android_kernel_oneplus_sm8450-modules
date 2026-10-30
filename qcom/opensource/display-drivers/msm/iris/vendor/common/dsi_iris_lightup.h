/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2020, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */
#ifndef _DSI_IRIS_LIGHTUP_H_
#define _DSI_IRIS_LIGHTUP_H_

#include <linux/completion.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <sde_connector.h>
#include "dsi_pwr.h"
#include "pw_iris_lightup.h"

struct iris_vendor_cfg {
	struct dsi_regulator_info iris_power_info; // iris pmic power
	struct dsi_display *display;
	struct dsi_panel *panel;

	/* secondary display related */
	struct dsi_display *display2;  // secondary display
	struct dsi_panel *panel2;      // secondary panel
};
struct iris_vendor_cfg *iris_get_vendor_cfg(void);

#ifdef IRIS_EXT_CLK
bool iris_clk_parse(struct device_node *np);
void iris_clk_enable(bool is_secondary);
void iris_clk_disable(bool is_secondary);
void iris_core_clk_set(bool enable, bool is_secondary);
#endif

int iris_lightup(struct dsi_panel *panel);
void iris_core_lightup(void);
int iris_lightoff(struct dsi_panel *panel, bool dead, struct iris_cmd_set *off_cmds);
int iris_wait_vsync(void);

bool iris_virtual_display(const struct dsi_display *display);

int32_t iris_parse_dtsi_cmd(const struct device_node *lightup_node,
		uint32_t cmd_index);
int32_t iris_parse_optional_seq(struct device_node *np, const uint8_t *key,
		struct iris_ctrl_seq *pseq);

//int iris_display_cmd_engine_enable(struct dsi_display *display);
//int iris_display_cmd_engine_disable(struct dsi_display *display);
int iris_display_engine_enable(struct dsi_display *display);
int iris_display_engine_disable(struct dsi_display *display);

void iris_insert_delay_us(uint32_t payload_size, uint32_t cmd_num);
int iris_get_vreg(void);
int iris_dbgfs_cont_splash_init(void *display);
int iris_lightoff_i5(struct dsi_panel *panel, struct iris_cmd_set *off_cmds);
int iris_lightoff_i7p(struct dsi_panel *panel, bool dead,
		struct iris_cmd_set *off_cmds);
int iris_lightoff_i7(struct dsi_panel *panel, bool dead,
		struct iris_cmd_set *off_cmds);
int iris_lightoff_i8(struct dsi_panel *panel, bool dead,
		struct iris_cmd_set *off_cmds);
void iris_init_i7p(struct dsi_display *display, struct dsi_panel *panel);
void iris_init_i7(struct dsi_display *display, struct dsi_panel *panel);
void iris_init_i8(struct dsi_display *display, struct dsi_panel *panel);
void iris_init_i5(struct dsi_display *display, struct dsi_panel *panel);
int iris_debug_display_info_get(char *kbuf, int size);
int iris_parse_param(struct dsi_display *display);
void iris_init(struct dsi_display *display, struct dsi_panel *panel);
int iris_sync_panel_brightness(int32_t step, void *phys_enc);

void dsi_iris_acquire_panel_lock(void);

void dsi_iris_release_panel_lock(void);
int dsi_iris_obtain_cur_timing_info(struct iris_mode_info *);
u32 dsi_iris_get_panel2_power_refcount(void);
int dsi_iris_get_panel_mode(void);
int iris_dbgfs_status_init(void *display);
int iris_dsi_send_cmds(struct iris_cmd_desc *cmds, u32 count,
	enum iris_cmd_set_state state, u8 vc_id);
int iris_configure_get_i7_selected(u32 display, u32 type, u32 count, u32 *values);
void iris_send_pwil_cmd(struct iris_cmd_set *pcmdset, u32 addr, u32 meta);
void iris_update_rd_ptr_time(void);
int iris_get_wait_vsync_count(void);
void iris_update_backlight_v2(struct sde_connector *c_conn);
#endif // _DSI_IRIS_LIGHTUP_H_
