#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"


typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    Value* stack;
    int stackSize;
    int stackCapasity;
    /* Value* stackTop; */
} VM;

typedef enum {
   INTERPRET_OK,
   INTERPRET_COMPILE_ERROR,
   INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpret(char* source);
void push(Value value);
Value pop();

#endif
