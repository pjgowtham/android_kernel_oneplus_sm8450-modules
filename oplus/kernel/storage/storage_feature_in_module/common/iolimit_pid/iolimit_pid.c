// SPDX-License-Identifier: GPL-2.0-only
/*
 * limit task's buffer write by PID.
 *
 * Copyright 2024 OPLUS
 *
 * Licensed under the GPL-2.0
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/rculist.h>
#include <linux/rhashtable.h>
#include <linux/export.h>
#include <linux/profile.h>
#include <trace/hooks/mm.h>

/* IO control's window is selected as (1/8)s. */
#define WAIT_PARTS_NUM		(8)
#define WAIT_INTERNAL_JIF	(HZ / WAIT_PARTS_NUM)

#define PROC_IOLIMIT_DIR		"iolimit"
#define PROC_WRITE_BYTES_LIMIT	"write_bytes_limit"
#define PROC_PROCS		"procs"

#define MAX_PID_STR_LEN		32
#define MAX_LINE_LEN		256

struct iolimit_pid_entry {
	struct pid *pid;  /* PID pointer for lookup, lifecycle managed by kernel */
	pid_t pid_nr;
	struct rhash_head node;
	struct rcu_head rcu;
};

static struct proc_dir_entry *iolimit_proc_dir;
static struct proc_dir_entry *write_bytes_limit_proc;
static struct proc_dir_entry *procs_proc;

/* Hash table for PID entries */
static struct rhashtable_params pid_hash_params = {
	.head_offset = offsetof(struct iolimit_pid_entry, node),
	.key_offset = offsetof(struct iolimit_pid_entry, pid_nr),
	.key_len = sizeof(pid_t),
	.automatic_shrinking = true,
};

static struct rhashtable pid_hash_table;
static DEFINE_SPINLOCK(pid_hash_lock);

/* Global write limit (can be set via procfs) */
static atomic64_t global_write_limit = ATOMIC64_INIT(0);

/* RUS control value for write_bytes_limit */
/* 0x5A: RUS disabled feature (return 0x5A, set limit to 0/unlimited) */
/* 0x5B: RUS enabled feature (return 0x5B, keep current limit unchanged) */
/* If set to 0x5A or 0x5B, read operations will return the RUS value directly */
static atomic_t write_bytes_limit_rus_value = ATOMIC_INIT(0);

/* Global write counter (shared by all limited PIDs) */
static s64 global_nr_written = 0;
static s64 global_nr_written_pause = 0;
static struct timer_list global_write_clear_timer;
static DEFINE_SPINLOCK(global_write_lock);
static wait_queue_head_t global_write_wq;

static void global_write_clear_timer_fn(struct timer_list *t)
{
	spin_lock_bh(&global_write_lock);
	global_nr_written = 0;
	spin_unlock_bh(&global_write_lock);
	wake_up_all(&global_write_wq);
}

static struct pid *find_pid_entry(pid_t pid_nr)
{
	struct iolimit_pid_entry *entry;
	struct pid *pid_ptr = NULL;

	rcu_read_lock();
	entry = rhashtable_lookup(&pid_hash_table, &pid_nr, pid_hash_params);
	if (entry && entry->pid) {
		pid_ptr = entry->pid;
	}
	rcu_read_unlock();

	return pid_ptr;
}

static struct iolimit_pid_entry *get_or_create_pid_entry(pid_t pid_nr)
{
	struct iolimit_pid_entry *entry, *new_entry;
	struct pid *pid_struct;
	struct task_struct *task;
	int ret;

	spin_lock(&pid_hash_lock);
	entry = rhashtable_lookup(&pid_hash_table, &pid_nr, pid_hash_params);
	if (entry) {
		spin_unlock(&pid_hash_lock);
		return entry;
	}
	spin_unlock(&pid_hash_lock);

	rcu_read_lock();
	task = find_task_by_vpid(pid_nr);
	if (!task) {
		rcu_read_unlock();
		pr_err("iolimit_pid: cannot find task for vpid=%d\n", pid_nr);
		return NULL;
	}

	if (task->group_leader)
		pid_struct = task_pid(task->group_leader);
	else
		pid_struct = task_pid(task);
	rcu_read_unlock();

	if (!pid_struct) {
		pr_warn("iolimit_pid: failed to get pid struct for pid=%d\n", pid_nr);
		return NULL;
	}

	new_entry = kzalloc(sizeof(*new_entry), GFP_KERNEL);
	if (!new_entry) {
		pr_warn("iolimit_pid: failed to allocate entry for pid=%d\n", pid_nr);
		return NULL;
	}

