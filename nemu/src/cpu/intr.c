#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  memcpy(&t1,&cpu.eflags,sizeof(cpu.eflags));
  rtl_li(&t0,t1);
  rtl_push(&t0);

  cpu.eflags.IF = 0;
  
  rtl_push(&cpu.cs);
  rtl_li(&t0,ret_addr);
  rtl_push(&t0);

  vaddr_t descripter=cpu.idtr.addr+NO*sizeof(GateDesc);
  uint32_t low=vaddr_read(descripter,2);
  uint32_t up=vaddr_read(descripter+sizeof(GateDesc)-2,2);
  uint32_t addr=(up<<16)+low;
  //jump to intr
  decoding.is_jmp=1;
  decoding.jmp_eip=addr;
}

void dev_raise_intr() {
	cpu.INTR = true;
}
