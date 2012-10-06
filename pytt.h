/* Pytt - A simple hash table in C.
 *
 * Copyright (c) 2009, Oscar Sundbom
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * *****************************************************************************
 *
 * Each table has a fixed number of buckets. Collisions are handled
 * by a doubly linked list which doubles as a total list of all the
 * items and can therefore be used to iterate over all items in the
 * hash table quickly.
 *
 * Each entry is of a fixed size and all data for it is allocated
 * in a single block. The key is stored at the end of the data in
 * the *data pointer. This allows implementations to extend the
 * HashEntry struct by creating a struct of its own and casting
 * between them, like so:
 *
 * struct int_entry_t
 * {
 *   struct pytt_entry_hdr_t hdr;
 *   int value;
 *   char key[];
 * };
 *
 * This way, typing your own data and accessing the key is provided
 * automatically by the compiler. (Well, after a single cast. :))
 *
 * This code uses lookup3.c by Bob Jenkis for hash key calculation.
 */

#ifndef PYTT_H
#define PYTT_H

#ifndef PYTT_NO_STDINT
#include <stdint.h>
#else
/* Might not always be appropriate. Stolen from lookup3.h. :) */
typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;
#endif

#define PYTT_ENTRY_LAST_IN_BUCKET   1

struct pytt_entry_t;

/*
   Each entry is a linked list node, so that collisions can be handled.
   It also allows for fast iteration through the hash map. Each step
   takes O(1) time.
*/

struct pytt_entry_hdr_t
{
  struct pytt_entry_t  *prev;
  struct pytt_entry_t  *next;

  uint16_t              keylen;
  uint16_t              flags;
};

#define PYTT_HDR        struct pytt_entry_hdr_t  hdr

typedef struct pytt_entry_t
{
	PYTT_HDR;
	char                     data[];
} pytt_entry_t;

#define PYTT_MALLOC_TABLE_HEADER    1  /**< Use malloc to allocate table and bucket pointers
					*   even if alloc / dealloc is set. */
				        
typedef void *(*pytt_allocator_f)(size_t bytes);
typedef void (*pytt_deallocator_f)(void *pointer);

/* HOLY MOLY IT'S ALL A BIG MACRO! */
#define PYTT_DECLARE_TYPED_TABLE(entry_type, prefix)				\
typedef struct prefix##_t							\
{										\
  uint16_t       bucket_bits;							\
  uint16_t       flags;								\
  uint32_t       hash_initializer;						\
  size_t         data_size;							\
										\
  /** These get called to initialize and free data in entries. */		\
  void         (*create_callback)(entry_type *ent);				\
  void         (*remove_callback)(entry_type *ent);				\
										\
  /** Memory management functions used when allocating entries and (optionally)	\
   *  when allocating the table itself. (See: PYTT_MALLOC_TABLE_HEADER flag).	\
   */										\
  pytt_allocator_f alloc;							\
  pytt_deallocator_f dealloc;							\
										\
  /** The first entry in the linked list. */					\
  entry_type  *first;								\
  /** Storage of the buckets that make up the hash table. */			\
  entry_type  *buckets[];							\
} prefix ## _t;

PYTT_DECLARE_TYPED_TABLE(pytt_entry_t, pytt)

/** Create a new hash table. Uses malloc and free for memory management. */
extern pytt_t       *pytt_create(unsigned int bucket_bits,
				 size_t       data_size);

/** Create a new hash table using custom parameters. */
extern pytt_t       *pytt_create_custom(unsigned int	   bucket_bits,
					size_t		   data_size,
					pytt_allocator_f   alloc,
					pytt_deallocator_f dealloc,
					uint32_t	   hash_initializer,
					uint16_t	   flags);

/** Destroy a previously created hash table. */
extern void          pytt_destroy(pytt_t *ht);

/** Get the total number of buckets in a hash table. */
extern uint32_t      pytt_get_bucket_count(pytt_t *ht);

/** Create an entry for the key, or return the one that already exists. */
extern pytt_entry_t *pytt_entry_create(pytt_t *ht, const void *key, uint16_t keylen);
/** Create the entry for the key or NULL if it doesn't exist. */
extern pytt_entry_t *pytt_entry_get(pytt_t *ht, const void *key, uint16_t keylen);
/** Destroy the entry for a key. */
extern void          pytt_entry_remove(pytt_t *ht, const void *key, uint16_t keylen);
/** Same as pytt_entry_create, but with key being a zero-terminated string. */
extern pytt_entry_t *pytt_entry_create_z(pytt_t *ht, const char *key);
/** Same as pytt_entry_get, but with key being a zero-terminated string. */
extern pytt_entry_t *pytt_entry_get_z(pytt_t *ht, const char *key);
/** Same as pytt_entry_remove, but with key being a zero-terminated string. */
extern void pytt_entry_remove_z(pytt_t *ht, const char *key);

