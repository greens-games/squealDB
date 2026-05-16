#ifndef GOOTS_STRINGS_H
#define GOOTS_STRINGS_H

typedef struct {
  char *data;
  u64 size;
} String8;

int string8_len(char *s);
String8 string8_from_lit(char *s);
void string8_expand(Arena *arena, String8 *str_8, u64 expand_mult);
void string8_concat(Arena *arena, String8 *str_8, char *new_data);
void string8_append(Arena *arena, String8 *str_8, char new_data);
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
}


void string8_append(Arena *arena, String8 *str_8, char new_data) {
  while (str_8->size > (str_8->size + 1)) {
    string8_expand(arena, str_8, 1);
  }
  
  memset(str_8->data + str_8->size, new_data, sizeof(char));
  str_8->size += 1;
}

void string8_clone(Arena *arena, String8 *s1, String8 *s2) {
  s1->data = arena_push(arena, s2->size, alignof(char));
  memcpy(s1->data, s2->data, sizeof(char) * s2->size);
  s1->size = s2->size;
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
#endif // GOOTS_ARENA_IMPL