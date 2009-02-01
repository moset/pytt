#ifndef LOOKUP_3_H
#define LOOKUP_3_H
#ifdef WIN32
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#else
#include <stdint.h>     /* defines uint32_t etc */
#endif

uint32_t hashword(const uint32_t *k, size_t length, uint32_t initval);
void hashword2(const uint32_t *k, size_t length, uint32_t *pc, uint32_t *pb);
uint32_t hashlittle( const void *key, size_t length, uint32_t initval);
void hashlittle2(const void *key, size_t length, uint32_t *pc, uint32_t *pb);
uint32_t hashbig( const void *key, size_t length, uint32_t initval);


#endif /* LOOKUP_3_H */
