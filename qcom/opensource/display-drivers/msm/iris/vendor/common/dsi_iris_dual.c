// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2020, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */

#include <linux/types.h>
#include <dsi_drm.h>
#include <sde_encoder_phys.h>
#include <sde_dsc_helper.h>
#include "dsi_display.h"
#include "dsi_panel.h"
#include "dsi_iris_lightup.h"
#include "dsi_iris_dual.h"
#include "pw_iris_api.h"
#include "pw_iris_log.h"

static irqreturn_t iris_osd_irq_handler_i7(int irq, void *data)
{
	struct dsi_display *display = data;
	struct drm_encoder *enc = NULL;

	if (display == NULL) {
		IRIS_LOGE("%s(), invalid display.", __func__);
		return IRQ_NONE;
	}

	IRIS_LOGV("%s(), irq: %d, display: %s", __func__, irq, display->name);
	if (display && display->bridge)
		enc = display->bridge->base.encoder;

	if (enc)
		sde_encoder_disable_autorefresh_handler(enc);
	else
		IRIS_LOGW("%s(), no encoder.", __func__);

	return IRQ_HANDLED;
}

void iris_register_osd_irq_ext_i7(void *disp)
{
	int rc = 0;
	int osd_gpio = -1;
	struct dsi_display *display = NULL;
	struct platform_device *pdev = NULL;
	struct iris_cfg *pcfg = NULL;
	struct device_node *np = NULL;

	if (!iris_is_dual_supported())
		return;

	if (!disp) {
		IRIS_LOGE("%s(), invalid display.", __func__);
		return;
	}

	display = (struct dsi_display *)disp;
	if (!iris_virtual_display(display))
		return;

	pcfg = iris_get_cfg();

	if (!pcfg || !pcfg->pdev) {
		IRIS_LOGE("%s(%d), invalid inparms!",
				__func__, __LINE__);
		return;
	}
	np = pcfg->pdev->dev.of_node;
	if (np)
		pcfg->iris_abyp_ready_gpio = of_get_named_gpio(np,
			"iris-abyp-ready-gpios", 0);

	IRIS_LOGI("%s(), abyp ready status gpio %d", __func__,
				pcfg->iris_abyp_ready_gpio);

	osd_gpio = pcfg->iris_osd_gpio = pcfg->iris_abyp_ready_gpio;
	IRIS_LOGI("%s(), for display %s, osd status gpio is %d",
			__func__,
			display->name, osd_gpio);
	if (!gpio_is_valid(osd_gpio)) {
		IRIS_LOGE("%s(%d), osd status gpio not specified",
				__func__, __LINE__);
		return;
	}
	gpio_direction_input(osd_gpio);
	pdev = display->pdev;
	IRIS_LOGI("%s, display: %s, irq: %d", __func__, display->name, gpio_to_irq(osd_gpio));
	rc = devm_request_irq(&pdev->dev, gpio_to_irq(osd_gpio), iris_osd_irq_handler_i7,
			IRQF_TRIGGER_RISING, "OSD_GPIO", display);
	if (rc) {
		IRIS_LOGE("%s(), IRIS OSD request irq failed", __func__);
		return;
	}

	disable_irq(gpio_to_irq(osd_gpio));
}


static irqreturn_t iris_osd_irq_handler_i8(int irq, void *data)
{
	iris_inc_osd_irq_cnt();
	return IRQ_HANDLED;
}

void iris_register_osd_irq_ext_i8(void *disp)
{
	int rc = 0;
	int osd_gpio = -1;
	struct dsi_display *display = NULL;
	struct platform_device *pdev = NULL;
	struct iris_cfg *pcfg = NULL;
	struct device_node *np = NULL;

	if (!iris_is_dual_supported())
		return;

	if (!disp) {
		IRIS_LOGE("%s(), invalid display.", __func__);
		return;
	}

	display = (struct dsi_display *)disp;
	if (!iris_virtual_display(display))
		return;

	pcfg = iris_get_cfg();

	if (!pcfg || !pcfg->pdev) {
		IRIS_LOGE("%s(%d), invalid inparms!",
				__func__, __LINE__);
		return;
	}
	np = pcfg->pdev->dev.of_node;
	if (np)
		pcfg->iris_abyp_ready_gpio = of_get_named_gpio(np,
			"iris-abyp-ready-gpios", 0);

	IRIS_LOGI("%s(), abyp ready status gpio %d", __func__,
				pcfg->iris_abyp_ready_gpio);

	osd_gpio = pcfg->iris_osd_gpio = pcfg->iris_abyp_ready_gpio;
	IRIS_LOGI("%s(), for display %s, osd status gpio is %d",
			__func__,
			display->name, osd_gpio);
	if (!gpio_is_valid(osd_gpio)) {
		IRIS_LOGE("%s(%d), osd status gpio not specified",
				__func__, __LINE__);
		return;
	}
	gpio_direction_input(osd_gpio);
	pdev = display->pdev;
	IRIS_LOGI("%s, display: %s, irq: %d", __func__, display->name, gpio_to_irq(osd_gpio));
	rc = devm_request_irq(&pdev->dev, gpio_to_irq(osd_gpio), iris_osd_irq_handler_i8,
			IRQF_TRIGGER_RISING, "OSD_GPIO", NULL);
	if (rc) {
		IRIS_LOGE("%s(), IRIS OSD request irq failed", __func__);
		return;
	}

	disable_irq(gpio_to_irq(osd_gpio));
}

