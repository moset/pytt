#include <stdlib.h>
#include <string.h>
#include "lookup3.h"
#include "pytt.h"

#define PYTT_DEFAULT_HASH_INITIALIZER         0x20071023

static void ll_insert_before(pytt_entry_t *pos, pytt_entry_t *node)
{
  node->hdr.prev = pos->hdr.prev;
  node->hdr.next = pos;
	
  if(node->hdr.prev) {
    node->hdr.prev->hdr.next = node;
  }

  if(node->hdr.next) {
    node->hdr.next->hdr.prev = node;
  }
}

pytt_t *pytt_create(int bucket_bits, int data_size)
{
  return pytt_create_custom(bucket_bits, data_size,
			    &malloc,
			    &free, 
			    PYTT_DEFAULT_HASH_INITIALIZER,
			    0);
}

pytt_t *pytt_create_custom(int bucket_bits, int data_size,
			   void *(*alloc)(unsigned int bytes),
			   void (*dealloc)(void *pointer),
			   uint32_t hash_initializer,
			   uint16_t flags)
{
  pytt_t *ht;
  uint32_t nbuckets = 1<<(bucket_bits);
  uint32_t table_size = sizeof(pytt_t) + nbuckets * sizeof(pytt_entry_t *);

  if((flags & PYTT_MALLOC_TABLE_HEADER) || !alloc) {
    ht = malloc(table_size);
  } else {
    ht = alloc(table_size);
  }

  memset(ht, 0, table_size);

  if(alloc) {
    ht->alloc = alloc;
  } else {
    ht->alloc = malloc;
  }

  if(dealloc) {
    ht->dealloc = dealloc;
  } else {
    ht->dealloc = free;
  }

  ht->data_size = data_size;
  ht->bucket_bits = bucket_bits;
  ht->flags = flags;
  ht->hash_initializer = hash_initializer;
  ht->first = NULL;

  return ht;
}

uint32_t pytt_get_bucket_count(pytt_t *ht)
{
  return 1<<(ht->bucket_bits);
}

void *pytt_entry_get_key_ptr(pytt_t *ht, pytt_entry_t *ent)
{
  return ent->data + ht->data_size;
}

pytt_entry_t *pytt_entry_create(pytt_t *ht, const void *key, uint16_t keylen)
{
  unsigned int mask = (1<<(ht->bucket_bits))-1;
  unsigned int bucket = hashlittle(key, keylen, ht->hash_initializer) & mask;
  pytt_entry_t *ent = NULL;
  pytt_entry_t *before = NULL;

  /* Check for possible collision */
  pytt_entry_t *b = ht->buckets[bucket];
  while(b) {

    /* If we find an entry already exists for this key, return it. */
    if(b->hdr.keylen == keylen && !memcmp(b->data + ht->data_size, key, keylen)) {
      return b;
    }

    /* If we're at the end of the collision list, we need look no further. */
    if(b->hdr.flags & PYTT_ENTRY_LAST_IN_BUCKET) {
      break;
    }

    b = b->hdr.next;
  }

  if(! ent) {
    uint32_t ent_size = sizeof(pytt_entry_t) + keylen + ht->data_size;
    ent = ht->alloc(ent_size);
    memcpy(ent->data + ht->data_size, key, keylen);
    ent->hdr.keylen = keylen;
	ent->hdr.prev = NULL;
	ent->hdr.next = NULL;
	ent->hdr.flags = 0;

    if(ht->buckets[bucket]) {
      before = ht->buckets[bucket];
    } else {
      before = ht->first;
      ent->hdr.flags |= PYTT_ENTRY_LAST_IN_BUCKET;
    }

    if(before) {
      ll_insert_before(before, ent);
    }

    ht->buckets[bucket] = ent;
  }

  if(! ent->hdr.prev) {
    ht->first = ent;
  }

  if(ht->create_callback) {
    ht->create_callback(ent);
  }

  return ent;
}

pytt_entry_t *pytt_entry_get(pytt_t *ht, const void *key, uint16_t keylen)
{
  unsigned int mask = (1<<(ht->bucket_bits))-1;
  unsigned int bucket = hashlittle(key, keylen, ht->hash_initializer) & mask;

  pytt_entry_t *b = ht->buckets[bucket];
  while(b) {
    if(b->hdr.keylen == keylen && !memcmp(b->data + ht->data_size, key, keylen)) {
      return b;
    }

    if(b->hdr.flags & PYTT_ENTRY_LAST_IN_BUCKET) {
      return NULL;
    }

    b = b->hdr.next;
  }

  return NULL;
}

void pytt_entry_remove(pytt_t *ht, const void *key, uint16_t keylen)
{
  pytt_entry_t *ent = pytt_entry_get(ht, key, keylen);

  if(ent) {
    pytt_entry_destroy(ht, ent);
  }
}

void pytt_entry_destroy(pytt_t *ht, pytt_entry_t *ent)
{
  if(ht->remove_callback) {
    ht->remove_callback(ent);
  }

  if(! ent->hdr.prev) {
    ht->first = ent->hdr.next;
  } else {
    ent->hdr.prev->hdr.next = ent->hdr.next;
  }

  if(ent->hdr.next) {
    ent->hdr.next->hdr.prev = ent->hdr.prev;
  }

  ht->dealloc(ent);
}

void pytt_destroy(pytt_t *ht)
{
  pytt_entry_t *ent = ht->first;
	
  while(ent) {
    pytt_entry_t *next = ent->hdr.next;
    if(ht->remove_callback) {
      ht->remove_callback(ent);
    }

    ht->dealloc(ent);
    ent = next;
  }

  if(ht->flags & PYTT_MALLOC_TABLE_HEADER) {
    free(ht);
  } else {
    ht->dealloc(ht);
  }
}
