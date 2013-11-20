# 1 "slab_cache.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "slab_cache.c"





# 1 "/usr/include/stdlib.h" 1 3 4
# 25 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/features.h" 1 3 4
# 329 "/usr/include/features.h" 3 4
# 1 "/usr/include/sys/cdefs.h" 1 3 4
# 313 "/usr/include/sys/cdefs.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 314 "/usr/include/sys/cdefs.h" 2 3 4
# 330 "/usr/include/features.h" 2 3 4
# 352 "/usr/include/features.h" 3 4
# 1 "/usr/include/gnu/stubs.h" 1 3 4



# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 5 "/usr/include/gnu/stubs.h" 2 3 4




# 1 "/usr/include/gnu/stubs-64.h" 1 3 4
# 10 "/usr/include/gnu/stubs.h" 2 3 4
# 353 "/usr/include/features.h" 2 3 4
# 26 "/usr/include/stdlib.h" 2 3 4







# 1 "/usr/lib/gcc/x86_64-redhat-linux/4.1.2/include/stddef.h" 1 3 4
# 214 "/usr/lib/gcc/x86_64-redhat-linux/4.1.2/include/stddef.h" 3 4
typedef long unsigned int size_t;
# 326 "/usr/lib/gcc/x86_64-redhat-linux/4.1.2/include/stddef.h" 3 4
typedef int wchar_t;
# 34 "/usr/include/stdlib.h" 2 3 4


# 96 "/usr/include/stdlib.h" 3 4


typedef struct
  {
    int quot;
    int rem;
  } div_t;



typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;



# 140 "/usr/include/stdlib.h" 3 4
extern size_t __ctype_get_mb_cur_max (void) __attribute__ ((__nothrow__)) ;




