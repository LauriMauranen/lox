#include <stdio.h>

#include "test.h"
#include "table.h"
#include "value.h"
#include "object.h"
#include "memory.h"

static void printEntry(Entry entry) {
  printf("Entry: ");
  printValue(entry->key);
  printf(" -> ");
  printValue(entry.value);
  printf("\n");
}

static void testTable() {
  Table table;
  initTable(&table);
  Value key1 = NUMBER_VAL(1);
  Value key2 = NUMBER_VAL(1.0);
  Value key3 = NUMBER_VAL(-2.0);
  Value key4 = NUMBER_VAL(2.1);
  Value key5 = BOOL_VAL(true);
  Value key6 = BOOL_VAL(false);
  Value key7 = NIL_VAL;

  ObjString* str1 = copyStringNoVM("jee", 3);
  ObjString* str2 = copyStringNoVM("jaa", 3);
  Value key8 = OBJ_VAL(str1);
  Value key9 = OBJ_VAL(str2);

  Value val = NIL_VAL;
  tableSet(&table, &key1, val);
  tableSet(&table, &key2, val);
  tableSet(&table, &key3, val);
  tableSet(&table, &key4, val);
  tableSet(&table, &key5, val);
  tableSet(&table, &key6, val);
  tableSet(&table, &key7, val);
  tableSet(&table, &key8, val);
  tableSet(&table, &key9, val);
  tableSet(&table, &key8, NUMBER_VAL(1));

  /* tableDelete(&table, ) */

  for (int i = 0; i < table.capacity; i++) {
    if (table.entries[i].key == NULL) continue;
    printEntry(table.entries[i]);
  }

  freeTable(&table);
  FREE_OBJ_STRING(str1);
  FREE_OBJ_STRING(str2);
}

void runTests() {
  testTable();
}
