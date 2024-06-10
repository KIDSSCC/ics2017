#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,
int getWidth();
int getHeight();

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};
int current_game = 0;
size_t events_read(void *buf, size_t len) {
	char tmp[10];
	bool whetherDown = false;
	int key = _read_key();
	if(key&0x8000){
		key^=0x8000;
		whetherDown = true;
	}
	if(whetherDown && key == _KEY_F12){
		current_game = current_game==0?2:0;
		Log("change the game");
	}

	if(key!=_KEY_NONE){
		if(whetherDown){
			sprintf(tmp, "%s, %s\n", "kd", keyname[key]);
		}else{
			sprintf(tmp, "%s, %s\n", "ku", keyname[key]);
		}
	}else{
		sprintf(tmp, "t, %d\n", _uptime());
	}
	if(strlen(tmp)<=len){
		strncpy((char*)buf, tmp, strlen(tmp));
		return strlen(tmp);
	}
  return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
	memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
	int width = getWidth();

	int index = offset / 4;
	int startX = index % width;
	int startY = index / width;
	int total = len / 4;
	int finish = 0;
	while(total>0){
		int currLeft = width - startX;
		if(currLeft<=total){
			if(startY!=0){
				_draw_rect(buf + finish * 4, startX, startY, currLeft, 1);
				total-= currLeft;
				startX = 0;
				startY+=1;
				finish+=currLeft;
			}else{
				int rectY = total / width;
				_draw_rect(buf + finish * 4, startX, startY, currLeft, rectY);
				total-=rectY * width;
				startY+=rectY;
				finish+=rectY * width;
			}
		}else{
			_draw_rect(buf + finish * 4, startX, startY, total, 1);
			total = 0;
			finish+=total;
		}
	}
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  int width = getWidth();
  int height = getHeight();
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", width, height);
}
