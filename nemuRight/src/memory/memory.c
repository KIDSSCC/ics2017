#include "nemu.h"
#include "device/mmio.h"
#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })


/*
 * 按照paddr_read的调用格式pmem_rw(addr, uint32_t)
 * 其被宏定义替换为 *(uint32_t*)(
 *  {
 *      Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr);
 *      guest_to_host(addr);
 *  })
 *  其中guest_to_host的作用是在pmem的基址之进行一定的偏移，拿到的是一个void*,需要再手动转换一下
 */
uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
    //~0U生成的是32位全1
    /*
     * len=1 8位全1
     * len=2 16位全1
     * len=3 24位全1
     * len=4 32位全1
     */
    //pmem_rw最终返回的是一个32位的地址，后面是一个掩码？
  int ctrl=is_mmio(addr);
  if(ctrl==-1)
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  else
    return mmio_read(addr,len,ctrl);
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int ctrl=is_mmio(addr);
  if(ctrl==-1)
    memcpy(guest_to_host(addr), &data, len);
  else
    mmio_write(addr,len,data,ctrl);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  paddr_write(addr, len, data);
}
