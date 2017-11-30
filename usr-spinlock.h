#ifndef _USR_SPINLOCK_H
#define _USR_SPINLOCK_H
#endif

typedef struct {
    volatile unsigned int lock;
    volatile pid_t thread_id;
} usr_spinlock;

#define SPINLOCK_ATTR static __inline __attribute__((always_inline, no_instrument_function))

/* Pause instruction to prevent excess processor bus usage */
#define cpu_relax() asm volatile("pause\n": : :"memory")

SPINLOCK_ATTR char __usrtestandset(usr_spinlock *p, long old_val, long new_val)
{
    char result = 0;
    asm volatile (
            "lock; cmpxchgq %4, %1; sete %0"
            : "=q" (result), "=m" (*p)
            : "m" (*p), "a" (old_val), "r" (new_val)
            : "memory");
    return (!result);
}

SPINLOCK_ATTR void usr_spin_lock(usr_spinlock *lock)
{
    long tid = syscall(__NR_gettid);
    tid = (tid << 32) + 1;
    while (__usrtestandset(lock, 0, tid)) {
        cpu_relax();
    }
}

SPINLOCK_ATTR char usr_spin_trylock(usr_spinlock *lock)
{
    long tid = syscall(__NR_gettid);
    tid = (tid << 32) + 1;
    return __usrtestandset(lock, 0, tid);
}

SPINLOCK_ATTR void usr_spin_unlock(usr_spinlock *s)
{
    long new_val = 0;
    asm volatile ("xchgq %0,%1"
                :
                :"m" (*s), "r" (new_val)
                :"memory");
}

SPINLOCK_ATTR void usr_spin_init(usr_spinlock *s)
{
    usr_spin_unlock(s); 
}

#define USR_SPINLOCK_INITIALIZER { 0 }
