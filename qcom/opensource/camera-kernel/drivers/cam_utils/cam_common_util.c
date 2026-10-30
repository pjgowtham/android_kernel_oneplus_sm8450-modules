// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2025, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/string.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/completion.h>
#include <linux/module.h>
#include <linux/iopoll.h>
#include <linux/moduleparam.h>
#ifdef OPLUS_FEATURE_CAMERA_COMMON
#include <linux/atomic.h>
#include <linux/spinlock.h>
#endif
#include "cam_common_util.h"
#include "cam_debug_util.h"
#include "cam_presil_hw_access.h"
#include "cam_hw.h"
#include "cam_mem_mgr_api.h"
#if IS_REACHABLE(CONFIG_QCOM_VA_MINIDUMP)
#include <soc/qcom/minidump.h>
static struct cam_common_mini_dump_dev_info g_minidump_dev_info;
#endif

#define CAM_PRESIL_POLL_DELAY 20

#ifdef OPLUS_FEATURE_CAMERA_COMMON
static const uint32_t block_sizes[MEMORY_POOL_COUNT] = {
	MEMORY_BLOCK_1,
	MEMORY_BLOCK_2
};

static const uint32_t block_counts[MEMORY_POOL_COUNT] = {
	MEMORY_BLOCK_1_COUNT,
	MEMORY_BLOCK_2_COUNT
};

static memory_block_t *mem_pools[MEMORY_POOL_COUNT];
/* Sensor power management structures */
static atomic_t sensor_power_count = ATOMIC_INIT(0);
static struct mutex sensor_power_mutex;
static memory_pool_state_t memory_pool_state = MEMORY_POOL_STATE_DESTROYED;

/* Thread management */
static struct task_struct *memory_pool_thread = NULL;
static atomic_t thread_task_count = ATOMIC_INIT(0);
static wait_queue_head_t thread_wait_queue;


/* Delay mechanism for sensor power down */
static struct timer_list power_down_timer;
static bool power_down_timer_pending = false;

#endif
static struct cam_common_inject_evt_info g_inject_evt_info;
static uint timeout_multiplier = 1;
module_param(timeout_multiplier, uint, 0644);
typedef int (*cam_common_evt_inject_cmd_parse_handler)(
	struct cam_common_inject_evt_param *inject_params,
	uint32_t param_counter, char *token);

#ifdef OPLUS_FEATURE_CAMERA_COMMON
static void power_down_timer_callback(struct timer_list *t);
static int memory_pool_thread_func(void *data);
static void createOrDestroyMemPools(void);

bool common_mem_pools_init(void) {
	int i, j;

	/* Allocate memory pool arrays (structures only, no blocks yet) */
	for (i = 0; i < MEMORY_POOL_COUNT; i++) {
		mem_pools[i] = kmalloc_array(block_counts[i], sizeof(memory_block_t), GFP_KERNEL);
		if (!mem_pools[i]) {
			CAM_ERR(CAM_UTIL, "common Memory pool, Failed to allocate memory pool array %d", i);
			/* Cleanup already allocated arrays */
			for (int cleanup_i = 0; cleanup_i < i; cleanup_i++) {
				kfree(mem_pools[cleanup_i]);
				mem_pools[cleanup_i] = NULL;
			}
			return false;
		}

		/* Initialize memory block structures */
		for (j = 0; j < block_counts[i]; j++) {
			mem_pools[i][j].address = NULL;  /* No blocks allocated yet */
			mem_pools[i][j].size = block_sizes[i];
			atomic_set(&mem_pools[i][j].allocated, 0);  /* false = 0 */
			CAM_DBG(CAM_UTIL, "common Memory pool, Initialized block %d in pool %d: size=%zu, address=%p",
					j, i, mem_pools[i][j].size, mem_pools[i][j].address);
		}
	}

	memory_pool_state = MEMORY_POOL_STATE_DESTROYED;
	power_down_timer_pending = false;

	/* Initialize mutex for sensor power management */
	mutex_init(&sensor_power_mutex);

	/* Initialize wait queue for thread */
	init_waitqueue_head(&thread_wait_queue);

	/* Create memory pool management thread */
	memory_pool_thread = kthread_create(memory_pool_thread_func, NULL, "memory_pool_mgr");
	if (IS_ERR(memory_pool_thread)) {
		CAM_ERR(CAM_UTIL, "common Memory pool, Failed to create memory pool management thread");
		return -ENOMEM;
	}
	wake_up_process(memory_pool_thread);

	/* Initialize timer for delayed memory pool destruction */
	timer_setup(&power_down_timer, power_down_timer_callback, 0);

	CAM_INFO(CAM_UTIL, "common Memory pool, management system initialized successfully");
	return true;
}
EXPORT_SYMBOL(common_mem_pools_init);

int get_pool_index(size_t size) {
	for (int i = 0; i < MEMORY_POOL_COUNT; i++) {
		if (size <= block_sizes[i]) {
			return i;
		}
	}
	return -1;
}

void* pools_allocate_memory(size_t size) {
	/* Only allow allocation when memory pools are in CREATED or DESTROY_DELAY state */
	if (memory_pool_state & MEMORY_POOL_CANNOT_ALLOC_MASK) {
		CAM_DBG(CAM_UTIL, "common Memory pool, allocate_memory: Memory pools not in CREATED or DESTROY_DELAY state (current state: 0x%02x)", memory_pool_state);
		return NULL;
	}

	int pool_index = get_pool_index(size);
	if (pool_index < 0) {
		CAM_ERR(CAM_UTIL, "common Memory pool, allocate_memory: Requested size %zu exceeds maximum pool sizes.\n", size);
		return NULL;
	}

	/* Try to allocate using atomic compare and exchange */
	for (int j = 0; j < block_counts[pool_index]; j++) {
		if (atomic_cmpxchg(&mem_pools[pool_index][j].allocated, 0, 1) == 0) {
			if (NULL != mem_pools[pool_index][j].address) {
				return mem_pools[pool_index][j].address;
			} else {
				/* Reset allocation flag since address is NULL */
				atomic_set(&mem_pools[pool_index][j].allocated, 0);
				CAM_ERR(CAM_UTIL, "common Memory pool, Block %d in pool %d has NULL address", j, pool_index);
			}
		}
	}

	CAM_INFO(CAM_UTIL, "common Memory pool, can't alloc pool memory, no free block found.");
	return NULL;
}

