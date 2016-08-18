/*
 * Copyright (C) 2010-2012 ARM Limited. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file mali_osk.h
 * Defines the OS abstraction layer for the kernel device driver (OSK)
 */

#ifndef __MALI_OSK_H__
#define __MALI_OSK_H__
#include "mali_osk_types.h"
#ifdef __cplusplus
extern "C"
{
#endif

	/* When DEBUG is enabled, this struct will be used to track owner, mode and order checking */
#ifdef DEBUG
	struct _mali_osk_lock_debug_s {
		u32 owner;
		_mali_osk_lock_flags_t orig_flags;
		_mali_osk_lock_order_t order;
		struct _mali_osk_lock_debug_s *next;
	};
#endif


/**
 * @addtogroup uddapi Unified Device Driver (UDD) APIs
 *
 * @{
 */

/**
 * @addtogroup oskapi UDD OS Abstraction for Kernel-side (OSK) APIs
 *
 * @{
 */

/** @defgroup _mali_osk_miscellaneous OSK Miscellaneous functions, constants and types
 * @{ */

/* Define integer types used by OSK. Note: these currently clash with Linux so we only define them if not defined already */
#ifndef __KERNEL__
	typedef unsigned char      u8;
	typedef signed char        s8;
	typedef unsigned short     u16;
	typedef signed short       s16;
	typedef unsigned int       u32;
	typedef signed int         s32;
	typedef unsigned long long u64;
	#define BITS_PER_LONG (sizeof(long)*8)
#else
	/* Ensure Linux types u32, etc. are defined */
	#include <linux/types.h>
#endif

/** @brief Mali Boolean type which uses MALI_TRUE and MALI_FALSE
  */
	typedef unsigned long mali_bool;

#ifndef MALI_TRUE
	#define MALI_TRUE ((mali_bool)1)
#endif

#ifndef MALI_FALSE
	#define MALI_FALSE ((mali_bool)0)
#endif

#define MALI_HW_CORE_NO_COUNTER     ((u32)-1)



/** @defgroup _mali_osk_wq OSK work queues
 * @{ */

/** @brief Private type for work objects */
typedef struct _mali_osk_wq_work_t_struct _mali_osk_wq_work_t_r3p2;

/** @brief Work queue handler function
 *
 * This function type is called when the work is scheduled by the work queue,
 * e.g. as an IRQ bottom-half handler.
 *
 * Refer to \ref _mali_osk_wq_schedule_work() for more information on the
 * work-queue and work handlers.
 *
 * @param arg resource-specific data
 */
typedef void (*_mali_osk_wq_work_handler_t)( void * arg );

/* @} */ /* end group _mali_osk_wq */

/** @defgroup _mali_osk_irq OSK IRQ handling
 * @{ */

/** @brief Private type for IRQ handling objects */
typedef struct _mali_osk_irq_t_struct _mali_osk_irq_t;

/** @brief Optional function to trigger an irq from a resource
 *
 * This function is implemented by the common layer to allow probing of a resource's IRQ.
 * @param arg resource-specific data */
typedef void  (*_mali_osk_irq_trigger_t)( void * arg );





/** @defgroup _mali_osk_atomic OSK Atomic counters
 * @{ */





/** @brief Private type for Mutual Exclusion lock objects */
typedef struct _mali_osk_lock_t_struct _mali_osk_lock_t;

#ifdef DEBUG
/** @brief Macro for asserting that the current thread holds a given lock
 */
#define MALI_DEBUG_ASSERT_LOCK_HELD(l) MALI_DEBUG_ASSERT(_mali_osk_lock_get_owner(l) == _mali_osk_get_tid());

/** @brief returns a lock's owner (thread id) if debugging is enabled
 */
u32 _mali_osk_lock_get_owner( _mali_osk_lock_t *lock );
#else
#define MALI_DEBUG_ASSERT_LOCK_HELD(l) do {} while(0)
#endif

/** @} */ /* end group _mali_osk_lock */

/** @defgroup _mali_osk_low_level_memory OSK Low-level Memory Operations
 * @{ */

/**
 * @brief Private data type for use in IO accesses to/from devices.
 *
 * This represents some range that is accessible from the device. Examples
 * include:
 * - Device Registers, which could be readable and/or writeable.
 * - Memory that the device has access to, for storing configuration structures.
 *
 * Access to this range must be made through the _mali_osk_mem_ioread32() and
 * _mali_osk_mem_iowrite32() functions.
 */
typedef struct _mali_io_address * mali_io_address;

/** @defgroup _MALI_OSK_CPU_PAGE CPU Physical page size macros.
 *
 * The order of the page size is supplied for
 * ease of use by algorithms that might require it, since it is easier to know
 * it ahead of time rather than calculating it.
 *
 * The Mali Page Mask macro masks off the lower bits of a physical address to
 * give the start address of the page for that physical address.
 *
 * @note The Mali device driver code is designed for systems with 4KB page size.
 * Changing these macros will not make the entire Mali device driver work with
 * page sizes other than 4KB.
 *
 * @note The CPU Physical Page Size has been assumed to be the same as the Mali
 * Physical Page Size.
 *
 * @{
 */

/** CPU Page Order, as log to base 2 of the Page size. @see _MALI_OSK_CPU_PAGE_SIZE */
#define _MALI_OSK_CPU_PAGE_ORDER ((u32)12)
/** CPU Page Size, in bytes.               */
#define _MALI_OSK_CPU_PAGE_SIZE (((u32)1) << (_MALI_OSK_CPU_PAGE_ORDER))
/** CPU Page Mask, which masks off the offset within a page */
#define _MALI_OSK_CPU_PAGE_MASK (~((((u32)1) << (_MALI_OSK_CPU_PAGE_ORDER)) - ((u32)1)))
/** @} */ /* end of group _MALI_OSK_CPU_PAGE */

/** @defgroup _mali_osk_notification OSK Notification Queues
 * @{ */

/** @brief Private type for notification queue objects */
typedef struct _mali_osk_notification_queue_t_struct _mali_osk_notification_queue_t;



/** @defgroup _mali_osk_timer OSK Timer Callbacks
 * @{ */

/** @brief Function to call when a timer expires
 *
 * When a timer expires, this function is called. Note that on many systems,
 * a timer callback will be executed in IRQ context. Therefore, restrictions
 * may apply on what can be done inside the timer callback.
 *
 * If a timer requires more work to be done than can be acheived in an IRQ
 * context, then it may defer the work with a work-queue. For example, it may
 * use \ref _mali_osk_wq_schedule_work() to make use of a bottom-half handler
 * to carry out the remaining work.
 *
 * Stopping the timer with \ref _mali_osk_timer_del() blocks on compeletion of
 * the callback. Therefore, the callback may not obtain any mutexes also held
 * by any callers of _mali_osk_timer_del(). Otherwise, a deadlock may occur.
 *
 * @param arg Function-specific data */
typedef void (*_mali_osk_timer_callback_t)(void * arg );

/** @brief Private type for Timer Callback Objects */
typedef struct _mali_osk_timer_t_struct _mali_osk_timer_t;
/** @} */ /* end group _mali_osk_timer */



/** @brief Initialize a list to be a head of an empty list
 * @param exp the list to initialize. */
#define _MALI_OSK_INIT_LIST_HEAD(exp) _mali_osk_list_init(exp)

/** @brief Find the containing structure of another structure
 *
 * This is the reverse of the operation 'offsetof'. This means that the
 * following condition is satisfied:
 *
 *   ptr == _MALI_OSK_CONTAINER_OF( &ptr->member, type, member )
 *
 * When ptr is of type 'type'.
 *
 * Its purpose it to recover a larger structure that has wrapped a smaller one.
 *
 * @note no type or memory checking occurs to ensure that a wrapper structure
 * does in fact exist, and that it is being recovered with respect to the
 * correct member.
 *
 * @param ptr the pointer to the member that is contained within the larger
 * structure
 * @param type the type of the structure that contains the member
 * @param member the name of the member in the structure that ptr points to.
 * @return a pointer to a \a type object which contains \a member, as pointed
 * to by \a ptr.
 */
#define _MALI_OSK_CONTAINER_OF(ptr, type, member) \
             ((type *)( ((char *)ptr) - offsetof(type,member) ))

/** @brief Find the containing structure of a list
 *
 * When traversing a list, this is used to recover the containing structure,
 * given that is contains a _mali_osk_list_t member.
 *
 * Each list must be of structures of one type, and must link the same members
 * together, otherwise it will not be possible to correctly recover the
 * sturctures that the lists link.
 *
 * @note no type or memory checking occurs to ensure that a structure does in
 * fact exist for the list entry, and that it is being recovered with respect
 * to the correct list member.
 *
 * @param ptr the pointer to the _mali_osk_list_t member in this structure
 * @param type the type of the structure that contains the member
 * @param member the member of the structure that ptr points to.
 * @return a pointer to a \a type object which contains the _mali_osk_list_t
 * \a member, as pointed to by the _mali_osk_list_t \a *ptr.
 */
#define _MALI_OSK_LIST_ENTRY(ptr, type, member) \
            _MALI_OSK_CONTAINER_OF(ptr, type, member)

/** @brief Enumerate a list safely
 *
 * With this macro, lists can be enumerated in a 'safe' manner. That is,
 * entries can be deleted from the list without causing an error during
 * enumeration. To achieve this, a 'temporary' pointer is required, which must
 * be provided to the macro.
 *
 * Use it like a 'for()', 'while()' or 'do()' construct, and so it must be
 * followed by a statement or compound-statement which will be executed for
 * each list entry.
 *
 * Upon loop completion, providing that an early out was not taken in the
 * loop body, then it is guaranteed that ptr->member == list, even if the loop
 * body never executed.
 *
 * @param ptr a pointer to an object of type 'type', which points to the
 * structure that contains the currently enumerated list entry.
 * @param tmp a pointer to an object of type 'type', which must not be used
 * inside the list-execution statement.
 * @param list a pointer to a _mali_osk_list_t, from which enumeration will
 * begin
 * @param type the type of the structure that contains the _mali_osk_list_t
 * member that is part of the list to be enumerated.
 * @param member the _mali_osk_list_t member of the structure that is part of
 * the list to be enumerated.
 */
#define _MALI_OSK_LIST_FOREACHENTRY(ptr, tmp, list, type, member)         \
        for (ptr = _MALI_OSK_LIST_ENTRY((list)->next, type, member),      \
             tmp = _MALI_OSK_LIST_ENTRY(ptr->member.next, type, member); \
             &ptr->member != (list);                                    \
             ptr = tmp, tmp = _MALI_OSK_LIST_ENTRY(tmp->member.next, type, member))
/** @} */ /* end group _mali_osk_list */








/** @brief Terminate work queues (for deferred work)
 */
void _mali_osk_wq_term(void);

/** @brief Create work in the work queue
 *
 * Creates a work object which can be scheduled in the work queue. When
 * scheduled, \a handler will be called with \a data as the argument.
 *
 * Refer to \ref _mali_osk_wq_schedule_work() for details on how work
 * is scheduled in the queue.
 *
 * The returned pointer must be freed with \ref _mali_osk_wq_delete_work()
 * when no longer needed.
 */
_mali_osk_wq_work_t_r3p2 *_mali_osk_wq_create_work( _mali_osk_wq_work_handler_t handler, void *data );

/** @brief Delete a work object
 *
 * This will flush the work queue to ensure that the work handler will not
 * be called after deletion.
 */
void _mali_osk_wq_delete_work( _mali_osk_wq_work_t_r3p2 *work );

/** @brief Delete a work object
 *
 * This will NOT flush the work queue, so only call this if you are sure that the work handler will
 * not be called after deletion.
 */
void _mali_osk_wq_delete_work_nonflush( _mali_osk_wq_work_t_r3p2 *work );

/** @brief Cause a queued, deferred call of the work handler
 *
 * _mali_osk_wq_schedule_work provides a mechanism for enqueuing deferred calls
 * to the work handler. After calling \ref _mali_osk_wq_schedule_work(), the
 * work handler will be scheduled to run at some point in the future.
 *
 * Typically this is called by the IRQ upper-half to defer further processing of
 * IRQ-related work to the IRQ bottom-half handler. This is necessary for work
 * that cannot be done in an IRQ context by the IRQ upper-half handler. Timer
 * callbacks also use this mechanism, because they are treated as though they
 * operate in an IRQ context. Refer to \ref _mali_osk_timer_t for more
 * information.
 *
 * Code that operates in a kernel-process context (with no IRQ context
 * restrictions) may also enqueue deferred calls to the IRQ bottom-half. The
 * advantage over direct calling is that deferred calling allows the caller and
 * IRQ bottom half to hold the same mutex, with a guarantee that they will not
 * deadlock just by using this mechanism.
 *
 * _mali_osk_wq_schedule_work() places deferred call requests on a queue, to
 * allow for more than one thread to make a deferred call. Therfore, if it is
 * called 'K' times, then the IRQ bottom-half will be scheduled 'K' times too.
 * 'K' is a number that is implementation-specific.
 *
 * _mali_osk_wq_schedule_work() is guaranteed to not block on:
 * - enqueuing a deferred call request.
 * - the completion of the work handler.
 *
 * This is to prevent deadlock. For example, if _mali_osk_wq_schedule_work()
 * blocked, then it would cause a deadlock when the following two conditions
 * hold:
 * - The work handler callback (of type _mali_osk_wq_work_handler_t) locks
 * a mutex
 * - And, at the same time, the caller of _mali_osk_wq_schedule_work() also
 * holds the same mutex
 *
 * @note care must be taken to not overflow the queue that
 * _mali_osk_wq_schedule_work() operates on. Code must be structured to
 * ensure that the number of requests made to the queue is bounded. Otherwise,
 * work will be lost.
 *
 * The queue that _mali_osk_wq_schedule_work implements is a FIFO of N-writer,
 * 1-reader type. The writers are the callers of _mali_osk_wq_schedule_work
 * (all OSK-registered IRQ upper-half handlers in the system, watchdog timers,
 * callers from a Kernel-process context). The reader is a single thread that
 * handles all OSK-registered work.
 *
 * @param work a pointer to the _mali_osk_wq_work_t object corresponding to the
 * work to begin processing.
 */
void _mali_osk_wq_schedule_work( _mali_osk_wq_work_t_r3p2 *work );

/** @brief Flush the work queue
 *
 * This will flush the OSK work queue, ensuring all work in the queue has
 * completed before returning.
 *
 * Since this blocks on the completion of work in the work-queue, the
 * caller of this function \b must \b not hold any mutexes that are taken by
 * any registered work handler. To do so may cause a deadlock.
 *
 */
void _mali_osk_wq_flush(void);


/** @} */ /* end group _mali_osk_wq */

/** @addtogroup _mali_osk_irq
 * @{ */


/** @brief Terminate IRQ handling on a resource.
 *
 * This will disable the interrupt from the device, and then waits for any
 * currently executing IRQ handlers to complete.
 *
 * @note If work is deferred to an IRQ bottom-half handler through
 * \ref _mali_osk_wq_schedule_work(), be sure to flush any remaining work
 * with \ref _mali_osk_wq_flush() or (implicitly) with \ref _mali_osk_wq_delete_work()
 *
 * @param irq a pointer to the _mali_osk_irq_t object corresponding to the
 * resource whose IRQ handling is to be terminated.
 */
void _mali_osk_irq_term( _mali_osk_irq_t *irq );

/** @} */ /* end group _mali_osk_irq */



/** @defgroup _mali_osk_memory OSK Memory Allocation
 * @{ */

/** @brief Allocate zero-initialized memory.
 *
 * Returns a buffer capable of containing at least \a n elements of \a size
 * bytes each. The buffer is initialized to zero.
 *
 * If there is a need for a bigger block of memory (16KB or bigger), then
 * consider to use _mali_osk_vmalloc() instead, as this function might
 * map down to a OS function with size limitations.
 *
 * The buffer is suitably aligned for storage and subsequent access of every
 * type that the compiler supports. Therefore, the pointer to the start of the
 * buffer may be cast into any pointer type, and be subsequently accessed from
 * such a pointer, without loss of information.
 *
 * When the buffer is no longer in use, it must be freed with _mali_osk_free().
 * Failure to do so will cause a memory leak.
 *
 * @note Most toolchains supply memory allocation functions that meet the
 * compiler's alignment requirements.
 *
 * @param n Number of elements to allocate
 * @param size Size of each element
 * @return On success, the zero-initialized buffer allocated. NULL on failure
 */
void *_mali_osk_calloc( u32 n, u32 size );

/** @brief Allocate memory.
 *
 * Returns a buffer capable of containing at least \a size bytes. The
 * contents of the buffer are undefined.
 *
 * If there is a need for a bigger block of memory (16KB or bigger), then
 * consider to use _mali_osk_vmalloc() instead, as this function might
 * map down to a OS function with size limitations.
 *
 * The buffer is suitably aligned for storage and subsequent access of every
 * type that the compiler supports. Therefore, the pointer to the start of the
 * buffer may be cast into any pointer type, and be subsequently accessed from
 * such a pointer, without loss of information.
 *
 * When the buffer is no longer in use, it must be freed with _mali_osk_free().
 * Failure to do so will cause a memory leak.
 *
 * @note Most toolchains supply memory allocation functions that meet the
 * compiler's alignment requirements.
 *
 * Remember to free memory using _mali_osk_free().
 * @param size Number of bytes to allocate
 * @return On success, the buffer allocated. NULL on failure.
 */
void *_mali_osk_malloc( u32 size );

/** @brief Free memory.
 *
 * Reclaims the buffer pointed to by the parameter \a ptr for the system.
 * All memory returned from _mali_osk_malloc() and _mali_osk_calloc()
 * must be freed before the application exits. Otherwise,
 * a memory leak will occur.
 *
 * Memory must be freed once. It is an error to free the same non-NULL pointer
 * more than once.
 *
 * It is legal to free the NULL pointer.
 *
 * @param ptr Pointer to buffer to free
 */
void _mali_osk_free( void *ptr );

/** @brief Allocate memory.
 *
 * Returns a buffer capable of containing at least \a size bytes. The
 * contents of the buffer are undefined.
 *
 * This function is potentially slower than _mali_osk_malloc() and _mali_osk_calloc(),
 * but do support bigger sizes.
 *
 * The buffer is suitably aligned for storage and subsequent access of every
 * type that the compiler supports. Therefore, the pointer to the start of the
 * buffer may be cast into any pointer type, and be subsequently accessed from
 * such a pointer, without loss of information.
 *
 * When the buffer is no longer in use, it must be freed with _mali_osk_free().
 * Failure to do so will cause a memory leak.
 *
 * @note Most toolchains supply memory allocation functions that meet the
 * compiler's alignment requirements.
 *
 * Remember to free memory using _mali_osk_free().
 * @param size Number of bytes to allocate
 * @return On success, the buffer allocated. NULL on failure.
 */
void *_mali_osk_valloc( u32 size );

/** @brief Free memory.
 *
 * Reclaims the buffer pointed to by the parameter \a ptr for the system.
 * All memory returned from _mali_osk_valloc() must be freed before the
 * application exits. Otherwise a memory leak will occur.
 *
 * Memory must be freed once. It is an error to free the same non-NULL pointer
 * more than once.
 *
 * It is legal to free the NULL pointer.
 *
 * @param ptr Pointer to buffer to free
 */
void _mali_osk_vfree( void *ptr );

/** @brief Copies memory.
 *
 * Copies the \a len bytes from the buffer pointed by the parameter \a src
 * directly to the buffer pointed by \a dst.
 *
 * It is an error for \a src to overlap \a dst anywhere in \a len bytes.
 *
 * @param dst Pointer to the destination array where the content is to be
 * copied.
 * @param src Pointer to the source of data to be copied.
 * @param len Number of bytes to copy.
 * @return \a dst is always passed through unmodified.
 */
void *_mali_osk_memcpy( void *dst, const void *src, u32 len );

/** @brief Fills memory.
 *
 * Sets the first \a n bytes of the block of memory pointed to by \a s to
 * the specified value
 * @param s Pointer to the block of memory to fill.
 * @param c Value to be set, passed as u32. Only the 8 Least Significant Bits (LSB)
 * are used.
 * @param n Number of bytes to be set to the value.
 * @return \a s is always passed through unmodified
 */
void *_mali_osk_memset( void *s, u32 c, u32 n );
/** @} */ /* end group _mali_osk_memory */


/** @brief Checks the amount of memory allocated
 *
 * Checks that not more than \a max_allocated bytes are allocated.
 *
 * Some OS bring up an interactive out of memory dialogue when the
 * system runs out of memory. This can stall non-interactive
 * apps (e.g. automated test runs). This function can be used to
 * not trigger the OOM dialogue by keeping allocations
 * within a certain limit.
 *
 * @return MALI_TRUE when \a max_allocated bytes are not in use yet. MALI_FALSE
 * when at least \a max_allocated bytes are in use.
 */
mali_bool _mali_osk_mem_check_allocated( u32 max_allocated );

/** @brief Terminate a lock
 *
 * This terminates a lock and frees all associated resources.
 *
 * It is a programming error to terminate the lock when it is held (unsignalled)
 * by a thread.
 *
 * @param lock the lock to terminate.
 */
void _mali_osk_lock_term( _mali_osk_lock_t *lock );
/** @} */ /* end group _mali_osk_lock */


/** @addtogroup _mali_osk_low_level_memory
 * @{ */

/** @brief Issue a memory barrier
 *
 * This defines an arbitrary memory barrier operation, which forces an ordering constraint
 * on memory read and write operations.
 */
void _mali_osk_mem_barrier( void );

/** @brief Issue a write memory barrier
 *
 * This defines an write memory barrier operation which forces an ordering constraint
 * on memory write operations.
 */
void _mali_osk_write_mem_barrier( void );

/** @brief Map a physically contiguous region into kernel space
 *
 * This is primarily used for mapping in registers from resources, and Mali-MMU
 * page tables. The mapping is only visable from kernel-space.
 *
 * Access has to go through _mali_osk_mem_ioread32 and _mali_osk_mem_iowrite32
 *
 * @param phys CPU-physical base address of the memory to map in. This must
 * be aligned to the system's page size, which is assumed to be 4K.
 * @param size the number of bytes of physically contiguous address space to
 * map in
 * @param description A textual description of the memory being mapped in.
 * @return On success, a Mali IO address through which the mapped-in
 * memory/registers can be accessed. NULL on failure.
 */
mali_io_address _mali_osk_mem_mapioregion( u32 phys, u32 size, const char *description );

/** @brief Unmap a physically contiguous address range from kernel space.
 *
 * The address range should be one previously mapped in through
 * _mali_osk_mem_mapioregion.
 *
 * It is a programming error to do (but not limited to) the following:
 * - attempt an unmap twice
 * - unmap only part of a range obtained through _mali_osk_mem_mapioregion
 * - unmap more than the range obtained through  _mali_osk_mem_mapioregion
 * - unmap an address range that was not successfully mapped using
 * _mali_osk_mem_mapioregion
 * - provide a mapping that does not map to phys.
 *
 * @param phys CPU-physical base address of the memory that was originally
 * mapped in. This must be aligned to the system's page size, which is assumed
 * to be 4K
 * @param size The number of bytes that were originally mapped in.
 * @param mapping The Mali IO address through which the mapping is
 * accessed.
 */
void _mali_osk_mem_unmapioregion( u32 phys, u32 size, mali_io_address mapping );

/** @brief Allocate and Map a physically contiguous region into kernel space
 *
 * This is used for allocating physically contiguous regions (such as Mali-MMU
 * page tables) and mapping them into kernel space. The mapping is only
 * visible from kernel-space.
 *
 * The alignment of the returned memory is guaranteed to be at least
 * _MALI_OSK_CPU_PAGE_SIZE.
 *
 * Access must go through _mali_osk_mem_ioread32 and _mali_osk_mem_iowrite32
 *
 * @note This function is primarily to provide support for OSs that are
 * incapable of separating the tasks 'allocate physically contiguous memory'
 * and 'map it into kernel space'
 *
 * @param[out] phys CPU-physical base address of memory that was allocated.
 * (*phys) will be guaranteed to be aligned to at least
 * _MALI_OSK_CPU_PAGE_SIZE on success.
 *
 * @param[in] size the number of bytes of physically contiguous memory to
 * allocate. This must be a multiple of _MALI_OSK_CPU_PAGE_SIZE.
 *
 * @return On success, a Mali IO address through which the mapped-in
 * memory/registers can be accessed. NULL on failure, and (*phys) is unmodified.
 */
mali_io_address _mali_osk_mem_allocioregion( u32 *phys, u32 size );

/** @brief Free a physically contiguous address range from kernel space.
 *
 * The address range should be one previously mapped in through
 * _mali_osk_mem_allocioregion.
 *
 * It is a programming error to do (but not limited to) the following:
 * - attempt a free twice on the same ioregion
 * - free only part of a range obtained through _mali_osk_mem_allocioregion
 * - free more than the range obtained through  _mali_osk_mem_allocioregion
 * - free an address range that was not successfully mapped using
 * _mali_osk_mem_allocioregion
 * - provide a mapping that does not map to phys.
 *
 * @param phys CPU-physical base address of the memory that was originally
 * mapped in, which was aligned to _MALI_OSK_CPU_PAGE_SIZE.
 * @param size The number of bytes that were originally mapped in, which was
 * a multiple of _MALI_OSK_CPU_PAGE_SIZE.
 * @param mapping The Mali IO address through which the mapping is
 * accessed.
 */
void _mali_osk_mem_freeioregion( u32 phys, u32 size, mali_io_address mapping );

/** @brief Un-request a region of physically contiguous memory
 *
 * This is used to release a regious of physically contiguous memory previously
 * requested through _mali_osk_mem_reqregion, so that other device drivers may
 * use it. This will be called at time of Mali device driver termination.
 *
 * It is a programming error to attempt to:
 * - unrequest a region twice
 * - unrequest only part of a range obtained through _mali_osk_mem_reqregion
 * - unrequest more than the range obtained through  _mali_osk_mem_reqregion
 * - unrequest an address range that was not successfully requested using
 * _mali_osk_mem_reqregion
 *
 * @param phys CPU-physical base address of the memory to un-request. This must
 * be aligned to the system's page size, which is assumed to be 4K
 * @param size the number of bytes of physically contiguous address space to
 * un-request.
 */
void _mali_osk_mem_unreqregion( u32 phys, u32 size );

/** @brief Read from a location currently mapped in through
 * _mali_osk_mem_mapioregion
 *
 * This reads a 32-bit word from a 32-bit aligned location. It is a programming
 * error to provide unaligned locations, or to read from memory that is not
 * mapped in, or not mapped through either _mali_osk_mem_mapioregion() or
 * _mali_osk_mem_allocioregion().
 *
 * @param mapping Mali IO address to read from
 * @param offset Byte offset from the given IO address to operate on, must be a multiple of 4
 * @return the 32-bit word from the specified location.
 */
u32 _mali_osk_mem_ioread32( volatile mali_io_address mapping, u32 offset );

/** @brief Write to a location currently mapped in through
 * _mali_osk_mem_mapioregion without memory barriers
 *
 * This write a 32-bit word to a 32-bit aligned location without using memory barrier.
 * It is a programming error to provide unaligned locations, or to write to memory that is not
 * mapped in, or not mapped through either _mali_osk_mem_mapioregion() or
 * _mali_osk_mem_allocioregion().
 *
 * @param mapping Mali IO address to write to
 * @param offset Byte offset from the given IO address to operate on, must be a multiple of 4
 * @param val the 32-bit word to write.
 */
void _mali_osk_mem_iowrite32_relaxed( volatile mali_io_address addr, u32 offset, u32 val );

/** @brief Write to a location currently mapped in through
 * _mali_osk_mem_mapioregion with write memory barrier
 *
 * This write a 32-bit word to a 32-bit aligned location. It is a programming
 * error to provide unaligned locations, or to write to memory that is not
 * mapped in, or not mapped through either _mali_osk_mem_mapioregion() or
 * _mali_osk_mem_allocioregion().
 *
 * @param mapping Mali IO address to write to
 * @param offset Byte offset from the given IO address to operate on, must be a multiple of 4
 * @param val the 32-bit word to write.
 */
void _mali_osk_mem_iowrite32( volatile mali_io_address mapping, u32 offset, u32 val );

/** @brief Flush all CPU caches
 *
 * This should only be implemented if flushing of the cache is required for
 * memory mapped in through _mali_osk_mem_mapregion.
 */
void _mali_osk_cache_flushall( void );

/** @brief Flush any caches necessary for the CPU and MALI to have the same view of a range of uncached mapped memory
 *
 * This should only be implemented if your OS doesn't do a full cache flush (inner & outer)
 * after allocating uncached mapped memory.
 *
 * Some OS do not perform a full cache flush (including all outer caches) for uncached mapped memory.
 * They zero the memory through a cached mapping, then flush the inner caches but not the outer caches.
 * This is required for MALI to have the correct view of the memory.
 */
void _mali_osk_cache_ensure_uncached_range_flushed( void *uncached_mapping, u32 offset, u32 size );

/** @} */ /* end group _mali_osk_low_level_memory */

/** @brief Create a notification queue
 *
 * Creates a notification queue which can be used to queue messages for user
 * delivery and get queued messages from
 *
 * The queue is a FIFO, and has no restrictions on the numbers of readers or
 * writers.
 *
 * When the queue is no longer in use, it must be terminated with
 * \ref _mali_osk_notification_queue_term(). Failure to do so will result in a
 * memory leak.
 *
 * @return Pointer to a new notification queue or NULL on error.
 */
_mali_osk_notification_queue_t *_mali_osk_notification_queue_init( void );

/** @brief Destroy a notification queue
 *
 * Destroys a notification queue and frees associated resources from the queue.
 *
 * A notification queue \b must \b not be destroyed in the following cases:
 * - while there are \ref _mali_osk_notification_t objects in the queue.
 * - while there are writers currently acting upon the queue. That is, while
 * a thread is currently calling \ref _mali_osk_notification_queue_send() on
 * the queue, or while a thread may call
 * \ref _mali_osk_notification_queue_send() on the queue in the future.
 * - while there are readers currently waiting upon the queue. That is, while
 * a thread is currently calling \ref _mali_osk_notification_queue_receive() on
 * the queue, or while a thread may call
 * \ref _mali_osk_notification_queue_receive() on the queue in the future.
 *
 * Therefore, all \ref _mali_osk_notification_t objects must be flushed and
 * deleted by the code that makes use of the notification queues, since only
 * they know the structure of the _mali_osk_notification_t::result_buffer
 * (even if it may only be a flat sturcture).
 *
 * @note Since the queue is a FIFO, the code using notification queues may
 * create its own 'flush' type of notification, to assist in flushing the
 * queue.
 *
 * Once the queue has been destroyed, it must not be used again.
 *
 * @param queue The queue to destroy
 */
void _mali_osk_notification_queue_term( _mali_osk_notification_queue_t *queue );



/** @} */ /* end group _mali_osk_notification */


/** @addtogroup _mali_osk_timer
 *
 * Timers use the OS's representation of time, which are 'ticks'. This is to
 * prevent aliasing problems between the internal timer time, and the time
 * asked for.
 *
 * @{ */

/** @brief Initialize a timer
 *
 * Allocates resources for a new timer, and initializes them. This does not
 * start the timer.
 *
 * @return a pointer to the allocated timer object, or NULL on failure.
 */
_mali_osk_timer_t *_mali_osk_timer_init(void);

/** @brief Start a timer
 *
 * It is an error to start a timer without setting the callback via
 * _mali_osk_timer_setcallback().
 *
 * It is an error to use this to start an already started timer.
 *
 * The timer will expire in \a ticks_to_expire ticks, at which point, the
 * callback function will be invoked with the callback-specific data,
 * as registered by _mali_osk_timer_setcallback().
 *
 * @param tim the timer to start
 * @param ticks_to_expire the amount of time in ticks for the timer to run
 * before triggering.
 */
void _mali_osk_timer_add( _mali_osk_timer_t *tim, u32 ticks_to_expire );

/** @brief Modify a timer
 *
 * Set the relative time at which a timer will expire, and start it if it is
 * stopped. If \a ticks_to_expire 0 the timer fires immediately.
 *
 * It is an error to modify a timer without setting the callback via
 *  _mali_osk_timer_setcallback().
 *
 * The timer will expire at \a ticks_to_expire from the time of the call, at
 * which point, the callback function will be invoked with the
 * callback-specific data, as set by _mali_osk_timer_setcallback().
 *
 * @param tim the timer to modify, and start if necessary
 * @param ticks_to_expire the \em absolute time in ticks at which this timer
 * should trigger.
 *
 */
void _mali_osk_timer_mod( _mali_osk_timer_t *tim, u32 ticks_to_expire);

/** @brief Stop a timer, and block on its completion.
 *
 * Stop the timer. When the function returns, it is guaranteed that the timer's
 * callback will not be running on any CPU core.
 *
 * Since stoping the timer blocks on compeletion of the callback, the callback
 * may not obtain any mutexes that the caller holds. Otherwise, a deadlock will
 * occur.
 *
 * @note While the callback itself is guaranteed to not be running, work
 * enqueued on the work-queue by the timer (with
 * \ref _mali_osk_wq_schedule_work()) may still run. The timer callback and
 * work handler must take this into account.
 *
 * It is legal to stop an already stopped timer.
 *
 * @param tim the timer to stop.
 *
 */
void _mali_osk_timer_del( _mali_osk_timer_t *tim );

/** @brief Stop a timer.
 *
 * Stop the timer. When the function returns, the timer's callback may still be
 * running on any CPU core.
 *
 * It is legal to stop an already stopped timer.
 *
 * @param tim the timer to stop.
 */
void _mali_osk_timer_del_async( _mali_osk_timer_t *tim );

/** @brief Check if timer is pending.
 *
 * Check if timer is active.
 *
 * @param tim the timer to check
 * @return MALI_TRUE if time is active, MALI_FALSE if it is not active
 */
mali_bool _mali_osk_timer_pending( _mali_osk_timer_t *tim);

/** @brief Set a timer's callback parameters.
 *
 * This must be called at least once before a timer is started/modified.
 *
 * After a timer has been stopped or expires, the callback remains set. This
 * means that restarting the timer will call the same function with the same
 * parameters on expiry.
 *
 * @param tim the timer to set callback on.
 * @param callback Function to call when timer expires
 * @param data Function-specific data to supply to the function on expiry.
 */
void _mali_osk_timer_setcallback( _mali_osk_timer_t *tim, _mali_osk_timer_callback_t callback, void *data );

/** @brief Terminate a timer, and deallocate resources.
 *
 * The timer must first be stopped by calling _mali_osk_timer_del().
 *
 * It is a programming error for _mali_osk_timer_term() to be called on:
 * - timer that is currently running
 * - a timer that is currently executing its callback.
 *
 * @param tim the timer to deallocate.
 */
void _mali_osk_timer_term( _mali_osk_timer_t *tim );
/** @} */ /* end group _mali_osk_timer */


/** @defgroup _mali_osk_time OSK Time functions
 *
 * \ref _mali_osk_time use the OS's representation of time, which are
 * 'ticks'. This is to prevent aliasing problems between the internal timer
 * time, and the time asked for.
 *
 * OS tick time is measured as a u32. The time stored in a u32 may either be
 * an absolute time, or a time delta between two events. Whilst it is valid to
 * use math opeartors to \em change the tick value represented as a u32, it
 * is often only meaningful to do such operations on time deltas, rather than
 * on absolute time. However, it is meaningful to add/subtract time deltas to
 * absolute times.
 *
 * Conversion between tick time and milliseconds (ms) may not be loss-less,
 * and are \em implementation \em depenedant.
 *
 * Code use OS time must take this into account, since:
 * - a small OS time may (or may not) be rounded
 * - a large time may (or may not) overflow
 *
 * @{ */

/** @brief Return whether ticka occurs after tickb
 *
 * Some OSs handle tick 'rollover' specially, and so can be more robust against
 * tick counters rolling-over. This function must therefore be called to
 * determine if a time (in ticks) really occurs after another time (in ticks).
 *
 * @param ticka ticka
 * @param tickb tickb
 * @return non-zero if ticka represents a time that occurs after tickb.
 * Zero otherwise.
 */
int	_mali_osk_time_after( u32 ticka, u32 tickb );

/** @brief Convert milliseconds to OS 'ticks'
 *
 * @param ms time interval in milliseconds
 * @return the corresponding time interval in OS ticks.
 */
u32	_mali_osk_time_mstoticks( u32 ms );

/** @brief Convert OS 'ticks' to milliseconds
 *
 * @param ticks time interval in OS ticks.
 * @return the corresponding time interval in milliseconds
 */
u32	_mali_osk_time_tickstoms( u32 ticks );


/** @brief Get the current time in OS 'ticks'.
 * @return the current time in OS 'ticks'.
 */
u32	_mali_osk_time_tickcount( void );

/** @brief Cause a microsecond delay
 *
 * The delay will have microsecond resolution, and is necessary for correct
 * operation of the driver. At worst, the delay will be \b at least \a usecs
 * microseconds, and so may be (significantly) more.
 *
 * This function may be implemented as a busy-wait, which is the most sensible
 * implementation. On OSs where there are situations in which a thread must not
 * sleep, this is definitely implemented as a busy-wait.
 *
 * @param usecs the number of microseconds to wait for.
 */
void _mali_osk_time_ubusydelay( u32 usecs );

/** @brief Return time in nano seconds, since any given reference.
 *
 * @return Time in nano seconds
 */
u64 _mali_osk_time_get_ns( void );


/** @} */ /* end group _mali_osk_time */

/** @defgroup _mali_osk_math OSK Math
 * @{ */

/** @brief Count Leading Zeros (Little-endian)
 *
 * @note This function must be implemented to support the reference
 * implementation of _mali_osk_find_first_zero_bit, as defined in
 * mali_osk_bitops.h.
 *
 * @param val 32-bit words to count leading zeros on
 * @return the number of leading zeros.
 */
u32 _mali_osk_clz( u32 val );
/** @} */ /* end group _mali_osk_math */

/** @defgroup _mali_osk_wait_queue OSK Wait Queue functionality
 * @{ */
/** @brief Private type for wait queue objects */
typedef struct _mali_osk_wait_queue_t_struct _mali_osk_wait_queue_t;

/** @brief Initialize an empty Wait Queue */
_mali_osk_wait_queue_t* _mali_osk_wait_queue_init( void );

/** @brief Sleep  if condition is false
 *
 * @param queue the queue to use
 * @param condition function pointer to a boolean function
 *
 * Put thread to sleep if the given \a codition function returns false. When
 * being asked to wake up again, the condition will be re-checked and the
 * thread only woken up if the condition is now true.
 */
void _mali_osk_wait_queue_wait_event( _mali_osk_wait_queue_t *queue, mali_bool (*condition)(void) );

/** @brief Wake up all threads in wait queue if their respective conditions are
 * true
 *
 * @param queue the queue whose threads should be woken up
 *
 * Wake up all threads in wait queue \a queue whose condition is now true.
 */
void _mali_osk_wait_queue_wake_up( _mali_osk_wait_queue_t *queue );

/** @brief terminate a wait queue
 *
 * @param queue the queue to terminate.
 */
void _mali_osk_wait_queue_term( _mali_osk_wait_queue_t *queue );
/** @} */ /* end group _mali_osk_wait_queue */


/** @addtogroup _mali_osk_miscellaneous
 * @{ */

/** @brief Output a device driver debug message.
 *
 * The interpretation of \a fmt is the same as the \c format parameter in
 * _mali_osu_vsnprintf().
 *
 * @param fmt a _mali_osu_vsnprintf() style format string
 * @param ... a variable-number of parameters suitable for \a fmt
 */
void _mali_osk_dbgmsg( const char *fmt, ... );

/** @brief Print fmt into buf.
 *
 * The interpretation of \a fmt is the same as the \c format parameter in
 * _mali_osu_vsnprintf().
 *
 * @param buf a pointer to the result buffer
 * @param size the total number of bytes allowed to write to \a buf
 * @param fmt a _mali_osu_vsnprintf() style format string
 * @param ... a variable-number of parameters suitable for \a fmt
 * @return The number of bytes written to \a buf
 */
u32 _mali_osk_snprintf( char *buf, u32 size, const char *fmt, ... );

/** @brief Abnormal process abort.
 *
 * Terminates the caller-process if this function is called.
 *
 * This function will be called from Debug assert-macros in mali_kernel_common.h.
 *
 * This function will never return - because to continue from a Debug assert
 * could cause even more problems, and hinder debugging of the initial problem.
 *
 * This function is only used in Debug builds, and is not used in Release builds.
 */
void _mali_osk_abort(void);

/** @brief Sets breakpoint at point where function is called.
 *
 * This function will be called from Debug assert-macros in mali_kernel_common.h,
 * to assist in debugging. If debugging at this level is not required, then this
 * function may be implemented as a stub.
 *
 * This function is only used in Debug builds, and is not used in Release builds.
 */
void _mali_osk_break(void);

/** @brief Return an identificator for calling process.
 *
 * @return Identificator for calling process.
 */
u32 _mali_osk_get_pid(void);

/** @brief Return an identificator for calling thread.
 *
 * @return Identificator for calling thread.
 */
u32 _mali_osk_get_tid(void);

/** @brief Enable OS controlled runtime power management
 */
void _mali_osk_pm_dev_enable(void);

/** @brief Disable OS controlled runtime power management
 */
void _mali_osk_pm_dev_disable(void);




/** @brief Release the reference to the power manger system for the Mali device.
 *
 * When reference count reach zero, the cores can be off.
 *
 * @note This must be used to release references taken with \a _mali_osk_pm_dev_ref_add().
 */
void _mali_osk_pm_dev_ref_dec(void);


/** @brief Take a reference to the power manager system for the Mali device.
 *
 * Will leave the cores powered off if they are already powered off.
 *
 * @note Call \a _mali_osk_pm_dev_ref_dec() to release this reference.
 *
 * @return MALI_TRUE if the Mali GPU is powered on, otherwise MALI_FALSE.
 */
mali_bool _mali_osk_pm_dev_ref_add_no_power_on(void);


/** @brief Releasing the reference to the power manger system for the Mali device.
 *
 * When reference count reach zero, the cores can be off.
 *
 * @note This must be used to release references taken with \a _mali_osk_pm_dev_ref_add_no_power_on().
 */
void _mali_osk_pm_dev_ref_dec_no_power_on(void);

/** @brief Block untill pending PM operations are done
 */
void _mali_osk_pm_dev_barrier(void);

/** @} */ /* end group  _mali_osk_miscellaneous */

/** @} */ /* end group osuapi */

/** @} */ /* end group uddapi */

#ifdef __cplusplus
}
#endif

#include "mali_osk_specific.h"           /* include any per-os specifics */

/* Check standard inlines */
#ifndef MALI_STATIC_INLINE
	#error MALI_STATIC_INLINE not defined on your OS
#endif

#ifndef MALI_NON_STATIC_INLINE
	#error MALI_NON_STATIC_INLINE not defined on your OS
#endif

#endif /* __MALI_OSK_H__ */
