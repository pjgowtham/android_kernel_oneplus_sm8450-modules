// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/haven/hh_rm_drv.h>
#include <linux/haven/hh_irq_lend.h>
#include <linux/haven/hh_mem_notifier.h>
#include <linux/sort.h>
#include <linux/bsearch.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include "sde_kms.h"
#include "sde_vm_common.h"
#include "sde_vm.h"

#define to_vm_trusted(vm) ((struct sde_vm_trusted *)vm)

static int __sgl_cmp(const void *a, const void *b)
{
	struct hh_sgl_entry *l = (struct hh_sgl_entry *)a;
	struct hh_sgl_entry *r = (struct hh_sgl_entry *)b;

	return  (l->ipa_base - r->ipa_base);
}

int _sde_vm_validate_sgl(struct hh_sgl_desc *expected,
			 struct hh_sgl_desc *assigned)
{
	u32 idx;

	/*
	 * fragmented address spaces are not supported.
	 * So the number of sgl entries is expected to be the same.
	 */
	if (expected->n_sgl_entries != assigned->n_sgl_entries)
		return -E2BIG;

	sort(assigned->sgl_entries, assigned->n_sgl_entries,
			sizeof(assigned->sgl_entries[0]), __sgl_cmp, NULL);

	for (idx = 0; idx < expected->n_sgl_entries; idx++) {
		struct hh_sgl_entry *e = &expected->sgl_entries[idx];
		struct hh_sgl_entry *a = &assigned->sgl_entries[idx];

		if ((e->ipa_base != a->ipa_base) || (e->size != a->size)) {
			SDE_DEBUG("sgl mismatch: (%ld - %d) vs (%ld - %d)\n",
				   e->ipa_base, e->size, a->ipa_base, a->size);
			return -EINVAL;
		}
	}

	return 0;
}

static int __irq_cmp(const void *a, const void *b)
{
	struct sde_vm_irq_entry *l = (struct sde_vm_irq_entry *)a;
	struct sde_vm_irq_entry *r = (struct sde_vm_irq_entry *)b;

	return  (l->label - r->label);
}

void sde_vm_irq_lend_notification_handler(void *req, enum hh_irq_label label)
{
	struct sde_vm_trusted *sde_vm;
	struct sde_kms *sde_kms;
	struct sde_vm_irq_desc *irq_desc;
	struct sde_vm_irq_entry irq_temp, *found = NULL;
	struct irq_data *exp_irq_data, *acc_irq_data;
	int accepted_irq, expected_irq;

	if (!req) {
		SDE_ERROR("invalid data on lend notification\n");
		return;
	}

	sde_vm = to_vm_trusted(req);
	sde_kms = sde_vm->base.sde_kms;
	irq_desc = sde_vm->irq_desc;

	mutex_lock(&sde_vm->base.vm_res_lock);

	memset(&irq_temp, 0, sizeof(irq_temp));

	irq_temp.label = label;
	found = bsearch((void *)&irq_temp, (void *)irq_desc->irq_entries,
			irq_desc->n_irq, sizeof(struct sde_vm_irq_entry),
			__irq_cmp);
	if (!found) {
		SDE_ERROR("irq mismatch for label: %d irq: %d\n",
			   irq_temp.label, irq_temp.irq);
		goto end;
	}

	expected_irq = found->irq;
	accepted_irq = hh_irq_accept(label, -1, IRQ_TYPE_LEVEL_HIGH);
	if (accepted_irq < 0) {
		SDE_ERROR("failed to accept irq for label: %d\n");
		goto end;
	}

	exp_irq_data = irq_get_irq_data(expected_irq);
	if (!exp_irq_data) {
		SDE_ERROR("failed to get irq data for irq: %d\n", exp_irq_data);
		goto end;
	}

	acc_irq_data = irq_get_irq_data(accepted_irq);
	if (!acc_irq_data) {
		SDE_ERROR("failed to get irq data for irq: %d\n", accepted_irq);
		goto end;
	}

	if (exp_irq_data->hwirq != acc_irq_data->hwirq) {
		SDE_ERROR("IRQ mismatch on ACCEPT for label %d\n", label);
		goto end;
	}

	SDE_INFO("IRQ accept succeeded for label %d irq: %d\n", label,
			exp_irq_data->hwirq);

	atomic_inc(&sde_vm->base.n_irq_lent);
end:
	mutex_unlock(&sde_vm->base.vm_res_lock);
}

