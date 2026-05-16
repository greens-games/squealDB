#ifndef GOOTS_ARENA_H
#define GOOTS_ARENA_H

#define ARENA_HEADER_SIZE 32
#define DEFAULT_ALIGN 64

#define alignof(T) __alignof(T)
#define AlignPow2(x,b) (((x) + (b) - 1)&(~((b) - 1)))
#define MAX(v, mx) (v > mx ? v : mx)
#define MIN(v, mn) (v < mn ? v : mn)
#define Clamp(v, mn, mx) (MAX(MIN(v, mx), mn))

#define KB 1024
#define MB KB * 1024

#define RESERVE_SIZE MB 
#define COMMIT_SIZE  KB 

#define STATIC  0x00000000
#define GROWING 0x00000001

typedef struct {
  //TODO add next a prev arenas so we can chain them
  u64 reserve_size;
  u64 first_free_pos;
  u64 commit_size;
  u64 commited;
} Arena;

typedef struct {
  Arena *arena;
  u64   base_pos;
} Scratch;

bool is_power_of_two(int x);
u64 max(int x, int y);

//NOTE Arena is itself the memory buffer, it contains it's header which is ARENA_HEADER_SIZE then data section starts after that
//We do this to avoid having to load a separate address to start allocating into and instead can have mostly contiguous memory in our arena
Arena *arena_init(u64 commit_size, u64 reserve_size);
Arena *arena_back_buffer_init(char back_buffer[], u64 commit_size, u64 reserve_size);
void *arena_push(Arena *arena, size_t size, u64 alignment);
void arena_pop_to(Arena *arena, u64 pos);
void arena_pop(Arena *arena, u64 amount);
void arena_destroy(Arena *arena);

Scratch scratch_begin(Arena *arena);
void scratch_end(Scratch *scratch);
#endif //GOOTS_ARENA_H

#ifdef GOOTS_ARENA_IMPL

//////////////////////
// ARENA
/////////////////////

//NOTE Arena is itself the memory buffer, it contains it's header which is ARENA_HEADER_SIZE then data section starts after that
//We do this to avoid having to load a separate address to start allocating into and instead can have mostly contiguous memory in our arena
Arena *arena_init(u64 commit_size, u64 reserve_size) {
  Arena *arena = mmap(0, reserve_size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0 );
  mprotect(arena, commit_size, PROT_WRITE | PROT_READ);
  arena->commit_size = commit_size;
  arena->reserve_size = reserve_size;
  arena->first_free_pos = ARENA_HEADER_SIZE;
  return arena;
}

Arena *arena_back_buffer_init(char back_buffer[], u64 commit_size, u64 reserve_size) {
  Arena *arena          = (Arena *)back_buffer;
  
  arena->commit_size    = commit_size;
  arena->reserve_size   = reserve_size;
  arena->first_free_pos = ARENA_HEADER_SIZE;
  return arena;
}

void *arena_push(Arena *arena, size_t size, u64 alignment) {
  //NOTE we align to make sure that the position (in bytes) is some POWER OF 2 for more efficient memory access
  //TODO I don't think this works how I think but for now it should be fine we really gotta figure out alignment better
  u64 pos = AlignPow2(arena->first_free_pos, alignment);
  
  if (pos + size >= arena->commit_size) { 
    mprotect(arena, arena->commit_size, PROT_READ | PROT_WRITE);
  }
  
  if (pos + size <= arena->commit_size) { 
    //NOTE We have to cast the arena to a char * so we most pos number of bytes instead of pos number of arenas
    void *ptr = (char *) arena + pos;
    arena->first_free_pos +=size;
    memset(ptr, 0, size);
    return ptr;
  }
  
  //NOTE I don't like return NULL should do something else if we have no memory to return
  return NULL;
}

void arena_pop_to(Arena *arena, u64 pos) {
  u64 _pos = Clamp(pos, ARENA_HEADER_SIZE, arena->reserve_size);
  u64 diff = arena->first_free_pos - _pos;
  memset((char *)arena + _pos, 0, diff);
  arena->first_free_pos = _pos;
}

void arena_pop(Arena *arena, u64 amount) {
  u64 new_pos = max(0, arena->first_free_pos - amount);
  if (new_pos < ARENA_HEADER_SIZE) {
    printf("trying to delete parts of arena header is invalid\n");
    return;
  }
  arena_pop_to(arena, new_pos);
}

void arena_destroy(Arena *arena) {
  //NOTE probably don't need to zero these out
  u64 amount_to_unmap   = arena->reserve_size;
  arena->commit_size    = 0;
  arena->reserve_size   = 0;
  arena->first_free_pos = 0;
  arena->commited       = 0;
  munmap(arena, amount_to_unmap);
}

///////////////////
// SCRATCH
///////////////////

Scratch scratch_begin(Arena *arena) {
  Scratch scratch = {arena, arena->first_free_pos};
  return scratch;
}

void scratch_end(Scratch *scratch) {
  arena_pop_to(scratch->arena, scratch->base_pos);
}

///////////////////
// UTILS
///////////////////

bool is_power_of_two(int x) {
	return (x & (x-1)) == 0;
}

u64 max(int x, int y) {
  return x > y ? x : y;
}

#endif //GOOTS_ARENA_IMPL