extern double atof (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern int atoi (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern long int atol (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;





__extension__ extern long long int atoll (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;





extern double strtod (__const char *__restrict __nptr,
        char **__restrict __endptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

# 182 "/usr/include/stdlib.h" 3 4


extern long int strtol (__const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

extern unsigned long int strtoul (__const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




__extension__
extern long long int strtoq (__const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

__extension__
extern unsigned long long int strtouq (__const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;





__extension__
extern long long int strtoll (__const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

__extension__
extern unsigned long long int strtoull (__const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

# 279 "/usr/include/stdlib.h" 3 4
extern double __strtod_internal (__const char *__restrict __nptr,
     char **__restrict __endptr, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
extern float __strtof_internal (__const char *__restrict __nptr,
    char **__restrict __endptr, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
extern long double __strtold_internal (__const char *__restrict __nptr,
           char **__restrict __endptr,
           int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

extern long int __strtol_internal (__const char *__restrict __nptr,
       char **__restrict __endptr,
       int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;



extern unsigned long int __strtoul_internal (__const char *__restrict __nptr,
          char **__restrict __endptr,
          int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




__extension__
extern long long int __strtoll_internal (__const char *__restrict __nptr,
      char **__restrict __endptr,
      int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;



__extension__
extern unsigned long long int __strtoull_internal (__const char *
         __restrict __nptr,
         char **__restrict __endptr,
         int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
# 429 "/usr/include/stdlib.h" 3 4
extern char *l64a (long int __n) __attribute__ ((__nothrow__)) ;


extern long int a64l (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;




# 1 "/usr/include/sys/types.h" 1 3 4
# 29 "/usr/include/sys/types.h" 3 4


# 1 "/usr/include/bits/types.h" 1 3 4
# 28 "/usr/include/bits/types.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 29 "/usr/include/bits/types.h" 2 3 4


# 1 "/usr/lib/gcc/x86_64-redhat-linux/4.1.2/include/stddef.h" 1 3 4
# 32 "/usr/include/bits/types.h" 2 3 4


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;







typedef long int __quad_t;
typedef unsigned long int __u_quad_t;
# 134 "/usr/include/bits/types.h" 3 4
# 1 "/usr/include/bits/typesizes.h" 1 3 4
# 135 "/usr/include/bits/types.h" 2 3 4


typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;

typedef int __daddr_t;
typedef long int __swblk_t;
typedef int __key_t;


typedef int __clockid_t;


typedef void * __timer_t;


typedef long int __blksize_t;




typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;


typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;


typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;

typedef long int __ssize_t;



typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;


typedef long int __intptr_t;


typedef unsigned int __socklen_t;
# 32 "/usr/include/sys/types.h" 2 3 4



typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;




typedef __loff_t loff_t;



typedef __ino_t ino_t;
# 62 "/usr/include/sys/types.h" 3 4
typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;





typedef __off_t off_t;
# 100 "/usr/include/sys/types.h" 3 4
typedef __pid_t pid_t;




typedef __id_t id_t;




typedef __ssize_t ssize_t;





typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;
# 133 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/time.h" 1 3 4
# 75 "/usr/include/time.h" 3 4


typedef __time_t time_t;



# 93 "/usr/include/time.h" 3 4
typedef __clockid_t clockid_t;
# 105 "/usr/include/time.h" 3 4
typedef __timer_t timer_t;
# 134 "/usr/include/sys/types.h" 2 3 4
# 147 "/usr/include/sys/types.h" 3 4
# 1 "/usr/lib/gcc/x86_64-redhat-linux/4.1.2/include/stddef.h" 1 3 4
# 148 "/usr/include/sys/types.h" 2 3 4



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
# 195 "/usr/include/sys/types.h" 3 4
typedef int int8_t __attribute__ ((__mode__ (__QI__)));
typedef int int16_t __attribute__ ((__mode__ (__HI__)));
typedef int int32_t __attribute__ ((__mode__ (__SI__)));
typedef int int64_t __attribute__ ((__mode__ (__DI__)));


typedef unsigned int u_int8_t __attribute__ ((__mode__ (__QI__)));
typedef unsigned int u_int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int u_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int u_int64_t __attribute__ ((__mode__ (__DI__)));

typedef int register_t __attribute__ ((__mode__ (__word__)));
# 217 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/endian.h" 1 3 4
# 37 "/usr/include/endian.h" 3 4
# 1 "/usr/include/bits/endian.h" 1 3 4
# 38 "/usr/include/endian.h" 2 3 4
# 218 "/usr/include/sys/types.h" 2 3 4


# 1 "/usr/include/sys/select.h" 1 3 4
# 31 "/usr/include/sys/select.h" 3 4
# 1 "/usr/include/bits/select.h" 1 3 4
# 32 "/usr/include/sys/select.h" 2 3 4


# 1 "/usr/include/bits/sigset.h" 1 3 4
# 23 "/usr/include/bits/sigset.h" 3 4
typedef int __sig_atomic_t;




typedef struct
  {
    unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
  } __sigset_t;
# 35 "/usr/include/sys/select.h" 2 3 4



typedef __sigset_t sigset_t;





# 1 "/usr/include/time.h" 1 3 4
# 121 "/usr/include/time.h" 3 4
struct timespec
  {
    __time_t tv_sec;
    long int tv_nsec;
  };
# 45 "/usr/include/sys/select.h" 2 3 4

# 1 "/usr/include/bits/time.h" 1 3 4
# 69 "/usr/include/bits/time.h" 3 4
struct timeval
  {
    __time_t tv_sec;
    __suseconds_t tv_usec;
  };
# 47 "/usr/include/sys/select.h" 2 3 4


typedef __suseconds_t suseconds_t;





typedef long int __fd_mask;
# 67 "/usr/include/sys/select.h" 3 4
typedef struct
  {






    __fd_mask __fds_bits[1024 / (8 * sizeof (__fd_mask))];


  } fd_set;






typedef __fd_mask fd_mask;
# 99 "/usr/include/sys/select.h" 3 4

# 109 "/usr/include/sys/select.h" 3 4
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
# 121 "/usr/include/sys/select.h" 3 4
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);



# 221 "/usr/include/sys/types.h" 2 3 4


# 1 "/usr/include/sys/sysmacros.h" 1 3 4
# 29 "/usr/include/sys/sysmacros.h" 3 4
__extension__
extern __inline unsigned int gnu_dev_major (unsigned long long int __dev)
     __attribute__ ((__nothrow__));
__extension__
extern __inline unsigned int gnu_dev_minor (unsigned long long int __dev)
     __attribute__ ((__nothrow__));
__extension__
extern __inline unsigned long long int gnu_dev_makedev (unsigned int __major,
       unsigned int __minor)
     __attribute__ ((__nothrow__));


__extension__ extern __inline unsigned int
__attribute__ ((__nothrow__)) gnu_dev_major (unsigned long long int __dev)
{
  return ((__dev >> 8) & 0xfff) | ((unsigned int) (__dev >> 32) & ~0xfff);
}

__extension__ extern __inline unsigned int
__attribute__ ((__nothrow__)) gnu_dev_minor (unsigned long long int __dev)
{
  return (__dev & 0xff) | ((unsigned int) (__dev >> 12) & ~0xff);
}

__extension__ extern __inline unsigned long long int
__attribute__ ((__nothrow__)) gnu_dev_makedev (unsigned int __major, unsigned int __minor)
{
  return ((__minor & 0xff) | ((__major & 0xfff) << 8)
   | (((unsigned long long int) (__minor & ~0xff)) << 12)
   | (((unsigned long long int) (__major & ~0xfff)) << 32));
}
# 224 "/usr/include/sys/types.h" 2 3 4
# 235 "/usr/include/sys/types.h" 3 4
typedef __blkcnt_t blkcnt_t;



typedef __fsblkcnt_t fsblkcnt_t;



typedef __fsfilcnt_t fsfilcnt_t;
# 270 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/bits/pthreadtypes.h" 1 3 4
# 23 "/usr/include/bits/pthreadtypes.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 24 "/usr/include/bits/pthreadtypes.h" 2 3 4
# 50 "/usr/include/bits/pthreadtypes.h" 3 4
typedef unsigned long int pthread_t;


typedef union
{
  char __size[56];
  long int __align;
} pthread_attr_t;



typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;
# 76 "/usr/include/bits/pthreadtypes.h" 3 4
typedef union
{
  struct __pthread_mutex_s
  {
    int __lock;
    unsigned int __count;
    int __owner;

    unsigned int __nusers;



    int __kind;

    int __spins;
    __pthread_list_t __list;
# 101 "/usr/include/bits/pthreadtypes.h" 3 4
  } __data;
  char __size[40];
  long int __align;
} pthread_mutex_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_mutexattr_t;




typedef union
{
  struct
  {
    int __lock;
    unsigned int __futex;
    __extension__ unsigned long long int __total_seq;
    __extension__ unsigned long long int __wakeup_seq;
    __extension__ unsigned long long int __woken_seq;
    void *__mutex;
    unsigned int __nwaiters;
    unsigned int __broadcast_seq;
  } __data;
  char __size[48];
  __extension__ long long int __align;
} pthread_cond_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_condattr_t;



typedef unsigned int pthread_key_t;



typedef int pthread_once_t;





typedef union
{

  struct
  {
    int __lock;
    unsigned int __nr_readers;
    unsigned int __readers_wakeup;
    unsigned int __writer_wakeup;
    unsigned int __nr_readers_queued;
    unsigned int __nr_writers_queued;
    int __writer;
    int __pad1;
    unsigned long int __pad2;
    unsigned long int __pad3;


    unsigned int __flags;
  } __data;
# 184 "/usr/include/bits/pthreadtypes.h" 3 4
  char __size[56];
  long int __align;
} pthread_rwlock_t;

typedef union
{
  char __size[8];
  long int __align;
} pthread_rwlockattr_t;





typedef volatile int pthread_spinlock_t;




typedef union
{
  char __size[32];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_barrierattr_t;
# 271 "/usr/include/sys/types.h" 2 3 4



# 439 "/usr/include/stdlib.h" 2 3 4






extern long int random (void) __attribute__ ((__nothrow__));


extern void srandom (unsigned int __seed) __attribute__ ((__nothrow__));





extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));



extern char *setstate (char *__statebuf) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));







struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };

extern int random_r (struct random_data *__restrict __buf,
       int32_t *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 4)));

extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));






extern int rand (void) __attribute__ ((__nothrow__));

extern void srand (unsigned int __seed) __attribute__ ((__nothrow__));




extern int rand_r (unsigned int *__seed) __attribute__ ((__nothrow__));







extern double drand48 (void) __attribute__ ((__nothrow__));
extern double erand48 (unsigned short int __xsubi[3]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern long int lrand48 (void) __attribute__ ((__nothrow__));
extern long int nrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern long int mrand48 (void) __attribute__ ((__nothrow__));
extern long int jrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern void srand48 (long int __seedval) __attribute__ ((__nothrow__));
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    unsigned long long int __a;
  };


extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));









extern void *malloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;

extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;







extern void *realloc (void *__ptr, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));

extern void free (void *__ptr) __attribute__ ((__nothrow__));




extern void cfree (void *__ptr) __attribute__ ((__nothrow__));



# 1 "/usr/include/alloca.h" 1 3 4
# 25 "/usr/include/alloca.h" 3 4
# 1 "/usr/lib/gcc/x86_64-redhat-linux/4.1.2/include/stddef.h" 1 3 4
# 26 "/usr/include/alloca.h" 2 3 4







extern void *alloca (size_t __size) __attribute__ ((__nothrow__));






# 613 "/usr/include/stdlib.h" 2 3 4




extern void *valloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;




extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




extern void abort (void) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));



extern int atexit (void (*__func) (void)) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern void exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));

# 658 "/usr/include/stdlib.h" 3 4


extern char *getenv (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




extern char *__secure_getenv (__const char *__name)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;





extern int putenv (char *__string) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int setenv (__const char *__name, __const char *__value, int __replace)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));


extern int unsetenv (__const char *__name) __attribute__ ((__nothrow__));






extern int clearenv (void) __attribute__ ((__nothrow__));
# 698 "/usr/include/stdlib.h" 3 4
extern char *mktemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
# 709 "/usr/include/stdlib.h" 3 4
extern int mkstemp (char *__template) __attribute__ ((__nonnull__ (1))) ;
# 729 "/usr/include/stdlib.h" 3 4
extern char *mkdtemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;








extern int system (__const char *__command) ;

# 756 "/usr/include/stdlib.h" 3 4
extern char *realpath (__const char *__restrict __name,
         char *__restrict __resolved) __attribute__ ((__nothrow__)) ;






typedef int (*__compar_fn_t) (__const void *, __const void *);









extern void *bsearch (__const void *__key, __const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) ;



extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));



