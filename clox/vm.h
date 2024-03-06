#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"

typedef struct {
  Chunk* chunk;
  uint8_t* ip;
  Value* stack;
  int stackSize;
  int stackCapasity;
  /* Value* stackTop; */
  Table strings;
  Obj* objects;
  Table globals;
} VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(char* source);
void push(Value value);
Value pop();

#endif
