#ifndef VECTOR_H
#define VECTOR_H

#define VECTOR_INIT_SIZE        8

#define VECTOR_DECLARE(vec) struct vector vec
#define VECTOR_ALLOC(vec) vector_alloc(&vec)
#define VECTOR_INIT(vec) VECTOR_DECLARE(vec); VECTOR_ALLOC(vec)
#define VECTOR_CLEAR(vec) vector_clear(&vec)
#define VECTOR_FREE(vec) vector_free(&vec)
#define VECTOR_GET(vec, n) vector_get(&vec, n)
#define VECTOR_PUSH_BACK(vec, val) vector_push_back(&(vec), (void *)val)
#define VECTOR_POP_BACK(vec) vector_pop_back(&vec)
#define VECTOR_REMOVE(vec, n) vector_remove(&vec, n)
#define VECTOR_RESIZE(vec, n) vector_resize(&vec, n)
#define VECTOR_SET(vec, n, val) vector_set(&vec, n, (void *)val)
#define VECTOR_SIZE(vec) vector_size(&vec)

typedef struct vector {
    int cur_elements;
    int max_elements;
    void **elements;
} vector_t;

void vector_alloc(struct vector *vector);
void vector_clear(struct vector *vector);
void vector_free(struct vector *vector);
void *vector_get(struct vector *vector, int n);
void vector_push_back(struct vector *vector, void *val);
void vector_pop_back(struct vector *vector);
void vector_remove(struct vector *vector, int n);
void vector_resize(struct vector *vector, int size);
void vector_set(struct vector *vector, int n, void *val);
int vector_size(struct vector *vector);

#endif // VECTOR_H
