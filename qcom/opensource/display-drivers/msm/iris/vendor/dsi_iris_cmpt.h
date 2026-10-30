/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2022, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */
#ifndef _IRIS_CMPT_H_
#define _IRIS_CMPT_H_
struct iris_cmd_desc;

#if defined(CONFIG_ARCH_LAHAINA) || defined(CONFIG_ARCH_KONA) //SM8350/SM8250
#define iris_cmd_desc_rsvd_data  \
	u32 ctrl;  \
	u32 ctrl_flags;  \
	ktime_t ts

#elif defined(CONFIG_ARCH_WAIPIO) || defined(CONFIG_ARCH_KALAMA) //SM8450/SM8550
#define iris_cmd_desc_rsvd_data  \
	ktime_t ts

#elif defined(CONFIG_ARCH_PINEAPPLE)  //SM8650
#define iris_cmd_desc_rsvd_data

#endif


bool iris_is_read_cmd(struct iris_cmd_desc *pdesc);

bool iris_is_last_cmd(const struct mipi_dsi_msg  *pmsg);

bool iris_is_curmode_cmd_mode(void);

bool iris_is_curmode_vid_mode(void);

void iris_set_msg_flags(struct iris_cmd_desc *pdesc, int type);

int iris_switch_cmd_type(int type);

void iris_set_msg_ctrl(struct iris_cmd_desc *pdesc);
#endif
