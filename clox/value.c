#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "value.h"
#include "object.h"

void initValueArray(ValueArray* array) {
    array->capacity = 0;
    array->count = 0;
    array->values = NULL;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values,
            oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

bool valuesEqual(Value a, Value b) {
  if (a.type != b.type) return false;

  switch (a.type) {
    case VAL_NIL: return true;
    case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ: return AS_OBJ(a) == AS_OBJ(b);
    default: return false;
  }
}

void printValue(Value value) {
  switch(value.type) {
    case VAL_BOOL:
      printf(AS_BOOL(value) ? "true" : "false");
      break;
    case VAL_NIL: printf("nil"); break;
    case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
    case VAL_OBJ: printObject(value); break;
  }
}

void printValueArray(ValueArray* array) {
  printf("[");
  for (int i = 0; i < array->count; i++) {
    printValue(array->values[i]);
    printf(", ");
  }
  printf("]\n");
}

/* static uint32_t hashStringValue(Value value) { */
/*   ObjString* str = (ObjString*)AS_OBJ(value); */
/*   return hashString(str->chars, str->length); */
/* } */

/* static uint32_t hashNumber(Value value) { */
/*   int size = 10; */
/*   char str[size]; */
/*   snprintf(str, size, "%f", AS_NUMBER(value)); */
/*   return hashString(str, strlen(str)); */
/* } */

/* static uint32_t hashBoolean(Value value) { */
/*   if (AS_BOOL(value)) return 1; */
/*   return 0; */
/* } */

/* uint32_t hashValue(Value value) { */
/*   switch(value.type) { */
/*     case VAL_OBJ: return hashStringValue(value); */
/*     case VAL_NUMBER: return hashNumber(value); */
/*     case VAL_BOOL: return hashBoolean(value); */
/*     case VAL_NIL: return 0; */
/*   } */
/* } */