bool pools_free_memory(void *ptr) {
	/* Memory block can be freed regardless of memory pool state
	 * because allocated blocks should always be returnable to avoid memory leaks */
	for (int i = 0; i < MEMORY_POOL_COUNT; i++) {
		for (int j = 0; j < block_counts[i]; j++) {
			if (mem_pools[i][j].address == ptr) {
				/* Use atomic compare and exchange to ensure thread safety */
				if (atomic_cmpxchg(&mem_pools[i][j].allocated, 1, 0) == 1) {
					/* Block returned to pool, keep address for future allocation */
					return true;
				} else {
					/* Block was not allocated or already freed by another thread */
					CAM_WARN(CAM_UTIL, "common Memory pool, fail to free unallocated block at %pK", ptr);
					return false;
				}
			}
		}
	}

	/* Pointer not found in any pool */
	return false;
}

/* Memory pool management thread function */
static int memory_pool_thread_func(void *data) {
	while (!kthread_should_stop()) {
		wait_event_interruptible(thread_wait_queue,
			atomic_read(&thread_task_count) > 0 || kthread_should_stop());

		if (kthread_should_stop())
			break;

		/* Process one task */
		if (atomic_dec_return(&thread_task_count) >= 0) {
			createOrDestroyMemPools();
		}
	}
	return 0;
}

/* Unified memory pool creation and destruction function */
static void createOrDestroyMemPools(void) {
	mutex_lock(&sensor_power_mutex);

	CAM_DBG(CAM_UTIL, "common Memory pool, createOrDestroyMemPools: sensor_count=%d, current_state=%d",
			atomic_read(&sensor_power_count), memory_pool_state);

	if (atomic_read(&sensor_power_count) > 0) {
		/* Cancel pending power down timer if sensors are powered up */
		if (power_down_timer_pending) {
			del_timer(&power_down_timer);
			power_down_timer_pending = false;
			CAM_INFO(CAM_UTIL, "common Memory pool, Cancelled pending power down timer due to sensor power up, count: %d", atomic_read(&sensor_power_count));
			if (memory_pool_state == MEMORY_POOL_STATE_DESTROY_DELAY) {
				memory_pool_state = MEMORY_POOL_STATE_CREATED;
				CAM_INFO(CAM_UTIL, "common Memory pool, Memory pools state rechanged to CREATED");
			} else {
				CAM_ERR(CAM_UTIL, "common Memory pool, state error:0x%02x", memory_pool_state);
			}
		}
		/* Need to create memory pools */
		if (memory_pool_state == MEMORY_POOL_STATE_DESTROYED) {
			memory_pool_state = MEMORY_POOL_STATE_CREATING;
			CAM_INFO(CAM_UTIL, "common Memory pool, Creating memory pools for sensor_count=%d", atomic_read(&sensor_power_count));

			/* Create memory pools */
			/* Allocate memory blocks (structures already allocated in init) */
			int total_new_allocated = 0;
			int total_existing = 0;

			for (int i = 0; i < MEMORY_POOL_COUNT; i++) {
				if (!mem_pools[i]) {
					CAM_ERR(CAM_UTIL, "common Memory pool, Memory pool array %d not initialized", i);
					memory_pool_state = MEMORY_POOL_STATE_DESTROYED;
					goto unlock_exit;
				}

				int pool_new_allocated = 0;
				int pool_existing = 0;

				for (int j = 0; j < block_counts[i]; j++) {
					if (mem_pools[i][j].address) {
						/* Block already allocated, skip */
						pool_existing++;
						continue;
					}

					/* Save block size to local variable to avoid compiler optimization issues */
					const size_t current_block_size = block_sizes[i];

					/* Try to allocate memory with retry mechanism */
					int retry_count = 0;
					do {
						mem_pools[i][j].address = kmalloc(current_block_size, GFP_KERNEL);
						if (mem_pools[i][j].address) {
							break; /* Success, exit retry loop */
						}
						retry_count++;
						CAM_WARN(CAM_UTIL, "common Memory pool, Memory allocation failed, retry %d/%d for block %d in pool %d (size: %zu), free_mem: %luKB", 
							retry_count, MEMORY_ALLOC_RETRY_COUNT, j, i, current_block_size,
							si_mem_available());
						if (retry_count < MEMORY_ALLOC_RETRY_COUNT) {
							/* Delay before retry to allow memory cleanup */
							msleep(MEMORY_ALLOC_RETRY_DELAY_MS);
						}
					} while (retry_count < MEMORY_ALLOC_RETRY_COUNT);

					if (!mem_pools[i][j].address) {
						CAM_ERR(CAM_UTIL, "common Memory pool, Failed to allocate memory block %d in pool %d (size: %zu) after %d retries, keeping existing blocks for next allocation", 
								j, i, current_block_size, MEMORY_ALLOC_RETRY_COUNT);
						/* Keep existing memory blocks for next allocation attempt */
						memory_pool_state = MEMORY_POOL_STATE_DESTROYED;
						goto unlock_exit;
					}
					pool_new_allocated++;
				}

				total_new_allocated += pool_new_allocated;
				total_existing += pool_existing;

				CAM_DBG(CAM_UTIL, "common Memory pool, Initialized memory pool %d: %zu bytes x %d blocks (new: %d, existing: %d)",
						i, block_sizes[i], block_counts[i], pool_new_allocated, pool_existing);
			}

			memory_pool_state = MEMORY_POOL_STATE_CREATED;
			CAM_INFO(CAM_UTIL, "common Memory pool, All memory pools created successfully - new allocated: %d blocks, existing: %d blocks, state changed to CREATED", 
					total_new_allocated, total_existing);
		} else {
			CAM_DBG(CAM_UTIL, "common Memory pool, Memory pools already created, no action needed, state: 0x%02x", memory_pool_state);
		}
	} else {
		/* Need to destroy memory pools */
		if (memory_pool_state == MEMORY_POOL_STATE_CREATED) {
			/* Set to destroy delay state and start timer */
			memory_pool_state = MEMORY_POOL_STATE_DESTROY_DELAY;
			CAM_INFO(CAM_UTIL, "common Memory pool, Setting to DESTROY_DELAY state, will destroy after delay %ds", MEMORY_POOL_DESTROY_DELAY_SEC);

			/* Schedule actual destruction after delay */
			power_down_timer_pending = true;
			mod_timer(&power_down_timer, jiffies + msecs_to_jiffies(MEMORY_POOL_DESTROY_DELAY_SEC * 1000));
		} else if (memory_pool_state == MEMORY_POOL_STATE_DESTROY_DELAY && !power_down_timer_pending) {
			/* Timer expired, set to warning state and sleep 200ms */
			memory_pool_state = MEMORY_POOL_STATE_DESTROY_WARNING;
			CAM_INFO(CAM_UTIL, "common Memory pool, Setting to DESTROY_WARNING state, sleeping %dms with lock held", MEMORY_POOL_DESTROY_WARNING_DELAY_MS);

			/* Sleep 200ms in thread context while holding the lock
			 * This prevents new memory allocations during the warning period */
			msleep(MEMORY_POOL_DESTROY_WARNING_DELAY_MS);

			/* After sleep, proceed to actual destruction */
			memory_pool_state = MEMORY_POOL_STATE_DESTROYING;
			CAM_INFO(CAM_UTIL, "common Memory pool, Destroying memory pools for sensor_count=%d", atomic_read(&sensor_power_count));

			/* Free each block individually using atomic operations */
			int total_allocated = 0;
			int total_freed = 0;

			for (int i = 0; i < MEMORY_POOL_COUNT; i++) {
				if (mem_pools[i]) {
					for (int j = 0; j < block_counts[i]; j++) {
						if (atomic_read(&mem_pools[i][j].allocated) == 1) {
							/* Skip allocated blocks and log */
							total_allocated++;
							CAM_INFO(CAM_UTIL, "common Memory pool, Skipping allocated block %d in pool %d (size: %zu), address: %pK",
									j, i, block_sizes[i], mem_pools[i][j].address);
						} else if (mem_pools[i][j].address) {
							/* Free unused block */
							kfree(mem_pools[i][j].address);
							mem_pools[i][j].address = NULL;
							total_freed++;
						}
					}
				}
			}

			if (total_allocated > 0) {
				/* Keep pool structures if there are allocated blocks */
				CAM_INFO(CAM_UTIL, "common Memory pool, Partial destruction: freed %d blocks, kept %d allocated blocks",
						total_freed, total_allocated);
			} else {
				/* All blocks were free, mark as uninitialized but keep pool structures */
				CAM_INFO(CAM_UTIL, "common Memory pool, Complete destruction: freed all %d blocks, pool structures kept", total_freed);
			}

			memory_pool_state = MEMORY_POOL_STATE_DESTROYED;
			CAM_INFO(CAM_UTIL, "common Memory pool, Memory pools state changed to DESTROYED");

		} else {
			CAM_DBG(CAM_UTIL, "common Memory pool, no need destroy, state:0x%02x", memory_pool_state);
		}
	}

unlock_exit:
	mutex_unlock(&sensor_power_mutex);
}