extern int abs (int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;
extern long int labs (long int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;












extern div_t div (int __numer, int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;

# 821 "/usr/include/stdlib.h" 3 4
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *gcvt (double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3))) ;




extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3))) ;




extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));

extern int qecvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qfcvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));







extern int mblen (__const char *__s, size_t __n) __attribute__ ((__nothrow__)) ;


extern int mbtowc (wchar_t *__restrict __pwc,
     __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__)) ;


extern int wctomb (char *__s, wchar_t __wchar) __attribute__ ((__nothrow__)) ;



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__));

extern size_t wcstombs (char *__restrict __s,
   __const wchar_t *__restrict __pwcs, size_t __n)
     __attribute__ ((__nothrow__));








extern int rpmatch (__const char *__response) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
# 926 "/usr/include/stdlib.h" 3 4
extern int posix_openpt (int __oflag) ;
# 961 "/usr/include/stdlib.h" 3 4
extern int getloadavg (double __loadavg[], int __nelem)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
# 977 "/usr/include/stdlib.h" 3 4

# 7 "slab_cache.c" 2
# 1 "/usr/include/assert.h" 1 3 4
# 65 "/usr/include/assert.h" 3 4



extern void __assert_fail (__const char *__assertion, __const char *__file,
      unsigned int __line, __const char *__function)
     __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));


extern void __assert_perror_fail (int __errnum, __const char *__file,
      unsigned int __line,
      __const char *__function)
     __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));




extern void __assert (const char *__assertion, const char *__file, int __line)
     __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));



# 8 "slab_cache.c" 2
# 1 "/usr/include/errno.h" 1 3 4
# 32 "/usr/include/errno.h" 3 4




# 1 "/usr/include/bits/errno.h" 1 3 4
# 25 "/usr/include/bits/errno.h" 3 4
# 1 "/usr/include/linux/errno.h" 1 3 4



# 1 "/usr/include/asm/errno.h" 1 3 4




# 1 "/usr/include/asm-x86_64/errno.h" 1 3 4



# 1 "/usr/include/asm-generic/errno.h" 1 3 4



# 1 "/usr/include/asm-generic/errno-base.h" 1 3 4
# 5 "/usr/include/asm-generic/errno.h" 2 3 4
# 5 "/usr/include/asm-x86_64/errno.h" 2 3 4
# 6 "/usr/include/asm/errno.h" 2 3 4
# 5 "/usr/include/linux/errno.h" 2 3 4
# 26 "/usr/include/bits/errno.h" 2 3 4
# 43 "/usr/include/bits/errno.h" 3 4
extern int *__errno_location (void) __attribute__ ((__nothrow__)) __attribute__ ((__const__));
# 37 "/usr/include/errno.h" 2 3 4
# 59 "/usr/include/errno.h" 3 4

