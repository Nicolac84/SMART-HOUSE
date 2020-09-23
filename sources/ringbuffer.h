// Smart House
// Circular buffer
#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H
#include <stdint.h>

typedef struct {
  uint8_t *base;
  uint8_t first;
  uint8_t last;
  uint8_t size;
  uint8_t full;
} ringbuf_t;

uint8_t ringbuffer_new(ringbuf_t*, uint8_t *raw, uint8_t raw_size);

uint8_t ringbuffer_size(const ringbuf_t*);

uint8_t ringbuffer_used(const ringbuf_t*);

uint8_t ringbuffer_pop(ringbuf_t*, uint8_t *dest);

uint8_t ringbuffer_push(ringbuf_t*, uint8_t val);

void ringbuffer_reset(ringbuf_t*);

inline uint8_t ringbuffer_empty(const ringbuf_t *b) {
  return ringbuffer_used(b) == 0 ? 1 : 0;
}

inline uint8_t ringbuffer_full(const ringbuf_t *b) {
  return ringbuffer_used(b) == ringbuffer_size(b) ? 1 : 0;
}

#endif
