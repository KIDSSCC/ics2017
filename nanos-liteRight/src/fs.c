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

int getWidth();
int getHeight();
void init_fs() {
  // TODO: initialize the size of /dev/fb
  int width = getWidth();
  int height = getHeight();
  Log("in fs.c/init_fs, width is: %d, height is: %d", width, height);
  file_table[3].size = width * height * sizeof(uint32_t);
}

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(void *buf, off_t offset, size_t len);

int fs_open(const char* pathName, int flags, int mode){
	int result = -1;
	for(int i=0;i<NR_FILES; i++){
		if(strcmp(pathName, file_table[i].name)==0){
			result = i;
			break;
		}
	}
	assert(result!=-1);
	return result;
}

void dispinfo_read(void* buf, off_t offset, size_t len);
size_t events_read(void* buf, size_t len);


ssize_t fs_read(int fd, void* buf, size_t len){
	int start = file_table[fd].disk_offset;
	int curr_off = file_table[fd].open_offset;
	int size = file_table[fd].size;
	int max = size-curr_off;
	if(max>len){
		max = len;
	}
	if(fd==FD_DISPINFO){
		dispinfo_read(buf, curr_off, max);
	}else if(fd==FD_EVENTS){
		return events_read(buf, len);
	}else{
		ramdisk_read(buf, start + curr_off, max);
	}
	file_table[fd].open_offset = curr_off + max;
	return max;
}

void fb_write(const void* buf, off_t offset, size_t len);
ssize_t fs_write(int fd, void * buf, size_t len){
	int start = file_table[fd].disk_offset;
	int curr_off = file_table[fd].open_offset;
	int size = file_table[fd].size;
	int max = size-curr_off;
	if(max>len){
		max = len;
	}
	if(fd==FD_FB){
		fb_write(buf, curr_off, max);
	}else{
		ramdisk_write(buf, start + curr_off, max);
	}
	file_table[fd].open_offset = curr_off + max;
	return max;
}

off_t fs_leek(int fd, off_t offset, int whence){
	switch(whence){
		case SEEK_SET:
			file_table[fd].open_offset = offset;
			return file_table[fd].open_offset;
		case SEEK_CUR:
			file_table[fd].open_offset = file_table[fd].open_offset + offset;
			return file_table[fd].open_offset;
		case SEEK_END:
			file_table[fd].open_offset = file_table[fd].size - offset;
			return file_table[fd].open_offset;
		default:
			panic("error no case in fs.c fs_leek");
			return -1;
	}
}

int fs_close(int fd){
	return 0;
}

size_t getSize(int fd){
	return file_table[fd].size;
}