	new_entry->pid = pid_struct;
	new_entry->pid_nr = pid_nr;

	spin_lock(&pid_hash_lock);
	entry = rhashtable_lookup(&pid_hash_table, &pid_nr, pid_hash_params);
	if (entry && entry->pid) {
		spin_unlock(&pid_hash_lock);
		kfree(new_entry);
		return entry;
	}

	ret = rhashtable_insert_fast(&pid_hash_table, &new_entry->node, pid_hash_params);
	spin_unlock(&pid_hash_lock);

	if (ret) {
		pr_warn("iolimit_pid: failed to insert entry for pid=%d: %d\n", pid_nr, ret);
		kfree(new_entry);
		return NULL;
	}

	return new_entry;
}

static void remove_pid_entry(pid_t pid_nr)
{
	struct iolimit_pid_entry *entry;

	spin_lock(&pid_hash_lock);
	entry = rhashtable_lookup(&pid_hash_table, &pid_nr, pid_hash_params);
	if (entry) {
		rhashtable_remove_fast(&pid_hash_table, &entry->node, pid_hash_params);
		spin_unlock(&pid_hash_lock);
		kfree_rcu(entry, rcu);
	} else {
		spin_unlock(&pid_hash_lock);
	}
}

static int io_process_notifier(struct notifier_block *self,
			       unsigned long action, void *data)
{
	struct task_struct *task = (struct task_struct *)data;
	struct iolimit_pid_entry *entry;
	pid_t pid_nr;

	if (!task)
		return NOTIFY_OK;

	pid_nr = task->pid;

	// Check if PID exists in hash table before removing
	// Use RCU read lock to safely check existence
	rcu_read_lock();
	entry = rhashtable_lookup(&pid_hash_table, &pid_nr, pid_hash_params);
	rcu_read_unlock();

	// Only remove if entry was found
	// Note: entry may be freed by another thread between lookup and removal,
	// but remove_pid_entry handles this safely with its own locking
	if (entry) {
		remove_pid_entry(pid_nr);
		pr_info("iolimit_pid: removed PID %d on task exit\n", pid_nr);
	}

	return NOTIFY_OK;
}

static struct notifier_block io_notifier_block = {
	.notifier_call = io_process_notifier,
};

static bool is_write_need_wakeup(size_t count)
{
	s64 write_max = atomic64_read(&global_write_limit);

	if (write_max == 0)
		return true;

	if (global_nr_written_pause > (global_nr_written + count))
		return true;

	return false;
}

void do_io_write_bandwidth_control_pid(int count)
{
	pid_t pid_nr;
	int io_space_cnt;
	int ret;
	unsigned long start_time = jiffies;
	unsigned long delta;

	if (atomic64_read(&global_write_limit) == 0) {
		return;
	}
	// Use group_leader's PID to match the stored PID (same as iolimit_proc.c)
	// This ensures we match the main thread's PID, not a thread's PID
	if (current->group_leader)
		pid_nr = task_pid_nr(current->group_leader);
	else
		pid_nr = task_pid_nr(current);

	if (pid_nr <= 0) {
		return;
	}

	// Check if this PID is in the limit list
	// find_pid_entry only checks if PID is in the list, no validation or refcount
	// Even if the PID is dead, throttling one more time is acceptable
	struct pid *pid_ptr = find_pid_entry(pid_nr);
	if (!pid_ptr) {
		return;
	}

repeat:
	if (fatal_signal_pending(current)) {
		goto out;
	}
	spin_lock_bh(&global_write_lock);
	io_space_cnt = global_nr_written_pause - global_nr_written;
	if (io_space_cnt < count) {
		spin_unlock_bh(&global_write_lock);

		ret = wait_event_interruptible(global_write_wq,
				is_write_need_wakeup(count));

		// here wake up by signal
		if (ret < 0) {
			pr_err("iolimit_pid: wait interrupted for pid=%d\n", pid_nr);
			goto out;
		}

		goto repeat;
	} else {
		if (global_nr_written == 0) {
			mod_timer(&global_write_clear_timer,
					jiffies + WAIT_INTERNAL_JIF);
		}

		global_nr_written += count;
		spin_unlock_bh(&global_write_lock);
	}

out:
	delta = jiffies - start_time;
	if (delta > 500) {
		pr_err("iolimit_pid: control took %lu jiffies for pid=%d\n", delta, pid_nr);
	}
}
EXPORT_SYMBOL_GPL(do_io_write_bandwidth_control_pid);

