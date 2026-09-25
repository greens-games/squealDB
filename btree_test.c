#include <stdio.h>
#define GOOTS_ARENA_IMPL
#include "arena.h"
#define GOOTS_BTREE_IMPL
#include "btree.h"


#define STRING_JUMP 10

typedef struct List List;
struct List {
  char *data;
  int count;
};


int main() {
  Arena *arena = arena_init(COMMIT_SIZE, RESERVE_SIZE);
  
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
//  btree_search(btree, 3);
  
  for (;;) {
    if (curr_node == NULL) {
      break;
    }
    for (int i  = 0; i < curr_node->key_count; ++i) {
      printf("%d:%d\t", i, curr_node->kv[i].key);
    }
    curr_node = curr_node->children;
    if (curr_node != NULL) {
      printf("\n|\n\t");
    }
  }
  
  return 0;
}