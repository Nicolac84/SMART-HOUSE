// Smart House
// Circular buffer
#include "ringbuffer.h"


// Compute the real index for an item
#define calc_real(b,v) ((v + b->first) % b->size)

// Compute the virtual index for an item
#define calc_virt(b,r) ((r + b->size - b->first) % b->size)

// Compute the virtual shifted index for an item
#define calc_shifted(b,i) ((i+1) % b->size)


uint8_t ringbuffer_new(ringbuf_t *buf, uint8_t *raw, uint8_t raw_size) {
  if (!buf || !raw || raw_size < 2) return 1;
  *buf = (ringbuf_t) { raw, 0, 0, raw_size, 0 };
  return 0;
}

uint8_t ringbuffer_size(const ringbuf_t *buf) { return buf ? buf->size : 0; }

uint8_t ringbuffer_used(const ringbuf_t *buf) {
  if (!buf) return 0;
  if (buf->full) return buf->size;
  return calc_virt(buf, buf->last);
}

uint8_t ringbuffer_pop(ringbuf_t *buf, uint8_t *dest) {
  if (!buf || ringbuffer_empty(buf))
    return 1;
  *dest = buf->base[buf->first];
  buf->first = calc_shifted(buf, buf->first);
  buf->full = 0;
  return 0;
}

uint8_t ringbuffer_push(ringbuf_t *buf, uint8_t val) {
  if (!buf || buf->full)
    return 1;
  buf->base[buf->last] = val;
  buf->last = calc_shifted(buf, buf->last);
  if (buf->first == buf->last)
    buf->full = 1;
  return 0;
}

void ringbuffer_reset(ringbuf_t *buf) {
  if (!buf) return;
  buf->first = 0;
  buf->last = 0;
  buf->full = 0;
}
