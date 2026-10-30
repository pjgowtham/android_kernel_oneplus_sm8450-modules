// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2017-2020, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */
#include <sde_hw_ctl.h>
#include <sde_hw_mdss.h>
#include <sde_hw_sspp.h>
#include <sde_encoder_phys.h>
#include <sde_plane.h>
#include <sde_connector.h>
#include "dsi_iris_api.h"
#include "pw_iris_log.h"
#include "dsi_iris_lightup.h"
#include "pw_iris_pq.h"
#include "pw_iris_ioctl.h"
#include "pw_iris_lp.h"
#include "dsi_iris_lp.h"
#include "pw_iris_api.h"

void iris_sde_plane_setup_csc(void *csc_ptr)
{
	static const struct sde_csc_cfg hdrYUV = {
		{
			0x00010000, 0x00000000, 0x00000000,
			0x00000000, 0x00010000, 0x00000000,
			0x00000000, 0x00000000, 0x00010000,
		},
		{ 0x0, 0x0, 0x0,},
		{ 0x0, 0x0, 0x0,},
		{ 0x0, 0x3ff, 0x0, 0x3ff, 0x0, 0x3ff,},
		{ 0x0, 0x3ff, 0x0, 0x3ff, 0x0, 0x3ff,},
	};
	static const struct sde_csc_cfg hdrRGB10 = {
		/* S15.16 format */
		{
			0x00012A15, 0x00000000, 0x0001ADBE,
			0x00012A15, 0xFFFFD00B, 0xFFFF597E,
			0x00012A15, 0x0002244B, 0x00000000,
		},
		/* signed bias */
		{ 0xffc0, 0xfe00, 0xfe00,},
		{ 0x0, 0x0, 0x0,},
		/* unsigned clamp */
		{ 0x40, 0x3ac, 0x40, 0x3c0, 0x40, 0x3c0,},
		{ 0x00, 0x3ff, 0x00, 0x3ff, 0x00, 0x3ff,},
	};

	if (!iris_is_chip_supported())
		return;

	if (iris_get_hdr_enable() == 1)
		csc_ptr = (void *)&hdrYUV;
	else if (iris_get_hdr_enable() == 2)
		csc_ptr = (void *)&hdrRGB10;
}


int iris_sde_kms_iris_operate(struct msm_kms *kms,
		u32 operate_type, struct msm_iris_operate_value *operate_value)
{
	int ret = -EINVAL;

	if (!iris_is_chip_supported() && !iris_is_softiris_supported())
		return 0;

	if (operate_type == DRM_MSM_IRIS_OPERATE_CONF)
		ret = iris_operate_conf(operate_value);
	else if (operate_type == DRM_MSM_IRIS_OPERATE_TOOL)
		ret = iris_operate_tool(operate_value);

	return ret;
}


void iris_sde_update_dither_depth_map(uint32_t *map, uint32_t depth)
{
	if (!iris_is_chip_supported())
		return;

	if (depth >= 9) {
		map[5] = 1;
		map[6] = 2;
		map[7] = 3;
		map[8] = 2;
	}
}

bool iris_sde_encoder_off_not_allow(struct drm_encoder *drm_enc)
{
	if (!iris_is_chip_supported())
		return false;

	if (!sde_encoder_is_dsi_display(drm_enc))
		return false;

	if (sde_encoder_is_primary_display(drm_enc)) {
		if (iris_not_allow_off_primary()) {
			IRIS_LOGD("not allow primary sde encoder off");
			return true;
		}
	} else {
		if (iris_not_allow_off_secondary()) {
			IRIS_LOGD("not allow secondary sde encoder off");
			return true;
		}
	}
	return false;
}

void iris_sde_prepare_commit(uint32_t num_phys_encs, void *phys_enc)
{
	if (!iris_is_chip_supported() && !iris_is_softiris_supported())
		return;

	if (num_phys_encs == 0)
		return;

	if (iris_is_chip_supported())
		iris_prepare_commit(phys_enc);
}

