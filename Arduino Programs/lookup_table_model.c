#include <stdio.h>
#include <string.h>

typedef struct {
  char opname[10];
  uint8_t opcode;
} op_lookup;

op_lookup instructions[] = {
  {"NOP", 0},
  {"OUT", 1},
  {"HLT", 14}
}

int return_opcode(char opname_search[]) {
  int op_num = sizeof(instructions) / sizeof(instructions[0]);
  int i, ret = -1;
  
  for (i = 0; i < op_num; i++) {
    if (strcmp(opname_search, instructions[i].opname) == 0)
      ret = instructions[i].opcode;
  }
  
  return ret;
}

void main() {
  printf("%s\t%d\n", "HLT", return_opcode("HLT"));
}
