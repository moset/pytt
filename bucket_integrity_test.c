#include <stdio.h>
#include "pytt.h"

typedef struct {
  struct pytt_entry_hdr_t hdr;
  int value;
} int_entry_t;

PYTT_TYPED_WITH_OPTIONS(int_entry_t, int_table,
			(void *) &key, sizeof(int),
			PYTT_NO_INITIALIZER,
			int key)

// Lists the buckets and finds the last entry in bucket 0.
int_entry_t *list_buckets(int_table_t *ht)
{
  int		 num_buckets = 1 << ht->bucket_bits;
  int_entry_t	*ie, *ret = NULL;
  int		 i;

  for (i = 0; i != num_buckets; ++i) {
    ie = ht->buckets[i];

    printf("Bucket %d: ", i);
    while(ie) {
      printf("%d ", ie->value);

      if (ie->hdr.flags & PYTT_ENTRY_LAST_IN_BUCKET) {
	if (i == 0)
	  ret = ie;

	printf("END\n");
	break;
      } else {
	ie = int_table_entry_next(ie);
      }
    }
  }

  return ret;
}

int main(int argc, char **argv)
{
  int_table_t *ht = int_table_create(4);
  int_entry_t *ie;
  int i;

  for (i = 0; i != 500; ++i) {
    ie = int_table_entry_create(ht, i);
    ie->value = i;
  }

  int_entry_t *last = list_buckets(ht);
  int bucket0_next_to_last_before = int_table_entry_prev(last)->value;

  int_table_entry_destroy(ht, last);

  puts("");

  last = list_buckets(ht);
  int bucket0_last_after  = last->value;

  int failure = (bucket0_last_after == bucket0_next_to_last_before) ? 0 : 1;

  printf("Bucket 0 integrity test %s. Last entry: %d, expected: %d\n",
	 failure ? "failed" : "succeeded",
	 bucket0_last_after, bucket0_next_to_last_before);

  return failure;
}
