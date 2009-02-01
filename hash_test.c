#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pytt.h"

typedef struct int_entry_t
{
  struct pytt_entry_hdr_t hdr;
  int value;
  char key[];
} int_entry_t;

const char *countWords[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", NULL};

int main(int argc, char **argv)
{
  pytt_t *ht = pytt_create(5, 4);
  int_entry_t *he;
  const char **word = countWords;
  int count = 0;

  while(*word) {
    he = (int_entry_t *) pytt_entry_create(ht, *word, strlen(*word)+1);
    he->value = count;
    ++word;
    ++count;
  }

  puts("Lookups:");
  word = countWords;
  while(*word) {
    he = (int_entry_t *) pytt_entry_create(ht, *word, strlen(*word)+1);
    printf("%s = %d\n", he->key, he->value);
    ++word;
  }


  puts("");
  puts("Iterating:");
  he = (int_entry_t *) ht->first;
  while(he) {
    printf("%s = %d\n", he->key, he->value);
    he =(int_entry_t *)  he->hdr.next;
  }

  puts("Press enter to do destruction tests.\n");
  getchar();

  word = countWords;
  while(*word) {
    int_entry_t *he2;
    printf("Removing %s\n", *word);
    pytt_entry_remove(ht, *word, strlen(*word)+1);

    puts("");
    puts("Iterating:");
    he2 = (int_entry_t *) ht->first;
    while(he2) {
      printf("%s = %d\n", he2->key, he2->value);
      he2 =(int_entry_t *)  he2->hdr.next;
    }

    ++word;
  }

  pytt_destroy(ht);

  return 0;
}
