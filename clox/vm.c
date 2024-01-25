#include <stdio.h>

#include "vm.h"
#include "common.h"
#include "debug.h"
#include "memory.h"
#include "compiler.h"

VM vm;

static void resetStack() {
  /* vm.stackTop = vm.stack; */
  vm.stackSize = 0;
  vm.stackCapasity = 0;
}

void initVM() {
  resetStack();
}

void freeVM() {
  FREE_ARRAY(Value, vm.stack, vm.stackCapasity);
}

void push(Value value) {
#define STACK_SIZE_INC 256
  if (vm.stackSize >= vm.stackCapasity) {
    int oldCapacity = vm.stackCapasity;
    vm.stackCapasity = vm.stackCapasity + STACK_SIZE_INC;
    vm.stack = GROW_ARRAY(Value, vm.stack, oldCapacity, vm.stackCapasity);
  }
  /* *vm.stackTop = value; */
  /* vm.stackTop++; */
  vm.stack[vm.stackSize] = value;
  vm.stackSize++;
#undef STACK_SIZE_INC
}

Value pop() {
    /* vm.stackTop--; */
    /* return *vm.stackTop; */
    vm.stackSize--;
    return vm.stack[vm.stackSize];
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
  do { \
    double b = pop(); \
    double a = pop(); \
    push(a op b); \
  } while(false) 

    for(;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("        ");
    /* for (Value* slot = vm.stack; slot < vm.stackTop; slot++) { */
    /*   printf("[ "); */
    /*   printValue(*slot); */
    /*   printf(" ]"); */
    /* } */
    for (int i = 0; i < vm.stackSize; i++) {
      printf("[ ");
      printValue(vm.stack[i]);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk,
        (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NEGATE: push(-pop()); break;
            case OP_ADD: BINARY_OP(+); break;
            case OP_SUBSTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE: BINARY_OP(/); break;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(char* source) {
  compile(source);
  return INTERPRET_OK;
}
