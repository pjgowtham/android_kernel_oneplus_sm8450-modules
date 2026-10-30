// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "sde_loopback.h"
#include "msm_kms.h"

static struct drm_connector *sde_get_primary_conn(struct drm_device *dev)
{
	struct drm_connector *conn, *prim_conn = NULL;
	struct drm_connector_list_iter conn_iter;

	if (!dev) {
		SDE_ERROR("Invalid drm device\n");
		return NULL;
	}

	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(conn, &conn_iter) {
		if (conn && conn->connector_type == DRM_MODE_CONNECTOR_DSI) {
			prim_conn = conn;
			break;
		}
	}
	drm_connector_list_iter_end(&conn_iter);

	return prim_conn;
}

enum drm_connector_status sde_lb_detect(struct drm_connector *connector,
		bool force, void *display)
{
	return connector_status_connected;
}

int sde_lb_set_info_blob(struct drm_connector *connector,
	void *info, void *display, struct msm_mode_info *mode_info)
{
	struct sde_kms *sde_kms;

	if (!info || !connector)
		return -EINVAL;

	sde_kms = sde_connector_get_kms(connector);
	if (!sde_kms || !sde_kms->catalog)
		return -EINVAL;

	if (sde_kms->catalog->cac_version != SDE_SSPP_CAC_LOOPBACK)
		return 0;

	sde_kms_info_add_keyint(info, "has_cac_loopback", true);

	return 0;
}

int sde_lb_display_get_info(struct drm_connector *connector,
	struct msm_display_info *info, void *display)
{
	if (!info)
		return -EINVAL;

	memset(info, 0, sizeof(struct msm_display_info));
	info->intf_type = DRM_MODE_CONNECTOR_VIRTUAL;
	info->num_of_h_tiles = 1;
	info->h_tile_instance[0] = 0;
	info->is_connected = true;
	info->capabilities = MSM_DISPLAY_LOOPBACK_MODE;

	return 0;
}

int sde_lb_get_mode_info(struct drm_connector *connector,
	const struct drm_display_mode *drm_mode,
	struct msm_sub_mode *sub_mode, struct msm_mode_info *mode_info,
	void *display, const struct msm_resource_caps_info *avail_res)
{
	struct drm_connector *prim_conn = NULL;
	struct msm_mode_info prim_mode_info;
	int rc = 0;

	if (!drm_mode || !mode_info || !avail_res)
		return -EINVAL;

	prim_conn = sde_get_primary_conn(connector->dev);
	if (!prim_conn) {
		SDE_DEBUG("primary connector is not created yet\n");
		return -EINVAL;
	}

	memset(mode_info, 0, sizeof(*mode_info));
	rc = sde_connector_get_mode_info(prim_conn, drm_mode, NULL, &prim_mode_info);
	if (rc)
		return -EINVAL;

	mode_info->topology.num_lm = prim_mode_info.topology.num_lm;
	mode_info->comp_info.comp_type = MSM_DISPLAY_COMPRESSION_NONE;
	mode_info->wide_bus_en = false;
	mode_info->comp_info.comp_ratio = MSM_DISPLAY_COMPRESSION_RATIO_NONE;

	return rc;
}

int sde_lb_connector_get_modes(struct drm_connector *connector, void *display,
	const struct msm_resource_caps_info *avail_res)
{
	struct drm_connector *prim_conn = NULL;
	struct drm_device *dev;
	struct drm_display_mode *mode, *m;
	int num_modes = 0;

	if (!connector)
		return -EINVAL;

	dev = connector->dev;
	prim_conn = sde_get_primary_conn(dev);

	if (!prim_conn) {
		SDE_DEBUG("primary connector is not created yet\n");
		return 0;
	}

	list_for_each_entry(mode, &prim_conn->modes, head) {
		m = drm_mode_duplicate(connector->dev, mode);
		drm_mode_probed_add(connector, m);
		num_modes++;
	}

	return num_modes;
}
