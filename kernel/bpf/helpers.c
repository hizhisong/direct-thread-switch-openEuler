// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2011-2014 PLUMgrid, http://plumgrid.com
 */
#include <linux/bpf.h>
#include <linux/rcupdate.h>
#include <linux/random.h>
#include <linux/smp.h>
#include <linux/topology.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/uidgid.h>
#include <linux/filter.h>
#include <linux/ctype.h>
#include <linux/jiffies.h>
#include <linux/pid_namespace.h>
#include <linux/proc_ns.h>
#include <linux/security.h>

#include "../../lib/kstrtox.h"

/* If kernel subsystem is allowing eBPF programs to call this function,
 * inside its own verifier_ops->get_func_proto() callback it should return
 * bpf_map_lookup_elem_proto, so that verifier can properly check the arguments
 *
 * Different map implementations will rely on rcu in map methods
 * lookup/update/delete, therefore eBPF programs must run under rcu lock
 * if program is allowed to access maps, so check rcu_read_lock_held in
 * all three functions.
 */
BPF_CALL_2(bpf_map_lookup_elem, struct bpf_map *, map, void *, key)
{
	WARN_ON_ONCE(!rcu_read_lock_held());
	return (unsigned long) map->ops->map_lookup_elem(map, key);
}

const struct bpf_func_proto bpf_map_lookup_elem_proto = {
	.func		= bpf_map_lookup_elem,
	.gpl_only	= false,
	.pkt_access	= true,
	.ret_type	= RET_PTR_TO_MAP_VALUE_OR_NULL,
	.arg1_type	= ARG_CONST_MAP_PTR,
	.arg2_type	= ARG_PTR_TO_MAP_KEY,
};

BPF_CALL_4(bpf_map_update_elem, struct bpf_map *, map, void *, key,
	   void *, value, u64, flags)
{
	WARN_ON_ONCE(!rcu_read_lock_held());
	return map->ops->map_update_elem(map, key, value, flags);
}

const struct bpf_func_proto bpf_map_update_elem_proto = {
	.func		= bpf_map_update_elem,
	.gpl_only	= false,
	.pkt_access	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_CONST_MAP_PTR,
	.arg2_type	= ARG_PTR_TO_MAP_KEY,
	.arg3_type	= ARG_PTR_TO_MAP_VALUE,
	.arg4_type	= ARG_ANYTHING,
};

BPF_CALL_2(bpf_map_delete_elem, struct bpf_map *, map, void *, key)
{
	WARN_ON_ONCE(!rcu_read_lock_held());
	return map->ops->map_delete_elem(map, key);
}

const struct bpf_func_proto bpf_map_delete_elem_proto = {
	.func		= bpf_map_delete_elem,
	.gpl_only	= false,
	.pkt_access	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_CONST_MAP_PTR,
	.arg2_type	= ARG_PTR_TO_MAP_KEY,
};

BPF_CALL_3(bpf_map_push_elem, struct bpf_map *, map, void *, value, u64, flags)
{
	return map->ops->map_push_elem(map, value, flags);
}

const struct bpf_func_proto bpf_map_push_elem_proto = {
	.func		= bpf_map_push_elem,
	.gpl_only	= false,
	.pkt_access	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_CONST_MAP_PTR,
	.arg2_type	= ARG_PTR_TO_MAP_VALUE,
	.arg3_type	= ARG_ANYTHING,
};

BPF_CALL_2(bpf_map_pop_elem, struct bpf_map *, map, void *, value)
{
	return map->ops->map_pop_elem(map, value);
}

const struct bpf_func_proto bpf_map_pop_elem_proto = {
	.func		= bpf_map_pop_elem,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_CONST_MAP_PTR,
	.arg2_type	= ARG_PTR_TO_UNINIT_MAP_VALUE,
};

BPF_CALL_2(bpf_map_peek_elem, struct bpf_map *, map, void *, value)
{
	return map->ops->map_peek_elem(map, value);
}

const struct bpf_func_proto bpf_map_peek_elem_proto = {
	.func		= bpf_map_peek_elem,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_CONST_MAP_PTR,
	.arg2_type	= ARG_PTR_TO_UNINIT_MAP_VALUE,
};

const struct bpf_func_proto bpf_get_prandom_u32_proto = {
	.func		= bpf_user_rnd_u32,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
};

BPF_CALL_0(bpf_get_smp_processor_id)
{
	return smp_processor_id();
}

const struct bpf_func_proto bpf_get_smp_processor_id_proto = {
	.func		= bpf_get_smp_processor_id,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
};