/* Sensor power management functions */
void mempool_set_sensor_powerup(void) {
	int count;

	/* Increment sensor power count atomically */
	count = atomic_inc_return(&sensor_power_count);
	CAM_DBG(CAM_UTIL, "common Memory pool, Sensor power up, count: %d", count);

	/* Increment thread task count and wake up thread */
	atomic_inc(&thread_task_count);
	wake_up(&thread_wait_queue);
}
EXPORT_SYMBOL(mempool_set_sensor_powerup);

/* Timer callback for delayed memory pool destruction */
static void power_down_timer_callback(struct timer_list *t) {
	CAM_INFO(CAM_UTIL, "common Memory pool, power_down_timer_callback");
	/* Increment thread task count and wake up thread */
	atomic_inc(&thread_task_count);
	/* Reset timer pending flag */
	power_down_timer_pending = false;
	wake_up(&thread_wait_queue);
}

void mempool_set_sensor_powerdown(void) {
	int count;

	/* Decrement sensor power count atomically */
	count = atomic_dec_return(&sensor_power_count);
	CAM_DBG(CAM_UTIL, "common Memory pool, Sensor power down, count: %d", count);

	/* Increment thread task count and wake up thread */
	atomic_inc(&thread_task_count);
	wake_up(&thread_wait_queue);
}
EXPORT_SYMBOL(mempool_set_sensor_powerdown);
#endif

int cam_common_util_get_string_index(const char **strings,
	uint32_t num_strings, const char *matching_string, uint32_t *index)
{
	int i;

	for (i = 0; i < num_strings; i++) {
		if (strnstr(strings[i], matching_string, strlen(strings[i]))) {
			CAM_DBG(CAM_UTIL, "matched %s : %d\n",
				matching_string, i);
			*index = i;
			return 0;
		}
	}

	return -EINVAL;
}

uint32_t cam_common_util_remove_duplicate_arr(int32_t *arr, uint32_t num)
{
	int i, j;
	uint32_t wr_idx = 1;

	if (!arr) {
		CAM_ERR(CAM_UTIL, "Null input array");
		return 0;
	}

	for (i = 1; i < num; i++) {
		for (j = 0; j < wr_idx ; j++) {
			if (arr[i] == arr[j])
				break;
		}
		if (j == wr_idx)
			arr[wr_idx++] = arr[i];
	}

	return wr_idx;
}

unsigned long cam_common_wait_for_completion_timeout(
	struct completion   *complete,
	unsigned long        timeout_jiffies)
{
	unsigned long wait_jiffies;
	unsigned long rem_jiffies;

	if (!complete) {
		CAM_ERR(CAM_UTIL, "Null complete pointer");
		return 0;
	}

	if (timeout_multiplier < 1)
		timeout_multiplier = 1;

	wait_jiffies = timeout_jiffies * timeout_multiplier;
	rem_jiffies = wait_for_completion_timeout(complete, wait_jiffies);

	return rem_jiffies;
}

