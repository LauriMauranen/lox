#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"
#include "object.h"

#define FRAMES_MAX 64

typedef struct {
  ObjClosure* closure;
  uint8_t* ip;
  int slot; // where frames stack starts
} CallFrame;

typedef struct {
  CallFrame frames[FRAMES_MAX];
  int frameCount;

  /* Chunk* chunk; */
  /* uint8_t* ip; */

  Value* stack;
  int stackSize;
  int stackCapasity;

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
