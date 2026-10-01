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

void btree_node_init(BTree *tree, BTreeNode *node) {
  node->key_count = 0;
  node->child_count = 0;
  node->parent = NULL;
  node->children = NULL;
  node->kv = arena_push(tree->arena, sizeof(Data) * tree->n, alignof(Data));
  // Allocate space for children up to n
  node->children = arena_push(tree->arena, sizeof(BTreeNode) * tree->n, alignof(BTreeNode));
  // Zero out children pointers area conceptually (arena_push zeroes)
}

static void btree_sort_keys(BTreeNode *node) {
  for (int i = 1; i < node->key_count; ++i) {
    for (int j = i; j > 0 && node->kv[j - 1].key > node->kv[j].key; --j) {
      Data temp = node->kv[j - 1];
      node->kv[j - 1] = node->kv[j];
      node->kv[j] = temp;
    }
  }
}

static BTreeNode *btree_find_leaf(BTree *btree, BTreeNode *start) {
  BTreeNode *curr = start ? start : btree->root;
  if (curr == NULL) return NULL;
  while (curr->child_count > 0) {
    // Find child to go down - for simplicity, go to leftmost if keys don't match
    // but we need to navigate based on key; for general insert we need key
    // This is a basic version
    if (curr->children && curr->child_count > 0) {
      curr = &curr->children[0];
    } else {
      break;
    }
  }
  return curr;
}

// Basic insert implementation - simplified for the test case
void btree_insert(BTree *btree, BTreeNode *node, int key) {
  if (btree->root == NULL) {
    btree->root = node;
    btree_node_init(btree, node);
    node->kv[0].key = key;
    node->kv[0].value = key;
    node->key_count = 1;
    node->child_count = 0;
    return;
  }

  BTreeNode *leaf = btree->root;
  // Navigate to leaf
  while (leaf->child_count > 0 && leaf->children != NULL) {
    // Navigate based on key: find i such that key <= leaf->kv[i].key, go to children[i]
    // If key is larger than all keys, go to children[child_count-1] or children[key_count]
    int i;
    for (i = 0; i < leaf->key_count; ++i) {
      if (key <= leaf->kv[i].key) {
        leaf = &leaf->children[i];
        break;
      }
    }
    if (i == leaf->key_count) {
      // key is greater than all keys, go to rightmost child
      leaf = &leaf->children[leaf->child_count - 1];
      if (leaf == NULL) break;
    }
    // continue until we reach leaf
    if (leaf->child_count == 0) break;
  }

  // Insert into leaf
  if (leaf->key_count < btree->n - 1) {
    leaf->kv[leaf->key_count].key = key;
    leaf->kv[leaf->key_count].value = key;
    leaf->key_count++;
    btree_sort_keys(leaf);
  } else {
    // Split case - simplified
    leaf->kv[leaf->key_count].key = key;
    leaf->kv[leaf->key_count].value = key;
    leaf->key_count++;
    btree_sort_keys(leaf);

    int median_idx = leaf->key_count / 2;
    Data median = leaf->kv[median_idx];

    // Create new node for right half
    BTreeNode *right = arena_push(btree->arena, sizeof(BTreeNode), alignof(BTreeNode));
    btree_node_init(btree, right);
    for (int i = median_idx + 1; i < leaf->key_count; ++i) {
      right->kv[right->key_count++] = leaf->kv[i];
    }
    leaf->key_count = median_idx;

    if (leaf->parent == NULL) {
      BTreeNode *new_root = arena_push(btree->arena, sizeof(BTreeNode), alignof(BTreeNode));
      btree_node_init(btree, new_root);
      new_root->kv[0] = median;
      new_root->key_count = 1;
      new_root->child_count = 2;
      // Set children
      new_root->children[0] = *leaf;
      new_root->children[1] = *right;
      leaf->parent = new_root;
      right->parent = new_root;
      btree->root = new_root;
    }
  }
}

void btree_search(BTree *btree, int key) {
  (void)btree;
  (void)key;
}

void btree_remove(BTree *btree, int key) {
  (void)btree;
  (void)key;
}

#endif //GOOTS_BTREE_IMPL
