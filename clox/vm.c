#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "vm.h"
#include "common.h"
#include "debug.h"
#include "memory.h"
#include "compiler.h"
#include "object.h"
#include "table.h"

VM vm;

static void resetStack() {
  /* vm.stackTop = vm.stack; */
  vm.stackSize = 0;
  vm.stackCapasity = 0;
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = getLine(vm.chunk, instruction);
  fprintf(stderr, "[line %d] in script\n", line);
  resetStack();
}

static Value peek(int distance) {
  return vm.stack[vm.stackSize - 1 - distance];
}

static bool isFalsey(Value val) {
  return IS_NIL(val) || (IS_BOOL(val) && !AS_BOOL(val));
}

static void concatenate() {
  ObjString* b = AS_STRING(pop());
  ObjString* a = AS_STRING(pop());
  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString* result = copyString(chars, length);
  FREE_ARRAY(char, chars, length + 1);
  push(OBJ_VAL(result));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op) \
  do { \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
      runtimeError("Operands must be numbers."); \
      return INTERPRET_RUNTIME_ERROR; \
    } \
    double b = AS_NUMBER(pop()); \
    double a = AS_NUMBER(pop()); \
    push(valueType(a op b)); \
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
                return INTERPRET_OK;
            }
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_PRINT: {
              printValue(pop());
              printf("\n");
              break;
            }
            case OP_POP: pop(); break;
            case OP_DEFINE_GLOBAL: {
              Value name = READ_CONSTANT();
              tableSet(&vm.globals, &name, pop());
              break;
            }
            case OP_GET_GLOBAL: {
              Value name = READ_CONSTANT();
              Value value;
              if (!tableGet(&vm.globals, &name, &value)) {
                runtimeError("Undefined variable. '%s'.", AS_CSTRING(name));
                return INTERPRET_RUNTIME_ERROR;
              }
              push(value);
              break;
            }
            case OP_NEGATE:
              if (!IS_NUMBER(peek(0))) {
                runtimeError("Operand must be number.");
                return INTERPRET_RUNTIME_ERROR;
              }
              push(NUMBER_VAL(-AS_NUMBER(pop())));
              break;
            case OP_NIL: push(NIL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_NOT:
              push(BOOL_VAL(isFalsey(pop())));
              break;
            case OP_EQUAL: {
              Value a = pop();
              Value b = pop();
              push(BOOL_VAL(valuesEqual(a, b)));
              break;
            }
            case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS: BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD: {
              if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                concatenate();
              } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) { \
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a + b));
              } else {
                runtimeError("Operands must be two strings or two numbers.");
                return INTERPRET_RUNTIME_ERROR;
              }
              break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

void initVM() {
  resetStack();
  vm.objects = NULL;
  initTable(&vm.strings);
  initTable(&vm.globals);
}

void freeVM() {
  FREE_ARRAY(Value, vm.stack, vm.stackCapasity);
  freeTable(&vm.strings);
  freeTable(&vm.globals);
  freeObjects();
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


InterpretResult interpret(char* source) {
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();
  freeChunk(&chunk);
  return result;
}