static void sde_vm_mem_lend_notification_handler(enum hh_mem_notifier_tag tag,
					       unsigned long notif_type,
					void *entry_data, void *notif_msg)
{
	struct hh_rm_notif_mem_shared_payload *payload;
	struct hh_sgl_desc *sgl_desc;
	struct hh_acl_desc *acl_desc;
	struct sde_kms *sde_kms;
	struct sde_vm_trusted *sde_vm;
	int rc = 0;

	if (notif_type != HH_RM_NOTIF_MEM_SHARED ||
			tag != HH_MEM_NOTIFIER_TAG_DISPLAY)
		return;

	if (!entry_data || !notif_msg)
		return;

	payload = (struct hh_rm_notif_mem_shared_payload *)notif_msg;

	if (payload->trans_type != HH_RM_TRANS_TYPE_LEND ||
	    payload->label != SDE_VM_MEM_LABEL)
		return;

	sde_vm = (struct sde_vm_trusted *)entry_data;
	sde_kms = sde_vm->base.sde_kms;

	mutex_lock(&sde_vm->base.vm_res_lock);

	acl_desc = sde_vm_populate_acl(HH_TRUSTED_VM);
	if (IS_ERR(acl_desc)) {
		SDE_ERROR("failed to populate acl data, rc=%d\n",
			   PTR_ERR(acl_desc));
		goto acl_fail;
	}

	sgl_desc = hh_rm_mem_accept(payload->mem_handle, HH_RM_MEM_TYPE_IO,
				    HH_RM_TRANS_TYPE_LEND,
				    HH_RM_MEM_ACCEPT_VALIDATE_ACL_ATTRS|
				    HH_RM_MEM_ACCEPT_VALIDATE_LABEL|
				    HH_RM_MEM_ACCEPT_DONE,
				    payload->label,
				    acl_desc, NULL, NULL, 0);
	if (IS_ERR_OR_NULL(sgl_desc)) {
		SDE_ERROR("hh_rm_mem_accept failed with error, rc=%d\n",
			   PTR_ERR(sgl_desc));
		goto accept_fail;
	}

	rc = _sde_vm_validate_sgl(sde_vm->sgl_desc, sgl_desc);
	if (rc) {
		SDE_ERROR("failed in sgl validation for label: %d, rc = %d\n",
				payload->label, rc);
		goto accept_fail;
	}

	sde_vm->base.io_mem_handle = payload->mem_handle;

	SDE_INFO("mem accept succeeded for tag: %d label: %d\n", tag,
				payload->label);

accept_fail:
	kfree(acl_desc);
acl_fail:
	mutex_unlock(&sde_vm->base.vm_res_lock);
}

static int _sde_vm_release_irq(struct sde_vm *vm)
{
	struct sde_vm_trusted *sde_vm = (struct sde_vm_trusted *)vm;
	struct sde_vm_irq_desc *irq_desc = sde_vm->irq_desc;
	int i, rc = 0;

	for (i = 0; i < irq_desc->n_irq; i++) {
		struct sde_vm_irq_entry *entry = &irq_desc->irq_entries[i];

		rc = hh_irq_release(entry->label);
		if (rc) {
			SDE_ERROR("failed to release IRQ label: %d rc = %d\n",
				  entry->label, rc);
			return rc;
		}

		atomic_dec(&sde_vm->base.n_irq_lent);
	}

	SDE_INFO("sde vm irq release succeeded, rc = %d\n", rc);

	return rc;
}

static int _sde_vm_release(struct sde_kms *kms)
{
	struct sde_vm_trusted *sde_vm;
	int rc = 0;

	if (!kms->vm)
		return 0;

	sde_vm = to_vm_trusted(kms->vm);

	mutex_lock(&sde_vm->base.vm_res_lock);

	rc = hh_rm_mem_release(sde_vm->base.io_mem_handle, 0);
	if (rc) {
		SDE_ERROR("hh_rm_mem_release failed, rc=%d\n", rc);
		goto end;
	}

	rc = hh_rm_mem_notify(sde_vm->base.io_mem_handle,
			HH_RM_MEM_NOTIFY_OWNER,	HH_MEM_NOTIFIER_TAG_DISPLAY, 0);
	if (rc) {
		SDE_ERROR("hyp mem notify on release failed, rc = %d\n", rc);
		goto end;
	}

	sde_vm->base.io_mem_handle = -1;

	SDE_INFO("sde vm mem release succeeded, rc = %d\n", rc);

	rc = _sde_vm_release_irq(kms->vm);
	if (rc) {
		SDE_ERROR("irq_release failed, rc = %d\n", rc);
		goto end;
	}

end:
	mutex_unlock(&sde_vm->base.vm_res_lock);

	return rc;
}

