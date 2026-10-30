/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2020, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */
#ifndef _DSI_IRIS_API_H_
#define _DSI_IRIS_API_H_

#include "dsi_display.h"
#include "pw_iris_api.h"

void iris_deinit(struct dsi_display *display);
void iris_power_on(struct dsi_panel *panel);
void iris_power_off(struct dsi_panel *panel);
int iris_enable(struct dsi_panel *panel, struct iris_cmd_set *on_cmds);
int iris_disable(struct dsi_panel *panel, bool dead, struct iris_cmd_set *off_cmds);
void iris_set_panel_timing(uint32_t index,
		const struct iris_mode_info *timing);
void iris_pre_switch(struct iris_mode_info *new_timing);
int iris_status_get(void);
int iris_set_aod(struct dsi_panel *panel, bool aod);
bool iris_get_aod(struct dsi_panel *panel);
int iris_set_fod(struct dsi_panel *panel, bool fod);

void iris_send_cont_splash(struct dsi_display *display);
void iris_prepare(struct dsi_display *display);

int iris_panel_ctrl_read_reg(struct dsi_display_ctrl *ctrl,
		struct dsi_panel *panel, u8 *rx_buf, int rlen,
		struct iris_cmd_desc *cmd);
int iris_get_drm_property(int id);
bool iris_osd_drm_autorefresh_enabled(bool is_secondary);
bool iris_is_virtual_encoder_phys(void *phys_enc);
void iris_register_osd_irq(void *disp);

void iris_sde_plane_setup_csc(void *csc_ptr);
int iris_sde_kms_iris_operate(struct msm_kms *kms,
		u32 operate_type, struct msm_iris_operate_value *operate_value);
void iris_sde_update_dither_depth_map(uint32_t *map, uint32_t depth);
void iris_sde_prepare_commit(uint32_t num_phys_encs, void *phys_enc);
void iris_sde_prepare_for_kickoff(uint32_t num_phys_encs, void *phys_enc);
void iris_sde_encoder_sync_panel_brightness(uint32_t num_phys_encs,
		void *phys_enc);
void iris_sde_encoder_kickoff(uint32_t num_phys_encs, void *phys_enc);
void iris_sde_encoder_wait_for_event(uint32_t num_phys_encs,
		void *phys_enc, uint32_t event);

int msm_ioctl_iris_operate_conf(struct drm_device *dev, void *data,
		struct drm_file *file);
int msm_ioctl_iris_operate_tool(struct drm_device *dev, void *data,
		struct drm_file *file);

void iris_dsi_display_res_init(struct dsi_display *display);
void iris_dsi_display_debugfs_init(struct dsi_display *display,
		struct dentry *dir, struct dentry *dump_file);

void iris_sde_hw_sspp_setup_csc_v2(void *pctx, const void *pfmt, void *pdata);
bool iris_check_reg_read(struct dsi_panel *panel);
void iris_fpga_split_set_max_return_size(struct dsi_ctrl *dsi_ctrl, u16 *pdflags);
bool iris_is_secondary_display(void *phys_enc);
void iris_sde_update_rd_ptr_time(void);
int iris_sde_get_wait_vsync_count(void);
void iris_sde_kickoff_update_backlight(struct drm_connector *c_conn);
u8 dsi_op_mode_to_iris_op_mode(u8 dsi_mode);
bool iris_sde_encoder_off_not_allow(struct drm_encoder *drm_enc);
void iris_sde_color_process_plane_disable(struct drm_plane *plane, void *hwctl);
#endif // _DSI_IRIS_API_H_
