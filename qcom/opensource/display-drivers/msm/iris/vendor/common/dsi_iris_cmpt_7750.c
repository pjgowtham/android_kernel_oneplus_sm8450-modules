// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2022, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2022.
 */


#include <drm/drm_mipi_dsi.h>
#include <video/mipi_display.h>
#include <dsi_drm.h>

#include "pw_iris_def.h"
#include "dsi_iris_lightup.h"
#include "dsi_iris_cmpt.h"
#include "dsi_iris_api.h"

bool iris_is_read_cmd(struct iris_cmd_desc *pdesc)
{
	if (!pdesc)
		return false;

	return (pdesc->ctrl_flags & DSI_CTRL_CMD_READ);
}

bool iris_is_last_cmd(const struct mipi_dsi_msg *pmsg)
{
	if (!pmsg)
		return false;

	return !(pmsg->flags & MIPI_DSI_MSG_BATCH_COMMAND);
}

bool iris_is_curmode_cmd_mode(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (pcfg_ven->panel)
		return (dsi_op_mode_to_iris_op_mode(pcfg_ven->panel->panel_mode) == IRIS_CMD_MODE);

	return false;
}

bool iris_is_curmode_vid_mode(void)
{
	struct iris_vendor_cfg *pcfg_ven = iris_get_vendor_cfg();

	if (pcfg_ven->panel)
		return (dsi_op_mode_to_iris_op_mode(pcfg_ven->panel->panel_mode) == IRIS_VIDEO_MODE);

	return false;
}

void iris_set_msg_flags(struct iris_cmd_desc *pdesc, int type)
{
	if (!pdesc)
		return;

	switch (type) {
	case LAST_FLAG:
		break;
	case READ_FLAG:
		pdesc->msg.flags |= MIPI_DSI_MSG_REQ_ACK;
		pdesc->ctrl_flags |= DSI_CTRL_CMD_READ;
		break;
	case BATCH_FLAG:
		pdesc->msg.flags |= MIPI_DSI_MSG_BATCH_COMMAND;
		break;
	}
}

int iris_switch_cmd_type(int type)
{
	int s_type = type;

	switch (type) {
	case MIPI_DSI_COMPRESSION_MODE:
		s_type = MIPI_DSI_DCS_SHORT_WRITE;
		break;
	case MIPI_DSI_PICTURE_PARAMETER_SET:
		s_type = MIPI_DSI_DCS_LONG_WRITE;
		break;
	}
	return s_type;
}

void iris_set_msg_ctrl(struct iris_cmd_desc *pdesc)
{
	return;
}
