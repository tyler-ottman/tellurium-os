#include <arch/terminal.h>
#include <libc/kmalloc.h>
#include <libc/vector.h>
#include <stdbool.h>

static inline bool index_in_bounds(struct vector *vector, int n) {
    return ((n >= 0) && (n < vector->cur_elements));
}

void vector_alloc(struct vector *vector) {
    vector->cur_elements = 0;
    vector->max_elements = VECTOR_INIT_SIZE;
    vector->elements = kmalloc(vector->max_elements * sizeof(void *));
}

void vector_free(struct vector *vector) {
    kfree(vector);
}

void vector_clear(struct vector *vector) {
    for (int i = 0; i < vector->cur_elements; i++) {
        vector->elements[i] = NULL;
    }

    vector_resize(vector, VECTOR_INIT_SIZE);
    vector->cur_elements = 0;
}

void* vector_get(struct vector *vector, int n) {
    return index_in_bounds(vector, n) ? vector->elements[n] : NULL;
}

void vector_push_back(struct vector *vector, void *val) {
    if (vector->cur_elements == vector->max_elements) {
        vector_resize(vector, vector->max_elements * 2);
    }

    vector->elements[vector->cur_elements++] = val;
}

void vector_pop_back(struct vector *vector) {
    vector->elements[vector->cur_elements--] = NULL;
}

void vector_remove(struct vector *vector, int n) {
    if (index_in_bounds(vector, n)) {
        return;
    }

    for (int i = n; i < vector->max_elements - 1; i++) {
        vector->elements[i] = vector->elements[i + 1];
    }

    vector->elements[--vector->max_elements] = NULL;
}

void vector_resize(struct vector *vector, int size) {
    if (size < 0) {
        return;
    }
    
    void **elements = krealloc(vector, sizeof(void *) * size);
    if (elements != NULL) {
        vector->elements = elements;
        vector->max_elements = size;
        if (size < vector->cur_elements) {
            vector->cur_elements = size;
        }
    }
}

void vector_set(struct vector *vector, int n, void *val) {
    if (index_in_bounds(vector , n))
        vector->elements[n] = val;
}

int vector_size(struct vector *vector) {
    return vector->cur_elements;
}
