/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef __SDE_VM_COMMON_H__
#define __SDE_VM_COMMON_H__

#include <linux/haven/hh_rm_drv.h>
#include "sde_vm.h"

#define SDE_VM_MEM_LABEL 0x11

/**
 * sde_vm_populate_vmid - create and populate the rm vmid desc structure with
 *			  the given vmid
 * @vmid: vmid of the destination vm
 * @return: populated hh_notify_vmid_desc structure
 */
struct hh_notify_vmid_desc *sde_vm_populate_vmid(hh_vmid_t vmid);

/**
 * sde_vm_populate_acl - create and populate the access control list structure
 *			 for the given vm name
 * @vm_name: vm name enum published by the RM driver
 * @return: populated hh_acl_desc structure
 */
struct hh_acl_desc *sde_vm_populate_acl(enum hh_vm_names vm_name);

/**
 * sde_vm_populate_sgl - create and populate the scatter/gather list structure
 *			 with the given io memory list
 * @io_res: io resource list containing the io memory
 * @return: populated hh_sgl_desc structure
 */
struct hh_sgl_desc *sde_vm_populate_sgl(struct msm_io_res *io_res);

/**
 * sde_vm_populate_irq - create and populate the hw irq descriptor structure
 *			 with the given hw irq lines
 * @io_res: io resource list containing the irq numbers
 * @return: populated sde_vm_irq_desc structure
 */
struct sde_vm_irq_desc *sde_vm_populate_irq(struct msm_io_res *io_res);

/**
 * sde_vm_free_irq - free up the irq description structure
 * @irq_desc: handle to irq descriptor
 */
void sde_vm_free_irq(struct sde_vm_irq_desc *irq_desc);

/**
 * sde_vm_get_resources - collect io resource from all the VM clients
 * @sde_kms: handle to sde_kms
 * @io_res: pointer to msm_io_res structure to populate the resources
 * @return: 0 on success.
 */
int sde_vm_get_resources(struct sde_kms *sde_kms, struct msm_io_res *io_res);

/**
 * sde_vm_free_resources - free up the io resource list
 * @io_res: pointer to msm_io_res structure
 */
void sde_vm_free_resources(struct msm_io_res *io_res);

#endif /* __SDE_VM_COMMON_H__ */
