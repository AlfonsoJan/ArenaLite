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

    arena_lite_free(&arena);
    return 0;
}
```

## API

```c
Arena_Add(&arena, Type)     // allocate one object
arena_lite_reset(&arena)  // reuse memory
arena_lite_free(&arena)   // free all memory
```

## Limitations

Not thread-safe