void iris_sde_prepare_for_kickoff(uint32_t num_phys_encs, void *phys_enc)
{
	if (!iris_is_chip_supported() && !iris_is_softiris_supported())
		return;

	if (num_phys_encs == 0)
		return;

	if (iris_is_chip_supported())
		iris_prepare_for_kickoff(phys_enc);
	iris_sync_panel_brightness(1, phys_enc);
}

void iris_sde_encoder_kickoff(uint32_t num_phys_encs, void *phys_enc)
{
	struct sde_encoder_phys *phys_encoder = phys_enc;
	struct sde_connector *c_conn = NULL;
	struct dsi_display *display = NULL;

	if (!iris_is_chip_supported() && !iris_is_softiris_supported())
		return;

	if (num_phys_encs == 0)
		return;

	if (phys_encoder == NULL)
		return;

	if (phys_encoder->connector == NULL)
		return;

	c_conn = to_sde_connector(phys_encoder->connector);
	if (c_conn == NULL)
		return;

	if (c_conn->connector_type != DRM_MODE_CONNECTOR_DSI)
		return;

	display = c_conn->display;
	if (display == NULL)
		return;

	if (iris_is_chip_supported())
		iris_kickoff(iris_virtual_display(display));
	iris_sync_panel_brightness(2, phys_enc);
}

void iris_sde_encoder_sync_panel_brightness(uint32_t num_phys_encs, void *phys_enc)
{
	if (!iris_is_chip_supported() && !iris_is_softiris_supported())
		return;

	if (num_phys_encs == 0)
		return;

	iris_sync_panel_brightness(3, phys_enc);
}

void iris_sde_encoder_wait_for_event(uint32_t num_phys_encs, void *phys_enc,
		uint32_t event)
{
	if (!iris_is_chip_supported() && !iris_is_softiris_supported())
		return;

	if (num_phys_encs == 0)
		return;

	if (event != MSM_ENC_COMMIT_DONE)
		return;

	iris_sync_panel_brightness(4, phys_enc);
}

#if defined(PXLW_IRIS_DUAL)
#define CSC_10BIT_OFFSET       4
#define DGM_CSC_MATRIX_SHIFT       0

extern int iris_sspp_subblk_offset(struct sde_hw_pipe *ctx, int s_id, u32 *idx);

