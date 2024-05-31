#include "common.h"
#include "memory.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

void ramdisk_read(void *buf, off_t offset, size_t len); 
size_t get_ramdisk_size();



uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  Log("file name is %s", filename);
  int fd = fs_open(filename, 0, 0);
  for(int i=0;i<getSize(fd);i+=0x1000){
	  void* new_pg=new_page();
	  _map(as,(void*)(DEFAULT_ENTRY+i),new_pg);
	  int leftdata=getSize(fd)-i>0x1000?0x1000:getSize(fd)-i;
	  fs_read(fd,new_pg,leftdata);
  }
  new_page();
  fs_close(fd);
  //size_t lenOfRamdisk = get_ramdisk_size();
  //ramdisk_read(DEFAULT_ENTRY, (off_t)0, lenOfRamdisk);
  return (uintptr_t)DEFAULT_ENTRY;
}
