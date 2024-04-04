#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"
#include "vm.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset, int* previousLine);
int getLine(Chunk* chunk, int offset);

#endif
