//kidsscc:provide the func def
int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  //kidsscc:the is)batch_mode is zero if init_monitor is success 
  //git log test
  ui_mainloop(is_batch_mode);

  return 0;
}