/* Export function to remove PID entry (for userspace to call when app exits) */
void iolimit_pid_remove(pid_t pid_nr)
{
	if (pid_nr <= 0)
		return;

	pr_info("iolimit_pid: removing PID %d (called from userspace)\n", pid_nr);
	remove_pid_entry(pid_nr);
}
EXPORT_SYMBOL_GPL(iolimit_pid_remove);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0))
static void io_write_bandwidth_control_pid(void *unuse, struct inode *inode)
{
	do_io_write_bandwidth_control_pid(PAGE_SIZE);
}
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0))
static void io_write_bandwidth_control_pid(void *data, void *unuse)
{
	do_io_write_bandwidth_control_pid(PAGE_SIZE);
}
#endif

/* Procfs handlers for write_bytes_limit */
static ssize_t write_bytes_limit_read(struct file *file, char __user *buf,
				      size_t count, loff_t *ppos)
{
	char buffer[64];
	s64 limit;
	int rus_value = atomic_read(&write_bytes_limit_rus_value);
	int len;

	if (*ppos > 0)
		return 0;

	// If RUS control value (0x5A or 0x5B) is set, return it directly without reading hardware
	if (rus_value == 0x5A || rus_value == 0x5B) {
		len = snprintf(buffer, sizeof(buffer), "%d\n", rus_value);
		if (len < 0)
			return -EINVAL;

		if (copy_to_user(buf, buffer, len))
			return -EFAULT;

		*ppos = len;
		return len;
	}

	limit = atomic64_read(&global_write_limit);
	len = snprintf(buffer, sizeof(buffer), "%lld\n", limit);
	if (len < 0)
		return -EINVAL;

	if (copy_to_user(buf, buffer, len))
		return -EFAULT;

	*ppos = len;
	return len;
}

