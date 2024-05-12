#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)

#define AS_NATIVE(value) ((ObjNative*)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
  OBJ_STRING,
  OBJ_FUNCTION,
  OBJ_NATIVE,
} ObjType;

struct Obj {
  ObjType type;
  struct Obj* next;
};

struct ObjString {
  Obj obj;
  int length;
  /* char* chars; */
  uint32_t hash;
  char chars[];
};

typedef struct ObjFunction ObjFunction;

struct ObjFunction {
  Obj obj;
  int arity;
  Chunk chunk;
  ObjString* name;
  ObjFunction* closures[MAX_CLOSURES];
  int closureCount;
  ValueArray* state; // eri kuin ylemmät!
};

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
  Obj obj;
  NativeFn function;
} ObjNative;

ObjString* copyString(const char* chars, int length);
/* ObjString* copyStringNoVM(char* chars, int length); */
/* ObjString* takeString(char* chars, int length); */
void printObject(Value value);
uint32_t hashString(const char* str, int length);
ObjFunction* newFunction();
/* ObjClosure* newClosure(); */
ObjNative* newNative(NativeFn function);

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif