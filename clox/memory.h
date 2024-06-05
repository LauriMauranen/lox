#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"
#include "table.h"

#define ALLOCATE(type, count) \
  (type*)reallocate(NULL, 0, sizeof(type) * count)

#define GROW_CAPACITY(capacity) \
   ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
   (type*)reallocate(pointer, sizeof(type) * oldCount, sizeof(type) * newCount)

#define FREE_ARRAY(type, pointer, oldCount) \
   reallocate(pointer, sizeof(type) * oldCount, 0)

#define FREE(type, pointer) \
   reallocate(pointer, sizeof(type), 0)

#define FREE_OBJ_STRING(pointer) \
   reallocate(pointer, sizeof(ObjString) + sizeof(char) * pointer->length, 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);
void freeObjects();
void freeTable(Table* table);
void collectGarbage();
void markValue(Value value);
void markObject(Obj* object);
void markTable(Table* table);

#endif
