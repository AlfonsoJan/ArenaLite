# ArenaLite

ArenaLite is a lite, single header only lirbary, block-based memory arena for C

## Example

```c
#include <stdio.h>

#define ARENA_LITE_IMPLEMENTATION
#include "ArenaLite.h"

typedef struct { float x, y, z; } Vec3;

int main(void) {
    ArenaLite arena = {0};

    Vec3 *v = Arena_Add(&arena, Vec3);

    v->x = 1; v->y = 2; v->z = 3;
    printf("Vec3(%f, %f, %f)\n", v->x, v->y, v->z);

    const size_t N = 256;
    Vec3 *arr = Arena_AddN(&arena, Vec3, N);
    for (size_t i = 0; i < N; ++i) {
        arr[i].x = (float)i;
        arr[i].y = (float)i + 10.0f;
        arr[i].z = (float)i + 20.0f;
    }

    ArenaLiteMark m = arena_lite_mark(&arena);
    int *a = Arena_Add(&arena, int);
    float *b = Arena_AddN(&arena, float, 100);
    // discard a and b (and anything after mark)
    arena_lite_rewind(&arena, m);

    arena_lite_free(&arena);
    return 0;
}
```

## API

```c
Arena_Add(&arena, Type)           // allocate one object
Arena_AddN(&arena, Type, N)       // allocate N objects

arena_lite_mark(&arena)           // create a rewind point
arena_lite_rewind(&arena, mark)   // rewind arena to a previous mark

arena_lite_reset(&arena)          // reuse memory (keeps blocks)
arena_lite_free(&arena)           // free all memory
```

## Limitations

Not thread-safe