BPF_CALL_0(bpf_get_numa_node_id)
{
	return numa_node_id();
}

const struct bpf_func_proto bpf_get_numa_node_id_proto = {
	.func		= bpf_get_numa_node_id,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
};

BPF_CALL_0(bpf_ktime_get_ns)
{
	/* NMI safe access to clock monotonic */
	return ktime_get_mono_fast_ns();
}

const struct bpf_func_proto bpf_ktime_get_ns_proto = {
	.func		= bpf_ktime_get_ns,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
};

BPF_CALL_0(bpf_ktime_get_boot_ns)
{
	/* NMI safe access to clock boottime */
	return ktime_get_boot_fast_ns();
}

const struct bpf_func_proto bpf_ktime_get_boot_ns_proto = {
	.func		= bpf_ktime_get_boot_ns,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
};

BPF_CALL_0(bpf_get_current_pid_tgid)
{
	struct task_struct *task = current;

	if (unlikely(!task))
		return -EINVAL;

	return (u64) task->tgid << 32 | task->pid;
}

const struct bpf_func_proto bpf_get_current_pid_tgid_proto = {
	.func		= bpf_get_current_pid_tgid,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
};

BPF_CALL_0(bpf_get_current_uid_gid)
{
	struct task_struct *task = current;
	kuid_t uid;
	kgid_t gid;

	if (unlikely(!task))
		return -EINVAL;

	current_uid_gid(&uid, &gid);
	return (u64) from_kgid(&init_user_ns, gid) << 32 |
		     from_kuid(&init_user_ns, uid);
}

const struct bpf_func_proto bpf_get_current_uid_gid_proto = {
	.func		= bpf_get_current_uid_gid,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
};

BPF_CALL_2(bpf_get_current_comm, char *, buf, u32, size)
{
	struct task_struct *task = current;

	if (unlikely(!task))
		goto err_clear;

	strncpy(buf, task->comm, size);

	/* Verifier guarantees that size > 0. For task->comm exceeding
	 * size, guarantee that buf is %NUL-terminated. Unconditionally
	 * done here to save the size test.
	 */
	buf[size - 1] = 0;
	return 0;
err_clear:
	memset(buf, 0, size);
	return -EINVAL;
}

const struct bpf_func_proto bpf_get_current_comm_proto = {
	.func		= bpf_get_current_comm,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_UNINIT_MEM,
	.arg2_type	= ARG_CONST_SIZE,
};

#if defined(CONFIG_QUEUED_SPINLOCKS) || defined(CONFIG_BPF_ARCH_SPINLOCK)

static inline void __bpf_spin_lock(struct bpf_spin_lock *lock)
{
	arch_spinlock_t *l = (void *)lock;
	union {
		__u32 val;
		arch_spinlock_t lock;
	} u = { .lock = __ARCH_SPIN_LOCK_UNLOCKED };

	compiletime_assert(u.val == 0, "__ARCH_SPIN_LOCK_UNLOCKED not 0");
	BUILD_BUG_ON(sizeof(*l) != sizeof(__u32));
	BUILD_BUG_ON(sizeof(*lock) != sizeof(__u32));
	arch_spin_lock(l);
}

static inline void __bpf_spin_unlock(struct bpf_spin_lock *lock)
{
	arch_spinlock_t *l = (void *)lock;

	arch_spin_unlock(l);
}

#else

static inline void __bpf_spin_lock(struct bpf_spin_lock *lock)
{
	atomic_t *l = (void *)lock;

	BUILD_BUG_ON(sizeof(*l) != sizeof(*lock));
	do {
		atomic_cond_read_relaxed(l, !VAL);
	} while (atomic_xchg(l, 1));
}

static inline void __bpf_spin_unlock(struct bpf_spin_lock *lock)
{
	atomic_t *l = (void *)lock;

	atomic_set_release(l, 0);
}

#endif

static DEFINE_PER_CPU(unsigned long, irqsave_flags);

static inline void __bpf_spin_lock_irqsave(struct bpf_spin_lock *lock)
{
	unsigned long flags;

	local_irq_save(flags);
	__bpf_spin_lock(lock);
	__this_cpu_write(irqsave_flags, flags);
}

notrace BPF_CALL_1(bpf_spin_lock, struct bpf_spin_lock *, lock)
{
	__bpf_spin_lock_irqsave(lock);
	return 0;
}