int cam_common_read_poll_timeout(
	void __iomem        *addr,
	unsigned long        delay,
	unsigned long        timeout,
	uint32_t             mask,
	uint32_t             check_val,
	uint32_t            *status)
{
	unsigned long wait_time_us;
	int rc = -EINVAL;

	if (!addr || !status) {
		CAM_ERR(CAM_UTIL, "Invalid param addr: %pK status: %pK",
			addr, status);
		return rc;
	}

	if (timeout_multiplier < 1)
		timeout_multiplier = 1;

	wait_time_us = timeout * timeout_multiplier;

	if (false == cam_presil_mode_enabled()) {
		rc = readl_poll_timeout(addr, *status, (*status & mask) == check_val, delay,
			wait_time_us);
	} else {
		rc = cam_presil_readl_poll_timeout(addr, mask,
			wait_time_us/(CAM_PRESIL_POLL_DELAY * 600), CAM_PRESIL_POLL_DELAY);
	}

	return rc;
}

int cam_common_modify_timer(struct timer_list *timer, int32_t timeout_val)
{
	if (!timer) {
		CAM_ERR(CAM_UTIL, "Invalid reference to system timer");
		return -EINVAL;
	}

	if (timeout_multiplier < 1)
		timeout_multiplier = 1;

	CAM_DBG(CAM_UTIL, "Starting timer to fire in %d ms. (jiffies=%lu)\n",
		(timeout_val * timeout_multiplier), jiffies);
	mod_timer(timer,
		(jiffies + msecs_to_jiffies(timeout_val * timeout_multiplier)));

	return 0;
}

void cam_common_util_thread_switch_delay_detect(char *wq_name, const char *state,
	void *cb, ktime_t scheduled_time, uint32_t threshold)
{
	uint64_t                         diff;
	ktime_t                          cur_time;
	struct timespec64                cur_ts;
	struct timespec64                scheduled_ts;

	cur_time = ktime_get();
	diff = ktime_ms_delta(cur_time, scheduled_time);

	if (diff > threshold) {
		scheduled_ts  = ktime_to_timespec64(scheduled_time);
		cur_ts = ktime_to_timespec64(cur_time);
		CAM_WARN_RATE_LIMIT_CUSTOM(CAM_UTIL, 5, 1,
			"%s cb: %ps delay in %s detected %ld:%06ld cur %ld:%06ld\n"
			"diff %ld: threshold %d",
			wq_name, cb, state, scheduled_ts.tv_sec,
			scheduled_ts.tv_nsec/NSEC_PER_USEC,
			cur_ts.tv_sec, cur_ts.tv_nsec/NSEC_PER_USEC,
			diff, threshold);
	}
}

#if IS_REACHABLE(CONFIG_QCOM_VA_MINIDUMP)
static void cam_common_mini_dump_handler(void *dst, unsigned long len)
{
	int                               i = 0;
	uint8_t                          *waddr;
	unsigned long                     bytes_written = 0;
	unsigned long                     remain_len = len;
	struct cam_common_mini_dump_data *md;

	if (len < sizeof(*md)) {
	    CAM_WARN(CAM_UTIL, "Insufficient len %lu", len);
	    return;
	}

	md = (struct cam_common_mini_dump_data *)dst;
	waddr = (uint8_t *)md + sizeof(*md);
	remain_len -= sizeof(*md);

	for (i = 0; i < CAM_COMMON_MINI_DUMP_DEV_NUM; i++) {
		if (!g_minidump_dev_info.dump_cb[i])
			continue;

		memcpy(md->name[i], g_minidump_dev_info.name[i],
			strlen(g_minidump_dev_info.name[i]));
		md->waddr[i] = (void *)waddr;
		bytes_written = g_minidump_dev_info.dump_cb[i](
			(void *)waddr, remain_len, g_minidump_dev_info.priv_data[i]);
		md->size[i] = bytes_written;
		if (bytes_written >= len) {
			CAM_WARN(CAM_UTIL, "No more space to dump");
			goto nomem;
		}

		remain_len -= bytes_written;
		waddr += bytes_written;
	}

	return;
nomem:
    for (; i >=0; i--)
	    CAM_WARN(CAM_UTIL, "%s: Dumped len: %lu", md->name[i], md->size[i]);
}

static int cam_common_md_notify_handler(struct notifier_block *this,
	unsigned long event, void *ptr)
{
	struct va_md_entry cbentry;
	int rc = 0;

	cbentry.vaddr = 0x0;
	strscpy(cbentry.owner, "Camera", sizeof(cbentry.owner));
	cbentry.size = CAM_COMMON_MINI_DUMP_SIZE;
	cbentry.cb = cam_common_mini_dump_handler;
	rc = qcom_va_md_add_region(&cbentry);
	if (rc) {
		CAM_ERR(CAM_UTIL, "Va Region add falied %d", rc);
		return NOTIFY_STOP_MASK;
	}

	return NOTIFY_OK;
}

static struct notifier_block cam_common_md_notify_blk = {
	.notifier_call = cam_common_md_notify_handler,
	.priority = INT_MAX,
};

int cam_common_register_mini_dump_cb(
	cam_common_mini_dump_cb mini_dump_cb,
	uint8_t *dev_name, void *priv_data)
{
	int rc = 0;
	uint32_t idx;

	if (g_minidump_dev_info.num_devs >= CAM_COMMON_MINI_DUMP_DEV_NUM) {
		CAM_ERR(CAM_UTIL, "No free index available");
		return -EINVAL;
	}

	if (!mini_dump_cb || !dev_name) {
		CAM_ERR(CAM_UTIL, "Invalid params");
		return -EINVAL;
	}

	idx = g_minidump_dev_info.num_devs;
	g_minidump_dev_info.dump_cb[idx] =
		mini_dump_cb;
	scnprintf(g_minidump_dev_info.name[idx],
		CAM_COMMON_MINI_DUMP_DEV_NAME_LEN, dev_name);
	g_minidump_dev_info.priv_data[idx] = priv_data;
	g_minidump_dev_info.num_devs++;
	if (!g_minidump_dev_info.is_registered) {
		rc = qcom_va_md_register("Camera", &cam_common_md_notify_blk);
		if (rc) {
			CAM_ERR(CAM_UTIL, "Camera VA minidump register failed");
			goto end;
		}
		g_minidump_dev_info.is_registered = true;
	}
end:
	return rc;
}
#endif

