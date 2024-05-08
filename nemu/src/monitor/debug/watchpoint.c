#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  if(free_ == NULL){
    Log("No free watchpoint");
    return NULL;
  }
  WP* new = free_;
  free_ = free_->next;
  new->next = head;
  head = new;
  new->enable = true;
  return new;
}


void free_wp(WP* wp){
  WP* p = head;
  if(p == wp){
    head = head->next;
    wp->next = free_;
    free_ = wp;
    wp->enable = false;
    return;
  }
  while(p->next != NULL){
    if(p->next == wp){
      p->next = wp->next;
      wp->next = free_;
      free_ = wp;
      wp->enable = false;
      return;
    }
    p = p->next;
  }
  Log("No such watchpoint");
}

void show_wp() {
    if (head == NULL) {
      printf("No watchpoint set.\n");
      return;
    }

    // printf(" NO | Enable | Expression                       | Initial Value\n");
    // printf("----+--------+----------------------------------+--------------\n");
    printf(" NO | Expression                       | Initial Value\n");
    printf("----+----------------------------------+--------------\n");
    
    // for (int i = 0; i < NR_WP; ++i) {
    //   printf("%3d | %-6s | %-32s | 0x%08x\n", wp_pool[i].NO, wp_pool[i].enable ? "Yes" : "No", wp_pool[i].expr, wp_pool[i].value);
    // }
    for (int i = 0; i < NR_WP; ++i) {
      if (wp_pool[i].enable) {
        printf("%3d | %-32s | 0x%08x\n", wp_pool[i].NO, wp_pool[i].expr, wp_pool[i].value);
      }
    }
}

WP* get_wp(int NO){
  WP* p = head;
  while(p != NULL){
    if(p->NO == NO){
      return p;
    }
    p = p->next;
  }
  return NULL;
}

bool check_wp() {
  WP* p = head;
  bool success = true;
  bool changed = false;
  while(p != NULL){
    uint32_t new_value = expr(p->expr, &success);
    if(new_value != p->value){
      printf("Watchpoint %d: %s\n", p->NO, p->expr);
      printf("Old value = 0x%08x\n", p->value);
      printf("New value = 0x%08x\n", new_value);
      p->value = new_value;
      changed = true;
    }
    p = p->next;
  }
  return changed;
}