#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <arch/lock.h>
#include <stddef.h>

#define RINGBUFFER_OK                       1
#define RINGBUFFER_ERR                      0

#define RINGBUFFER_DECLARE(ring) struct ringbuffer ring
#define RINGBUFFER_ALLOC(ring, size) ringbuffer_alloc(&ring, size)
#define RINGBUFFER_READ(ring, buff, len) ringbuffer_read(&ring, (uint8_t *)buff, len)
#define RINGBUFFER_WRITE(ring, buff, len) ringbuffer_write(&ring, (uint8_t *)buff, len)
#define RINGBUFFER_SIZE(ring) ringbuffer_size(&ring)
#define RINGBUFFER_CAPACITY(ring) ringbuffer_capacity(&ring)

typedef struct ringbuffer {
    spinlock_t lock;
    size_t capacity;
    size_t read_head;
    size_t write_head;
    uint8_t *buff;
} ringbuffer_t;

int ringbuffer_alloc(ringbuffer_t *ring, size_t size);
size_t ringbuffer_read(ringbuffer_t *ring, uint8_t *buff, size_t len);
size_t ringbuffer_write(ringbuffer_t *ring, uint8_t *buff, size_t len);
size_t ringbuffer_size(ringbuffer_t *ring);
size_t ringbuffer_capacity(ringbuffer_t *ring);

#endif // RINGBUFFER_H