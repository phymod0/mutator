/**
 * @file avl.h
 *
 * @brief Function declarations for AVL-tree implementation
 */

#ifndef __AVL_H
#define __AVL_H

typedef char *avl_key;
typedef unsigned long long avl_value;
typedef int (*keycmp_t)(avl_key key_a, avl_key key_b);
typedef void (*keyfree_t)(avl_key key);
typedef void (*valfree_t)(avl_value value);
typedef int (*traverse_cb_t)(avl_key key, avl_value value, void *cb_data);

struct avl_node {
	struct avl_node *right, *left;
	avl_key key;
	avl_value value;
	int height;
};

struct avl_tree {
	keycmp_t keycmp;
	keyfree_t keyfree;
	valfree_t valfree;
	struct avl_node *root;
};

struct avl_tree *avl_create_tree(keycmp_t keycmp, keyfree_t keyfree,
				 valfree_t valfree);
/** 0 if success, -1 otherwise */
int avl_insert(struct avl_tree *tree, avl_key key, avl_value value);
void avl_delete(struct avl_tree *tree, avl_key key);
/** 0 if found, -1 otherwise */
int avl_lookup(struct avl_tree *tree, avl_key key, avl_value *value);
struct avl_node *avl_get_node(struct avl_tree *tree, avl_key key);
int avl_traverse(struct avl_tree *tree, traverse_cb_t traverse_cb,
		 void *cb_data);
void avl_destroy_tree(struct avl_tree *tree);

#endif /* __AVL_H */
