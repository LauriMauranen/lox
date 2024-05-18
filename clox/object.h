#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)

#define AS_NATIVE(value) ((ObjNative*)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
  OBJ_STRING,
  OBJ_FUNCTION,
  OBJ_CLOSURE,
  OBJ_UPVALUE,
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

typedef struct {
  Obj obj;
  Value* location;
} ObjUpvalue;

typedef struct {
  Obj obj;
  int arity;
  Chunk chunk;
  ObjString* name;
  int upvalueCount;
} ObjFunction;

typedef struct {
  Obj obj;
  ObjFunction* function;
  ObjUpvalue** upvalues;
  int upvalueCount;
  Value* closedUpvalues;
} ObjClosure;

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
ObjClosure* newClosure(ObjFunction* function);
ObjUpvalue* newUpvalue(Value* slot);
ObjNative* newNative(NativeFn function);

static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
