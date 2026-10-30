/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2015-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2020, Pixelworks, Inc.
 *
 * These files contain modifications made by Pixelworks, Inc., in 2019-2020.
 */
#ifndef _DSI_IRIS_LP_H_
#define _DSI_IRIS_LP_H_

int iris_prepare_for_kickoff(void *phys_enc);
void _iris_wait_prev_frame_done(void);
void iris_set_esd_status(bool enable);
int iris_prepare_commit(void *phys_enc);
#endif // _DSI_IRIS_LP_H_