void *cam_common_user_dump_clock(
	void *dump_struct, uint8_t *addr_ptr)
{
	struct cam_hw_info  *hw_info = NULL;
	uint64_t            *addr = NULL;

	hw_info = (struct cam_hw_info *)dump_struct;

	if (!hw_info || !addr_ptr) {
		CAM_ERR(CAM_ISP, "HW info or address pointer NULL");
		return addr;
	}

	addr = (uint64_t *)addr_ptr;
	*addr++ = cam_soc_util_get_applied_src_clk(&hw_info->soc_info, true);

	return addr;
}

int cam_common_user_dump_helper(
	void *cmd_args,
	void *(*func)(void *dump_struct, uint8_t *addr_ptr),
	void *dump_struct,
	size_t size,
	const char *tag, ...)
{

	uint8_t                                   *dst;
	uint8_t                                   *addr, *start;
	void                                      *returned_ptr;
	struct cam_common_hw_dump_args            *dump_args;
	struct cam_common_hw_dump_header          *hdr;
	va_list                                    args;
	void*(*func_ptr)(void *dump_struct, uint8_t *addr_ptr);

	dump_args = (struct cam_common_hw_dump_args *)cmd_args;

	if (!dump_args) {
		CAM_ERR(CAM_UTIL, "dump_args is NULL!");
		return -EINVAL;
	}
	if (!dump_args->cpu_addr || !dump_args->buf_len) {
		CAM_ERR(CAM_UTIL,
			"Invalid params: cpu_addr=%pk, buf_len=%zu",
			(void *)dump_args->cpu_addr,
			dump_args->buf_len);
		return -EINVAL;
	}
	if (dump_args->buf_len <= dump_args->offset) {
		CAM_WARN(CAM_UTIL,
			"Dump offset overshoot offset %zu buf_len %zu",
			dump_args->offset, dump_args->buf_len);
		return -ENOSPC;
	}
	if (dump_args->offset + size + sizeof(struct cam_common_hw_dump_header)
		> dump_args->buf_len) {
		CAM_ERR(CAM_UTIL,
			"Insufficient buffer space: offset %zu, required %zu, buf_len %zu",
			dump_args->offset,
			size + sizeof(struct cam_common_hw_dump_header),
			dump_args->buf_len);
		return -EINVAL;
	}

	dst = (uint8_t *)dump_args->cpu_addr + dump_args->offset;
	hdr = (struct cam_common_hw_dump_header *)dst;

	va_start(args, tag);
	vscnprintf(hdr->tag, CAM_COMMON_HW_DUMP_TAG_MAX_LEN, tag, args);
	va_end(args);

	hdr->word_size = size;

	addr = (uint8_t *)(dst + sizeof(struct cam_common_hw_dump_header));
	start = addr;

	if (!func || !dump_struct) {
		CAM_ERR(CAM_UTIL, "function ptr / dump struct is NULL");
		return -EINVAL;
	}
	func_ptr = func;
	returned_ptr = func_ptr(dump_struct, addr);

	if (IS_ERR(returned_ptr) || !returned_ptr) {
		CAM_ERR(CAM_UTIL, "function call failed!");
		return PTR_ERR(returned_ptr);
	}

	addr = (uint8_t *)returned_ptr;
	hdr->size = addr - start;
	CAM_DBG(CAM_UTIL, "hdr size: %d, word size: %d, addr: %x, start: %x",
		hdr->size, hdr->word_size, addr, start);
	dump_args->offset += hdr->size +
		sizeof(struct cam_common_hw_dump_header);

	return 0;
}

int cam_common_register_evt_inject_cb(cam_common_evt_inject_cb evt_inject_cb,
	enum cam_common_evt_inject_hw_id hw_id)
{
	int rc = 0;

	if (g_inject_evt_info.num_hw_registered >= CAM_COMMON_EVT_INJECT_HW_MAX) {
		CAM_ERR(CAM_UTIL, "No free index available");
		return -EINVAL;
	}

	if (!evt_inject_cb || hw_id >= CAM_COMMON_EVT_INJECT_HW_MAX) {
		CAM_ERR(CAM_UTIL, "Invalid params evt_inject_cb %s hw_id: %d",
			CAM_IS_NULL_TO_STR(evt_inject_cb), hw_id);
		return -EINVAL;
	}

	g_inject_evt_info.evt_inject_cb[hw_id] = evt_inject_cb;
	g_inject_evt_info.num_hw_registered++;
	CAM_DBG(CAM_UTIL, "Evt inject cb registered for HW_id: %d, total registered: %d", hw_id,
		g_inject_evt_info.num_hw_registered);
	return rc;
}

void cam_common_release_evt_params(int32_t dev_hdl)
{
	struct list_head *pos = NULL, *pos_next = NULL;
	struct cam_common_inject_evt_param *inject_params;

	if (!g_inject_evt_info.is_list_initialised)
		return;

	if (list_empty(&g_inject_evt_info.active_evt_ctx_list)) {
		CAM_DBG(CAM_UTIL, "Event injection list is initialized but empty");
		return;
	}

	list_for_each_safe(pos, pos_next, &g_inject_evt_info.active_evt_ctx_list) {
		inject_params = list_entry(pos, struct cam_common_inject_evt_param, list);
		if (inject_params->dev_hdl == dev_hdl) {
			CAM_INFO(CAM_UTIL, "entry deleted for %d dev hdl", dev_hdl);
			list_del(pos);
			CAM_MEM_FREE(inject_params);
		}
	}
}