/** Destroy an entry */
extern void          pytt_entry_destroy(pytt_t *ht, pytt_entry_t *ent);
/** Return the next entry in order */
extern pytt_entry_t *pytt_entry_next(pytt_entry_t *ent);

/** Get a pointer to the key for an entry. */
extern void         *pytt_entry_get_key_ptr(pytt_t *ht, pytt_entry_t *ent);

#define PYTT_DECLARE_TYPED(entry_type, prefix)								\
  PYTT_DECLARE_TYPED_TABLE(entry_type, prefix)								\
  extern prefix ## _t *prefix ## _create(int bucket_bits);						\
  extern void prefix ## _destroy(prefix ## _t *ht);							\
  extern entry_type *prefix ## _entry_create(prefix ## _t *ht, const void *key, uint16_t keylen);	\
  extern entry_type *prefix ## _entry_get(prefix ## _t *ht, const void *key, uint16_t keylen);		\
  extern void prefix ## _entry_remove(prefix ## _t *ht, const void *key, uint16_t keylen);		\
  extern entry_type *prefix ## _entry_create_z(prefix ## _t *ht, const char *key);			\
  extern entry_type *prefix ## _entry_get_z(prefix ## _t *ht, const char *key);				\
  extern void prefix ## _entry_remove_z(prefix ## _t *ht, const char *key);				\
  extern void prefix ## _entry_destroy(prefix ## _t *ht, entry_type *ent);				\
  extern entry_type *prefix ## _entry_prev(entry_type *ent);						\
  extern entry_type *prefix ## _entry_next(entry_type *ent); 

#define PYTT_IMPLEMENT_TYPED_WITH_INITCODE(entry_type, prefix, initcode)			\
  prefix ## _t *prefix ## _create(int bucket_bits)						\
  {												\
	prefix ## _t *table =									\
          (prefix ## _t *) pytt_create(bucket_bits, sizeof(entry_type) - sizeof(pytt_entry_t)); \
	initcode										\
	return table;										\
  }												\
												\
  void prefix ## _destroy(prefix ## _t *ht)							\
  { pytt_destroy((pytt_t *) ht); }								\
												\
  entry_type *prefix ## _entry_create(prefix ## _t *ht, const void *key, uint16_t keylen)	\
  { return (entry_type *) pytt_entry_create((pytt_t *) ht, key, keylen); }			\
												\
  entry_type *prefix ## _entry_get(prefix ## _t *ht, const void *key, uint16_t keylen)		\
  { return (entry_type *) pytt_entry_get((pytt_t *) ht, key, keylen); }				\
												\
  void prefix ## _entry_remove(prefix ## _t *ht, const void *key, uint16_t keylen)		\
  { pytt_entry_remove((pytt_t *) ht, key, keylen); }						\
												\
  entry_type *prefix ## _entry_create_z(prefix ## _t *ht, const char *key)			\
  { return (entry_type *) pytt_entry_create_z((pytt_t *) ht, key); }				\
												\
  entry_type *prefix ## _entry_get_z(prefix ## _t *ht, const char *key)				\
  { return (entry_type *) pytt_entry_get_z((pytt_t *) ht, key); }				\
												\
  void prefix ## _entry_remove_z(prefix ## _t *ht, const char *key)				\
  { pytt_entry_remove_z((pytt_t *) ht, key); }							\
												\
  void prefix ## _entry_destroy(prefix ## _t *ht, entry_type *ent)				\
  { pytt_entry_destroy((pytt_t *) ht, (pytt_entry_t *) ent); }					\
												\
  extern entry_type *prefix ## _entry_prev(entry_type *ent)					\
  { return (entry_type *) ent->hdr.prev; }							\
												\
  extern entry_type *prefix ## _entry_next(entry_type *ent)					\
  { return (entry_type *) ent->hdr.next; }


#define PYTT_IMPLEMENT_TYPED(entry_type, prefix)  \
	PYTT_IMPLEMENT_TYPED_WITH_INITCODE(entry_type, prefix, ;)

#define PYTT_TYPED(entry_type, prefix)           \
  PYTT_DECLARE_TYPED_TABLE(entry_type, prefix)   \
  PYTT_IMPLEMENT_TYPED(entry_type, prefix);

#endif /* PYTT_H */