const struct bpf_func_proto bpf_spin_lock_proto = {
	.func		= bpf_spin_lock,
	.gpl_only	= false,
	.ret_type	= RET_VOID,
	.arg1_type	= ARG_PTR_TO_SPIN_LOCK,
};

static inline void __bpf_spin_unlock_irqrestore(struct bpf_spin_lock *lock)
{
	unsigned long flags;

	flags = __this_cpu_read(irqsave_flags);
	__bpf_spin_unlock(lock);
	local_irq_restore(flags);
}

notrace BPF_CALL_1(bpf_spin_unlock, struct bpf_spin_lock *, lock)
{
	__bpf_spin_unlock_irqrestore(lock);
	return 0;
}

const struct bpf_func_proto bpf_spin_unlock_proto = {
	.func		= bpf_spin_unlock,
	.gpl_only	= false,
	.ret_type	= RET_VOID,
	.arg1_type	= ARG_PTR_TO_SPIN_LOCK,
};

void copy_map_value_locked(struct bpf_map *map, void *dst, void *src,
			   bool lock_src)
{
	struct bpf_spin_lock *lock;

	if (lock_src)
		lock = src + map->spin_lock_off;
	else
		lock = dst + map->spin_lock_off;
	preempt_disable();
	__bpf_spin_lock_irqsave(lock);
	copy_map_value(map, dst, src);
	__bpf_spin_unlock_irqrestore(lock);
	preempt_enable();
}

BPF_CALL_0(bpf_jiffies64)
{
	return get_jiffies_64();
}

const struct bpf_func_proto bpf_jiffies64_proto = {
	.func		= bpf_jiffies64,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
};

#ifdef CONFIG_CGROUPS
BPF_CALL_0(bpf_get_current_cgroup_id)
{
	struct cgroup *cgrp = task_dfl_cgroup(current);

	return cgroup_id(cgrp);
}

const struct bpf_func_proto bpf_get_current_cgroup_id_proto = {
	.func		= bpf_get_current_cgroup_id,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
};

BPF_CALL_1(bpf_get_current_ancestor_cgroup_id, int, ancestor_level)
{
	struct cgroup *cgrp = task_dfl_cgroup(current);
	struct cgroup *ancestor;

	ancestor = cgroup_ancestor(cgrp, ancestor_level);
	if (!ancestor)
		return 0;
	return cgroup_id(ancestor);
}

const struct bpf_func_proto bpf_get_current_ancestor_cgroup_id_proto = {
	.func		= bpf_get_current_ancestor_cgroup_id,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_ANYTHING,
};

#ifdef CONFIG_CGROUP_BPF

BPF_CALL_2(bpf_get_local_storage, struct bpf_map *, map, u64, flags)
{
	/* flags argument is not used now,
	 * but provides an ability to extend the API.
	 * verifier checks that its value is correct.
	 */
	enum bpf_cgroup_storage_type stype = cgroup_storage_type(map);
	struct bpf_cgroup_storage *storage;
	struct bpf_cg_run_ctx *ctx;
	void *ptr;

	/* get current cgroup storage from BPF run context */
	ctx = container_of(current->bpf_ctx, struct bpf_cg_run_ctx, run_ctx);
	storage = ctx->prog_item->cgroup_storage[stype];

	if (stype == BPF_CGROUP_STORAGE_SHARED)
		ptr = &READ_ONCE(storage->buf)->data[0];
	else
		ptr = this_cpu_ptr(storage->percpu_buf);

	return (unsigned long)ptr;
}

const struct bpf_func_proto bpf_get_local_storage_proto = {
	.func		= bpf_get_local_storage,
	.gpl_only	= false,
	.ret_type	= RET_PTR_TO_MAP_VALUE,
	.arg1_type	= ARG_CONST_MAP_PTR,
	.arg2_type	= ARG_ANYTHING,
};
#endif

#define BPF_STRTOX_BASE_MASK 0x1F

