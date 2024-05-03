#include "nemu.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "monitor/expr.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 10

int nemu_state = NEMU_STOP;

void exec_wrapper(bool);

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  //在命令c下，参数为-1,在uint64_t下，其为上限值，没有断点的话会执行完全部的指令
  if (nemu_state == NEMU_END) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  nemu_state = NEMU_RUNNING;

  bool print_flag = n < MAX_INSTR_TO_PRINT;
  for (; n > 0; n --) {

    /* Execute one instruction, including instruction fetch,
     * instruction decode, and the actual execution. */
    exec_wrapper(print_flag);

#ifdef DEBUG
    /* TODO: check watchpoints here. */
    //检查各个观察点的表达式值是否发生了变化
    WP* itea=getHead();
    while(itea!=NULL)
    {
        int error_type=0;
        int curr_value=expr(itea->expr,&error_type);
        if(curr_value!=itea->value)
        {
            itea->value=curr_value;
          nemu_state = NEMU_STOP;
          //printf("watchpoint:%d is triggered at 0x%08x,the expression is %s\n",itea->NO,cpu.eip,itea->expr);
          printf("watchpoint:%d is triggered at 0x%08x,the expression is %s and the eax is %08x\n",itea->NO,cpu.eip,itea->expr,cpu.eax);
          break;
        }
        itea=itea->next;
    }


#endif

#ifdef HAS_IOE
    extern void device_update();
    device_update();
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
