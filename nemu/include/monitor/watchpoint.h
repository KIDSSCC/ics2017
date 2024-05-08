#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[32];
  uint32_t value;
  bool enable;

} WP;

WP* new_wp();

void free_wp(WP* wp);

void show_wp();

WP* get_wp(int NO);

bool check_wp();

#endif