static int __bpf_strtoull(const char *buf, size_t buf_len, u64 flags,
			  unsigned long long *res, bool *is_negative)
{
	unsigned int base = flags & BPF_STRTOX_BASE_MASK;
	const char *cur_buf = buf;
	size_t cur_len = buf_len;
	unsigned int consumed;
	size_t val_len;
	char str[64];

	if (!buf || !buf_len || !res || !is_negative)
		return -EINVAL;

	if (base != 0 && base != 8 && base != 10 && base != 16)
		return -EINVAL;

	if (flags & ~BPF_STRTOX_BASE_MASK)
		return -EINVAL;

	while (cur_buf < buf + buf_len && isspace(*cur_buf))
		++cur_buf;

	*is_negative = (cur_buf < buf + buf_len && *cur_buf == '-');
	if (*is_negative)
		++cur_buf;

	consumed = cur_buf - buf;
	cur_len -= consumed;
	if (!cur_len)
		return -EINVAL;

	cur_len = min(cur_len, sizeof(str) - 1);
	memcpy(str, cur_buf, cur_len);
	str[cur_len] = '\0';
	cur_buf = str;

	cur_buf = _parse_integer_fixup_radix(cur_buf, &base);
	val_len = _parse_integer(cur_buf, base, res);

	if (val_len & KSTRTOX_OVERFLOW)
		return -ERANGE;

	if (val_len == 0)
		return -EINVAL;

	cur_buf += val_len;
	consumed += cur_buf - str;

	return consumed;
}

static int __bpf_strtoll(const char *buf, size_t buf_len, u64 flags,
			 long long *res)
{
	unsigned long long _res;
	bool is_negative;
	int err;

	err = __bpf_strtoull(buf, buf_len, flags, &_res, &is_negative);
	if (err < 0)
		return err;
	if (is_negative) {
		if ((long long)-_res > 0)
			return -ERANGE;
		*res = -_res;
	} else {
		if ((long long)_res < 0)
			return -ERANGE;
		*res = _res;
	}
	return err;
}

BPF_CALL_4(bpf_strtol, const char *, buf, size_t, buf_len, u64, flags,
	   long *, res)
{
	long long _res;
	int err;

	err = __bpf_strtoll(buf, buf_len, flags, &_res);
	if (err < 0)
		return err;
	if (_res != (long)_res)
		return -ERANGE;
	*res = _res;
	return err;
}

const struct bpf_func_proto bpf_strtol_proto = {
	.func		= bpf_strtol,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_MEM | MEM_RDONLY,
	.arg2_type	= ARG_CONST_SIZE,
	.arg3_type	= ARG_ANYTHING,
	.arg4_type	= ARG_PTR_TO_LONG,
};

BPF_CALL_4(bpf_strtoul, const char *, buf, size_t, buf_len, u64, flags,
	   unsigned long *, res)
{
	unsigned long long _res;
	bool is_negative;
	int err;

	err = __bpf_strtoull(buf, buf_len, flags, &_res, &is_negative);
	if (err < 0)
		return err;
	if (is_negative)
		return -EINVAL;
	if (_res != (unsigned long)_res)
		return -ERANGE;
	*res = _res;
	return err;
}

const struct bpf_func_proto bpf_strtoul_proto = {
	.func		= bpf_strtoul,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_MEM | MEM_RDONLY,
	.arg2_type	= ARG_CONST_SIZE,
	.arg3_type	= ARG_ANYTHING,
	.arg4_type	= ARG_PTR_TO_LONG,
};
#endif

BPF_CALL_4(bpf_get_ns_current_pid_tgid, u64, dev, u64, ino,
	   struct bpf_pidns_info *, nsdata, u32, size)
{
	struct task_struct *task = current;
	struct pid_namespace *pidns;
	int err = -EINVAL;

	if (unlikely(size != sizeof(struct bpf_pidns_info)))
		goto clear;

	if (unlikely((u64)(dev_t)dev != dev))
		goto clear;

	if (unlikely(!task))
		goto clear;

	pidns = task_active_pid_ns(task);
	if (unlikely(!pidns)) {
		err = -ENOENT;
		goto clear;
	}

	if (!ns_match(&pidns->ns, (dev_t)dev, ino))
		goto clear;

	nsdata->pid = task_pid_nr_ns(task, pidns);
	nsdata->tgid = task_tgid_nr_ns(task, pidns);
	return 0;
clear:
	memset((void *)nsdata, 0, (size_t) size);
	return err;
}

const struct bpf_func_proto bpf_get_ns_current_pid_tgid_proto = {
	.func		= bpf_get_ns_current_pid_tgid,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_ANYTHING,
	.arg2_type	= ARG_ANYTHING,
	.arg3_type      = ARG_PTR_TO_UNINIT_MEM,
	.arg4_type      = ARG_CONST_SIZE,
};

static const struct bpf_func_proto bpf_get_raw_smp_processor_id_proto = {
	.func		= bpf_get_raw_cpu_id,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
};

BPF_CALL_5(bpf_event_output_data, void *, ctx, struct bpf_map *, map,
	   u64, flags, void *, data, u64, size)
{
	if (unlikely(flags & ~(BPF_F_INDEX_MASK)))
		return -EINVAL;

	return bpf_event_output(map, flags, data, size, NULL, 0, NULL);
}

