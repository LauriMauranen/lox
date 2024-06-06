#include <stdlib.h>

#include "memory.h"
#include "vm.h"
#include "object.h"
#include "value.h"
#include "compiler.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

static void freeObject(Obj* obj) {
#ifdef DEBUG_LOG_GC
  printf("%p free type %d\n", obj, obj->type);
#endif

  switch(obj->type) {
    case OBJ_STRING: {
      ObjString* str = (ObjString*)obj;
      FREE_OBJ_STRING(str);
      break;
    }
    case OBJ_FUNCTION: {
      ObjFunction* func = (ObjFunction*)obj;
      freeChunk(&func->chunk);
      FREE(ObjFunction, func);
      break;
    }
    case OBJ_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)obj;
      FREE_ARRAY(ObjUpvalue*, closure->upvalues,
                 closure->upvalueCount);
      FREE(ObjClosure, obj);
      break;
    }
    case OBJ_UPVALUE: {
      FREE(ObjUpvalue, obj);
      break;
    }
    case OBJ_NATIVE: {
      FREE(ObjNative, obj);
      break;
    }
  }
}

static void markRoots() {
  for (int i = 0; i < vm.stackSize; i++) {
    markValue(vm.stack[i]);
  }

  for (int i = 0; i < vm.frameCount; i++) {
    markObject((Obj*)vm.frames[i].closure);
  }

  for (ObjUpvalue* upvalue = vm.openUpvalues;
       upvalue != NULL;
       upvalue = upvalue->next) {
    markObject((Obj*)upvalue);
  }

  markTable(&vm.globals);
  markCompilerRoots();
}

static void markArray(ValueArray array) {
  for (int i = 0; i < array.count; i++) {
    markValue(array.values[i]);
  }
}

static void blackenObject(Obj* object) {
#ifdef DEBUG_LOG_GC
  printf("%p blacken ", object);
  printValue(OBJ_VAL(object));
  printf("\n");
#endif

  switch (object->type) {
    case OBJ_STRING:
    case OBJ_NATIVE: break;
    case OBJ_UPVALUE: {
      markValue(((ObjUpvalue*)object)->closed);
      break;
    }
    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      markObject((Obj*)function->name);
      markArray(function->chunk.constants);
      break;
    }
    case OBJ_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)object;
      markObject((Obj*)closure->function);
      for (int i = 0; i < closure->upvalueCount; i++) {
        markObject((Obj*)closure->upvalues[i]);
      }
      break;
    }
  }
}

static void traceReferences() {
  while (vm.grayCount) {
    Obj* object = vm.grayStack[--vm.grayCount];
    blackenObject(object);
  }
}

static void sweep() {
  Obj* previous = NULL;
  Obj* object = vm.objects;
  while (object != NULL) {
    if (object->isMarked) {
      object->isMarked = false;
      previous = object;
      object = object->next;
    } else {
      Obj* next = object->next;
      if (previous != NULL) {
        previous->next = next;
      }
      freeObject(object);
      object = next;
    }
  }
}

void markTable(Table* table) {
  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    markObject((Obj*)entry->key);
    markValue(entry->value);
  }
}

void markValue(Value value) {
  if (IS_OBJ(value)) markObject(AS_OBJ(value));
}

void markObject(Obj* object) {
  if (object == NULL) return;
  if (object->isMarked) return;
  object->isMarked = true;

#ifdef DEBUG_LOG_GC
  printf("%p mark ", object);
  printValue(OBJ_VAL(object));
  printf("\n");
#endif

  if (vm.grayCapacity < vm.grayCount + 1) {
    vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
    vm.grayStack = (Obj**)realloc(vm.grayStack, sizeof(Obj*) * vm.grayCapacity);
    if (vm.grayStack == NULL) exit(1);
  }

  vm.grayStack[vm.grayCount++] = object;
}

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
  if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
    collectGarbage();
#endif
  }

  if (newSize == 0) {
      free(pointer);
      return NULL;
  }

  void* result = realloc(pointer, newSize);
  if (result == NULL) exit(1);

  return result;
}

void freeObjects() {
  Obj* object = vm.objects;
  while (object != NULL) {
    Obj* next = object->next;
    freeObject(object);
    object = next;
  }

  free(vm.grayStack);
}

void collectGarbage() {
#ifdef DEBUG_LOG_GC
  printf("-- gc begin\n");
#endif

  markRoots();
  traceReferences();
  tableRemoveWhite(&vm.strings);
  sweep();

#ifdef DEBUG_LOG_GC
  printf("-- gc end\n");
#endif
}
