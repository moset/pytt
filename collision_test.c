#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pytt.h"

typedef struct int_entry_t
{
  struct pytt_entry_hdr_t hdr;
  int value;
  char key[];
} int_entry_t;

int main(int argc, char **argv)
{
  pytt_t *ht;
  int_entry_t *he;
  int bits = 16;
  FILE *datafile;
  char buffer[512];
  int entry_count = 0;
  int collision_count = 0;
  int bucket_count = 0;
  int largest_bucket = 0;
  pytt_entry_t **bucketptr;

  if(argc > 1 && atoi(argv[1])>0) {
    bits = atoi(argv[1]);
  }

  // Really. This won't work.
  if(bits > 32) {
    bits = 32;
  }

  datafile = fopen("data.txt", "r");
  if(! datafile) {
    fprintf(stderr, "Unable to open data.txt.\n");
    exit(1);
  }

  ht = pytt_create(bits, 4);

  while(fgets(buffer, 512, datafile)) {
    char *word = strchr(buffer, ' ');
    int idx = strlen(buffer);
    if(word) {
      *word = 0;
      ++word;
    }
    
    while(idx-- > 0) {
      if(buffer[idx] >= 0 && buffer[idx] <= 32) {
	buffer[idx] = 0;
      } else {
	break;
      }
    }
   
    he = (int_entry_t *) pytt_entry_create(ht, word, strlen(word));
    he->value = atoi(buffer);
  }

  fclose(datafile);

  he = (int_entry_t *) ht->first;
  while(he) {
    ++entry_count;
    if(! (he->hdr.flags & PYTT_ENTRY_LAST_IN_BUCKET)) {
      ++collision_count;
    }
    he = (int_entry_t *) he->hdr.next;
  }

  bucketptr = ht->buckets+pytt_get_bucket_count(ht);
  while(bucketptr-- > ht->buckets) {
    if(*bucketptr) {
      int bucketsize = 0;
      
      he = (int_entry_t *) *bucketptr;
      while(he) {
	++bucketsize;
	if(he->hdr.flags & PYTT_ENTRY_LAST_IN_BUCKET) {
	  break;
	}
	he = (int_entry_t *) he->hdr.next;
      }

      if(bucketsize > largest_bucket) {
	largest_bucket = bucketsize;
      }

      ++bucket_count;
    } 
  }

  puts("");
  printf("Entries:        %8d\n", entry_count);
  printf("Buckets total:  %8d\n", pytt_get_bucket_count(ht));
  printf("Buckets used:   %8d\n", bucket_count);
  printf("Collisions:     %8d\n", collision_count);
  printf("Collision ratio:  %.4f\n", (double) collision_count / (double) entry_count);
  printf("Largest bucket: %8d\n", largest_bucket);

  pytt_destroy(ht);

  return 0;
}