const struct bpf_func_proto bpf_event_output_data_proto =  {
	.func		= bpf_event_output_data,
	.gpl_only       = true,
	.ret_type       = RET_INTEGER,
	.arg1_type      = ARG_PTR_TO_CTX,
	.arg2_type      = ARG_CONST_MAP_PTR,
	.arg3_type      = ARG_ANYTHING,
	.arg4_type      = ARG_PTR_TO_MEM | MEM_RDONLY,
	.arg5_type      = ARG_CONST_SIZE_OR_ZERO,
};

BPF_CALL_3(bpf_copy_from_user, void *, dst, u32, size,
	   const void __user *, user_ptr)
{
	int ret = copy_from_user(dst, user_ptr, size);

	if (unlikely(ret)) {
		memset(dst, 0, size);
		ret = -EFAULT;
	}

	return ret;
}

const struct bpf_func_proto bpf_copy_from_user_proto = {
	.func		= bpf_copy_from_user,
	.gpl_only	= false,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_UNINIT_MEM,
	.arg2_type	= ARG_CONST_SIZE_OR_ZERO,
	.arg3_type	= ARG_ANYTHING,
};

BPF_CALL_2(bpf_per_cpu_ptr, const void *, ptr, u32, cpu)
{
	if (cpu >= nr_cpu_ids)
		return (unsigned long)NULL;

	return (unsigned long)per_cpu_ptr((const void __percpu *)ptr, cpu);
}

const struct bpf_func_proto bpf_per_cpu_ptr_proto = {
	.func		= bpf_per_cpu_ptr,
	.gpl_only	= false,
	.ret_type	= RET_PTR_TO_MEM_OR_BTF_ID | PTR_MAYBE_NULL | MEM_RDONLY,
	.arg1_type	= ARG_PTR_TO_PERCPU_BTF_ID,
	.arg2_type	= ARG_ANYTHING,
};

BPF_CALL_1(bpf_this_cpu_ptr, const void *, percpu_ptr)
{
	return (unsigned long)this_cpu_ptr((const void __percpu *)percpu_ptr);
}

const struct bpf_func_proto bpf_this_cpu_ptr_proto = {
	.func		= bpf_this_cpu_ptr,
	.gpl_only	= false,
	.ret_type	= RET_PTR_TO_MEM_OR_BTF_ID | MEM_RDONLY,
	.arg1_type	= ARG_PTR_TO_PERCPU_BTF_ID,
};

static int bpf_trace_copy_string(char *buf, void *unsafe_ptr, char fmt_ptype,
		size_t bufsz)
{
	void __user *user_ptr = (__force void __user *)unsafe_ptr;

	buf[0] = 0;

	switch (fmt_ptype) {
	case 's':
#ifdef CONFIG_ARCH_HAS_NON_OVERLAPPING_ADDRESS_SPACE
		if ((unsigned long)unsafe_ptr < TASK_SIZE)
			return strncpy_from_user_nofault(buf, user_ptr, bufsz);
		fallthrough;
#endif
	case 'k':
		return strncpy_from_kernel_nofault(buf, unsafe_ptr, bufsz);
	case 'u':
		return strncpy_from_user_nofault(buf, user_ptr, bufsz);
	}

	return -EINVAL;
}

/* Per-cpu temp buffers which can be used by printf-like helpers for %s or %p
 */
#define MAX_PRINTF_BUF_LEN	512

struct bpf_printf_buf {
	char tmp_buf[MAX_PRINTF_BUF_LEN];
};
static DEFINE_PER_CPU(struct bpf_printf_buf, bpf_printf_buf);
static DEFINE_PER_CPU(int, bpf_printf_buf_used);

static int try_get_fmt_tmp_buf(char **tmp_buf)
{
	struct bpf_printf_buf *bufs;
	int used;

	if (*tmp_buf)
		return 0;

	preempt_disable();
	used = this_cpu_inc_return(bpf_printf_buf_used);
	if (WARN_ON_ONCE(used > 1)) {
		this_cpu_dec(bpf_printf_buf_used);
		preempt_enable();
		return -EBUSY;
	}
	bufs = this_cpu_ptr(&bpf_printf_buf);
	*tmp_buf = bufs->tmp_buf;

	return 0;
}

void bpf_printf_cleanup(void)
{
	if (this_cpu_read(bpf_printf_buf_used)) {
		this_cpu_dec(bpf_printf_buf_used);
		preempt_enable();
	}
}