#if defined(CONFIG_ARCH_PINEAPPLE)
void iris_sde_hw_sspp_setup_csc_v2(void *pctx, const void *pfmt, void *pdata)
{
}
#else
void iris_sde_hw_sspp_setup_csc_v2(void *pctx, const void *pfmt, void *pdata)
{
	u32 idx = 0;
	u32 op_mode = 0;
	u32 clamp_shift = 0;
	u32 val;
	u32 op_mode_off = 0;
	bool csc10 = false;
	const struct sde_sspp_sub_blks *sblk;
	struct sde_hw_pipe *ctx = pctx;
	const struct sde_format *fmt = pfmt;
	struct sde_csc_cfg *data = pdata;

	if (!iris_is_dual_supported())
		return;

	if (!ctx || !ctx->cap || !ctx->cap->sblk)
		return;

	if (SDE_FORMAT_IS_YUV(fmt))
		return;

	if (!iris_is_chip_supported())
		return;

	sblk = ctx->cap->sblk;
	if (iris_sspp_subblk_offset(ctx, SDE_SSPP_CSC_10BIT, &idx))
		return;

	op_mode_off = idx;
	if (test_bit(SDE_SSPP_CSC_10BIT, &ctx->cap->features)) {
		idx += CSC_10BIT_OFFSET;
		csc10 = true;
	}
	clamp_shift = csc10 ? 16 : 8;
	if (data && !SDE_FORMAT_IS_YUV(fmt)) {
		op_mode |= BIT(0);
		sde_hw_csc_matrix_coeff_setup(&ctx->hw,
				idx, data, DGM_CSC_MATRIX_SHIFT);
		/* Pre clamp */
		val = (data->csc_pre_lv[0] << clamp_shift) | data->csc_pre_lv[1];
		SDE_REG_WRITE(&ctx->hw, idx + 0x14, val);
		val = (data->csc_pre_lv[2] << clamp_shift) | data->csc_pre_lv[3];
		SDE_REG_WRITE(&ctx->hw, idx + 0x18, val);
		val = (data->csc_pre_lv[4] << clamp_shift) | data->csc_pre_lv[5];
		SDE_REG_WRITE(&ctx->hw, idx + 0x1c, val);

		/* Post clamp */
		val = (data->csc_post_lv[0] << clamp_shift) | data->csc_post_lv[1];
		SDE_REG_WRITE(&ctx->hw, idx + 0x20, val);
		val = (data->csc_post_lv[2] << clamp_shift) | data->csc_post_lv[3];
		SDE_REG_WRITE(&ctx->hw, idx + 0x24, val);
		val = (data->csc_post_lv[4] << clamp_shift) | data->csc_post_lv[5];
		SDE_REG_WRITE(&ctx->hw, idx + 0x28, val);

		/* Pre-Bias */
		SDE_REG_WRITE(&ctx->hw, idx + 0x2c, data->csc_pre_bv[0]);
		SDE_REG_WRITE(&ctx->hw, idx + 0x30, data->csc_pre_bv[1]);
		SDE_REG_WRITE(&ctx->hw, idx + 0x34, data->csc_pre_bv[2]);

		/* Post-Bias */
		SDE_REG_WRITE(&ctx->hw, idx + 0x38, data->csc_post_bv[0]);
		SDE_REG_WRITE(&ctx->hw, idx + 0x3c, data->csc_post_bv[1]);
		SDE_REG_WRITE(&ctx->hw, idx + 0x40, data->csc_post_bv[2]);
	}
	IRIS_LOGVV("%s(), name:%s offset:%x ctx->idx:%x op_mode:%x",
			__func__, sblk->csc_blk.name, idx, ctx->idx, op_mode);
	SDE_REG_WRITE(&ctx->hw, op_mode_off, op_mode);
	wmb();
}
#endif // CONFIG_ARCH_PINEAPPLE
#endif // PXLW_IRIS_DUAL

void iris_sde_update_rd_ptr_time(void)
{
	if (iris_is_chip_supported() || iris_is_softiris_supported())
		iris_update_rd_ptr_time();
}

int iris_sde_get_wait_vsync_count(void)
{
	return iris_get_wait_vsync_count();
}

void iris_sde_kickoff_update_backlight(struct drm_connector *conn)
{
	struct sde_connector *c_conn;

	if (iris_is_chip_supported() || iris_is_softiris_supported()) {
		c_conn = to_sde_connector(conn);
		iris_update_backlight_v2(c_conn);
	}
}