# 9 "slab_cache.c" 2
# 1 "/usr/include/string.h" 1 3 4
# 28 "/usr/include/string.h" 3 4





# 1 "/usr/lib/gcc/x86_64-redhat-linux/4.1.2/include/stddef.h" 1 3 4
# 34 "/usr/include/string.h" 2 3 4




extern void *memcpy (void *__restrict __dest,
       __const void *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern void *memmove (void *__dest, __const void *__src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));






extern void *memccpy (void *__restrict __dest, __const void *__restrict __src,
        int __c, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));





extern void *memset (void *__s, int __c, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int memcmp (__const void *__s1, __const void *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern void *memchr (__const void *__s, int __c, size_t __n)
      __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

# 82 "/usr/include/string.h" 3 4


extern char *strcpy (char *__restrict __dest, __const char *__restrict __src)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncpy (char *__restrict __dest,
        __const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *strcat (char *__restrict __dest, __const char *__restrict __src)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncat (char *__restrict __dest, __const char *__restrict __src,
        size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcmp (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern int strncmp (__const char *__s1, __const char *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcoll (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern size_t strxfrm (char *__restrict __dest,
         __const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

# 130 "/usr/include/string.h" 3 4
extern char *strdup (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
# 165 "/usr/include/string.h" 3 4


extern char *strchr (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

extern char *strrchr (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

# 181 "/usr/include/string.h" 3 4



extern size_t strcspn (__const char *__s, __const char *__reject)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern size_t strspn (__const char *__s, __const char *__accept)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strpbrk (__const char *__s, __const char *__accept)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strstr (__const char *__haystack, __const char *__needle)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));



extern char *strtok (char *__restrict __s, __const char *__restrict __delim)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));




extern char *__strtok_r (char *__restrict __s,
    __const char *__restrict __delim,
    char **__restrict __save_ptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3)));

extern char *strtok_r (char *__restrict __s, __const char *__restrict __delim,
         char **__restrict __save_ptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3)));
# 240 "/usr/include/string.h" 3 4


extern size_t strlen (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

# 254 "/usr/include/string.h" 3 4


extern char *strerror (int __errnum) __attribute__ ((__nothrow__));

# 270 "/usr/include/string.h" 3 4
extern int strerror_r (int __errnum, char *__buf, size_t __buflen) __asm__ ("" "__xpg_strerror_r") __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
# 288 "/usr/include/string.h" 3 4
extern void __bzero (void *__s, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern void bcopy (__const void *__src, void *__dest, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern void bzero (void *__s, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int bcmp (__const void *__s1, __const void *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *index (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


extern char *rindex (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));



extern int ffs (int __i) __attribute__ ((__nothrow__)) __attribute__ ((__const__));
# 325 "/usr/include/string.h" 3 4
extern int strcasecmp (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strncasecmp (__const char *__s1, __const char *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 348 "/usr/include/string.h" 3 4
extern char *strsep (char **__restrict __stringp,
       __const char *__restrict __delim)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
# 426 "/usr/include/string.h" 3 4

# 10 "slab_cache.c" 2
# 29 "slab_cache.c"
typedef unsigned int slab_bufctl_t;
# 38 "slab_cache.c"
typedef struct slab_block {
 struct list_head sb_list;
 slab_size_t object_inuse;
 slab_size_t color_offset;
 slab_bufctl_t next_free;

 slab_cache_t *sc;
 struct rb_node rb_list;
} slab_block_t;

struct sized_slab_cache {
 slab_size_t size;
 slab_cache_t* sc;
};
# 73 "slab_cache.c"
static int slab_max_blocks = 16;
static struct list_head slab_cache_head = LIST_HEAD_INIT(slab_cache_head);
static struct rb_root slabs_root = RB_ROOT;
static struct sized_slab_cache sized_caches[] = {
                {4,((void *)0)},,{8,((void *)0)},,{12,((void *)0)},,{16,((void *)0)},,{20,((void *)0)},,{24,((void *)0)},,{28,((void *)0)},,{32,((void *)0)},,{36,((void *)0)},,{40,((void *)0)},,
         {44,((void *)0)},,{48,((void *)0)},,{52,((void *)0)},,{56,((void *)0)},,{60,((void *)0)},,{64,((void *)0)},,{68,((void *)0)},,{72,((void *)0)},,{76,((void *)0)},,{80,((void *)0)},,
                {88,((void *)0)},,{96,((void *)0)},,{104,((void *)0)},,{112,((void *)0)},,{120,((void *)0)},,{128,((void *)0)},,{136,((void *)0)},,{144,((void *)0)},,{152,((void *)0)},,{160,((void *)0)},,
                {176,((void *)0)},,{192,((void *)0)},,{208,((void *)0)},,{224,((void *)0)},,{240,((void *)0)},,{256,((void *)0)},,{272,((void *)0)},,{288,((void *)0)},,{304,((void *)0)},,{320,((void *)0)},,
                {352,((void *)0)},,{384,((void *)0)},,{416,((void *)0)},,{448,((void *)0)},,{480,((void *)0)},,{512,((void *)0)},,{544,((void *)0)},,{576,((void *)0)},,{608,((void *)0)},,{640,((void *)0)},,
                {704,((void *)0)},,{768,((void *)0)},,{832,((void *)0)},,{896,((void *)0)},,{960,((void *)0)},,{1024,((void *)0)},,{1088,((void *)0)},,{1152,((void *)0)},,{1216,((void *)0)},,{1280,((void *)0)},,
                {1408,((void *)0)},,{1536,((void *)0)},,{1664,((void *)0)},,{1792,((void *)0)},,{1920,((void *)0)},,{2048,((void *)0)},,{2176,((void *)0)},,{2304,((void *)0)},,{2432,((void *)0)},,{2560,((void *)0)},,
                {2816,((void *)0)},,{3072,((void *)0)},,{3328,((void *)0)},,{3584,((void *)0)},,{3840,((void *)0)},,{4096,((void *)0)},,{4352,((void *)0)},,{4608,((void *)0)},,{4864,((void *)0)},,{5120,((void *)0)},,
                {5632,((void *)0)},,{6144,((void *)0)},,{6656,((void *)0)},,{7168,((void *)0)},,{7680,((void *)0)},,{8192,((void *)0)},,{8704,((void *)0)},,{9216,((void *)0)},,{9728,((void *)0)},,{10240,((void *)0)},,
                {11264,((void *)0)},,{12288,((void *)0)},,{13312,((void *)0)},,{14336,((void *)0)},,{15360,((void *)0)},,{16384,((void *)0)},,
         {17408,((void *)0)},,{18432,((void *)0)},,{19456,((void *)0)},,{20480,((void *)0)},,
                {22528,((void *)0)},,{24576,((void *)0)},,{26624,((void *)0)},,{28672,((void *)0)},,{30720,((void *)0)},,{32768,((void *)0)},,
         {34816,((void *)0)},,{36864,((void *)0)},,{38912,((void *)0)},,{40960,((void *)0)},,
                {45056,((void *)0)},,{49152,((void *)0)},,{53248,((void *)0)},,{57344,((void *)0)},,{61440,((void *)0)},,{65536,((void *)0)},,
         {69632,((void *)0)},,{73728,((void *)0)},,{77824,((void *)0)},,{81920,((void *)0)},,
                {90112,((void *)0)},,{98304,((void *)0)},,{106496,((void *)0)},,{114688,((void *)0)},,{122880,((void *)0)},,{131072,((void *)0)},,
         {139264,((void *)0)},,{147456,((void *)0)},,{155648,((void *)0)},,{163840,((void *)0)},,
                {180224,((void *)0)},,{196608,((void *)0)},,{212992,((void *)0)},,{229376,((void *)0)},,{245760,((void *)0)},,{262144,((void *)0)},,
         {278528,((void *)0)},,{294912,((void *)0)},,{311296,((void *)0)},,{327680,((void *)0)},,
                {360448,((void *)0)},,{393216,((void *)0)},,{425984,((void *)0)},,{458752,((void *)0)},,{491520,((void *)0)},,{524288,((void *)0)},,
         {557056,((void *)0)},,{589824,((void *)0)},,{622592,((void *)0)},,{655360,((void *)0)},,
                {720896,((void *)0)},,{786432,((void *)0)},,{851968,((void *)0)},,{917504,((void *)0)},,{983040,((void *)0)},,{1048576,((void *)0)},,
         {1114112,((void *)0)},,{1179648,((void *)0)},,{1245184,((void *)0)},,{1310720,((void *)0)},
};


static slab_cache_t slab_cache_cache = {
 ((void *)0),
 PTHREAD_MUTEX_INITIALIZER,
};

static inline void* index_to_obj(slab_cache_t* sc, slab_block_t* sb, slab_size_t idx)
{
 return (void *)((char *)(sb + 1) + idx * sc->object_size);
}

static slab_size_t obj_to_index(slab_cache_t* sc, slab_block_t* sb, void* objp)
{
 unsigned long idx = (unsigned long)((char *)objp - (char *)(sb + 1));
 idx = idx / sc->object_size;
 ((idx < (slab_size_t)~0UL) ? (void) (0) : (__assert_fail ("idx < (slab_size_t)~0UL", "slab_cache.c", 117, __PRETTY_FUNCTION__), (void) (0)));
 return (slab_size_t)idx;
}

static inline slab_block_t* __obj_to_slab(void* obj)
{
 struct rb_node* pn = slabs_root.rb_node;
 unsigned long objp = (unsigned long)obj;

 while (pn != ((void *)0)) {
  slab_block_t* sb = rb_entry(pn, slab_block_t, rb_list);
  unsigned long sbp = (unsigned long)((char *)sb - sb->color_offset);
  if (objp < sbp)
   pn = pn->rb_left;
  else if (objp >= (sbp + sb->sc->slab_size))
   pn = pn->rb_right;
  else
   return sb;
 }

 return ((void *)0);
}

static inline slab_cache_t* obj_to_cache(void* objp)
{
 slab_block_t* sb = __obj_to_slab(objp);
 if (sb != ((void *)0)) return sb->sc;
 return ((void *)0);
}

static inline slab_block_t* obj_to_slab(slab_cache_t* sc, void* objp)
{
 slab_block_t* sb = __obj_to_slab(objp);
 return sb;
}

static inline void* slab_addr(slab_block_t* sb)
{
 return (void *)((char *)sb - sb->color_offset);
}

static inline slab_block_t* __insert_slab(struct rb_root* root, struct rb_node* node, void* obj)
{
 unsigned long objp = (unsigned long)obj;
 struct rb_node** p = &root->rb_node;
 struct rb_node* parent = ((void *)0);
 slab_block_t* sb;

 while (*p) {
  parent = *p;
  sb = rb_entry(parent, slab_block_t, rb_list);
  unsigned long sbp = (unsigned long)((char *)sb - sb->color_offset);

  if (objp < sbp)
   p = &(*p)->rb_left;
  else if (objp >= (sbp + sb->sc->slab_size))
   p = &(*p)->rb_right;
  else
   return sb;
 }

 rb_link_node(node, parent, p);
 return ((void *)0);
}

static inline int link_slab_list(slab_block_t* sb)
{
 void* obj = (void *)((char *)sb - sb->color_offset);
 slab_block_t* old = __insert_slab(&slabs_root, &sb->rb_list, obj);
 if (old != ((void *)0)) {
  ((old == ((void *)0)) ? (void) (0) : (__assert_fail ("old == ((void *)0)", "slab_cache.c", 187, __PRETTY_FUNCTION__), (void) (0)));
  return -1;
 }

 rb_insert_color(&sb->rb_list, &slabs_root);
 return 0;
}

static inline int unlink_slab_list(slab_block_t* sb)
{
 rb_erase(&sb->rb_list, &slabs_root);
 return 0;
}

static inline slab_bufctl_t* slab_bufctl(slab_block_t* sb, slab_cache_t* sc, slab_size_t idx)
{
 return (slab_bufctl_t *)((char *)(sb + 1) + (idx * sc->object_size));
}

static slab_block_t* slab_cache_grow(slab_cache_t* sc)
{
 ((sc != ((void *)0)) ? (void) (0) : (__assert_fail ("sc != ((void *)0)", "slab_cache.c", 208, __PRETTY_FUNCTION__), (void) (0)));
 void* objp = (*allocate)(sc->slab_size);
 unsigned int offset;
 if (++sc->color_next >= sc->color_range) {
  sc->color_next = 0;
 }

 offset = sc->color_next * sc->color_align;
 slab_block_t* sb = (slab_block_t *)((char *)objp + offset);
 sb->object_inuse = 0;
 sb->color_offset = offset;
 sb->next_free = 0;

 ((((char *)(sb + 1) + sc->object_num * sc->object_size) <= ((char *)objp + sc->slab_size)) ? (void) (0) : (__assert_fail ("((char *)(sb + 1) + sc->object_num * sc->object_size) <= ((char *)objp + sc->slab_size)", "slab_cache.c", 221, __PRETTY_FUNCTION__), (void) (0)));

 slab_size_t i = 0;
 for ( ; i < sc->object_num - 1; ++i) {
  *slab_bufctl(sb, sc, i) = i + 1;
 }

 *slab_bufctl(sb, sc, i) = (((slab_bufctl_t)(~0UL))-0);

 list_add(&sb->sb_list, &sc->list_free);
 sc->free_objects += sc->object_num;

 sb->sc = sc;
 link_slab_list(sb);
 return sb;
}

static void slab_cache_reap(slab_cache_t* sc)
{
 struct list_head* pl, *n;
 slab_block_t* sb;
 void* addr;
 ((sc != ((void *)0)) ? (void) (0) : (__assert_fail ("sc != ((void *)0)", "slab_cache.c", 243, __PRETTY_FUNCTION__), (void) (0)));

 list_for_each_safe(pl, n, &sc->list_free) {
  if (sc->free_objects < sc->free_limit)
   break;
  sb = list_entry(pl, slab_block_t, sb_list);
  list_del_init(&sb->sb_list);

  ((sc->free_objects >= sc->object_num) ? (void) (0) : (__assert_fail ("sc->free_objects >= sc->object_num", "slab_cache.c", 251, __PRETTY_FUNCTION__), (void) (0)));
  sc->free_objects -= sc->object_num;

  unlink_slab_list(sb);
  addr = slab_addr(sb);
  (*deallocate)(addr);
 }

 return;
}

static slab_size_t __calc_slab_sizes(slab_size_t slab_size, slab_size_t object_size, slab_size_t* num_p_slab, int* up)
{
 *num_p_slab = (slab_size - sizeof(slab_block_t)) / object_size;
 *up = *num_p_slab * object_size * 100 / slab_size;
 return (slab_size - sizeof(slab_block_t) - *num_p_slab * object_size);
}

static slab_size_t __estimate_slab_size(size_t size, slab_size_t* num_p_slab, slab_size_t* slab_size)
{
 slab_size_t i, left;
 for (i = 0; i < slab_max_blocks; ++i) {
  int up;
  *slab_size = i * (1UL << 16);
  if (*slab_size < (size + sizeof(slab_block_t)))
   continue;
  left = __calc_slab_sizes(*slab_size, size, num_p_slab, &up);
  if (*num_p_slab > 1 && up >= 95)
   break;
 }

 return left;
}

static int __create(slab_cache_t* sc, const char* name, slab_size_t size, slab_size_t align, slab_size_t maxfree)
{
 if (align == 0) {

  if (size < 2)
   align = 1;
  else if (size < 4)
   align = 2;
  else if (size < 8)
   align = 4;
  else if (size < 16)
   align = 8;
  else
   align = 16;
 }

 ((align > 0) ? (void) (0) : (__assert_fail ("align > 0", "slab_cache.c", 301, __PRETTY_FUNCTION__), (void) (0)));
 size = (size + align -1) & ~(align - 1);


 if (name == ((void *)0) || strlen(name) > (SLAB_NAMLEN - 1)) {
  (*__errno_location ()) = 22;
  return -1;
 }


 if (size < (sizeof(slab_bufctl_t)) || size > ((slab_size_t)~0UL)) {
  (*__errno_location ()) = 22;
  return -1;
 }

 slab_size_t num_p_slab = 0, slab_size, color_align = 16;
 slab_size_t left = __estimate_slab_size(size, &num_p_slab, &slab_size);
 if (num_p_slab == 0)
  return -1;

 memset(sc, 0, sizeof(*sc));
 strncpy(sc->name, name, sizeof(sc->name));

 INIT_LIST_HEAD(&sc->list_partial);
 INIT_LIST_HEAD(&sc->list_full);
 INIT_LIST_HEAD(&sc->list_free);

 sc->object_size = size;
 sc->object_align = align;
 sc->object_num = num_p_slab;
 sc->slab_size = slab_size;

 sc->free_limit = maxfree;
 sc->free_objects = 0;

 sc->color_next = 0;
 sc->color_align = color_align;
 sc->color_range = left / sc->color_align;


 list_add(&sc->sc_list, &slab_cache_head);

 return 0;
}

slab_cache_t* slab_cache_create(const char* name, slab_size_t size, slab_size_t align, slab_size_t maxfree)
{
 slab_cache_t* sc = (slab_cache_t *)slab_cache_allocate(&slab_cache_cache);
 if (sc == ((void *)0))
  return ((void *)0);

 if (__create(sc, name, size, align, maxfree) < 0) {
  slab_cache_free(&slab_cache_cache, sc);
  return ((void *)0);
 }

 pthread_mutex_init(&sc->lock_, ((void *)0));
 return sc;
}

static void __destroy_blocks(slab_cache_t* sc, struct list_head* head)
{
 struct list_head* pl, *n;
 list_for_each_safe(pl, n, head) {
  slab_block_t* sb = list_entry(pl, slab_block_t, sb_list);
  void* addr;

  list_del_init(&sb->sb_list);
  unlink_slab_list(sb);

  addr = slab_addr(sb);
  system_free(addr);
 }
}

static void __destroy(slab_cache_t* sc)
{
 __destroy_blocks(sc, &sc->list_free);
 __destroy_blocks(sc, &sc->list_partial);
 __destroy_blocks(sc, &sc->list_full);

 list_del(&sc->sc_list);
}

void slab_cache_destroy(slab_cache_t* sc)
{
 __destroy(sc);
 slab_cache_free(&slab_cache_cache, sc);
}

static inline void* __slab_cache_allocate_partial(slab_block_t* sb, slab_cache_t* sc)
{
 void *objp = index_to_obj(sc, sb, sb->next_free);

 ++sb->object_inuse;
 ((sb->object_inuse > 1 && sb->object_inuse <= sc->object_num) ? (void) (0) : (__assert_fail ("sb->object_inuse > 1 && sb->object_inuse <= sc->object_num", "slab_cache.c", 396, __PRETTY_FUNCTION__), (void) (0)));

 ((sc->free_objects > 0) ? (void) (0) : (__assert_fail ("sc->free_objects > 0", "slab_cache.c", 398, __PRETTY_FUNCTION__), (void) (0)));
 --sc->free_objects;

 sb->next_free = *slab_bufctl(sb, sc, sb->next_free);
 if (sb->next_free == (((slab_bufctl_t)(~0UL))-0)) {
  list_del(&sb->sb_list);
  list_add(&sb->sb_list, &sc->list_full);
 }
 else {
  ((sb->next_free < sc->object_num) ? (void) (0) : (__assert_fail ("sb->next_free < sc->object_num", "slab_cache.c", 407, __PRETTY_FUNCTION__), (void) (0)));
 }

 return objp;
}

static inline void* __slab_cache_allocate_free(slab_block_t* sb, slab_cache_t* sc)
{
 void *objp = index_to_obj(sc, sb, sb->next_free);

 ++sb->object_inuse;
 ((sb->object_inuse == 1) ? (void) (0) : (__assert_fail ("sb->object_inuse == 1", "slab_cache.c", 418, __PRETTY_FUNCTION__), (void) (0)));

 ((sc->free_objects > 0) ? (void) (0) : (__assert_fail ("sc->free_objects > 0", "slab_cache.c", 420, __PRETTY_FUNCTION__), (void) (0)));
 --sc->free_objects;
 sb->next_free = *slab_bufctl(sb, sc, sb->next_free);
 ((sb->next_free != (((slab_bufctl_t)(~0UL))-0)) ? (void) (0) : (__assert_fail ("sb->next_free != (((slab_bufctl_t)(~0UL))-0)", "slab_cache.c", 423, __PRETTY_FUNCTION__), (void) (0)));

 list_del(&sb->sb_list);
 list_add(&sb->sb_list, &sc->list_partial);

 return objp;
}

void* slab_cache_allocate(slab_cache_t *sc)
{
 ((sc != ((void *)0)) ? (void) (0) : (__assert_fail ("sc != ((void *)0)", "slab_cache.c", 433, __PRETTY_FUNCTION__), (void) (0)));
 void* objp;
 pthread_mutex_lock(&sc->lock_);

 if (!list_empty(&sc->list_partial)) {
  slab_block_t* sb = list_entry(sc->list_partial.next, slab_block_t, sb_list);
  objp = __slab_cache_allocate_partial(sb, sc);
 }
 else if (!list_empty(&sc->list_free)) {
  slab_block_t* sb = list_entry(sc->list_free.next, slab_block_t, sb_list);
  objp = __slab_cache_allocate_free(sb, sc);
 }
 else {
  slab_block_t* sb = slab_cache_grow(sc);
  objp = __slab_cache_allocate_free(sb, sc);
 }
 pthread_mutex_unlock(&sc->lock_);

 return objp;
}

int slab_cache_free(slab_cache_t* sc, void* addr)
{
 slab_block_t* sb;
 slab_size_t idx;

 if ((sb = obj_to_slab(sc, addr)) == ((void *)0)) {
  return -1;
 }

 idx = obj_to_index(sc, sb, addr);
 *slab_bufctl(sb, sc, idx) = sb->next_free;
 sb->next_free = idx;

 ((sb->object_inuse > 0 && sb->object_inuse <= sc->object_num) ? (void) (0) : (__assert_fail ("sb->object_inuse > 0 && sb->object_inuse <= sc->object_num", "slab_cache.c", 467, __PRETTY_FUNCTION__), (void) (0)));
 --sb->object_inuse;

 ++sc->free_objects;
 ((sc->free_objects > 0) ? (void) (0) : (__assert_fail ("sc->free_objects > 0", "slab_cache.c", 471, __PRETTY_FUNCTION__), (void) (0)));

 if (sb->object_inuse == 0) {
  list_del(&sb->sb_list);
  list_add(&sb->sb_list, &sc->list_free);


  if (sc->free_objects > sc->free_limit) {
   slab_cache_reap(sc);
  }
 }

 return 0;
}

void* slab_malloc(slab_size_t size)
{
 slab_size_t i;
 for (i = 0; i < (sizeof(sized_caches) / sizeof(sized_caches[0])); ++i) {
  if (size <= sized_caches[i].size) {
   void* addr = slab_cache_allocate(sized_caches[i].sc);
   if (addr != ((void *)0)) {
    return addr;
   }


   return ((void *)0);
  }
 }



 return ((void *)0);
}

void* slab_realloc(void* old, slab_size_t nsize)
{
 void* addr = slab_malloc(nsize);
 if (addr != ((void *)0) && old != ((void *)0)) {
  slab_cache_t* sc = obj_to_cache(old);
  if (sc == ((void *)0)) {
   slab_free(addr);
  }
  else {
   memcpy(addr, old, sc->object_size > nsize ? sc->object_size : nsize);
   slab_cache_free(sc, old);
  }
 }

 return addr;
}

void slab_free(void* addr)
{
 if (addr == ((void *)0))
  return;

 slab_cache_t* sc;
 if ((sc = obj_to_cache(addr)) == ((void *)0)
   || slab_cache_free(sc, addr) < 0) {

  ((0) ? (void) (0) : (__assert_fail ("0", "slab_cache.c", 532, __PRETTY_FUNCTION__), (void) (0)));

 }
}

size_t slab_capacity(void *addr)
{
 slab_cache_t* sc = obj_to_cache(addr);
 if (sc != ((void *)0)) {
  return sc->object_size;
 }
 else {
  ((0) ? (void) (0) : (__assert_fail ("0", "slab_cache.c", 544, __PRETTY_FUNCTION__), (void) (0)));
 }

 return 0;
}

int slab_cache_init(slab_size_t __max_blocks)
{
 if (__create(&slab_cache_cache, "cache-cache", sizeof slab_cache_t, sizeof(void *), 0) < 0)
  return -1;
 initialized = 1;

 size_t i, sizes_count = sizeof(sized_caches) / sizeof(sized_caches[0]);
 for (i = 0; i < sizes_count; ++i) {
  ((sized_caches[i].size > 0) ? (void) (0) : (__assert_fail ("sized_caches[i].size > 0", "slab_cache.c", 558, __PRETTY_FUNCTION__), (void) (0)));
  char name[64];
  snprintf(name, sizeof(name), "size-%ld", (long)sized_caches[i].size);
  sized_caches[i].sc = slab_cache_create(name, sized_caches[i].size, 0, 0);
  if (sized_caches[i].sc == ((void *)0)) {
   goto free_sizes;
  }
 }

 return 0;

free_sizes:
 for ( ; i >= 0; --i)
  slab_cache_destroy(sized_caches[i].sc);
 __destroy(&slab_cache_cache);
fail:
 return -1;
}

void slab_cache_fini()
{
 int i = 0;
 for (i = 0; i < (sizeof(sized_caches) / sizeof(sized_caches[0])); ++i)
  slab_cache_destroy(sized_caches[i].sc);
 __destroy(&slab_cache_cache);
}

static void __cache_info(FILE* fp, slab_cache_t* sc, int i)
{
 size_t pc = list_count(&sc->list_partial);
 size_t fc = list_count(&sc->list_full);
 size_t ff = list_count(&sc->list_free);

 double total = (double)(pc + fc + ff) * (1UL << 16) / 1024.0 / 1024.0;
 double used = (double)((pc + fc + ff) * sc->object_num - sc->free_objects) * sc->object_size / 1024.0 / 1024.0;
 double ratio = 100 * used / ((pc + fc + ff) == 0 ? 1 : total);

 fprintf(fp, "*cache[%d]: %s, partial=%ld, full=%ld, free=%ld, object(size=%ld, align=%ld),\n"
      "    slab(size=%ld, num/p=%ld), free(limit=%ld, num=%ld),\n"
      "    color(next=%ld, rng=%ld, align=%ld)\n"
      "    total-allocated=%.3fMb, used=%.3fMb(%.3f%%)\n",
  i, sc->name,
  (long)pc, (long)fc, (long)ff,
  (long)sc->object_size, (long)sc->object_align,
  (long)sc->slab_size, (long)sc->object_num,
  (long)sc->free_limit, (long)sc->free_objects,
  (long)sc->color_next, (long)sc->color_range,
  (long)sc->color_align, total, used, ratio);
}

void slab_cache_info(FILE* fp)
{
 struct list_head* pl;
 int i;
 fprintf(fp, "slab cache infomation\n"
      "   allocate: %p%s, deallocate: %p%s, block-size=%d, slab-max-blocks=%d\n"
      "   sized-caches: [%d] (",
       allocate, allocate == malloc ? "(malloc)" : "",
      deallocate, deallocate == free ? "(free)" : "",
      (int)(1UL << 16), slab_max_blocks, (sizeof(sized_caches) / sizeof(sized_caches[0])));
 for (i = 0; i < (sizeof(sized_caches) / sizeof(sized_caches[0])); ++i) {
  fprintf(fp, "%s%ld", (i == 0) ? "" : ", ", (long)sized_caches[i].size);
 }

 fprintf(fp, ")\n");

 i = 0;
 list_for_each(pl, &slab_cache_head) {
  slab_cache_t* sc = list_entry(pl, slab_cache_t, sc_list);
  __cache_info(fp, sc, i++);
 }

 fprintf(fp, "slabs information:\n");
 struct rb_node* pn;
 i = 0;
 for (pn = rb_first(&slabs_root); pn != ((void *)0); pn = rb_next(pn)) {
  slab_block_t* sb = rb_entry(pn, slab_block_t, rb_list);
  fprintf(fp, "slab[%d]: inuse=%ld, offset=%ld, next-free=%ld, sc=%s\n", i++,
   (long)sb->object_inuse, (long)sb->color_offset, (long)sb->next_free, sb->sc->name);
 }

 return;
}
