#pragma once

#include <stdint.h>

typedef uint8_t  u8;
typedef uint64_t u64;

typedef struct {
    u8 *_memory_start;
    u8 *_position;
    u8 *_next_reserved_page;
    u64 _capacity;
    u64 _page_size;
} Arena;

Arena arena_alloc();
void  arena_free(Arena *arena);

void *arena_push(Arena *arena, u64 size);
void *arena_push_nozero(Arena *arena, u64 size);
#define arena_push_array(arena, type, length)        (type*) arena_push(arena, length*sizeof(type))
#define arena_push_array_nozero(arena, type, length) (type*) arena_push_nozero(arena, length*sizeof(type))
#define arena_push_type(arena, type)                 (type*) arena_push(arena, sizeof(type))
#define arena_push_type_nozero(arena, type)          (type*) arena_push_nozero(arena, sizeof(type))

u64  arena_get_capacity(Arena *arena);
u64  arena_get_pos(Arena *arena);
void arena_set_pos(Arena *arena, u64 pos);
void arena_clear(Arena *arena);
