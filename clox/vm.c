#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "vm.h"
#include "common.h"
#include "debug.h"
#include "memory.h"
#include "compiler.h"
#include "object.h"
#include "table.h"
#include "value.h"

VM vm;

static Value clockNative(int argCount, Value* args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void resetStack() {
  FREE_ARRAY(Value, vm.stack, vm.stackCapasity);
  vm.stackSize = 0;
  vm.stackCapasity = 0;
  vm.frameCount = 0;
}

static void freeClosureStates() {
  for (int i = 0; i < vm.closureStateCount; i++) {
    freeValueArray(vm.closureStates[i]);
  }
  vm.closureStateCount = 0;
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame* frame = &vm.frames[i];
    ObjFunction* function = frame->function;
    size_t instruction = frame->ip - function->chunk.code - 1;
    int line = getLine(&function->chunk, instruction);
    fprintf(stderr, "[line %d] in ", line);
    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }
  resetStack();
}

static void defineNative(const char* name, NativeFn function) {
  push(OBJ_VAL(copyString(name, (int)(strlen(name)))));
  push(OBJ_VAL(newNative(function)));
  tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
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

static bool call(ObjFunction* function, int argCount) {
  if (function->arity != argCount) {
    runtimeError("%s got %d arguments, expected %d.\n", function->name->chars, argCount, function->arity);
    return false;
  }

  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  CallFrame* newFrame = &vm.frames[vm.frameCount++];
  newFrame->function = function;
  newFrame->ip = function->chunk.code;
  newFrame->slot = vm.stackSize - argCount - 1;

  return true;
}

static bool callValue(Value callee, int argCount) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_FUNCTION:
        return call(AS_FUNCTION(callee), argCount);
        break;
      case OBJ_NATIVE: {
        NativeFn function = AS_NATIVE(callee)->function;
        Value args[argCount];
        for (int i = argCount - 1; i >= 0; i--) {
          args[i] = pop();
        }
        vm.stackSize--; // otetaan funktio pois pinosta;
        push(function(argCount, args));
        return true;
        break;
      }
      default:
        break;
    }
  }
  runtimeError("Can only call functions and classes.");
  return false;
}

static bool handleClosures(CallFrame* frame) {
  if (vm.closureStateCount == MAX_CLOSURE_STATES) {
    runtimeError("Too many closures.");
    return false;
  }

  ValueArray* state = (ValueArray*)reallocate(NULL, 0, sizeof(ValueArray));
  initValueArray(state);
  vm.closureStates[vm.closureStateCount++] = state;

  for (int i = frame->slot; i < vm.stackSize; i++) {
    writeValueArray(state, vm.stack[i]);
  }

  for (int i = 0; i < frame->function->closureCount; i++) {
    frame->function->closures[i]->state = state;
    /* printValueArray(frame->function->closures[i]->state); */
    /* printf("%p\n", frame->function->closures[i]); */
  }

  return true;
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
              Value result = pop();
              vm.frameCount--;
              if (vm.frameCount == 0) {
                pop();
                return INTERPRET_OK;
              }

              if (IS_FUNCTION(result) && frame->function->closureCount) {
                if (!handleClosures(frame)) return INTERPRET_RUNTIME_ERROR;
              }

              vm.stackSize = frame->slot;
              push(result);
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
              break;
            }
            case OP_GET_CLOSURE: {
              uint8_t slot = READ_BYTE();
              ObjFunction* func = frame->function;
              if (func->state == NULL) {
                push(vm.stack[vm.stackSize - 1 - slot]);
              } else {
                int size = func->state->count;
                push(func->state->values[size - 1 - slot]);
              }
              break;
            }
            case OP_SET_CLOSURE: {
              uint8_t slot = READ_BYTE();
              ObjFunction* func = frame->function;
              if (func->state == NULL) {
                vm.stack[vm.stackSize - 1 - slot] = peek(0);
              } else {
                int size = func->state->count;
                func->state->values[size - 1 - slot] = peek(0);
              }
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
              uint8_t argCount = READ_BYTE();
              if (!callValue(peek(argCount), argCount)) {
                return INTERPRET_RUNTIME_ERROR;
              }
              frame = &vm.frames[vm.frameCount - 1];
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

  vm.closureStateCount = 0;

  defineNative("clock", clockNative);
}

void freeVM() {
  resetStack();
  freeTable(&vm.strings);
  freeTable(&vm.globals);
  freeObjects();
  freeClosureStates();
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
    vm.stackSize--;
    return vm.stack[vm.stackSize];
}

InterpretResult interpret(char* source) {
  ObjFunction* function = compile(source);
  if (function == NULL) return INTERPRET_COMPILE_ERROR;

  push(OBJ_VAL(function));
  call(function, 0);
  return run();
}
