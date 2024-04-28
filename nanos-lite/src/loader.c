#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

void ramdisk_read(void *buf, off_t offset, size_t len); 
size_t get_ramdisk_size();



uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  Log("file name is %s", filename);
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, DEFAULT_ENTRY, getSize(fd));
  fs_close(fd);
  //size_t lenOfRamdisk = get_ramdisk_size();
  //ramdisk_read(DEFAULT_ENTRY, (off_t)0, lenOfRamdisk);
  return (uintptr_t)DEFAULT_ENTRY;
}
