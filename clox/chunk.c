#include <stdlib.h>
#include <stdio.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL; // line numbres
    chunk->linesCount = 0;
    chunk->linesChunk = NULL; // chunk count where line starts
    initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code,
            oldCapacity, chunk->capacity);
        /* chunk->lines = GROW_ARRAY(int, chunk->lines, */
        /*     oldCapacity, chunk->capacity); */
    }

    const int linesCount = chunk->linesCount;
    if (line > linesCount) {
        chunk->lines = GROW_ARRAY(int, chunk->lines,
            linesCount, linesCount + 1);
        chunk->linesChunk = GROW_ARRAY(int, chunk->linesChunk,
            linesCount, linesCount + 1);

        chunk->lines[linesCount] = line;
        chunk->linesChunk[linesCount] = chunk->count;
        chunk->linesCount++;
    }

    chunk->code[chunk->count] = byte;
    /* chunk->lines[chunk->count] = line; */
    chunk->count++;
}

void freeChunk(Chunk* chunk) {
   FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
   FREE_ARRAY(int, chunk->lines, chunk->linesCount);
   FREE_ARRAY(int, chunk->linesChunk, chunk->linesCount);
   freeValueArray(&chunk->constants);
   initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}
