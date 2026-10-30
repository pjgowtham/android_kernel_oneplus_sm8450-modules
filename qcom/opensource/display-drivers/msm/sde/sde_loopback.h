/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _SDE_LOOPBACK_H
#define _SDE_LOOPBACK_H

#include "sde_connector.h"

int sde_lb_display_get_info(struct drm_connector *connector,
	struct msm_display_info *info, void *display);

int sde_lb_get_mode_info(struct drm_connector *connector,
	const struct drm_display_mode *drm_mode,
	struct msm_sub_mode *sub_mode, struct msm_mode_info *mode_info,
	void *display, const struct msm_resource_caps_info *avail_res);

int sde_lb_connector_get_modes(struct drm_connector *connector, void *display,
	const struct msm_resource_caps_info *avail_res);

int sde_lb_set_info_blob(struct drm_connector *connector,
	void *info, void *display, struct msm_mode_info *mode_info);

enum drm_connector_status sde_lb_detect(struct drm_connector *connector,
	bool force, void *display);
#endif
