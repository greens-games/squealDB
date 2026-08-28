#ifndef GOOTS_BTREE_H
#define GOOTS_BTREE_H

#ifndef GOOTS_ARENA_IMPL
#define GOOTS_ARENA_IMPL
#include "arena.h"
#endif

#define HEADER_SIZE sizeof(int) * 3

/*
NOTES:
  All data is stored in leaves
  Only keys are in not leaves
*/
typedef struct Data Data;
struct Data {
  int key;
  int value;
};

typedef struct BTreeNode BTreeNode;
struct BTreeNode {
  BTreeNode *parent;
  BTreeNode *children;
  Data *kv;
  int key_count;
  int child_count;
};

#define BTREE_HEADER sizeof(int) * 3
typedef struct BTree BTree;
struct BTree {
  int n;
  int max_keys;
  int min_keys;
  Arena *arena;
  BTreeNode *root;
};


void btree_init(BTree *root_node, int order);
void btree_search(BTree *root_node, int key);
void btree_insert(BTree *root_node, BTreeNode *node, int key); //Will need to handle values and stuffs
void btree_remove(BTree *root_node, int key);
#endif //GOOTS_BTREE_H

#ifdef GOOTS_BTREE_IMPL

void btree_init(BTree *btree, int order) {
  btree->n = order;
  btree->max_keys = order - 1;
  btree->min_keys = (order/2) - 1;
  btree->arena = arena_init(COMMIT_SIZE, RESERVE_SIZE);
}

void btree_insert(BTree *btree, BTreeNode *node, int key) {
  //I believe we need bounds checking since we are using our arena? We can allocate 4 things
  if (btree->root == NULL) {
    btree->root = node;
    node->kv = arena_push(btree->arena, sizeof(Data) * btree->n - 1, alignof(Data));
    node->kv[node->key_count].key = key;
    node->kv[node->key_count].value = key;
    node->key_count += 1;
  } else {
    /*
      Inserting logic flow:
        Start at root
        Compare until you get to leaf (no children)
        Once at leaf insert in order
          If at max keys split
            Once split add median to parent
              If parent is full of key and children we have to split children amongst them and make the root
    */
    BTreeNode *curr_node = btree->root;
    for (;;) {
      if (curr_node == NULL) {
        break;
      }
      bool is_leaf = curr_node->children == NULL;
      if (curr_node->key_count >= btree->n ) {
        printf("hit max count\n");
        //split node based on median
        int median_key = -1;
        int median_index = curr_node->key_count/2;
        Data median_node = curr_node->kv[median_index]; //This becomes new root (this node)
        //Split the other 2 based on this value into 2 new children
        //Allocate for children of this node
        BTreeNode *new_children = arena_push(btree->arena, sizeof(BTreeNode) * btree->n, alignof(BTreeNode));
         for (int i = 0;i < curr_node->key_count; ++i) {
          
          if (curr_node->kv[i - 1].key > curr_node->kv[i].key) {
            Data temp = curr_node->kv[i - 1];
            curr_node->kv[i - 1] = curr_node->kv[i];
            curr_node->kv[i] = temp;
          }
        } 
      }
      //add key
      curr_node->kv[curr_node->key_count].key = key;
      curr_node->kv[curr_node->key_count].value = key;
      curr_node->key_count += 1;
      //sort keys
      for (int i = 1;i < curr_node->key_count; ++i) {
        if (curr_node->kv[i - 1].key > curr_node->kv[i].key) {
          Data temp = curr_node->kv[i - 1];
          curr_node->kv[i - 1] = curr_node->kv[i];
          curr_node->kv[i] = temp;
        }
      }
      curr_node = curr_node->children;
    }
  }

}
#endif //GOOTS_BTREE_IMPL