#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  //return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  int ctrl=is_mmio(addr);
  if(ctrl==-1){
	  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }else{
	  return mmio_read(addr,len,ctrl);
  }
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  //memcpy(guest_to_host(addr), &data, len);
  int ctrl=is_mmio(addr);
  if(ctrl==-1){
	  memcpy(guest_to_host(addr), &data, len);
  }else{
	  mmio_write(addr,len,data,ctrl);
  }
}

paddr_t page_translate(vaddr_t vaddr, bool is_write){
	uint32_t dir_index = vaddr >>22;
	uint32_t pt_index=vaddr>>12&0x3ff;
	uint32_t offset=vaddr&0xfff;

	paddr_t paddr=vaddr;
	if(cpu.CR0 &0x80000000){
		PDE pde=(PDE)paddr_read((uint32_t)(cpu.CR3+4*dir_index),4);
		Assert(pde.present,"addr=0x%x",vaddr);
		pde.accessed=1;

		uint32_t PTbase=pde.val&0xFFFFF000;
		PTE pte=(PTE)paddr_read((uint32_t)(PTbase+4*pt_index),4);
		Assert(pte.present,"addr=0x%x",vaddr);
		pte.accessed=1;

		uint32_t pgbase=pte.val&0xFFFFF000;
		paddr=pgbase+offset;
		if(is_write){
			pte.dirty=1;
		}
	}
	return paddr;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
	uint32_t curr_offset=addr&0xfff;
	if(len>0x1000-curr_offset){
		int len1=0x1000-curr_offset;
		int len2=len-len1;
		paddr_t paddr1=page_translate(addr,false);
		paddr_t paddr2=page_translate(addr+len1,false);
		uint32_t data1=paddr_read(paddr1,len1);
		uint32_t data2=paddr_read(paddr2,len2);
		return (data2<<(len1*8))+data1;
	}else{
		paddr_t paddr=page_translate(addr,false);
		return paddr_read(paddr, len);
	}
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
	uint32_t curr_offset=addr&0xfff;
	if(len>0x1000-curr_offset){
		int len1=0x1000-curr_offset;
		int len2=len-len1;
		paddr_t paddr1=page_translate(addr,false);
		paddr_t paddr2=page_translate(addr+len1,false);

		paddr_write(paddr1,len1,data);
		data=data>>(len1*8);
		paddr_write(paddr2,len2,data);
	}else{
		paddr_t paddr=page_translate(addr,true);
		paddr_write(paddr, len, data);
	}
}