static inline int cam_common_evt_inject_get_hw_id(uint8_t *hw_id, char *token)
{
	if (strcmp(token, CAM_COMMON_IFE_NODE) == 0)
		*hw_id = CAM_COMMON_EVT_INJECT_HW_ISP;
	else if (strcmp(token, CAM_COMMON_ICP_NODE) == 0)
		*hw_id = CAM_COMMON_EVT_INJECT_HW_ICP;
	else if (strcmp(token, CAM_COMMON_JPEG_NODE) == 0)
		*hw_id = CAM_COMMON_EVT_INJECT_HW_JPEG;
	else {
		CAM_ERR(CAM_UTIL, "Invalid camera hardware [ %s ]", token);
		return -EINVAL;
	}

	return 0;
}

static inline int cam_common_evt_inject_get_str_id_type(uint8_t *id_type, char *token)
{
	if (!strcmp(token, CAM_COMMON_EVT_INJECT_BUFFER_ERROR))
		*id_type = CAM_COMMON_EVT_INJECT_BUFFER_ERROR_TYPE;
	else if (!strcmp(token, CAM_COMMON_EVT_INJECT_NOTIFY_EVENT))
		*id_type = CAM_COMMON_EVT_INJECT_NOTIFY_EVENT_TYPE;
	else {
		CAM_ERR(CAM_UTIL, "Invalid string id: %s", token);
		return -EINVAL;
	}

	return 0;
}

static int cam_common_evt_inject_parse_buffer_error_evt_params(
	struct cam_common_inject_evt_param *inject_params,
	uint32_t param_counter, char *token)
{
	struct cam_hw_inject_buffer_error_param *buf_err_params =
		&inject_params->evt_params.u.buf_err_evt;
	int rc = 0;

	switch (param_counter) {
	case SYNC_ERROR_CAUSE:
		if (kstrtou32(token, 0, &buf_err_params->sync_error)) {
			CAM_ERR(CAM_UTIL, "Invalid event type %s", token);
			rc = -EINVAL;
		}
		break;
	default:
		CAM_ERR(CAM_UTIL, "Invalid extra parameters: %s", token);
		rc = -EINVAL;
	}

	return rc;
}

static int cam_common_evt_inject_parse_node_evt_params(
	struct cam_common_inject_evt_param *inject_params,
	uint32_t param_counter, char *token)
{
	struct cam_hw_inject_node_evt_param *node_params =
		&inject_params->evt_params.u.evt_notify.u.node_evt_params;
	int rc = 0;

	switch (param_counter) {
	case EVENT_TYPE:
		if (kstrtou32(token, 0, &node_params->event_type)) {
			CAM_ERR(CAM_UTIL, "Invalid event type %s", token);
			rc = -EINVAL;
		}
		break;
	case EVENT_CAUSE:
		if (kstrtou32(token, 0, &node_params->event_cause)) {
			CAM_ERR(CAM_UTIL, "Invalid event cause %s", token);
			rc = -EINVAL;
		}
		break;
	default:
		CAM_ERR(CAM_UTIL, "Invalid extra parameters: %s", token);
		rc = -EINVAL;
	}

	return rc;
}

static int cam_common_evt_inject_parse_pf_params(
	struct cam_common_inject_evt_param *inject_params,
	uint32_t param_counter, char *token)
{
	struct cam_hw_inject_pf_evt_param *pf_params =
		&inject_params->evt_params.u.evt_notify.u.pf_evt_params;
	int rc = 0;

	switch (param_counter) {
	case PF_PARAM_CTX_FOUND:
		if (kstrtobool(token, &pf_params->ctx_found)) {
			CAM_ERR(CAM_UTIL, "Invalid context found value %s", token);
			rc = -EINVAL;
		}
		break;
	default:
		CAM_ERR(CAM_UTIL, "Invalid extra parameters %s", token);
		rc = -EINVAL;
	}

	return rc;
}

static int cam_common_evt_inject_parse_err_evt_params(
	struct cam_common_inject_evt_param *inject_params,
	uint32_t param_counter, char *token)
{
	struct cam_hw_inject_err_evt_param *err_params =
		&inject_params->evt_params.u.evt_notify.u.err_evt_params;
	int rc = 0;

	switch (param_counter) {
	case ERR_PARAM_ERR_TYPE:
		if (kstrtou32(token, 0, &err_params->err_type)) {
			CAM_ERR(CAM_UTIL, "Invalid error type %s", token);
			rc = -EINVAL;
		}
		break;
	case ERR_PARAM_ERR_CODE:
		if (kstrtou32(token, 0, &err_params->err_code)) {
			CAM_ERR(CAM_UTIL, "Invalid error code %s", token);
			rc = -EINVAL;
		}
		break;
	default:
		CAM_ERR(CAM_UTIL, "Invalid extra parameters: %s", token);
		rc = -EINVAL;
	}

	return rc;
}

static int cam_common_evt_inject_parse_event_notify(
	struct cam_common_inject_evt_param *inject_params,
	uint32_t param_counter, char *token)
{
	int rc = 0;

	switch (param_counter) {
	case EVT_NOTIFY_TYPE:
		if (kstrtou32(token, 0,
			&inject_params->evt_params.u.evt_notify.evt_notify_type)) {
			CAM_ERR(CAM_UTIL, "Invalid Event notify type %s", token);
			rc = -EINVAL;
		}
		break;
	default:
		CAM_ERR(CAM_UTIL, "Invalid extra parameters: %s", token);
		rc = -EINVAL;
	}

	return rc;
}

static int cam_common_evt_inject_parse_common_params(
	struct cam_common_inject_evt_param *inject_params,
	uint32_t param_counter, char *token)
{
	int rc = 0;
	struct cam_hw_inject_evt_param *evt_param = &inject_params->evt_params;

	switch (param_counter) {
	case STRING_ID:
		rc = cam_common_evt_inject_get_str_id_type(&evt_param->inject_id, token);
		break;
	case HW_NAME:
		rc = cam_common_evt_inject_get_hw_id(&inject_params->hw_id, token);
		break;
	case DEV_HDL:
		if (kstrtos32(token, 0, &inject_params->dev_hdl)) {
			CAM_ERR(CAM_UTIL, "Invalid device handle %s", token);
			rc = -EINVAL;
		}
		break;
	case REQ_ID:
		if (kstrtou64(token, 0, &evt_param->req_id)) {
			CAM_ERR(CAM_UTIL, "Invalid request id %s", token);
			rc = -EINVAL;
		}
		break;
	default:
		 CAM_ERR(CAM_UTIL, "Invalid extra parameter: %s", token);
		 rc = -EINVAL;
	}

	return rc;
}

