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

  //root-> {child[0] -> {....}, ..., maxChildren -> {...}}
  typedef struct BTreeNode BTreeNode;
  struct BTreeNode {
    BTreeNode *parent;
    BTreeNode *next;
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
  BTreeNode btree_search(BTree root_node, int key);
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

  //  BTreeNode btree_search(BTree btree, int key) {
    //    int found = 0;
    //    BTreeNode *curr = btree.root;
    //    for (;!found && curr != NULL;) {
      //      for (int i = 0; i < curr->key_count; i++) {
        //        Data curr_key = curr->kv[i];
        //        if (curr_key.key == key) {
          //          found = 1;
          //          break;
          //        } else if (key < curr_key.key) {
          //          //go left
          //          curr = &curr->children[i];
          //          break;
          //        }
        //      }
      //    }
    //    return *curr;
    //  }
  //
  void btree_append(BTree *btree, BTreeNode *node, int key) {
    //I believe we need bounds checking since we are using our arena? We can allocate 4 things
    if (btree->root == NULL) {
      btree->root = node;
      node->kv = arena_push(btree->arena, sizeof(Data) * btree->n - 1, alignof(Data));
      node->kv[node->key_count].key = key;
      node->kv[node->key_count].value = key;
      node->key_count += 1;
    } else {
      BTreeNode *curr_node = btree->root;
      bool is_leaf = curr_node->children == NULL;
      if (is_leaf) {
        //Add to node
        curr_node->kv[curr_node->key_count].key = key;
        curr_node->kv[curr_node->key_count].value = key;
        curr_node->key_count += 1;
        if (curr_node->key_count == btree->n) {
          //split curr_node by
          int median_index = curr_node->key_count/2;
          Data median_node = curr_node->kv[median_index];
          //if we are root create children
          BTreeNode *new_node = arena_push(btree->arena, sizeof(BTreeNode), alignof(BTreeNode));
          new_node->kv = arena_push(btree->arena, sizeof(Data) * btree->n - 1, alignof(Data));
          int extra_index = 0;
          for (int i = median_index + 1;i < curr_node->key_count; i++) {
            new_node->kv[extra_index] = curr_node->kv[i];
            extra_index += 1;
          }
          curr_node->key_count -= extra_index;
          curr_node->next = new_node;
        }
        //Sort keys
        for (int i = 1;i < curr_node->key_count; ++i) {
          if (curr_node->kv[i - 1].key > curr_node->kv[i].key) {
            Data temp = curr_node->kv[i - 1];
            curr_node->kv[i - 1] = curr_node->kv[i];
            curr_node->kv[i] = temp;
          }
        }

      }
    }

  }

#endif //GOOTS_BTREE_IMPL


//#if 0
//  int correct_child = -1;
//  for (;;) {
  //    if (curr_node == NULL) {
    //      break;
    //    }
  //    bool is_leaf = curr_node->children == NULL;
  //    if (!is_leaf) {
    //      //Go to correct child
    //      for (int i = 0; i < curr_node->key_count; i++) {
      //        if (key < curr_node->kv[i].key) {
        //          correct_child = i;
        //          break;
        //        }
      //      }
    //    }
  //    if (curr_node->key_count >= btree->n ) {
    //      //split node based on median
    //      int median_key = -1;
    //      int median_index = curr_node->key_count/2;
    //      Data median_node = curr_node->kv[median_index]; //This becomes new root (this node)
    //      //Split the other 2 based on this value into 2 new children
    //      //Allocate for children of this node
    //      BTreeNode *new_children = arena_push(btree->arena, sizeof(BTreeNode) * btree->n, alignof(BTreeNode));
    //      new_children->parent = curr_node;
    //      for (int i = 0; i < btree->n; i++) {
      //        new_children[i].kv = arena_push(btree->arena, sizeof(Data) * btree->n - 1, alignof(Data));
      //      }
    //      curr_node->children = new_children;
    //      curr_node->child_count = 2;
    //      for (int i = 0;i < median_index; ++i) {
      //        curr_node->children[0].kv[i] = curr_node->kv[i];
      //        curr_node->children[0].key_count += 1;
      //        //          curr_node->children[i] = curr_node->kv[i];
      //        //          printf("Child %d: %d", i, curr_node->children[i]);
      //      }
    //      int extra_index = 0;
    //      for (int i = median_index + 1;i < curr_node->key_count; i++) {
      //        curr_node->children[1].kv[extra_index] = curr_node->kv[i];
      //        curr_node->children[1].key_count += 1;
      //        extra_index += 1;
      //      }
    //      //When doing the split we need to account for our new key
    //      curr_node->kv[0] = median_node;
    //      curr_node->key_count = 1;
    //    } else if (is_leaf) {
    //      //add key
    //      curr_node->kv[curr_node->key_count].key = key;
    //      curr_node->kv[curr_node->key_count].value = key;
    //      curr_node->key_count += 1;
    //      //sort keys
    //      for (int i = 1;i < curr_node->key_count; ++i) {
      //        if (curr_node->kv[i - 1].key > curr_node->kv[i].key) {
        //          Data temp = curr_node->kv[i - 1];
        //          curr_node->kv[i - 1] = curr_node->kv[i];
        //          curr_node->kv[i] = temp;
        //        }
      //      }
    //    } else {
    //      curr_node = curr_node->children;
    //    }
  //  }
//
//#endif