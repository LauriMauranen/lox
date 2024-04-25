#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"
#include "table.h"

#define ALLOCATE_OBJ(type, objectType) \
  (type*)allocateObject(sizeof(type), objectType)

#define ALLOCATE_OBJ_STRING(length) \
  (ObjString*)allocateObject(sizeof(ObjString) + length * sizeof(char), OBJ_STRING)

static Obj* allocateObject(size_t size, ObjType type) {
  Obj* object = (Obj*)reallocate(NULL, 0, size);
  object->type = type;
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

static ObjString* allocateString(const char* chars, int length, uint32_t hash) {
  ObjString* string = ALLOCATE_OBJ_STRING(length + 1);
  string->length = length;
  string->hash = hash;
  memcpy(string->chars, chars, length);
  string->chars[length] = '\0';
  tableSet(&vm.strings, string, NIL_VAL);
  return string;
}

uint32_t hashString(const char* key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

ObjString* copyString(const char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) return interned;
  return allocateString(chars, length, hash);
}

ObjFunction* newFunction() {
  ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  function->arity = 0;
  function->name = NULL;
  initChunk(&function->chunk);
  return function;
}

void printObject(Value value) {
  switch(OBJ_TYPE(value)) {
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
    case OBJ_FUNCTION: {
      ObjFunction* func = AS_FUNCTION(value);
      if (func->name == NULL) {
        printf("<script>");
      } else {
        printf("<fn %s>", func->name->chars);
      }
      break;
    }
    case OBJ_NATIVE: {
      printf("<native fn>");
      break;
    }
  }
}

ObjNative* newNative(NativeFn function) {
  ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->function = function;
  return native;
}

// alkuperäinen toteutus

/* static Obj* allocateObject(size_t size, ObjType type) { */
/*   Obj* object = (Obj*)reallocate(NULL, 0, size); */
/*   object->type = type; */
/*   object->next = vm.objects; */
/*   vm.objects = object; */
/*   return object; */
/* } */

/* static ObjString* allocateString(char* chars, int length) { */
/*   ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING); */
/*   string->length = length; */
/*   string->chars = chars; */
/*   return string; */
/* } */

/* ObjString* takeString(char* chars, int length) { */
/*   return allocateString(chars, length); */
/* } */

/* ObjString* copyString(const char* chars, int length) { */
/*   char* heapChars = ALLOCATE(char, length + 1); */
/*   memcpy(heapChars, chars, length); */
/*   heapChars[length] = '\0'; */
/*   return allocateString(heapChars, length); */
/* } */
