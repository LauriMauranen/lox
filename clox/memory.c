#include <stdlib.h>

#include "memory.h"
#include "vm.h"
#include "object.h"

static void freeObject(Obj* obj) {
  switch(obj->type) {
    case OBJ_STRING: {
      ObjString* str = (ObjString*)obj;
      FREE_ARRAY(char, str->chars, str->length + 1);
      FREE(ObjString, str);
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
