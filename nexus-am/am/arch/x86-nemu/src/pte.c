#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
	uint32_t *pgdir=p->ptr;
	uint32_t dir_index=(uint32_t)va>>22;
	uint32_t pt_index=(uint32_t)va>>12&0x3ff;

	if((pgdir[dir_index]&PTE_P)==0){
		pgdir[dir_index]=(uint32_t)(palloc_f());
		pgdir[dir_index]=pgdir[dir_index]|PTE_P;
		for(int i=0;i<NR_PTE;i++){
			((uint32_t *)(pgdir[dir_index]))[i] = 0;
		}
	}
	uint32_t ptbase=(uint32_t)pgdir[dir_index]&0xfffff000;
	*(uint32_t*)(ptbase+pt_index*4)=(uint32_t)pa;
	*(uint32_t*)(ptbase+pt_index*4)=*(uint32_t*)(ptbase+pt_index*4)|PTE_P;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
	//envp,argv
	*(uint32_t*)(ustack.end-4)=0;
	*(uint32_t*)(ustack.end-8)=0;
	//_start函数参数与返回地址
	*(uint32_t*)(ustack.end-12)=0;
	*(uint32_t*)(ustack.end-16)=0;
	//_RegSet中断帧
	*(uint32_t*)(ustack.end-20)=2|FL_IF;
	*(uint32_t*)(ustack.end-24)=8;
	*(uint32_t*)(ustack.end-28)=(uint32_t)entry;
	*(uint32_t*)(ustack.end-32)=0;
	*(uint32_t*)(ustack.end-36)=0x81;
	// register
	*(uint32_t*)(ustack.end-40)=0;
	*(uint32_t*)(ustack.end-44)=0;
	*(uint32_t*)(ustack.end-48)=0;
	*(uint32_t*)(ustack.end-52)=0;
	*(uint32_t*)(ustack.end-56)=0;
	*(uint32_t*)(ustack.end-60)=0;
	*(uint32_t*)(ustack.end-64)=0;
	*(uint32_t*)(ustack.end-68)=0;
	return (_RegSet*)(ustack.end - 68);
}
