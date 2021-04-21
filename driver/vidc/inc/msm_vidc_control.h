/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021,, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_CONTROL_H_
#define _MSM_VIDC_CONTROL_H_

#include <media/v4l2_vidc_extensions.h>
#include "msm_vidc_inst.h"
#include "msm_vidc_internal.h"

enum msm_vidc_ctrl_list_type {
	CHILD_LIST          = BIT(0),
	FW_LIST             = BIT(1),
};

int msm_vidc_ctrl_init(struct msm_vidc_inst *inst);
int msm_vidc_ctrl_deinit(struct msm_vidc_inst *inst);
int msm_v4l2_op_s_ctrl(struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_bitrate(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_bitrate_mode(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_entropy_mode(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_profile(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_ltr_count(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_use_ltr(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_mark_ltr(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_ir_random(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_delta_based_rc(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_transform_8x8(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_slice_count(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_layer_count(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_gop_size(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_b_frame(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_bitrate(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_peak_bitrate(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_hevc_min_qp(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_hevc_max_qp(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_hevc_frame_qp(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_v4l2_properties(struct msm_vidc_inst *inst);
int msm_vidc_adjust_session_priority(void *instance, struct v4l2_ctrl *ctrl);
int msm_vidc_adjust_roi_info(void *instance, struct v4l2_ctrl *ctrl);

int msm_vidc_set_header_mode(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_deblock_mode(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_min_qp(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_max_qp(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_frame_qp(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_req_sync_frame(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_chroma_qp_index_offset(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_slice_count(void* instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_layer_count_and_type(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_gop_size(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_bitrate(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_u32(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_u32_enum(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_constant_quality(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_vbr_related_properties(
	void *instance, enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_cbr_related_properties(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_use_and_mark_ltr(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_nal_length(void* instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_session_priority(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_flip(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_rotation(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_s32(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_q16(void *instance,
	enum msm_vidc_inst_capability_type cap_id);
int msm_vidc_set_v4l2_properties(struct msm_vidc_inst *inst);
int msm_vidc_v4l2_menu_to_hfi(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id, u32 *value);
int msm_vidc_v4l2_to_hfi_enum(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_capability_type cap_id, u32 *value);
int msm_vidc_update_cap_value(struct msm_vidc_inst *inst, u32 cap,
	s32 adjusted_val, const char *func);

#endif
