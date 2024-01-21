#include <stdlib.h>
#include <stdio.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL; // pairs, first chunk count and second line number
    chunk->linesCount = 0;
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
        /* if (line != linesCount + 1) { */
        /*     printf("Expected line: %d, got %d\n", */
        /*         linesCount + 1, line); */
        /* } */

        chunk->lines = GROW_ARRAY(int, chunk->lines,
            linesCount * 2, (linesCount + 1) * 2);

        chunk->lines[linesCount * 2] = chunk->count;
        chunk->lines[(linesCount * 2) + 1] = line;
        chunk->linesCount++;
    }

    chunk->code[chunk->count] = byte;
    /* chunk->lines[chunk->count] = line; */
    chunk->count++;
}

void freeChunk(Chunk* chunk) {
   FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
   /* FREE_ARRAY(int, chunk->lines, chunk->capacity); */
   freeValueArray(&chunk->constants);
   initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}