bool iris_is_secondary_display(void *phys_enc)
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

	if (display && display->panel && display->panel->is_secondary)
		return true;

	return false;
}

int iris_create_pps_buf_cmd(char *buf, int pps_id, u32 len, bool is_secondary)
{
	struct drm_dsc_config *dsc = NULL;
	char *bp = buf;
	char data;
	u32 i, bpp;
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (len < IRIS_DSC_PPS_SIZE || !buf || !pcfg_ven)
		return -EINVAL;

	if (is_secondary) {
		if (!pcfg_ven->panel2 || !pcfg_ven->panel2->cur_mode
				|| !pcfg_ven->panel2->cur_mode->priv_info) {
			IRIS_LOGE("%s(), aux invalid params!", __func__);
			return -EINVAL;
		}
		dsc = &pcfg_ven->panel2->cur_mode->priv_info->dsc.config;
	} else {
		if (!pcfg_ven->panel || !pcfg_ven->panel->cur_mode
				|| !pcfg_ven->panel->cur_mode->priv_info) {
			IRIS_LOGE("%s(), invalid params!", __func__);
			return -EINVAL;
		}
		dsc = &pcfg_ven->panel->cur_mode->priv_info->dsc.config;
	}

	memset(buf, 0, len);
	/* pps0 */
	*bp++ = (dsc->dsc_version_minor |
			dsc->dsc_version_major << 4);
	*bp++ = (pps_id & 0xff);		/* pps1 */
	bp++;					/* pps2, reserved */

	data = dsc->line_buf_depth & 0x0f;
	data |= ((dsc->bits_per_component & 0xf) << DSC_PPS_BPC_SHIFT);
	*bp++ = data;				/* pps3 */

	bpp = dsc->bits_per_pixel;
	data = (bpp >> DSC_PPS_MSB_SHIFT);
	data &= 0x03;				/* upper two bits */
	data |= ((dsc->block_pred_enable & 0x1) << 5);
	data |= ((dsc->convert_rgb & 0x1) << 4);
	data |= ((dsc->simple_422 & 0x1) << 3);
	data |= ((dsc->vbr_enable & 0x1) << 2);
	*bp++ = data;				/* pps4 */
	*bp++ = (bpp & DSC_PPS_LSB_MASK);	/* pps5 */

	*bp++ = ((dsc->pic_height >> 8) & 0xff); /* pps6 */
	*bp++ = (dsc->pic_height & 0x0ff);	/* pps7 */
	*bp++ = ((dsc->pic_width >> 8) & 0xff);	/* pps8 */
	*bp++ = (dsc->pic_width & 0x0ff);	/* pps9 */

	*bp++ = ((dsc->slice_height >> 8) & 0xff);/* pps10 */
	*bp++ = (dsc->slice_height & 0x0ff);	/* pps11 */
	*bp++ = ((dsc->slice_width >> 8) & 0xff); /* pps12 */
	*bp++ = (dsc->slice_width & 0x0ff);	/* pps13 */

	*bp++ = ((dsc->slice_chunk_size >> 8) & 0xff);/* pps14 */
	*bp++ = (dsc->slice_chunk_size & 0x0ff);	/* pps15 */

	*bp++ = (dsc->initial_xmit_delay >> 8) & 0x3; /* pps16 */
	*bp++ = (dsc->initial_xmit_delay & 0xff);/* pps17 */

	*bp++ = ((dsc->initial_dec_delay >> 8) & 0xff); /* pps18 */
	*bp++ = (dsc->initial_dec_delay & 0xff);/* pps19 */

	bp++;				/* pps20, reserved */

	*bp++ = (dsc->initial_scale_value & 0x3f); /* pps21 */

	*bp++ = ((dsc->scale_increment_interval >> 8) & 0xff); /* pps22 */
	*bp++ = (dsc->scale_increment_interval & 0xff); /* pps23 */

	*bp++ = ((dsc->scale_decrement_interval >> 8) & 0xf); /* pps24 */
	*bp++ = (dsc->scale_decrement_interval & 0x0ff);/* pps25 */

	bp++;					/* pps26, reserved */

	*bp++ = (dsc->first_line_bpg_offset & 0x1f);/* pps27 */

	*bp++ = ((dsc->nfl_bpg_offset >> 8) & 0xff);/* pps28 */
	*bp++ = (dsc->nfl_bpg_offset & 0x0ff);	/* pps29 */
	*bp++ = ((dsc->slice_bpg_offset >> 8) & 0xff);/* pps30 */
	*bp++ = (dsc->slice_bpg_offset & 0x0ff);/* pps31 */

	*bp++ = ((dsc->initial_offset >> 8) & 0xff);/* pps32 */
	*bp++ = (dsc->initial_offset & 0x0ff);	/* pps33 */

	*bp++ = ((dsc->final_offset >> 8) & 0xff);/* pps34 */
	*bp++ = (dsc->final_offset & 0x0ff);	/* pps35 */

	*bp++ = (dsc->flatness_min_qp & 0x1f);	/* pps36 */
	*bp++ = (dsc->flatness_max_qp & 0x1f);	/* pps37 */

	*bp++ = ((dsc->rc_model_size >> 8) & 0xff);/* pps38 */
	*bp++ = (dsc->rc_model_size & 0x0ff);	/* pps39 */

	*bp++ = (dsc->rc_edge_factor & 0x0f);	/* pps40 */

	*bp++ = (dsc->rc_quant_incr_limit0 & 0x1f);	/* pps41 */
	*bp++ = (dsc->rc_quant_incr_limit1 & 0x1f);	/* pps42 */

	data = ((dsc->rc_tgt_offset_high & 0xf) << 4);
	data |= (dsc->rc_tgt_offset_low & 0x0f);
	*bp++ = data;				/* pps43 */

	for (i = 0; i < DSC_NUM_BUF_RANGES - 1; i++)
		*bp++ = (dsc->rc_buf_thresh[i] & 0xff); /* pps44 - pps57 */

	for (i = 0; i < DSC_NUM_BUF_RANGES; i++) {
		/* pps58 - pps87 */
		data = (dsc->rc_range_params[i].range_min_qp & 0x1f);
		data <<= 3;
		data |= ((dsc->rc_range_params[i].range_max_qp >> 2) & 0x07);
		*bp++ = data;
		data = (dsc->rc_range_params[i].range_max_qp & 0x03);
		data <<= 6;
		data |= (dsc->rc_range_params[i].range_bpg_offset & 0x3f);
		*bp++ = data;
	}

	if (dsc->dsc_version_minor == 0x2) {
		if (dsc->native_422)
			data = BIT(0);
		else if (dsc->native_420)
			data = BIT(1);
		*bp++ = data;				/* pps88 */
		*bp++ = dsc->second_line_bpg_offset;	/* pps89 */

		*bp++ = ((dsc->nsl_bpg_offset >> 8) & 0xff);/* pps90 */
		*bp++ = (dsc->nsl_bpg_offset & 0x0ff);	/* pps91 */

		*bp++ = ((dsc->second_line_offset_adj >> 8) & 0xff); /* pps92*/
		*bp++ = (dsc->second_line_offset_adj & 0x0ff);	/* pps93 */

		/* rest bytes are reserved and set to 0 */
	}

	return 0;
}

bool iris_main_panel_existed(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (unlikely(!pcfg_ven || !pcfg_ven->panel)) {
		IRIS_LOGE("%s(), No secondary panel configured!", __func__);
		return false;
	}

	return true;
}

bool iris_aux_panel_existed(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (unlikely(!pcfg_ven || !pcfg_ven->panel2)) {
		IRIS_LOGE("%s(), No secondary panel configured!", __func__);
		return false;
	}

	return true;
}

bool iris_main_panel_initialized(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (unlikely(!pcfg_ven || !pcfg_ven->panel)) {
		IRIS_LOGE("%s(), No secondary panel configured!", __func__);
		return false;
	}

	return pcfg_ven->panel->panel_initialized;
}

bool iris_aux_panel_initialized(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (unlikely(!pcfg_ven || !pcfg_ven->panel2)) {
		IRIS_LOGE("%s(), No secondary panel configured!", __func__);
		return false;
	}

	return pcfg_ven->panel2->panel_initialized;
}

void iris_set_idlemgr(unsigned int crtc_id, unsigned int enable, bool need_lock)
{
	return;
}
