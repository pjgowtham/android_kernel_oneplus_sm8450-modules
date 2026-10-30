/***
  notify for other driver
**/

/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "magcvr_notify.h"
#include <linux/printk.h>
#include <linux/spinlock.h>

#define MAGCVR_MAX_INSTANCE 2

static int g_magcvr_current_pos[MAGCVR_MAX_INSTANCE] = {-1, -1};
static DEFINE_RWLOCK(magcvr_pos_lock);

static BLOCKING_NOTIFIER_HEAD(magcvr_notifier_list);

int magcvr_event_register_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&magcvr_notifier_list, nb);
}
EXPORT_SYMBOL(magcvr_event_register_notifier);

int magcvr_event_unregister_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&magcvr_notifier_list, nb);
}
EXPORT_SYMBOL(magcvr_event_unregister_notifier);

int magcvr_event_call_notifier(unsigned long action, void *data)
{
	blocking_notifier_call_chain(&magcvr_notifier_list, action, data);
	return 0;
}
EXPORT_SYMBOL(magcvr_event_call_notifier);

void magcvr_set_current_pos(int magcvr_index, int magcvr_pos)
{
	if (magcvr_index >= 0 && magcvr_index < MAGCVR_MAX_INSTANCE) {
		write_lock(&magcvr_pos_lock);
		pr_err("[magcvr_set_current_pos]->index:%d pos:%d\n", magcvr_index, magcvr_pos);
		g_magcvr_current_pos[magcvr_index] = magcvr_pos;
		write_unlock(&magcvr_pos_lock);
	}
}
EXPORT_SYMBOL(magcvr_set_current_pos);

int magcvr_get_current_pos(void)
{
	int pos;
	read_lock(&magcvr_pos_lock);
	pos = g_magcvr_current_pos[MAGCVR_INSTANCE_CHARGE];
	read_unlock(&magcvr_pos_lock);
	pr_err("[magcvr_get_current_pos]->pos:%d\n", pos);
	return pos;
}
EXPORT_SYMBOL(magcvr_get_current_pos);

int magcvr_get_current_pos_pen(void)
{
	int pos;
	read_lock(&magcvr_pos_lock);
	pos = g_magcvr_current_pos[MAGCVR_INSTANCE_PEN];
	read_unlock(&magcvr_pos_lock);
	pr_err("[magcvr_get_current_pos_pen]->pos:%d\n", pos);
	return pos;
}
EXPORT_SYMBOL(magcvr_get_current_pos_pen);

MODULE_DESCRIPTION("magcvr Event Notify Driver");
MODULE_LICENSE("GPL");