int _sde_vm_populate_res(struct sde_kms *sde_kms, struct sde_vm_trusted *vm)
{
	struct msm_io_res io_res;
	int rc = 0;

	INIT_LIST_HEAD(&io_res.mem);
	INIT_LIST_HEAD(&io_res.irq);

	rc = sde_vm_get_resources(sde_kms, &io_res);
	if (rc) {
		SDE_ERROR("fail to get resources\n");
		return rc;
	}

	vm->sgl_desc = sde_vm_populate_sgl(&io_res);
	if (IS_ERR_OR_NULL(vm->sgl_desc)) {
		SDE_ERROR("failed to parse sgl list\n");
		return PTR_ERR(vm->sgl_desc);
	}

	vm->irq_desc = sde_vm_populate_irq(&io_res);
	if (IS_ERR_OR_NULL(vm->irq_desc)) {
		SDE_ERROR("failed to parse irq list\n");
		return PTR_ERR(vm->irq_desc);
	}

	sort(vm->irq_desc->irq_entries, vm->irq_desc->n_irq,
		sizeof(vm->irq_desc->irq_entries[0]), __irq_cmp, NULL);
	sort(vm->sgl_desc->sgl_entries, vm->sgl_desc->n_sgl_entries,
		sizeof(vm->sgl_desc->sgl_entries[0]), __sgl_cmp, NULL);

	return rc;
}

static bool sde_vm_owns_hw(struct sde_kms *sde_kms)
{
	struct sde_vm_trusted *sde_vm;
	bool owns_irq, owns_mem_io;

	sde_vm = to_vm_trusted(sde_kms->vm);

	owns_irq = (sde_vm->irq_desc->n_irq ==
			atomic_read(&sde_vm->base.n_irq_lent));
	owns_mem_io = (sde_vm->base.io_mem_handle >= 0);

	return (owns_irq && owns_mem_io);
}

static void  _sde_vm_deinit(struct sde_kms *kms, struct sde_vm_ops *ops)
{
	struct sde_vm_trusted *sde_vm;

	if (!kms->vm)
		return;

	sde_vm = to_vm_trusted(kms->vm);

	memset(ops, 0, sizeof(*ops));

	if (sde_vm->base.mem_notification_cookie)
		hh_mem_notifier_unregister(
				sde_vm->base.mem_notification_cookie);

	kfree(sde_vm->sgl_desc);

	if (sde_vm->irq_desc)
		sde_vm_free_irq(sde_vm->irq_desc);

	kfree(sde_vm);
}

static void _sde_vm_set_ops(struct sde_vm_ops *ops)
{
	memset(ops, 0, sizeof(*ops));

	ops->vm_release = _sde_vm_release;
	ops->vm_owns_hw = sde_vm_owns_hw;
	ops->vm_deinit = _sde_vm_deinit;
}

int sde_vm_trusted_init(struct sde_kms *kms, struct sde_vm_ops *ops)
{
	struct sde_vm_trusted *sde_vm;
	void *cookie;
	int rc = 0;

	sde_vm = kzalloc(sizeof(*sde_vm), GFP_KERNEL);
	if (!sde_vm)
		return -ENOMEM;

	_sde_vm_set_ops(ops);

	sde_vm->base.sde_kms = kms;

	mutex_init(&sde_vm->base.vm_res_lock);

	/**
	 * Optimize resource population by reading the entire HW resource
	 * space once during init. Once trusted vm starts supporting
	 * per-display space assignment, this read has to be done on each event
	 * notification.
	 */
	rc = _sde_vm_populate_res(kms, sde_vm);
	if (rc) {
		SDE_ERROR("failed to populate trusted vm res, rc= %d\n", rc);
		goto init_fail;
	}

	cookie = hh_mem_notifier_register(HH_MEM_NOTIFIER_TAG_DISPLAY,
			       sde_vm_mem_lend_notification_handler, sde_vm);
	if (!cookie) {
		SDE_ERROR("fails to register RM mem lend notifier\n");
		goto init_fail;
	}
	sde_vm->base.mem_notification_cookie = cookie;

	rc = hh_irq_wait_for_lend(HH_IRQ_LABEL_SDE, HH_PRIMARY_VM,
				  sde_vm_irq_lend_notification_handler,
				  (void *)sde_vm);
	if (rc) {
		SDE_ERROR("wait for irq lend on label: %d failed, rc=%d\n",
			   HH_IRQ_LABEL_SDE, rc);
		goto init_fail;
	}

	kms->vm = &sde_vm->base;

	atomic_set(&sde_vm->base.n_irq_lent, 0);

	return 0;
init_fail:
	_sde_vm_deinit(kms, ops);

	return rc;
}
