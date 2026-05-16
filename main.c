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
} Token;

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

Token *parse_query(Arena *arena, String8 str) {
  Token *tokens = arena_push(arena, sizeof(Token), alignof(Token));
  int token_count = 0;
  
  bool done = false;
  while (!done) {
    String8 token_from_query = read_until_delimeter(arena, str, string8_from_lit(" "));
    Token tok = {0};
    if (string8_compare(token_from_query, string8_from_lit("select"))) {
      tok.kind = SELECT;
    }
    *tokens = tok;
    tokens += 1;
    token_count += 1;
  }
  
  return tokens - token_count;
}

int main() {
  Arena *arena = arena_init(COMMIT_SIZE, RESERVE_SIZE);
  Arena *quick_temp = arena_init(1024, 1024);
  
  int base_size = 256;
  String8 query = string8_from_lit("query something");
  bool found = string8_contains_char(string8_from_lit("\n "), ' ');
  printf("%d\n", found);
  
  String8 str = read_until_delimeter(arena, query, string8_from_lit("\n "));
  printf("%s\n", str.data);
  String8 test_str = string8_from_lit("query");
  printf("%d\n", string8_compare(str, test_str));
  
  Token *tokens = parse_query(arena, string8_from_lit("select"));
  int x = 1;
#if 0
  char *test_s = arena_push(arena, sizeof(char) * base_size, alignof(char));
  String8 str_8 = {0};
  str_8.data = test_s;
  while (true) {
    printf("Say something: ");
    fgets(str_8.data, base_size, stdin);
    str_8.size = string8_len(str_8.data);
    char *temp = arena_push(quick_temp, base_size * sizeof(char), alignof(char));
    while (str_8.data[str_8.size - 1] != '\n') {
      fgets(temp, base_size, stdin);
      string8_concat(arena, &str_8, temp);
      arena_pop_to(quick_temp, 0);
    }
    printf("%s", str_8.data);
    
  }
#endif
  arena_destroy(quick_temp);
  
  return 0;
}