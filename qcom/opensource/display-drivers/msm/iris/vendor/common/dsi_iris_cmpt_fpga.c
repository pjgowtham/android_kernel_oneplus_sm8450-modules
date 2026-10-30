// SPDX-License-Identifier: GPL-2.0-only
  /*
   * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
   * Copyright (C) 2017-2022, Pixelworks, Inc.
   *
   * These files contain modifications made by Pixelworks, Inc., in 2019-2022.
   */

#include <dsi_ctrl.h>


void iris_fpga_split_set_max_return_size(struct dsi_ctrl *dsi_ctrl, u16 *pdflags)
{
/* pls do not porting to customer, only used for 480*800 dsc video 60Hz panel*/
	if ((dsi_ctrl->host_config.panel_mode == DSI_OP_VIDEO_MODE) &&
		(dsi_ctrl->host_config.video_timing.h_active == 480) &&
		(dsi_ctrl->host_config.video_timing.v_active == 800))
			*pdflags |= BIT(3);
}
