#ifndef GOOTS_BTREE_H
#define GOOTS_BTREE_H

#ifndef GOOTS_ARENA_IMPL
#define GOOTS_ARENA_IMPL
#include "arena.h"
#endif

#define HEADER_SIZE sizeof(int) * 3
#define EachIndex(index, count) (uint64_t index = 0; index < (count); index += 1)
#define EachIndexOffset(index, start, end) (uint64_t index = start; index < (end); index += 1)

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
  int key_count;
  int child_count;
  BTreeNode *parent;
  BTreeNode *children;
  Data *kv;
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

void btree_node_root_init(BTree *tree, BTreeNode *node) {
  node->kv = arena_push(tree->arena, sizeof(Data) * tree->n - 1, alignof(Data));
  node->children = arena_push(tree->arena, sizeof(BTreeNode) * tree->n, alignof(BTreeNode));
}


void btree_insert_test(BTree *btree, BTreeNode *node, Data key) { //key should be Data
  if (btree->root == NULL) {
    BTreeNode new_root = {0};
    btree_node_root_init(btree, &new_root);
    btree->root = &new_root;
    node->kv[node->key_count] = key;
    node->key_count += 1;
    return; //I don't like early returns so remove this
  }
  if (node == NULL) {
    node = btree->root;
  }
  bool is_leaf = node->children == NULL;
  if (is_leaf) {

    if (node->key_count >= btree->n - 1) {
      //split node
      //call insert from parent
      int median_key = -1;
      int median_index = node->key_count/2;
      Data median_node_data = node->kv[median_index]; //This becomes new root (this node)
      //Create a parent
      BTreeNode new_parent = {0};
      if (node->child_count == 0) {
        BTreeNode *base_child = node->children;
        for EachIndex(i, btree->n) {
          BTreeNode  *new_node = (BTreeNode *)arena_push(btree->arena, sizeof(BTreeNode), alignof(BTreeNode));
          Data *data = (Data *)arena_push(btree->arena, sizeof(Data) * btree->n - 1, alignof(Data));
          base_child = new_node;
          base_child += 1;
        }
      }
      
      //insert into left child
      for EachIndex(i, median_index) {
        Data data_to_inser = node->kv[i];
        
      }

      new_parent.kv[0] = median_node;
      node->parent = &new_parent;
      new_parent.children = node;
      //btree_insert_test(btree, new_parent, median_node)
      //Split this node into 2 children
      BTreeNode *new_child = (BTreeNode *)arena_push(btree->arena, sizeof(BTreeNode), alignof(BTreeNode));


      //add child to parent
    }
    //Add the node
    //add key
    node->kv[node->key_count] = key;
    node->key_count += 1;
    //sort keys
    for (int i = 1;i < node->key_count; ++i) {
      if (node->kv[i - 1].key > node->kv[i].key) {
        Data temp = node->kv[i - 1];
        node->kv[i - 1] = node->kv[i];
        node->kv[i] = temp;
      }
    }
  } else {
    //Decide which child (go through each child and compare to key (DataKey)
      //node = chosen_child;
      btree_insert_test(btree, node, key);
    }
  }

  void btree_insert(BTree *btree, BTreeNode *node, int key) {
    //I believe we need bounds checking since we are using our arena? We can allocate 4 things
    if (btree->root == NULL) {
      btree->root = node;
      node->kv = (Data *)arena_push(btree->arena, sizeof(Data) * btree->n - 1, alignof(Data));
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
        if (curr_node->key_count >= btree->n - 1) {
          printf("hit max count\n");
          //split node based on median
          int median_key = -1;
          int median_index = curr_node->key_count/2;
          Data median_node = curr_node->kv[median_index]; //This becomes new root (this node)
          if (curr_node->parent == NULL || curr_node->parent->child_count < btree->n) {
            //Create a parent
            BTreeNode new_parent = {0};
            btree_node_root_init(btree, &new_parent);
            new_parent.kv[0] = median_node;
            //Split this node into 2 children
            //Set the parent data in the parent to the median
            //
            BTreeNode *new_children = (BTreeNode *)arena_push(btree->arena, sizeof(BTreeNode) * btree->n, alignof(BTreeNode));
            curr_node->children = new_children;
            for (int i = 0; i < btree->n; ++i) {
              curr_node->children[i].kv = (Data *)arena_push(btree->arena, sizeof(Data) * btree->n - 1, alignof(Data));
            }
            if (curr_node->child_count == 0) {
              BTreeNode *curr_child = &curr_node->children[0];
              for (int j = 0;j < median_index; ++j) {
                curr_child->kv[j] = curr_node->kv[j];
              }
              curr_child = &curr_node->children[1];
              for (int j = median_index + 1;j < curr_node->key_count; ++j) {
                curr_child->kv[j] = curr_node->kv[j];
              }
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