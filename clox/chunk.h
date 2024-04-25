#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

#define str(x) #x
#define ENUM(e) str(e)

typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_PRINT,
    OP_POP,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_CLOSURE,
    OP_SET_CLOSURE,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_LOOP,
    OP_CALL,
    OP_RETURN,
} OpCode;

typedef struct {
  int* lines;
  int* offsets;
  int count;
  int capacity;
} Lines;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    ValueArray constants;
    Lines lines;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);

#endif
