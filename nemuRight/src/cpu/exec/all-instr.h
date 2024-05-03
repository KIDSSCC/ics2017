#include "cpu/exec.h"

make_EHelper(mov);
make_EHelper(push);
make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);
//dummy.c
make_EHelper(call);
make_EHelper(pop);
make_EHelper(sub);
make_EHelper(xor);
make_EHelper(ret);

//阶段二
//add.c
make_EHelper(lea);
make_EHelper(and);
make_EHelper(nop);
make_EHelper(add);
make_EHelper(cmp);
make_EHelper(setcc);
make_EHelper(movzx);
make_EHelper(movsx);
make_EHelper(test);
//add-longlong.c
make_EHelper(jcc);
make_EHelper(adc);
make_EHelper(or);

//bit.c
make_EHelper(sar);
make_EHelper(shl);
make_EHelper(shr);
make_EHelper(dec);
make_EHelper(not);
//bubble-sort.c
make_EHelper(inc);
//fact
make_EHelper(jmp);
make_EHelper(imul1);
make_EHelper(imul2);
make_EHelper(imul3);
//goldbach
make_EHelper(cltd);
make_EHelper(idiv);
//hello-str
make_EHelper(jmp_rm);
//jmp指令的E9形式
make_EHelper(leave);
make_EHelper(div);
//recursion
make_EHelper(call_rm);
//sub-longlong
make_EHelper(sbb);


//阶段三
make_EHelper(in);
make_EHelper(out);
//coreMark
make_EHelper(mul);
make_EHelper(cwtl);
//microbench
make_EHelper(neg);
make_EHelper(rol);