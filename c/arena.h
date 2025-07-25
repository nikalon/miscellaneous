#pragma once

#include <stdint.h>

typedef struct {
    uint8_t *_memory_start;
    uint8_t *_position;
    uint8_t *_next_reserved_page;
    uint64_t _capacity;
    uint64_t _page_size;
} Arena;

Arena arena_alloc();
void  arena_free(Arena *arena);

void *arena_push(Arena *arena, uint64_t size);
void *arena_push_nozero(Arena *arena, uint64_t size);
#define arena_push_array(arena, type, length)        (type*) arena_push(arena, length*sizeof(type))
#define arena_push_array_nozero(arena, type, length) (type*) arena_push_nozero(arena, length*sizeof(type))
#define arena_push_type(arena, type)                 (type*) arena_push(arena, sizeof(type))
#define arena_push_type_nozero(arena, type)          (type*) arena_push_nozero(arena, sizeof(type))

uint64_t  arena_get_capacity(Arena *arena);
uint64_t  arena_get_pos(Arena *arena);
void arena_set_pos(Arena *arena, uint64_t pos);
void arena_clear(Arena *arena);
