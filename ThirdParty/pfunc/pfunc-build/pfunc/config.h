/** Do we have errno.h */
#define PFUNC_HAVE_ERRNO_H 1

/** Do we have limits.h */
#define PFUNC_HAVE_LIMITS_H 1

/** Do we have syscall.h */
#define PFUNC_HAVE_SYSCALL_H 1

/** Do we have unistd.h */
#define PFUNC_HAVE_UNISTD_H 1

/** Do we have time.h */
#define PFUNC_HAVE_TIME_H 1

/** Do we have sched.h */
#define PFUNC_HAVE_SCHED_H 1

/** Do we have sysctl.h */
#define PFUNC_HAVE_SYSCTL_H 1

/** Do we have stdint.h */
#define PFUNC_HAVE_STDINT_H 1

/** Do we have hash_map.h */
#define PFUNC_HAVE_HASH_MAP_H 1

/** Are we running under Intel x86 architecture */
#define PFUNC_X86 1

/** Are we running on powerpc */
/* #undef PFUNC_PPC */

/** Are we running Linux */
#define PFUNC_LINUX 1

/** Are we running Windows */
/* #undef PFUNC_WINDOWS */

/** Are we running Darwin */
/* #undef PFUNC_DARWIN */

/** Are we running AIX */
/* #undef PFUNC_AIX */

/** Do we have Windows threads */
/* #undef PFUNC_HAVE_WINDOWS_THREADS */

/** Do we have pthreads */
#define PFUNC_HAVE_PTHREADS 1

/** Do we have futexes (Linux 2.6 and later) */
#define PFUNC_HAVE_FUTEX 1

/** Do we have thread local storage (__thread). Compiler specific */
#define PFUNC_HAVE_TLS 1

/** Do we have a means of setting the scheduler affinities */
#define PFUNC_HAVE_SCHED_AFFINITY 1

/** Does the user want to compile with exceptions */
/* #undef PFUNC_USE_EXCEPTIONS */

/** Does the user want to compile with PAPI */
/* #undef PFUNC_USE_PAPI */
