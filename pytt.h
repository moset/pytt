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

#ifdef WIN32
/* Might not always be appropriate. Stolen from lookup3.h. :) */
typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;
#else
#include <stdint.h>
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

typedef struct pytt_entry_t
{
  struct pytt_entry_hdr_t  hdr;
  char                     data[];
} pytt_entry_t;

#define PYTT_MALLOC_TABLE_HEADER    1  /**< Use malloc to allocate table and bucket pointers. */

typedef struct
{
  uint16_t       bucket_bits;
  uint16_t       flags;
  uint32_t       data_size;
  uint32_t       hash_initializer;

  /** These get called to initialize and free data in entries. */
  void         (*create_callback)(pytt_entry_t *ent);
  void         (*remove_callback)(pytt_entry_t *ent);

  /** Memory management functions used when allocating entries and (optionally)
   *  when allocating the table itself. (See: PYTT_MALLOC_TABLE_HEADER flag).
   */
  void        *(*alloc)(uint32_t bytes);
  void         (*dealloc)(void *pointer);

  /** The first entry in the linked list. */
  pytt_entry_t  *first;
  /** Storage of the buckets that make up the hash table. */
  pytt_entry_t  *buckets[];
} pytt_t;


/** Create a new hash table. Uses malloc and free for memory management. */
pytt_t       *pytt_create(int bucket_bits, int data_size);

/** Create a new hash table using custom parameters. */
pytt_t       *pytt_create_custom(int bucket_bits, int data_size,
				 void *(*alloc)(uint32_t bytes),
				 void (*dealloc)(void *pointer),
				 uint32_t hash_initializer,
				 uint16_t flags);

/** Destroy a previously created hash table. */
void          pytt_destroy(pytt_t *ht);

/** Get the total number of buckets in a hash table. */
uint32_t      pytt_get_bucket_count(pytt_t *ht);

/** Create an entry for the key, or return the one that already exists. */
pytt_entry_t *pytt_entry_create(pytt_t *ht, const void *key, uint16_t keylen);
/** Create the entry for the key or NULL if it doesn't exist. */
pytt_entry_t *pytt_entry_get(pytt_t *ht, const void *key, uint16_t keylen);
/** Destroy the entry for a key. */
void          pytt_entry_remove(pytt_t *ht, const void *key, uint16_t keylen);
/** Destroy an entry */
void          pytt_entry_destroy(pytt_t *ht, pytt_entry_t *ent);

/** Get a pointer to the key for an entry. */
void         *pytt_entry_get_key_ptr(pytt_t *ht, pytt_entry_t *ent);

#endif /* PYTT_H */