static int cam_common_evt_inject_generic_command_parser(
	struct cam_common_inject_evt_param *inject_params,
	char **msg, uint32_t max_params, cam_common_evt_inject_cmd_parse_handler cmd_parse_cb)
{
	char *token = NULL;
	int rc = 0, param_counter = 0;

	token = strsep(msg, ":");
	while (token != NULL) {
		rc = cmd_parse_cb(inject_params, param_counter, token);
		if (rc) {
			CAM_ERR(CAM_UTIL, "Parsed Command failed rc: %d", rc);
			return rc;
		}

		param_counter++;
		if (param_counter == max_params)
			break;
		token = strsep(msg, ":");
	}

	if (param_counter < max_params) {
		CAM_ERR(CAM_UTIL,
			"Insufficient parameters passed for total parameters: %u",
			param_counter);
		return -EINVAL;
	}

	return rc;
}

static int cam_common_evt_inject_set(const char *kmessage,
	const struct kernel_param *kp)
{
	struct   cam_common_inject_evt_param *inject_params   = NULL;
	struct   cam_hw_inject_evt_param *hw_evt_params       = NULL;
	cam_common_evt_inject_cmd_parse_handler parse_handler = NULL;
	int      rc                                           = 0;
	char     tmp_buff[CAM_COMMON_EVT_INJECT_BUFFER_LEN];
	char    *msg                                          = NULL;
	uint32_t param_output                                 = 0;

	inject_params = CAM_MEM_ZALLOC(sizeof(struct cam_common_inject_evt_param), GFP_KERNEL);
	if (!inject_params) {
		CAM_ERR(CAM_UTIL, "no free memory");
		return -ENOMEM;
	}

	rc = strscpy(tmp_buff, kmessage, CAM_COMMON_EVT_INJECT_BUFFER_LEN);
	if (rc == -E2BIG)
		goto free;

	CAM_INFO(CAM_UTIL, "parsing input param for cam event injection: %s", tmp_buff);

	msg = tmp_buff;
	hw_evt_params = &inject_params->evt_params;

	rc = cam_common_evt_inject_generic_command_parser(inject_params, &msg,
		COMMON_PARAM_MAX, cam_common_evt_inject_parse_common_params);
	if (rc) {
		CAM_ERR(CAM_UTIL, "Fail to parse common params %d", rc);
		goto free;
	}

	switch (hw_evt_params->inject_id) {
	case CAM_COMMON_EVT_INJECT_NOTIFY_EVENT_TYPE:
		rc = cam_common_evt_inject_generic_command_parser(inject_params, &msg,
			EVT_NOTIFY_PARAM_MAX, cam_common_evt_inject_parse_event_notify);
		if (rc) {
			CAM_ERR(CAM_UTIL, "Fail to parse event notify type param %d", rc);
			goto free;
		}

		switch (hw_evt_params->u.evt_notify.evt_notify_type) {
		case V4L_EVENT_CAM_REQ_MGR_ERROR:
			parse_handler = cam_common_evt_inject_parse_err_evt_params;
			param_output = ERR_PARAM_MAX;
			break;
		case V4L_EVENT_CAM_REQ_MGR_NODE_EVENT:
			parse_handler = cam_common_evt_inject_parse_node_evt_params;
			param_output = NODE_PARAM_MAX;
			break;
		case V4L_EVENT_CAM_REQ_MGR_PF_ERROR:
			parse_handler = cam_common_evt_inject_parse_pf_params;
			param_output = PF_PARAM_MAX;
			break;
		default:
			CAM_ERR(CAM_UTIL, "Invalid event notification type: %u",
				hw_evt_params->u.evt_notify.evt_notify_type);
			goto free;
		}
		break;
	case CAM_COMMON_EVT_INJECT_BUFFER_ERROR_TYPE:
		parse_handler = cam_common_evt_inject_parse_buffer_error_evt_params;
		param_output = BUFFER_ERROR_PARAM_MAX;
		break;
	default:
		CAM_ERR(CAM_UTIL, "Invalid Injection id: %u", hw_evt_params->inject_id);
	}

	if (!parse_handler)
		goto free;

	rc = cam_common_evt_inject_generic_command_parser(inject_params, &msg,
		param_output, parse_handler);
	if (rc) {
		CAM_ERR(CAM_UTIL, "Command Parsed failed with Inject id: %u rc: %d",
			hw_evt_params->inject_id, rc);
		goto free;
	}

	if (g_inject_evt_info.evt_inject_cb[inject_params->hw_id]) {
		rc = g_inject_evt_info.evt_inject_cb[inject_params->hw_id](inject_params);
		if (rc)
			goto free;
	} else {
		CAM_ERR(CAM_UTIL, "Handler for HW_id [%hhu] not registered", inject_params->hw_id);
		goto free;
	}

	if (!g_inject_evt_info.is_list_initialised) {
		INIT_LIST_HEAD(&g_inject_evt_info.active_evt_ctx_list);
		g_inject_evt_info.is_list_initialised = true;
	}

	list_add(&inject_params->list, &g_inject_evt_info.active_evt_ctx_list);

	return rc;

free:
	CAM_MEM_FREE(inject_params);
	return rc;
}

