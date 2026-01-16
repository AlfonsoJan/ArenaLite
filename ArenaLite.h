#ifndef ARENA_LITE_H_
#define ARENA_LITE_H_

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

// Define ARENA_LITEDEF before including this file to control function linkage.
// Example: #define ARENA_LITEDEF static inline
#ifndef ARENA_LITEDEF
#define ARENA_LITEDEF
#endif // ARENA_LITEDEF

// Define ARENA_LITE_INITAL_SIZE if you want a smaller initial size
#define ARENA_LITE_INITAL_SIZE (1024*4)

typedef struct ArenaLiteBlock {
    unsigned char *base;
    size_t capacity;
    size_t offset;
    struct ArenaLiteBlock *next;
} ArenaLiteBlock;

typedef struct {
    ArenaLiteBlock *head;
} ArenaLite;

typedef struct {
    ArenaLiteBlock *block;
    size_t offset;
} ArenaLiteMark;

#define Arena_Add(a, T) ((T*)__arena_alloc_align((a), sizeof(T), _Alignof(T)))
#define Arena_AddN(a, T, n) ((T*)__arena_alloc_align((a), sizeof(T) * (size_t)(n), _Alignof(T)))

static inline void *__arena_alloc_align(ArenaLite *a, size_t size, size_t align);

ARENA_LITEDEF void arena_lite_reset(ArenaLite *a);
ARENA_LITEDEF void arena_lite_free(ArenaLite *a);
ARENA_LITEDEF ArenaLiteMark arena_lite_mark(ArenaLite *a);
ARENA_LITEDEF void arena_lite_rewind(ArenaLite *a, ArenaLiteMark m);

#ifdef ARENA_LITE_IMPLEMENTATION

static inline int __arena_is_pow2(size_t x) {
    return x && ((x & (x - 1)) == 0);
}

static inline size_t __arena_align_up(size_t p, size_t align) {
    return (p + (align - 1)) & ~(align - 1);
}

static inline size_t __arena_max(size_t a, size_t b) {
    return a > b ? a : b;
}

static ArenaLiteBlock *__arena_new_block(size_t capacity) {
    ArenaLiteBlock *b = (ArenaLiteBlock*)malloc(sizeof(ArenaLiteBlock));
    assert(b && "malloc block header failed");
    b->base = (unsigned char*)malloc(capacity);
    assert(b->base && "malloc block memory failed");
    b->capacity = capacity;
    b->offset = 0;
    b->next = NULL;
    return b;
}

ARENA_LITEDEF void * __arena_alloc_align(ArenaLite *a, size_t size, size_t align) {
    if (size == 0) size = 1;
    if (align == 0) align = sizeof(void*);
    assert(__arena_is_pow2(align) && "align must be power of two");

    if (!a->head) {
        a->head = __arena_new_block(ARENA_LITE_INITAL_SIZE);
    }

    ArenaLiteBlock *b = a->head;

    size_t aligned = __arena_align_up(b->offset, align);
    size_t needed  = aligned + size;

    if (needed > b->capacity) {
        size_t new_cap = __arena_max(b->capacity * 2, __arena_max(ARENA_LITE_INITAL_SIZE, size + align));
        ArenaLiteBlock *nb = __arena_new_block(new_cap);
        nb->next = a->head;
        a->head = nb;

        b = nb;
        aligned = __arena_align_up(b->offset, align);
        needed  = aligned + size;
        // Should always fit, but to keep it safe
        assert(needed <= b->capacity && "new block still too small");
    }

    void *ptr = b->base + aligned;
    b->offset = needed;
    return ptr;
}

ARENA_LITEDEF void arena_lite_reset(ArenaLite *a) {
    for (ArenaLiteBlock *b = a->head; b; b = b->next) {
        b->offset = 0;
    }
}

ARENA_LITEDEF void arena_lite_free(ArenaLite *a) {
    ArenaLiteBlock *b = a->head;
    while (b) {
        ArenaLiteBlock *next = b->next;
        free(b->base);
        free(b);
        b = next;
    }
    a->head = NULL;
}

ARENA_LITEDEF ArenaLiteMark arena_lite_mark(ArenaLite *a) {
    ArenaLiteMark m;
    m.block  = a ? a->head : NULL;
    m.offset = (a && a->head) ? a->head->offset : 0;
    return m;
}

ARENA_LITEDEF void arena_lite_rewind(ArenaLite *a, ArenaLiteMark m) {
    if (!a) return;
    if (m.block == NULL) {
        arena_lite_free(a);
        return;
    }
    while (a->head && a->head != m.block) {
        ArenaLiteBlock *next = a->head->next;
        free(a->head->base);
        free(a->head);
        a->head = next;
    }
    assert(a->head == m.block && "arena_lite_rewind: mark does not belong to this arena (or was already freed)");
    assert(m.offset <= a->head->capacity && "arena_lite_rewind: mark offset out of range");
    a->head->offset = m.offset;
}

#endif // ARENA_LITE_IMPLEMENTATION

#endif // ARENA_LITE_H_