/*
 * bpf_parse_fmt_str - Generic pass on format strings for printf-like helpers
 *
 * Returns a negative value if fmt is an invalid format string or 0 otherwise.
 *
 * This can be used in two ways:
 * - Format string verification only: when final_args and mod are NULL
 * - Arguments preparation: in addition to the above verification, it writes in
 *   final_args a copy of raw_args where pointers from BPF have been sanitized
 *   into pointers safe to use by snprintf. This also writes in the mod array
 *   the size requirement of each argument, usable by BPF_CAST_FMT_ARG for ex.
 *
 * In argument preparation mode, if 0 is returned, safe temporary buffers are
 * allocated and bpf_printf_cleanup should be called to free them after use.
 */
int bpf_printf_prepare(char *fmt, u32 fmt_size, const u64 *raw_args,
			u64 *final_args, enum bpf_printf_mod_type *mod,
			u32 num_args)
{
	char *unsafe_ptr = NULL, *tmp_buf = NULL, *fmt_end;
	size_t tmp_buf_len = MAX_PRINTF_BUF_LEN;
	int err, i, num_spec = 0, copy_size;
	enum bpf_printf_mod_type cur_mod;
	u64 cur_arg;
	char fmt_ptype;

	if (!!final_args != !!mod)
		return -EINVAL;

	fmt_end = strnchr(fmt, fmt_size, 0);
	if (!fmt_end)
		return -EINVAL;
	fmt_size = fmt_end - fmt;

	for (i = 0; i < fmt_size; i++) {
		if ((!isprint(fmt[i]) && !isspace(fmt[i])) || !isascii(fmt[i])) {
			err = -EINVAL;
			goto cleanup;
		}

		if (fmt[i] != '%')
			continue;

		if (fmt[i + 1] == '%') {
			i++;
			continue;
		}

		if (num_spec >= num_args) {
			err = -EINVAL;
			goto cleanup;
		}

		/* The string is zero-terminated so if fmt[i] != 0, we can
		 * always access fmt[i + 1], in the worst case it will be a 0
		 */
		i++;

		/* skip optional "[0 +-][num]" width formatting field */
		while (fmt[i] == '0' || fmt[i] == '+'  || fmt[i] == '-' ||
		       fmt[i] == ' ')
			i++;
		if (fmt[i] >= '1' && fmt[i] <= '9') {
			i++;
			while (fmt[i] >= '0' && fmt[i] <= '9')
				i++;
		}

		if (fmt[i] == 'p') {
			cur_mod = BPF_PRINTF_LONG;

			if ((fmt[i + 1] == 'k' || fmt[i + 1] == 'u') &&
			    fmt[i + 2] == 's') {
				fmt_ptype = fmt[i + 1];
				i += 2;
				goto fmt_str;
			}

			if (fmt[i + 1] == 0 || isspace(fmt[i + 1]) ||
			    ispunct(fmt[i + 1]) || fmt[i + 1] == 'K' ||
			    fmt[i + 1] == 'x' || fmt[i + 1] == 'B' ||
			    fmt[i + 1] == 's' || fmt[i + 1] == 'S') {
				/* just kernel pointers */
				if (final_args)
					cur_arg = raw_args[num_spec];
				goto fmt_next;
			}

			/* only support "%pI4", "%pi4", "%pI6" and "%pi6". */
			if ((fmt[i + 1] != 'i' && fmt[i + 1] != 'I') ||
			    (fmt[i + 2] != '4' && fmt[i + 2] != '6')) {
				err = -EINVAL;
				goto cleanup;
			}

			i += 2;
			if (!final_args)
				goto fmt_next;

			if (try_get_fmt_tmp_buf(&tmp_buf)) {
				err = -EBUSY;
				goto out;
			}

			copy_size = (fmt[i + 2] == '4') ? 4 : 16;
			if (tmp_buf_len < copy_size) {
				err = -ENOSPC;
				goto cleanup;
			}

			unsafe_ptr = (char *)(long)raw_args[num_spec];
			err = copy_from_kernel_nofault(tmp_buf, unsafe_ptr,
						       copy_size);
			if (err < 0)
				memset(tmp_buf, 0, copy_size);
			cur_arg = (u64)(long)tmp_buf;
			tmp_buf += copy_size;
			tmp_buf_len -= copy_size;

			goto fmt_next;
		} else if (fmt[i] == 's') {
			cur_mod = BPF_PRINTF_LONG;
			fmt_ptype = fmt[i];
fmt_str:
			if (fmt[i + 1] != 0 &&
			    !isspace(fmt[i + 1]) &&
			    !ispunct(fmt[i + 1])) {
				err = -EINVAL;
				goto cleanup;
			}

			if (!final_args)
				goto fmt_next;

			if (try_get_fmt_tmp_buf(&tmp_buf)) {
				err = -EBUSY;
				goto out;
			}

			if (!tmp_buf_len) {
				err = -ENOSPC;
				goto cleanup;
			}

			unsafe_ptr = (char *)(long)raw_args[num_spec];
			err = bpf_trace_copy_string(tmp_buf, unsafe_ptr,
						    fmt_ptype, tmp_buf_len);
			if (err < 0) {
				tmp_buf[0] = '\0';
				err = 1;
			}

			cur_arg = (u64)(long)tmp_buf;
			tmp_buf += err;
			tmp_buf_len -= err;

			goto fmt_next;
		}

		cur_mod = BPF_PRINTF_INT;

		if (fmt[i] == 'l') {
			cur_mod = BPF_PRINTF_LONG;
			i++;
		}
		if (fmt[i] == 'l') {
			cur_mod = BPF_PRINTF_LONG_LONG;
			i++;
		}

		if (fmt[i] != 'i' && fmt[i] != 'd' && fmt[i] != 'u' &&
		    fmt[i] != 'x' && fmt[i] != 'X') {
			err = -EINVAL;
			goto cleanup;
		}

		if (final_args)
			cur_arg = raw_args[num_spec];
fmt_next:
		if (final_args) {
			mod[num_spec] = cur_mod;
			final_args[num_spec] = cur_arg;
		}
		num_spec++;
	}

	err = 0;
cleanup:
	if (err)
		bpf_printf_cleanup();
out:
	return err;
}

