#ifndef GOOTS_STRINGS_H
#define GOOTS_STRINGS_H

#define EachIndex(it, count) (u64 it = 0; it < (count); it += 1)

typedef struct {
  char *data;
  u64 size;
} String8;

int string8_len(char *s);
String8 string8_from_lit(char *s);
//TODO: Should these all that take arenas instead just make scratch arenas YOU send the memory?
void string8_expand(Arena *arena, String8 *str_8, u64 expand_mult);
//TODO: We might want to always \0 the next byte in all the allocations strings
void string8_concat(Arena *arena, String8 *str_8, char *new_data);
String8 string8_clone(Arena *arena, String8 str_8);
void string8_append(Arena *arena, String8 *str_8, char new_data);
String8 string8_to_lower(Arena *arena, String8 str_8);
bool string8_contains_char(String8 str_8, char comparator);

#endif //GOOTS_STRINGS_H



#ifdef GOOTS_ARENA_IMPL
int string8_len(char *s) {
  char *end = s;
  while (*end != '\0') {
    end += 1;
  }
  return end - s;
}

String8 string8_from_lit(char *s) {
  String8 res = {s, string8_len(s)};
  return res;
}

void string8_expand(Arena *arena, String8 *str_8, u64 amount) {
  str_8->size += amount;
  char *new_mem = arena_push(arena, sizeof(char) * str_8->size, alignof(char));
  memcpy(new_mem, str_8->data, sizeof(char) * str_8->size);
  str_8->data = new_mem;
}

void string8_concat(Arena *arena, String8 *str_8, char *new_data) {
  int new_data_len = string8_len(new_data);
  while (str_8->size > (str_8->size + new_data_len)) {
    int amount = str_8->size + new_data_len;
    string8_expand(arena, str_8, amount);
  }
  memcpy(str_8->data + str_8->size, new_data, sizeof(char) * new_data_len);
  str_8->size += new_data_len;
  str_8->data[str_8->size] = 0;
}


void string8_append(Arena *arena, String8 *str_8, char new_data) {
  while (str_8->size > (str_8->size + 1)) {
    string8_expand(arena, str_8, 1);
  }
  
  memset(str_8->data + str_8->size, new_data, sizeof(char));
  str_8->size += 1;
}

String8 string8_clone(Arena *arena, String8 s) {
  String8 res = {0};
  res.data = arena_push(arena, s.size + 1, alignof(char));
  memcpy(res.data, s.data, sizeof(char) * s.size);
  res.size = s.size;
  res.data[res.size] = 0;
  return res;
}

bool string8_compare(String8 s1, String8 s2) {
  bool same = s1.size == s2.size;
  if (same) {
    for (int i = 0; i < s1.size; ++i) {
      if (s1.data[i] != s2.data[i]) {
        same = false;
        break;
      }
    }
  }
  return same;
}

bool string8_contains_char(String8 str_8, char comparator) {
  bool found = false;
  int count = 0;
  for (;count < str_8.size ;) {
    if (str_8.data[count] == comparator) {
      found = true;
    }
    count += 1;
  }
  return found;
}

String8 string8_to_lower(Arena *arena, String8 str_8) {
  String8 new_str = {0};
  new_str.data = arena_push(arena, str_8.size, alignof(char));
  for EachIndex(i, str_8.size) {
    char curr_char = str_8.data[i];
    if (curr_char >= 'A' && curr_char <= 'Z') {
      curr_char += 32;
    }
    string8_append(arena, &new_str, curr_char);
  }
  return new_str;
}
#endif // GOOTS_ARENA_IMPL