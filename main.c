#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define GOOTS_ARENA_IMPL
#include "arena.h"
#define GOOTS_STRINGS_IMPL
#include "strings.h"

typedef enum {
  SELECT,
  INSERT,
  VALUE,
} TokenKind;


typedef struct {
  TokenKind kind;
  String8  value;
} Token;

typedef struct {
  Token *tokens;
  u64   count;
} TokenArray;

typedef struct {
  u64 id;
  String8 name; 
} UserTable;

String8 read_until_delimeter(Arena *arena, String8 str, String8 delimeters) {
  String8 res = {0};
  res.data = arena_push(arena, str.size, alignof(char));
  String8 temp_str = {0};
  temp_str.data = arena_push(arena, str.size, alignof(char));
  
  Scratch scratch = scratch_begin(arena);
  for (int i = 0; i < str.size; ++i) {
    char curr_char = str.data[i];
    if (string8_contains_char(delimeters, curr_char)) {
      break;
    }
    string8_append(scratch.arena, &temp_str, curr_char);
  }
  string8_concat(arena, &res, temp_str.data);
  scratch_end(&scratch);
  
  arena_pop(arena, str.size - res.size);
  return res;
}

//TODO: have an arena for  tokens and arena for values, the value for a token points to an addr in value arena
TokenArray parse_query(Arena *token_arena, Arena *value_arena, String8 str) {
  TokenArray token_array = {0};
  
  int token_count = 0;
  
  bool done = false;
  String8 *str_ptr = &str;
  int offset = 0;
  
  Token *tok = arena_push(token_arena, sizeof(Token), alignof(Token));
  Token *base_token = tok;
  for (;;) {
    //TODO: current issue is that here we read until some chunk AND we allocate the result on our base arena
    //Which means we then fragment our token array since it shares the same arena region
    char back_buffer[256];
    Arena *arena_back_buffer = arena_back_buffer_init(back_buffer, 256, 256);
    String8 token_from_query = read_until_delimeter(arena_back_buffer, *str_ptr, string8_from_lit("\n "));
    
    if (str.size <= offset) {
      break;
    }
    //TODO this is stopping 1 early
    offset += token_from_query.size + 1;
    
    str_ptr->data += token_from_query.size + 1;
    
    
    Scratch scratch = scratch_begin(arena_back_buffer);
    if (string8_compare(string8_to_lower(scratch.arena, token_from_query), string8_from_lit("select"))) {
      tok->kind = SELECT;
    } else if (string8_compare(string8_to_lower(scratch.arena, token_from_query), string8_from_lit("insert"))) {
      tok->kind = INSERT;
    } else {
      tok->kind = VALUE;
    }
    String8 value = string8_clone(value_arena, token_from_query);
    scratch_end(&scratch);
    
    tok->value = value;
    token_array.tokens = tok;
    token_array.tokens += 1;
    token_count += 1;
    
    tok = arena_push(token_arena, sizeof(Token), alignof(Token));
  }
  
  token_array.tokens = base_token;
  token_array.count = token_count;
  return token_array;
}

int main() {
  Arena *arena = arena_init(COMMIT_SIZE, RESERVE_SIZE);
  Arena *token_arena = arena_init(COMMIT_SIZE, RESERVE_SIZE);
  Arena *value_arena = arena_init(COMMIT_SIZE, RESERVE_SIZE);
  Arena *quick_temp = arena_init(1024, 1024);
  int base_size = 256;
  
#if 0
  String8 query = string8_from_lit("query something");
  bool found = string8_contains_char(string8_from_lit("\n "), ' ');
  printf("%d\n", found);
  String8 str = read_until_delimeter(arena, query, string8_from_lit("\n "));
  printf("%s\n", str.data);
  String8 test_str = string8_from_lit("query");
  printf("%d\n", string8_compare(str, test_str));
  TokenArray token_array = parse_query(token_arena, value_arena, string8_from_lit("select SELECT INSERT insert matth"));
  int x = 1;
  for EachIndex(i, token_array.count) {
    Token token = token_array.tokens[i];
    char *kind_value = "";
    switch (token.kind) {
      case INSERT:
      kind_value = "INSERT";
      //insert into our table (later this could use a from)
      break;
      case SELECT:
      kind_value = "SELECT";
      break;
      case VALUE:
      kind_value = "VALUE";
      break;
      default:
      break;
    }
    printf("%s ", kind_value);
    printf("%s\n", token.value.data);
  }
  
#endif
  
#if 1
  char *test_s = arena_push(arena, sizeof(char) * base_size, alignof(char));
  String8 str_8 = {0};
  str_8.data = test_s;
  
  while (true) {
    printf("db >> ");
    fgets(str_8.data, base_size, stdin);
    str_8.size = string8_len(str_8.data);
    char *temp = arena_push(quick_temp, base_size * sizeof(char), alignof(char));
    
    while (str_8.data[str_8.size - 1] != '\n') {
      fgets(temp, base_size, stdin);
      string8_concat(arena, &str_8, temp);
      arena_pop_to(quick_temp, 0);
    }
    
    printf("%s", str_8.data);
    TokenArray token_array = parse_query(token_arena, value_arena, str_8);
    arena_pop_to(arena, 0);
    for EachIndex(i, token_array.count) {
      Token token = token_array.tokens[i];
      char *kind_value = "";
      switch (token.kind) {
        case INSERT:
        kind_value = "INSERT";
        break;
        case SELECT:
        kind_value = "SELECT";
        break;
        case VALUE:
        kind_value = "VALUE";
        break;
        default:
        break;
      }
      printf("%s ", kind_value);
      printf("%s\n", token.value.data);
    }
    
  }
  
#endif
  arena_destroy(quick_temp);
  arena_destroy(arena);
  
  return 0;
}