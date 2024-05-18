#include <stdlib.h>

#include "memory.h"
#include "vm.h"
#include "object.h"
#include "value.h"

static void freeObject(Obj* obj) {
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
      if (closure->closedUpvalues != NULL) {
        FREE_ARRAY(Value, closure->closedUpvalues,
                   closure->upvalueCount);
      }
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

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
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
}
