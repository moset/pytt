#include <stdio.h>
#include "pytt.h"

typedef struct {
  struct pytt_entry_hdr_t hdr;
  int value;
} int_entry_t;

PYTT_DECLARE_TYPED(int_entry_t, int_table)

int main(int argc, char **argv)
{
  int_table_t *ht = int_table_create(5);
  int_entry_t *ie;

  ie = int_table_entry_create_z(ht, "four");
  ie->value = 4;

  ie = int_table_entry_create_z(ht, "twenty-seven");
  ie->value = 27;

  ie = int_table_entry_get_z(ht, "four");
  printf("%d\n", ie->value);

  ie = int_table_entry_get_z(ht, "twenty-seven");
  printf("%d\n", ie->value);

  ie = ht->first;
  while(ie) {
    printf("%d\n", ie->value);
    ie = int_table_entry_next(ie);
  }

  return 0;
}

PYTT_IMPLEMENT_TYPED(int_entry_t, int_table)