#define MAX_SNPRINTF_VARARGS		12

BPF_CALL_5(bpf_snprintf, char *, str, u32, str_size, char *, fmt,
	   const void *, data, u32, data_len)
{
	enum bpf_printf_mod_type mod[MAX_SNPRINTF_VARARGS];
	u64 args[MAX_SNPRINTF_VARARGS];
	int err, num_args;

	if (data_len % 8 || data_len > MAX_SNPRINTF_VARARGS * 8 ||
	    (data_len && !data))
		return -EINVAL;
	num_args = data_len / 8;

	/* ARG_PTR_TO_CONST_STR guarantees that fmt is zero-terminated so we
	 * can safely give an unbounded size.
	 */
	err = bpf_printf_prepare(fmt, UINT_MAX, data, args, mod, num_args);
	if (err < 0)
		return err;

	/* Maximumly we can have MAX_SNPRINTF_VARARGS parameters, just give
	 * all of them to snprintf().
	 */
	err = snprintf(str, str_size, fmt, BPF_CAST_FMT_ARG(0, args, mod),
		BPF_CAST_FMT_ARG(1, args, mod), BPF_CAST_FMT_ARG(2, args, mod),
		BPF_CAST_FMT_ARG(3, args, mod), BPF_CAST_FMT_ARG(4, args, mod),
		BPF_CAST_FMT_ARG(5, args, mod), BPF_CAST_FMT_ARG(6, args, mod),
		BPF_CAST_FMT_ARG(7, args, mod), BPF_CAST_FMT_ARG(8, args, mod),
		BPF_CAST_FMT_ARG(9, args, mod), BPF_CAST_FMT_ARG(10, args, mod),
		BPF_CAST_FMT_ARG(11, args, mod));

	bpf_printf_cleanup();

	return err + 1;
}

const struct bpf_func_proto bpf_snprintf_proto = {
	.func		= bpf_snprintf,
	.gpl_only	= true,
	.ret_type	= RET_INTEGER,
	.arg1_type	= ARG_PTR_TO_MEM_OR_NULL,
	.arg2_type	= ARG_CONST_SIZE_OR_ZERO,
	.arg3_type	= ARG_PTR_TO_CONST_STR,
	.arg4_type	= ARG_PTR_TO_MEM_OR_NULL,
	.arg5_type	= ARG_CONST_SIZE_OR_ZERO,
};

