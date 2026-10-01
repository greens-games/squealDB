#include <stdio.h>
#define GOOTS_ARENA_IMPL
#include "arena.h"
#define GOOTS_BTREE_IMPL
#include "btree.h"



typedef struct List List;
struct List {
  char *data;
  int count;
};

int my_thing(int x, int y) {
  int a = x + y;
  int b = x * y;
  return a + b;
}

int main() {
  Arena *arena = arena_init(COMMIT_SIZE, RESERVE_SIZE);
  int yo = my_thing(10, 15);
  BTree btree = {0};
  btree_init(&btree, 3);
  
  BTreeNode first_node = {0};
  btree_insert(&btree, &first_node, 12);
  BTreeNode second_node = {0};
  btree_insert(&btree, &second_node, 1);
  BTreeNode third_node = {0};
  btree_insert(&btree, &third_node, 3);
  BTreeNode fourth_node = {0};
  btree_insert(&btree, &fourth_node, 25); 
  BTreeNode *curr_node = btree.root;
  
  for (;;) {
    if (curr_node == NULL) {
      break;
    }
    for (int i  = 0; i < curr_node->key_count; ++i) {
      printf("Key %d: %d;\t", i, curr_node->kv[i].key);
    }
    printf("\n-----\n");
    curr_node = curr_node->children;
  }
  
  #if 0
  InitWindow(980, 620, "Visualizer");
  List list = {0};
  for (;;) {
    if (WindowShouldClose()) {
      break;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
     BTreeNode *curr_node = btree.root;
     for (;;) {
      if (curr_node == NULL) {
        break;
      }
      int offset = 12;
      int base_x = 100;
      int base_y = 100;
      DrawRectangle(base_x, base_y, 96, 48, RED);
      for (int i  = 0; i < curr_node->key_count; ++i) {
          char *buff = arena_push(arena, sizeof(char) * 10, alignof(char));
          sprintf(buff, "%d,", curr_node->kv[i].key);
          DrawText(buff, base_x + offset, base_y + 12, 24, BLACK);
          offset += 24;
      }
      curr_node = curr_node->children;
    }

    EndDrawing();
    arena_pop_to(arena, 0);
  }
  #endif
  
  return 0;
}
