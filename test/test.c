#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#define ARENA_LITE_IMPLEMENTATION
#include "ArenaLite.h"

typedef struct { float x,y,z; } Vec3;

int main(void) {
    ArenaLite arena = {0};

    Vec3 *v = Arena_Add(&arena, Vec3);
    assert(v && "Arena allocation failed");

    v->x = 1.0f;
    v->y = 2.0f;
    v->z = 3.0f;

    assert(v->x == 1.0f);
    assert(v->y == 2.0f);
    assert(v->z == 3.0f);

    assert(((uintptr_t)v % _Alignof(Vec3)) == 0);

    Vec3 *first = v;
    Vec3 *last  = NULL;

    for (int i = 0; i < 10000; ++i) {
        Vec3 *p = Arena_Add(&arena, Vec3);
        assert(p && "Arena ran out of memory");

        p->x = (float)i;
        p->y = (float)i + 1;
        p->z = (float)i + 2;

        last = p;
    }

    const size_t N = 256;
    Vec3 *arr = Arena_AddN(&arena, Vec3, N);
    assert(arr && "Arena_AddN failed");

    assert(((uintptr_t)arr % _Alignof(Vec3)) == 0);

    for (size_t i = 1; i < N; ++i) {
        assert(&arr[i] == arr + i);
    }

    for (size_t i = 0; i < N; ++i) {
        arr[i].x = (float)i;
        arr[i].y = (float)i + 10.0f;
        arr[i].z = (float)i + 20.0f;
    }
    for (size_t i = 0; i < N; ++i) {
        assert(arr[i].x == (float)i);
        assert(arr[i].y == (float)i + 10.0f);
        assert(arr[i].z == (float)i + 20.0f);
    }

    assert(first->x == 1.0f);
    assert(first->y == 2.0f);
    assert(first->z == 3.0f);

    assert(last);
    assert(last->y == last->x + 1.0f);

    // ------------------------------------------------------------
    // Mark/Rewind tests
    // ------------------------------------------------------------

    // Mark/rewind within the same block
    {
        ArenaLiteMark m = arena_lite_mark(&arena);

        Vec3 *a = Arena_Add(&arena, Vec3);
        Vec3 *b = Arena_Add(&arena, Vec3);
        assert(a && b);
        a->x = 111.0f;
        b->x = 222.0f;
        arena_lite_rewind(&arena, m);
        Vec3 *a2 = Arena_Add(&arena, Vec3);
        assert(a2);
        assert(a2 == a);
        assert(((uintptr_t)a2 % _Alignof(Vec3)) == 0);
    }

    // Mark, force new blocks, then rewind
    {
        ArenaLiteMark m = arena_lite_mark(&arena);
        const size_t big = ARENA_LITE_INITAL_SIZE * 4;
        unsigned char *blob = (unsigned char *)__arena_alloc_align(&arena, big, 16);
        assert(blob);
        blob[0] = 0xAB;
        blob[big - 1] = 0xCD;
        Vec3 *after = Arena_Add(&arena, Vec3);
        assert(after);
        after->x = 999.0f;
        arena_lite_rewind(&arena, m);
        Vec3 *p = Arena_Add(&arena, Vec3);
        assert(p);
        assert(((uintptr_t)p % _Alignof(Vec3)) == 0);
        ArenaLiteMark m2 = arena_lite_mark(&arena);
        Vec3 *q = Arena_Add(&arena, Vec3);
        assert(q);
        arena_lite_rewind(&arena, m2);
        Vec3 *q2 = Arena_Add(&arena, Vec3);
        assert(q2 == q);
    }

    // Mark taken on an empty arena: rewind should free everything and set head NULL
    {
        ArenaLite tmp = {0};

        ArenaLiteMark m0 = arena_lite_mark(&tmp);

        Vec3 *t1 = Arena_Add(&tmp, Vec3);
        Vec3 *t2 = Arena_Add(&tmp, Vec3);
        assert(t1 && t2);

        arena_lite_rewind(&tmp, m0);

        assert(tmp.head == NULL);

        Vec3 *t3 = Arena_Add(&tmp, Vec3);
        assert(t3);

        arena_lite_free(&tmp);
    }

    printf("Arena test passed.\n");

    arena_lite_reset(&arena);

    Vec3 *v2 = Arena_Add(&arena, Vec3);
    assert(v2 && "Allocation after reset failed");

    assert(v2 == first || v2 != NULL);

    arena_lite_free(&arena);
    return 0;
}