static int cam_common_evt_inject_get(char *buffer,
	const struct kernel_param *kp)
{
	uint8_t hw_name[16], string_id[16];
	uint16_t buff_max_size = CAM_COMMON_EVT_INJECT_MODULE_PARAM_MAX_LENGTH;
	struct cam_common_inject_evt_param *inject_params = NULL;
	struct cam_hw_inject_evt_param *evt_params = NULL;
	uint32_t  ret = 0;

	if (!g_inject_evt_info.is_list_initialised)
		return scnprintf(buffer, buff_max_size, "uninitialised");

	if (list_empty(&g_inject_evt_info.active_evt_ctx_list))
		return scnprintf(buffer, buff_max_size, "Active err inject list is empty");

	list_for_each_entry(inject_params, &g_inject_evt_info.active_evt_ctx_list, list) {
		evt_params = &inject_params->evt_params;

		switch (inject_params->hw_id) {
		case CAM_COMMON_EVT_INJECT_HW_ISP:
			strscpy(hw_name, CAM_COMMON_IFE_NODE, sizeof(hw_name));
			break;
		case CAM_COMMON_EVT_INJECT_HW_ICP:
			strscpy(hw_name, CAM_COMMON_ICP_NODE, sizeof(hw_name));
			break;
		case CAM_COMMON_EVT_INJECT_HW_JPEG:
			strscpy(hw_name, CAM_COMMON_JPEG_NODE, sizeof(hw_name));
			break;
		default:
			ret += scnprintf(buffer+ret, buff_max_size, "Undefined HW id\n");
			goto undefined_param;
		}

		switch (evt_params->inject_id) {
		case CAM_COMMON_EVT_INJECT_BUFFER_ERROR_TYPE:
			strscpy(string_id, CAM_COMMON_EVT_INJECT_BUFFER_ERROR, sizeof(string_id));
			break;
		case CAM_COMMON_EVT_INJECT_NOTIFY_EVENT_TYPE:
			strscpy(string_id, CAM_COMMON_EVT_INJECT_NOTIFY_EVENT, sizeof(string_id));
			break;
		default:
			ret += scnprintf(buffer+ret, buff_max_size, "Undefined string id\n");
			goto undefined_param;
		}

		ret += scnprintf(buffer+ret, buff_max_size,
			"string_id: %s hw_name: %s dev_hdl: %d req_id: %llu ",
			string_id, hw_name,
			inject_params->dev_hdl, evt_params->req_id);

		if (buff_max_size > ret) {
			buff_max_size -= ret;
		} else {
			CAM_WARN(CAM_UTIL, "out buff max limit reached");
			break;
		}

		if (evt_params->inject_id ==
			CAM_COMMON_EVT_INJECT_BUFFER_ERROR_TYPE) {
			ret += scnprintf(buffer+ret, buff_max_size,
				"sync_error: %u\n", evt_params->u.buf_err_evt.sync_error);
		} else {
			switch (evt_params->u.evt_notify.evt_notify_type) {
			case V4L_EVENT_CAM_REQ_MGR_ERROR: {
				struct cam_hw_inject_err_evt_param *err_evt_params =
					&evt_params->u.evt_notify.u.err_evt_params;
				ret += scnprintf(buffer+ret, buff_max_size,
					"Error event: error type: %u error code: %u\n",
					err_evt_params->err_type, err_evt_params->err_code);
				break;
			}
			case V4L_EVENT_CAM_REQ_MGR_NODE_EVENT: {
				struct cam_hw_inject_node_evt_param *node_evt_params =
					&evt_params->u.evt_notify.u.node_evt_params;
				ret += scnprintf(buffer+ret, buff_max_size,
					"Node event: event type: %u event cause: %u\n",
					node_evt_params->event_type, node_evt_params->event_cause);
				break;
			}
			case V4L_EVENT_CAM_REQ_MGR_PF_ERROR: {
				struct cam_hw_inject_pf_evt_param *pf_evt_params =
					&evt_params->u.evt_notify.u.pf_evt_params;
				ret += scnprintf(buffer+ret, buff_max_size,
					"PF event: ctx found %hhu\n",
					pf_evt_params->ctx_found);
				break;
			}
			default:
				ret += scnprintf(buffer+ret, buff_max_size,
					"Undefined notification event\n");
			}
		}

undefined_param:
		CAM_DBG(CAM_UTIL, "output buffer: %s", buffer);

		if (buff_max_size > ret) {
			buff_max_size -= ret;
		} else {
			CAM_WARN(CAM_UTIL, "out buff max limit reached");
			break;
		}
	}

	return ret;
}

static const struct kernel_param_ops cam_common_evt_inject = {
	.set = cam_common_evt_inject_set,
	.get = cam_common_evt_inject_get
};

module_param_cb(cam_event_inject, &cam_common_evt_inject, NULL, 0644);

int cam_common_mem_kdup(void **dst,
	void *src, size_t size)
{
#ifndef OPLUS_FEATURE_CAMERA_COMMON
	gfp_t flag = GFP_KERNEL;
#endif

	if (!src || !dst || !size) {
		CAM_ERR(CAM_UTIL, "Invalid params src: %pK dst: %pK size: %u",
			src, dst, size);
		return -EINVAL;
	}

#ifdef OPLUS_FEATURE_CAMERA_COMMON
	if (in_task()) {
		*dst = pools_allocate_memory(size);
		if (!*dst) {
			*dst = vzalloc(size);
			CAM_DBG(CAM_UTIL, "common Memory pool, use vzalloc size: %u, *dst: %p", size, *dst);
		}
	} else {
		*dst = kvzalloc(size, GFP_ATOMIC);
		CAM_INFO(CAM_UTIL, "common Memory pool, use kvzalloc size: %u, *dst: %p", size, *dst);
	}
#else
	if (!in_task())
		flag = GFP_ATOMIC;

	*dst = kvzalloc(size, flag);
#endif

	if (!*dst) {
		CAM_ERR(CAM_UTIL, "Failed to allocate memory with size: %u", size);
		return -ENOMEM;
	}

	memcpy(*dst, src, size);
	CAM_DBG(CAM_UTIL, "Allocate and copy memory with size: %u", size);

	return 0;
}
EXPORT_SYMBOL(cam_common_mem_kdup);

void cam_common_mem_free(void *memory)
{
#ifdef OPLUS_FEATURE_CAMERA_COMMON
	if (!pools_free_memory(memory)) {
		kvfree(memory);
	}
#else
	kvfree(memory);
#endif
}
EXPORT_SYMBOL(cam_common_mem_free);