static ssize_t write_bytes_limit_write(struct file *file, const char __user *buf,
				       size_t count, loff_t *ppos)
{
	char buffer[64];
	s64 limit;
	int ret;

	if (count >= sizeof(buffer))
		return -EINVAL;

	if (copy_from_user(buffer, buf, count))
		return -EFAULT;

	ret = kstrtoll(buffer, 0, &limit);  // Base 0 to support decimal and hex (0x5A)
	if (ret < 0 || limit < 0) {
		pr_warn("iolimit_pid: invalid limit value: %s\n", buffer);
		return -EINVAL;
	}

	// If value is RUS control value (0x5A), save it and restore state (unlimited)
	if (limit == 0x5A) {
		atomic_set(&write_bytes_limit_rus_value, 0x5A);
		pr_info("iolimit_pid: RUS control value 0x5A set, restoring state (unlimited)\n");
		// Restore state: set limit to 0 (unlimited)
		limit = 0;
		goto set_limit;
	}

	// If value is RUS control value (0x5B), save it only, don't modify limit
	if (limit == 0x5B) {
		atomic_set(&write_bytes_limit_rus_value, 0x5B);
		pr_info("iolimit_pid: RUS control value 0x5B set, keeping current limit unchanged\n");
		// Don't modify limit, just return
		return count;
	}

	// Clear RUS control value if setting a normal value
	atomic_set(&write_bytes_limit_rus_value, 0);

set_limit:
	atomic64_set(&global_write_limit, limit);

	// Update global write pause limit (shared by all PIDs)
	spin_lock_bh(&global_write_lock);
	global_nr_written_pause = limit / WAIT_PARTS_NUM;
	spin_unlock_bh(&global_write_lock);

	pr_info("iolimit_pid: global limit updated, pause=%lld bytes\n", global_nr_written_pause);

	return count;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
static const struct proc_ops write_bytes_limit_proc_ops = {
	.proc_read = write_bytes_limit_read,
	.proc_write = write_bytes_limit_write,
	.proc_lseek = default_llseek,
};
#else
static const struct file_operations write_bytes_limit_proc_ops = {
	.read = write_bytes_limit_read,
	.write = write_bytes_limit_write,
	.llseek = default_llseek,
};
#endif

/* Procfs handlers for procs */
static ssize_t procs_read(struct file *file, char __user *buf,
			  size_t count, loff_t *ppos)
{
	char *buffer;
	size_t len = 0;
	size_t total_len = 0;
	size_t available;
	struct iolimit_pid_entry *entry;
	struct rhashtable_iter iter;
	int ret;
	int entry_count = 0;

	// Only read once, return 0 if already read
	if (*ppos > 0)
		return 0;

	// Allocate buffer for output (use count if smaller than PAGE_SIZE)
	available = min_t(size_t, count, PAGE_SIZE);
	buffer = kmalloc(available, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	rcu_read_lock();
	rhashtable_walk_enter(&pid_hash_table, &iter);
	rhashtable_walk_start(&iter);

	// Collect all PID entries
	while ((entry = rhashtable_walk_next(&iter)) != NULL) {
		if (IS_ERR(entry))
			continue;

		entry_count++;
		len = snprintf(buffer + total_len, available - total_len,
			       "%d\n", entry->pid_nr);

		if (len > 0 && total_len + len < available) {
			total_len += len;
		} else {
			// Buffer full, stop here
			pr_warn("iolimit_pid: buffer full, truncated output (entries: %d)\n",
				entry_count);
			break;
		}
	}

	rhashtable_walk_stop(&iter);
	rhashtable_walk_exit(&iter);
	rcu_read_unlock();

	if (total_len > 0) {
		if (copy_to_user(buf, buffer, total_len)) {
			kfree(buffer);
			return -EFAULT;
		}
		*ppos = total_len;
		ret = total_len;
	} else {
		ret = 0;
	}

	kfree(buffer);
	return ret;
}

static ssize_t procs_write(struct file *file, const char __user *buf,
			   size_t count, loff_t *ppos)
{
	char *buffer;
	char *ptr;
	pid_t pid_nr;
	struct iolimit_pid_entry *entry;
	int ret;
	bool remove_mode = false;

	if (count >= PAGE_SIZE)
		return -EINVAL;

	buffer = kzalloc(count + 1, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	if (copy_from_user(buffer, buf, count)) {
		kfree(buffer);
		return -EFAULT;
	}

	ptr = buffer;

	// Check for 'd' prefix to indicate removal
	if (*ptr == 'd') {
		remove_mode = true;
		ptr++;  // Skip 'd' prefix
	}

	// Parse single PID (only one PID allowed)
	// kstrtoint will handle whitespace and validate the input
	ret = kstrtoint(ptr, 10, &pid_nr);
	if (ret < 0) {
		pr_warn("iolimit_pid: invalid PID: %s\n", buffer);
		kfree(buffer);
		return -EINVAL;
	}

	// Special case: pid_nr == 0 means remove all PIDs (only if not in remove_mode)
	if (pid_nr == 0 && !remove_mode) {
		struct iolimit_pid_entry *entry;
		struct rhashtable_iter iter;
		int removed_count = 0;
		pid_t pid_to_remove;

		pr_info("iolimit_pid: removing all throttled PIDs\n");

		// Remove all PID entries
		rcu_read_lock();
		rhashtable_walk_enter(&pid_hash_table, &iter);
		rhashtable_walk_start(&iter);

		while ((entry = rhashtable_walk_next(&iter)) != NULL) {
			if (IS_ERR(entry))
				continue;

			pid_to_remove = entry->pid_nr;
			rhashtable_walk_stop(&iter);
			rhashtable_walk_exit(&iter);
			rcu_read_unlock();

			remove_pid_entry(pid_to_remove);
			removed_count++;

			rcu_read_lock();
			rhashtable_walk_enter(&pid_hash_table, &iter);
			rhashtable_walk_start(&iter);
		}

		rhashtable_walk_stop(&iter);
		rhashtable_walk_exit(&iter);
		rcu_read_unlock();

		pr_info("iolimit_pid: removed %d PID(s)\n", removed_count);
		kfree(buffer);
		return count;
	}

	if (pid_nr <= 0) {
		pr_warn("iolimit_pid: invalid PID: %s\n", buffer);
		kfree(buffer);
		return -EINVAL;
	}

	if (remove_mode) {
		// Remove PID entry
		remove_pid_entry(pid_nr);
		kfree(buffer);
		return count;
	}

	// Add PID entry (all PIDs share the global limit)
	entry = get_or_create_pid_entry(pid_nr);
	if (entry && entry->pid) {
		// Don't put_pid(), PID lifecycle is managed by kernel
	} else {
		pr_warn("iolimit_pid: failed to create entry for pid=%d\n", pid_nr);
		kfree(buffer);
		return -EINVAL;
	}

	kfree(buffer);
	return count;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
static const struct proc_ops procs_proc_ops = {
	.proc_read = procs_read,
	.proc_write = procs_write,
	.proc_lseek = default_llseek,
};
#else
static const struct file_operations procs_proc_ops = {
	.read = procs_read,
	.write = procs_write,
	.llseek = default_llseek,
};
#endif

static int __init iolimit_pid_init(void)
{
	int ret;

	// Initialize hash table
	ret = rhashtable_init(&pid_hash_table, &pid_hash_params);
	if (ret < 0) {
		pr_err("iolimit_pid: failed to initialize hash table: %d\n", ret);
		return ret;
	}

	// Initialize global write counter and timer
	global_nr_written = 0;
	global_nr_written_pause = 0;
	timer_setup(&global_write_clear_timer, global_write_clear_timer_fn, 0);
	spin_lock_init(&global_write_lock);
	init_waitqueue_head(&global_write_wq);

	// Create procfs directory
	iolimit_proc_dir = proc_mkdir(PROC_IOLIMIT_DIR, NULL);
	if (!iolimit_proc_dir) {
		pr_err("iolimit_pid: failed to create proc directory\n");
		rhashtable_destroy(&pid_hash_table);
		return -ENOMEM;
	}

	// Create write_bytes_limit proc file
	// Use 0666 permission: S_IRUGO | S_IWUGO (rw-rw-rw-)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	write_bytes_limit_proc = proc_create(PROC_WRITE_BYTES_LIMIT,
					      0666,
					      iolimit_proc_dir,
					      &write_bytes_limit_proc_ops);
#else
	write_bytes_limit_proc = proc_create(PROC_WRITE_BYTES_LIMIT,
					      0666,
					      iolimit_proc_dir,
					      &write_bytes_limit_proc_ops);
#endif
	if (!write_bytes_limit_proc) {
		pr_err("iolimit_pid: failed to create write_bytes_limit proc file\n");
		remove_proc_entry(PROC_IOLIMIT_DIR, NULL);
		rhashtable_destroy(&pid_hash_table);
		return -ENOMEM;
	}

	// Create procs proc file
	// Use 0666 permission: S_IRUGO | S_IWUGO (rw-rw-rw-)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	procs_proc = proc_create(PROC_PROCS,
				 0666,
				 iolimit_proc_dir,
				 &procs_proc_ops);
#else
	procs_proc = proc_create(PROC_PROCS,
				 0666,
				 iolimit_proc_dir,
				 &procs_proc_ops);
#endif
	if (!procs_proc) {
		pr_err("iolimit_pid: failed to create procs proc file\n");
		remove_proc_entry(PROC_WRITE_BYTES_LIMIT, iolimit_proc_dir);
		remove_proc_entry(PROC_IOLIMIT_DIR, NULL);
		rhashtable_destroy(&pid_hash_table);
		return -ENOMEM;
	}

	// Register trace hook
	register_trace_android_rvh_ctl_dirty_rate(io_write_bandwidth_control_pid, NULL);

	// Register task exit notifier to cleanup PID entries when tasks exit
	profile_event_register(PROFILE_TASK_EXIT, &io_notifier_block);

	pr_info("iolimit_pid: module initialized successfully\n");
	pr_info("iolimit_pid: procfs interfaces: /proc/%s/%s, /proc/%s/%s\n",
		PROC_IOLIMIT_DIR, PROC_WRITE_BYTES_LIMIT,
		PROC_IOLIMIT_DIR, PROC_PROCS);

	return 0;
}

static void __exit iolimit_pid_exit(void)
{
	// Unregister task exit notifier
	profile_event_unregister(PROFILE_TASK_EXIT, &io_notifier_block);

	// Remove procfs entries
	if (procs_proc) {
		remove_proc_entry(PROC_PROCS, iolimit_proc_dir);
		procs_proc = NULL;
	}

	if (write_bytes_limit_proc) {
		remove_proc_entry(PROC_WRITE_BYTES_LIMIT, iolimit_proc_dir);
		write_bytes_limit_proc = NULL;
	}

	if (iolimit_proc_dir) {
		remove_proc_entry(PROC_IOLIMIT_DIR, NULL);
		iolimit_proc_dir = NULL;
	}

	// Destroy hash table
	rhashtable_destroy(&pid_hash_table);

	pr_info("iolimit_pid: module exited\n");
}

/*
 * Note: iolimit_pid registers rvh (register_trace_android_rvh_ctl_dirty_rate),
 * and rvh cannot be unregisterred. So, iolimit_pid cannot support rmmod,
 * or task calling vendor hook will panic as mem abort.
 */

module_init(iolimit_pid_init);
module_exit(iolimit_pid_exit);

MODULE_AUTHOR("OPLUS");
MODULE_DESCRIPTION("IO limit control by PID");
MODULE_LICENSE("GPL");
