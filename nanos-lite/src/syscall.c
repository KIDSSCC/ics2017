#include "common.h"
#include "syscall.h"

static inline int sys_none() {
  return 1;
}

int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);

static inline int sys_open(const char *path, int flags, mode_t mode) {
  return fs_open(path, flags, mode);
}

static inline int sys_read(int fd, void *buf, size_t count) {
  return fs_read(fd, buf, count);
}

static inline int sys_write(int fd, const void *buf, size_t count) {
  return fs_write(fd, buf, count);
}

static inline int sys_exit(int status) {
  _halt(status);
  return 0;
}

static inline int sys_kill(int pid, int sig) {
  assert(0);
  return -1;
}

static inline pid_t sys_getpid() {
  assert(0);
  return 1;
}

static inline int sys_close(int fd) {
  return fs_close(fd);
}

static inline off_t sys_lseek(int fd, off_t offset, int whence) {
  return fs_lseek(fd, offset, whence);
}

static inline int sys_brk(void *addr) {
  _heap.end = (void *)addr;
  return 0;
}

struct stat;

static inline int sys_fstat(int fd, struct stat *buf) {
  assert(0);
  return -1;
}

static inline time_t sys_time(time_t *tloc) {
  assert(0);
  return 0;
}

static inline int sys_signal(int signum, void *handler) {
  assert(0);
  return -1;
}

static inline int sys_execve(const char *fname, char * const argv[], char *const envp[]) {
  assert(0);
  return -1;
}

static inline pid_t sys_fork() {
  assert(0);
  return -1;
}

static inline int sys_link(const char *oldpath, const char *newpath) {
  assert(0);
  return -1;
}

static inline int sys_unlink(const char *pathname) {
  assert(0);
  return -1;
}

static inline pid_t sys_wait(int *status) {
  assert(0);
  return -1;
}

struct tms;

static inline clock_t sys_times(struct tms *buf) {
  assert(0);
  return -1;
}

struct timezone;

static inline int sys_gettimeofday(struct timeval *tv, struct timezone *tz) {
  assert(0);
  return -1;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none: SYSCALL_ARG1(r) = sys_none(); break;
    case SYS_open: SYSCALL_ARG1(r) = sys_open((const char *)a[1], a[2], a[3]); break;
    case SYS_read: SYSCALL_ARG1(r) = sys_read(a[1], (void *)a[2], a[3]); break;
    case SYS_write: SYSCALL_ARG1(r) = sys_write(a[1], (void *)a[2], a[3]); break;
    case SYS_exit: SYSCALL_ARG1(r) = sys_exit(a[1]); break;
    case SYS_kill: TODO();
    case SYS_getpid: TODO();
    case SYS_close: SYSCALL_ARG1(r) = sys_close(a[1]); break;
    case SYS_lseek: SYSCALL_ARG1(r) = sys_lseek(a[1], a[2], a[3]); break;
    case SYS_brk: SYSCALL_ARG1(r) = sys_brk((void *)a[1]); break;
    case SYS_fstat: TODO();
    case SYS_time: TODO();
    case SYS_signal: TODO();
    case SYS_execve: TODO();
    case SYS_fork: TODO();
    case SYS_link: TODO();
    case SYS_unlink: TODO();
    case SYS_wait: TODO();
    case SYS_times: TODO();
    case SYS_gettimeofday: TODO();
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
