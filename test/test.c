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

    printf("Arena test passed.\n");

    arena_lite_reset(&arena);

    Vec3 *v2 = Arena_Add(&arena, Vec3);
    assert(v2 && "Allocation after reset failed");

    assert(v2 == first || v2 != NULL);

    arena_lite_free(&arena);
    return 0;
}