void iris_sde_color_process_plane_disable(struct drm_plane *plane, void *hwctl)
{
	struct sde_plane *psde;
	struct sde_plane_state *pstate;
	struct sde_hw_cp_cfg hw_cfg = {};
	struct sde_hw_ctl *ctl = (struct sde_hw_ctl *)hwctl;
	bool fp16_igc, fp16_unmult, ucsc_unmult, ucsc_alpha_dither;
	int ucsc_gc, ucsc_igc;

	psde = to_sde_plane(plane);
	pstate = to_sde_plane_state(plane->state);

	if (pstate->dirty & SDE_PLANE_DIRTY_VIG_GAMUT &&
			psde->pipe_hw->ops.setup_vig_gamut) {
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = 0;
		hw_cfg.payload = NULL;
		psde->pipe_hw->ops.setup_vig_gamut(psde->pipe_hw, &hw_cfg);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_VIG_IGC &&
			psde->pipe_hw->ops.setup_vig_igc) {
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = 0;
		hw_cfg.payload = NULL;
		psde->pipe_hw->ops.setup_vig_igc(psde->pipe_hw, &hw_cfg);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_DMA_IGC &&
			psde->pipe_hw->ops.setup_dma_igc) {
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = 0;
		hw_cfg.payload = NULL;
		psde->pipe_hw->ops.setup_dma_igc(psde->pipe_hw, &hw_cfg,
				pstate->multirect_index);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_DMA_GC &&
			psde->pipe_hw->ops.setup_dma_gc) {
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = 0;
		hw_cfg.payload = NULL;
		psde->pipe_hw->ops.setup_dma_gc(psde->pipe_hw, &hw_cfg,
				pstate->multirect_index);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_FP16_IGC &&
			psde->pipe_hw->ops.setup_fp16_igc) {
		fp16_igc = false;
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = sizeof(bool);
		hw_cfg.payload = &fp16_igc;
		psde->pipe_hw->ops.setup_fp16_igc(psde->pipe_hw,
				pstate->multirect_index, &hw_cfg);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_FP16_GC &&
			psde->pipe_hw->ops.setup_fp16_gc) {
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = 0;
		hw_cfg.payload = NULL;
		psde->pipe_hw->ops.setup_fp16_gc(psde->pipe_hw,
				pstate->multirect_index, &hw_cfg);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_FP16_CSC &&
			psde->pipe_hw->ops.setup_fp16_csc) {
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = 0;
		hw_cfg.payload = NULL;
		psde->pipe_hw->ops.setup_fp16_csc(psde->pipe_hw,
				pstate->multirect_index, &hw_cfg);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_FP16_UNMULT &&
			psde->pipe_hw->ops.setup_fp16_unmult) {
		fp16_unmult = false;
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = sizeof(bool);
		hw_cfg.payload = &fp16_unmult;
		psde->pipe_hw->ops.setup_fp16_unmult(psde->pipe_hw,
				pstate->multirect_index, &hw_cfg);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_UCSC_IGC &&
			psde->pipe_hw->ops.setup_ucsc_igc) {
		ucsc_igc = UCSC_IGC_MODE_DISABLE;
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = sizeof(int);
		hw_cfg.payload = &ucsc_igc;
		psde->pipe_hw->ops.setup_ucsc_igc(psde->pipe_hw,
				pstate->multirect_index, &hw_cfg);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_UCSC_GC &&
			psde->pipe_hw->ops.setup_ucsc_gc) {
		ucsc_gc = UCSC_GC_MODE_DISABLE;
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = sizeof(int);
		hw_cfg.payload = &ucsc_gc;
		psde->pipe_hw->ops.setup_ucsc_gc(psde->pipe_hw,
				pstate->multirect_index, &hw_cfg);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_UCSC_CSC &&
			psde->pipe_hw->ops.setup_ucsc_csc) {
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = 0;
		hw_cfg.payload = NULL;
		psde->pipe_hw->ops.setup_ucsc_csc(psde->pipe_hw,
				pstate->multirect_index, &hw_cfg);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_UCSC_UNMULT &&
			psde->pipe_hw->ops.setup_ucsc_unmult) {
		ucsc_unmult = false;
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = sizeof(bool);
		hw_cfg.payload = &ucsc_unmult;
		psde->pipe_hw->ops.setup_ucsc_unmult(psde->pipe_hw,
				pstate->multirect_index, &hw_cfg);
	}

	if (pstate->dirty & SDE_PLANE_DIRTY_UCSC_ALPHA_DITHER &&
			psde->pipe_hw->ops.setup_ucsc_alpha_dither) {
		ucsc_alpha_dither = false;
		hw_cfg.last_feature = 0;
		hw_cfg.ctl = ctl;
		hw_cfg.len = sizeof(bool);
		hw_cfg.payload = &ucsc_alpha_dither;
		psde->pipe_hw->ops.setup_ucsc_alpha_dither(psde->pipe_hw,
				pstate->multirect_index, &hw_cfg);
	}
}
