#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;


  /* TODO: Add more members if necessary */
  char expr[32];
  int value;


} WP;
WP* new_wp();
void free_up(WP*wp);
void printlist();
void flash(WP*wp);
WP* getHead();
void free_watchpoint(int n);
#endif
