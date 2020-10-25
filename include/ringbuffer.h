// Smart House - Nicola Colao
// Circular buffer data structure
#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H
#include <stdint.h>

// Circular buffer data type
typedef struct {
  uint8_t *base; // Raw buffer
  uint8_t first; // First item index
  uint8_t last;  // Last item index
  uint8_t size;
  uint8_t full;
} ringbuf_t;

// Create a new ringbuffer
uint8_t ringbuffer_new(ringbuf_t*, uint8_t *raw, uint8_t raw_size);

// Return the ringbuffer capacity
uint8_t ringbuffer_size(const ringbuf_t*);

// Return the number of items in the ringbuffer
uint8_t ringbuffer_used(const ringbuf_t*);

// Pop an element out of the ringbuffer
uint8_t ringbuffer_pop(ringbuf_t*, uint8_t *dest);

// Push an element into the ringbuffer
uint8_t ringbuffer_push(ringbuf_t*, uint8_t val);

// Flush the ringbuffer
void ringbuffer_reset(ringbuf_t*);

// Is the ringbuffer empty?
inline uint8_t ringbuffer_empty(const ringbuf_t *b) {
  return ringbuffer_used(b) == 0 ? 1 : 0;
}

// Is the ringbuffer full?
inline uint8_t ringbuffer_full(const ringbuf_t *b) {
  return ringbuffer_used(b) == ringbuffer_size(b) ? 1 : 0;
}

#endif
