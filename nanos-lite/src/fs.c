#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = _screen.width * _screen.height * 4;
}

int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < NR_FILES; i++) {
    if (strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  Log("fs_open: %s not found!", pathname);
  assert(0);
  return 0;
}

void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);
void dispinfo_read(void *buf, off_t offset, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);

ssize_t fs_read(int fd, void *buf, size_t len) {
  Finfo *f = &file_table[fd];
  switch (fd)
  {
  case FD_STDIN:
  case FD_STDOUT:
  case FD_STDERR:
    break;
  case FD_FB:
    assert(0);
    break;
  case FD_EVENTS:
    assert(0);
    break;
  case FD_DISPINFO:
    dispinfo_read(buf, f->open_offset, len);
    f->open_offset += len;
    break;
  default:
    if (f->open_offset + len > f->size) {
      len = f->size - f->open_offset;
    }
    ramdisk_read(buf, f->disk_offset + f->open_offset, len);
    f->open_offset += len;
    break;
  }
  return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  Finfo *f = &file_table[fd];
  switch (fd)
  {
  case FD_STDIN:
    break;
  case FD_STDOUT:
  case FD_STDERR:
    for (int i = 0; i < len; i++) {
      _putc(((char *)buf)[i]);
    }
    break;
  case FD_FB:
    fb_write(buf, f->open_offset, len);
    f->open_offset += len;
    break;
  case FD_EVENTS:
    assert(0);
    break;
  case FD_DISPINFO:
    assert(0);
    break;
  default:
    if (f->open_offset + len > f->size) {
      len = f->size - f->open_offset;
    }
    ramdisk_write(buf, f->disk_offset + f->open_offset, len);
    f->open_offset += len;
    break;
  }
  return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  Finfo *f = &file_table[fd];
  switch (whence) {
    case SEEK_SET:
      f->open_offset = offset;
      break;
    case SEEK_CUR:
      f->open_offset += offset;
      break;
    case SEEK_END:
      f->open_offset = f->size + offset;
      break;
    default:
      assert(0);
  }
  return f->open_offset;
}

int fs_close(int fd) {
  return 0;
}

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}