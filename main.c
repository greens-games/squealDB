#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
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

typedef enum {
  NUMBER,
  VARCHAR,
} ColumnType;

typedef struct {
  TokenKind kind;
  String8  value;
} Token;

typedef struct {
  Token *tokens;
  u64   count;
} TokenArray;

typedef struct UserRow UserRow;
struct UserRow {
  u64     id;
  String8 name;
};

typedef struct RowHeader RowHeader;
struct RowHeader {
  RowHeader *next;
  RowHeader *prev;
};

typedef struct {
  RowHeader *first; //Rows could just be an arena the thing we lose here is the ability to prev + next for the rows
  RowHeader *last_added;
  Arena *arena;
  //We could add an arena to the table that contains all the rows, then a row is a pointer into this arena with 
  //next + prev
  //Add some header info for the table about the rows
  u64 row_count;
  u64 row_size;
  
  //These should just be a hash map
  u64 column_count;
  ColumnType columns[2]; //Defines order and types for each column for indexing
  String8 column_names[2];
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
      String8 value = string8_clone(value_arena, token_from_query);
      tok->value = value;
    }
    scratch_end(&scratch);
    
    token_array.tokens = tok;
    token_array.tokens += 1;
    token_count += 1;
    
    tok = arena_push(token_arena, sizeof(Token), alignof(Token));
  }
  
  token_array.tokens = base_token;
  token_array.count = token_count;
  return token_array;
}

u64 user_table_column_get(UserTable table, String8 column_name){
  u64 index = 0;
  
  for (;index < 2; ++index) {
    if (string8_compare(table.column_names[index], column_name)) {
      break;
    }
  }
  
  return index;
}

int main() {
  Arena *arena = arena_init(COMMIT_SIZE, RESERVE_SIZE);
  Arena *token_arena = arena_init(COMMIT_SIZE, RESERVE_SIZE);
  Arena *value_arena = arena_init(COMMIT_SIZE, RESERVE_SIZE); //Growing string arena
  Arena *quick_temp = arena_init(1024, 1024);
  int base_size = 256;
  char *test_s = arena_push(arena, sizeof(char) * base_size, alignof(char));
  String8 str_8 = {0};
  str_8.data = test_s;
  UserTable table = {0};
  table.arena = arena_init(100 * sizeof(UserRow), 100 * sizeof(UserRow));
  //TODO: I don't like this
  table.columns[0] = NUMBER;
  table.column_names[0] = string8_from_lit("id");
  table.columns[1] = VARCHAR;
  table.column_names[1] = string8_from_lit("name");
  table.row_size = sizeof(String8) + sizeof(u64);
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
    arena_pop_to(quick_temp, 0);
    
    Token first_token = token_array.tokens[0];
    char *kind_value = "";
    switch (first_token.kind) {
      case INSERT: 
      {
        kind_value = "INSERT";
        
        int value_count = 0;
        //TODO: we don't want this token count because our inserting will change depending on our data structure
        RowHeader *prev = table.last_added;
        for (int i = 1; i < token_array.count; ++i) {
          Token token = token_array.tokens[i];
          if (token.kind != VALUE) {
            continue;
          }
          RowHeader *base = arena_push(table.arena, sizeof(RowHeader), alignof(RowHeader));
          UserRow *row_data = arena_push(table.arena, sizeof(UserRow), alignof(UserRow));
          row_data->id = table.row_count + 1;
          row_data->name = string8_clone(table.arena, token.value);
          if (table.row_count == 0) {
            table.first = base;
            table.last_added = table.first;
          } else {
            table.last_added->next = (void *)base;
            table.last_added = base;
          }
          table.row_count += 1;
          printf(" INSERTING: ");
          printf("%s\n", token.value.data);
        }
        

      break;
     }      
    
      case SELECT: 
      {
      int value_count = 0;
      int cols = 0;
      kind_value = "SELECT";
        //TODO: we don't want this token count because our inserting will change depending on our data structure
        for (int i = 1; i < token_array.count; ++i) {
          Token token = token_array.tokens[i];
          if (token.kind != VALUE) {
            continue;
          }
          if (string8_compare(token.value, string8_from_lit("*"))) {
            //select all
            u64 offset = 0;
            RowHeader *curr = table.first;
            RowHeader *prev = curr;
            for (int i = 0; i < table.row_count; ++i) {
            //TODO: This fails on a multiple rows right now we need to know how big the previous row was (this varies because of the String8)
              UserRow *row = (UserRow *)((char *)curr + sizeof(RowHeader) + offset);
              printf(" Value %lu: ", row->id);
              printf("%s\n", row->name.data);
//              offset += sizeof(row->id) + row->name.size + sizeof(String8);
              prev = curr;
              curr = prev->next;
            }
            break;
          } else {
            //We need to know the size of the column and what it's offset into the row is
            u64 index = user_table_column_get(table, token.value);
            u64 offset = 0;
            ColumnType column_kind = table.columns[index];
            switch(column_kind) {
              case NUMBER:
              offset = sizeof(u64);
              break;
              case VARCHAR:
              offset = sizeof(String8);
              break;
            }
            RowHeader *curr = table.first;
            RowHeader *prev = curr;
            u64 size_of_prev = 0;
            for (int i = 0; i < table.row_count; ++i) {
              UserRow *row = (UserRow *)((char *)curr + sizeof(RowHeader) + size_of_prev);
              printf(" Value %lu: ", row->id);
              printf("%s\n", row->name.data);
            }
          }
        }
        //match on value names for the columns to select

      break;
      }
      
      case VALUE:
      kind_value = "VALUE";
      break;
      default:
      break;
    }    
    arena_pop_to(token_arena, 0);
    arena_pop_to(token_arena, 0);
  } // main loop
  arena_destroy(arena);
  //TODO: For some reason if thsi is destroyed before doing any of the query stuff we seg fault
  arena_destroy(quick_temp);
  return 0;
}