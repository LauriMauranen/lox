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
  FREE_ARRAY(Value, vm.stack, vm.stackCapasity);
  vm.stackSize = 0;
  vm.stackCapasity = 0;
  vm.frameCount = 0;
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  CallFrame* frame = &vm.frames[vm.frameCount - 1];
  size_t instruction = frame->ip - frame->function->chunk.code - 1;
  int line = getLine(&frame->function->chunk, instruction);
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
  CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)

#define READ_CONSTANT() (frame->function->chunk.constants.values[READ_BYTE()])

#define READ_STRING() AS_STRING(READ_CONSTANT())

#define READ_SHORT() \
  (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8 | frame->ip[-1])))

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

#ifdef DEBUG_TRACE_EXECUTION
    int previousLine = 0;
#endif

    for(;;) {

#ifdef DEBUG_TRACE_EXECUTION
      printf("        ");
      for (int i = 0; i < vm.stackSize; i++) {
        printf("[ ");
        printValue(vm.stack[i]);
        printf(" ]");
      }
      printf("\n");
      disassembleInstruction(&frame->function->chunk,
          (int)(frame->ip - frame->function->chunk.code), &previousLine);
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_RETURN: {
              if (vm.frameCount == 1) return INTERPRET_OK;
              Value value = pop();
              uint8_t nLocals = READ_BYTE();
              // ehkei paras paikka muokata paljon pinoa
              while (nLocals--) pop();
              push(value); // return value
              vm.frameCount--;
              frame = &vm.frames[vm.frameCount - 1];
              break;
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
              ObjString* name = READ_STRING();
              tableSet(&vm.globals, name, pop());
              break;
            }
            case OP_GET_GLOBAL: {
              ObjString* name = READ_STRING();
              Value value;
              if (!tableGet(&vm.globals, name, &value)) {
                runtimeError("Undefined variable. '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
              }
              push(value);
              break;
            }
            case OP_SET_GLOBAL: {
              ObjString* name = READ_STRING();
              if (tableSet(&vm.globals, name, peek(0))) {
                tableDelete(&vm.globals, name);
                runtimeError("Undefined variable. '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
              }
              break;
            }
            case OP_GET_LOCAL: {
              uint8_t slot = READ_BYTE();
              push(vm.stack[slot + frame->slot]);
              break;
            }
            case OP_SET_LOCAL: {
              uint8_t slot = READ_BYTE();
              vm.stack[slot + frame->slot] = peek(0);
              /* printf("stack[%d]: ", slot + frame->slot); */
              /* printValue(vm.stack[slot + frame->slot]); */
              /* printf("\n"); */
              break;
            }
            case OP_JUMP_IF_FALSE: {
              uint16_t offset = READ_SHORT();
              if (isFalsey(peek(0))) frame->ip += offset;
              break;
            }
            case OP_JUMP: {
              uint16_t offset = READ_SHORT();
              frame->ip += offset;
              break;
            }
            case OP_LOOP: {
              uint16_t offset = READ_SHORT();
              frame->ip -= offset;
              break;
            }
            case OP_CALL: {
              uint8_t nArgs = READ_BYTE();
              frame = &vm.frames[vm.frameCount++];
              frame->function = AS_FUNCTION(pop());
              frame->ip = frame->function->chunk.code;

              int arity = frame->function->arity;
              if (nArgs != arity) {
                runtimeError("Function called with wrong number of arguments.");
                return INTERPRET_RUNTIME_ERROR;
              }
              frame->slot = vm.stackSize - 1 - arity; // onko näin?
              Value arguments[arity];

              // argumentit pinosta
              for (int i = 0; i < arity; i++) {
                arguments[i] = pop();
              }
              // argumentit muuttujiksi
              for (int i = 0; i < arity; i++) {
                push(NIL_VAL);
              }
              // argumentit takaisin pinoon
              for (int i = arity - 1; i >= 0; i--) {
                push(arguments[i]);
              }
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
#undef READ_SHORT
}

void initVM() {
  resetStack();
  vm.objects = NULL;
  initTable(&vm.strings);
  initTable(&vm.globals);
}

void freeVM() {
  resetStack();
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
  ObjFunction* function = compile(source);
  if (function == NULL) return INTERPRET_COMPILE_ERROR;

  push(OBJ_VAL(function));
  CallFrame* frame = &vm.frames[vm.frameCount++];
  frame->function = function;
  frame->ip = function->chunk.code;
  frame->slot = vm.stackSize - 1; // onko varma?

  return run();
}
