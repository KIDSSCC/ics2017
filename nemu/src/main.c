int init_monitor(int, char *[]);
void ui_mainloop(int);
#include "stdio.h"
int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);
  /* Receive commands from user. */
  //is_batch_mode=0
  ui_mainloop(is_batch_mode);
  return 0;
}
