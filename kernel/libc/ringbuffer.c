#include <libc/kmalloc.h>
#include <libc/ringbuffer.h>

int ringbuffer_alloc(ringbuffer_t *ring, size_t size) {
    ring->lock = 0;
    ring->capacity = size;
    ring->read_head = ring->write_head = 0;
    ring->buff = kmalloc(ring->capacity);

    return ring->buff ? RINGBUFFER_OK : RINGBUFFER_ERR;
}

size_t ringbuffer_read(ringbuffer_t *ring, uint8_t *buff, size_t len) {
    size_t read_bytes = 0;
    spinlock_acquire(&ring->lock);

    while (read_bytes < len) {
        if (ring->read_head == ring->write_head) {
            break;
        }

        *(buff + read_bytes) = *(ring->buff + (ring->read_head % ring->capacity));

        ring->read_head++;
        read_bytes++;
    }

    spinlock_release(&ring->lock);
    return read_bytes;
}

size_t ringbuffer_write(ringbuffer_t *ring, uint8_t *buff, size_t len) {
    size_t sent_bytes = 0;
    spinlock_acquire(&ring->lock);

    while (sent_bytes < len) {
        if (ring->read_head + ring->capacity == ring->write_head) {
            break;
        }

        *(ring->buff + (ring->write_head % ring->capacity)) = *(buff + sent_bytes);

        ring->write_head++;
        sent_bytes++;
    }

    spinlock_release(&ring->lock);
    return sent_bytes;
}

size_t ringbuffer_size(ringbuffer_t *ring) {
    spinlock_acquire(&ring->lock);

    size_t size = ring->write_head - ring->read_head;
    
    spinlock_release(&ring->lock);
    return size;
}

size_t ringbuffer_capacity(ringbuffer_t *ring) {
    spinlock_acquire(&ring->lock);

    size_t capacity = ring->capacity;

    spinlock_release(&ring->lock);
    return capacity;
}