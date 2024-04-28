#include "common.h"
#include "syscall.h"

extern int fs_open(const char* pathName, int flags, int mode);
extern ssize_t fs_read(int fd, void* buf, size_t len);
extern ssize_t fs_write(int fd, void * buf, size_t len);
extern off_t fs_leek(int fd, off_t offset, int whence);
extern int close(int fd);

int sys_none(){
	return 1;
}
void sys_exit(int i){
	_halt(i);
}
int sys_write(int fd, void* buf, size_t len){
	if(fd==1||fd==2){
		Log("buffer is: %s", (char*)buf);
		char tmp;
		for(int i=0;i<len;i++){
			memcpy(&tmp, buf+i, 1);
			_putc(tmp);
		}
		return len;
	}
	else if(fd >=3){
		return fs_write(fd, buf, len);
	}
	return -1;
}
int sys_brk(int addr){
	return 0;
}

int sys_open(const char* name){
	return fs_open(name, 0, 0);
}

int sys_read(int fd, void* buf, size_t len){
	return fs_read(fd, buf, len);
}

int sys_lseek(int fd, off_t offset, int whence){
	return fs_leek(fd, offset, whence);
}

int sys_close(int fd){
	return fs_close(fd);
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
	  case SYS_none:
		  SYSCALL_ARG1(r) = sys_none();
		  break;
	  case SYS_exit:
		  sys_exit(a[1]);
		  break;
	  case SYS_write:
		  SYSCALL_ARG1(r) = sys_write(a[1], (void*)a[2], a[3]);
		  break;
	  case SYS_brk:
		  SYSCALL_ARG1(r) = sys_brk(a[1]);
		  break;
	  case SYS_open:
		  SYSCALL_ARG1(r) = sys_open((char*)a[1]);
		  break;
      case SYS_read:
		  SYSCALL_ARG1(r) = sys_read(a[1], (void*)a[2], a[3]);
		  break;
	  case SYS_lseek:
		  SYSCALL_ARG1(r) = sys_lseek(a[1], a[2], a[3]);
		  break;
	  case SYS_close:
		  SYSCALL_ARG1(r) = sys_close(a[1]);
		  break;


		 




    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
