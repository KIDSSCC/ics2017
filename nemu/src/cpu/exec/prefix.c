#include "cpu/exec.h"

//宏定义，在exec.h中，等效为void exec_real(vaddr_t *eip)
make_EHelper(real);

make_EHelper(operand_size) {
  decoding.is_operand_size_16 = true;
  exec_real(eip);
  decoding.is_operand_size_16 = false;
}