const struct bpf_func_proto bpf_get_current_task_proto __weak;
const struct bpf_func_proto bpf_probe_read_user_proto __weak;
const struct bpf_func_proto bpf_probe_read_user_str_proto __weak;
const struct bpf_func_proto bpf_probe_read_kernel_proto __weak;
const struct bpf_func_proto bpf_probe_read_kernel_str_proto __weak;
const struct bpf_func_proto bpf_sched_tg_tag_of_proto __weak;
const struct bpf_func_proto bpf_sched_task_tag_of_proto __weak;
const struct bpf_func_proto bpf_sched_set_tg_tag_proto __weak;
const struct bpf_func_proto bpf_sched_set_task_tag_proto __weak;

const struct bpf_func_proto *
bpf_base_func_proto(enum bpf_func_id func_id)
{
	switch (func_id) {
	case BPF_FUNC_map_lookup_elem:
		return &bpf_map_lookup_elem_proto;
	case BPF_FUNC_map_update_elem:
		return &bpf_map_update_elem_proto;
	case BPF_FUNC_map_delete_elem:
		return &bpf_map_delete_elem_proto;
	case BPF_FUNC_map_push_elem:
		return &bpf_map_push_elem_proto;
	case BPF_FUNC_map_pop_elem:
		return &bpf_map_pop_elem_proto;
	case BPF_FUNC_map_peek_elem:
		return &bpf_map_peek_elem_proto;
	case BPF_FUNC_get_prandom_u32:
		return &bpf_get_prandom_u32_proto;
	case BPF_FUNC_get_smp_processor_id:
		return &bpf_get_raw_smp_processor_id_proto;
	case BPF_FUNC_get_numa_node_id:
		return &bpf_get_numa_node_id_proto;
	case BPF_FUNC_tail_call:
		return &bpf_tail_call_proto;
	case BPF_FUNC_ktime_get_ns:
		return &bpf_ktime_get_ns_proto;
	case BPF_FUNC_ktime_get_boot_ns:
		return &bpf_ktime_get_boot_ns_proto;
	case BPF_FUNC_ringbuf_output:
		return &bpf_ringbuf_output_proto;
	case BPF_FUNC_ringbuf_reserve:
		return &bpf_ringbuf_reserve_proto;
	case BPF_FUNC_ringbuf_submit:
		return &bpf_ringbuf_submit_proto;
	case BPF_FUNC_ringbuf_discard:
		return &bpf_ringbuf_discard_proto;
	case BPF_FUNC_ringbuf_query:
		return &bpf_ringbuf_query_proto;
	case BPF_FUNC_sched_tg_tag_of:
		return &bpf_sched_tg_tag_of_proto;
	case BPF_FUNC_sched_task_tag_of:
		return &bpf_sched_task_tag_of_proto;
	case BPF_FUNC_for_each_map_elem:
		return &bpf_for_each_map_elem_proto;
	default:
		break;
	}

	if (!bpf_capable())
		return NULL;

	switch (func_id) {
	case BPF_FUNC_spin_lock:
		return &bpf_spin_lock_proto;
	case BPF_FUNC_spin_unlock:
		return &bpf_spin_unlock_proto;
	case BPF_FUNC_jiffies64:
		return &bpf_jiffies64_proto;
	case BPF_FUNC_per_cpu_ptr:
		return &bpf_per_cpu_ptr_proto;
	case BPF_FUNC_this_cpu_ptr:
		return &bpf_this_cpu_ptr_proto;
	case BPF_FUNC_sched_set_tg_tag:
		return &bpf_sched_set_tg_tag_proto;
	case BPF_FUNC_sched_set_task_tag:
		return &bpf_sched_set_task_tag_proto;
	default:
		break;
	}

	if (!perfmon_capable())
		return NULL;

	switch (func_id) {
	case BPF_FUNC_trace_printk:
		return bpf_get_trace_printk_proto();
	case BPF_FUNC_get_current_task:
		return &bpf_get_current_task_proto;
	case BPF_FUNC_probe_read_user:
		return &bpf_probe_read_user_proto;
	case BPF_FUNC_probe_read_kernel:
		return security_locked_down(LOCKDOWN_BPF_READ) < 0 ?
		       NULL : &bpf_probe_read_kernel_proto;
	case BPF_FUNC_probe_read_user_str:
		return &bpf_probe_read_user_str_proto;
	case BPF_FUNC_probe_read_kernel_str:
		return security_locked_down(LOCKDOWN_BPF_READ) < 0 ?
		       NULL : &bpf_probe_read_kernel_str_proto;
	case BPF_FUNC_snprintf_btf:
		return &bpf_snprintf_btf_proto;
	case BPF_FUNC_snprintf:
		return &bpf_snprintf_proto;
	default:
		return NULL;
	}
}
