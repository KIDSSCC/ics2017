#include <am.h>
#include <klib.h>

int main(){
  _ioe_init();
  int sec = 1;
  while (1) {
	  unsigned long curr = _uptime();
    while(curr < 1000 * sec){
	   printf("I want to see _uptime() is: %lu\n", curr);
	   curr = _uptime();
    }
    if (sec == 1) {
      printf("%d second.\n", sec);
    } else {
      printf("%d seconds.\n", sec);
    }
    sec ++;
  }
  return 0;
}
