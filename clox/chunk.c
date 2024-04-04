#include <stdlib.h>
#include <stdio.h>

#include "chunk.h"
#include "memory.h"

static void initLines(Lines* lines) {
  lines->count = 0;
  lines->capacity = 0;
  lines->lines = NULL;
  lines->offsets = NULL;
}

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    initValueArray(&chunk->constants);
    initLines(&chunk->lines);
}

static void setLines(Chunk* chunk, int line) {
    if (chunk->lines.count > 0 && line <= chunk->lines.lines[chunk->lines.count - 1]) {
      return;
    }

    if (chunk->lines.capacity < chunk->lines.count + 1) {
      int oldCapacity = chunk->lines.capacity;
      chunk->lines.capacity = GROW_CAPACITY(oldCapacity);
      chunk->lines.lines = GROW_ARRAY(int, chunk->lines.lines,
        oldCapacity, chunk->lines.capacity);
      chunk->lines.offsets = GROW_ARRAY(int, chunk->lines.offsets,
        oldCapacity, chunk->lines.capacity);
    }

    chunk->lines.lines[chunk->lines.count] = line;
    chunk->lines.offsets[chunk->lines.count] = chunk->count;
    chunk->lines.count++;
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code,
            oldCapacity, chunk->capacity);
    }

    setLines(chunk, line);
    chunk->code[chunk->count++] = byte;
}

void freeChunk(Chunk* chunk) {
   FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
   FREE_ARRAY(int, chunk->lines.lines, chunk->lines.count);
   FREE_ARRAY(int, chunk->lines.offsets, chunk->lines.count);
   freeValueArray(&chunk->constants);
   initChunk(chunk);
}

static int valueIndex(ValueArray* array, Value value) {
  for (int i = 0; i < array->count; i++) {
    if (valuesEqual(value, array->values[i])) return i;
  }
  return -1;
}

int addConstant(Chunk* chunk, Value value) {
    int index = valueIndex(&chunk->constants, value);
    if (index != -1) return index;
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}
