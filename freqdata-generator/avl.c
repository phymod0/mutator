/**
 * @file avl.c
 *
 * @brief Function definitions for AVL-tree implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include "avl.h"

#define AVL_INVARIANT


static struct avl_node *avl_node_alloc(avl_key key, avl_value value)
{
	struct avl_node *new;

	if (!(new = malloc(sizeof *new))) {
		printf("avl_node_alloc: Failed allocation\n");
		return NULL;
	}
	new->right = NULL;
	new->left = NULL;
	new->key = key;
	new->value = value;
	new->height = 0;

	return new;
}


static void avl_node_free(struct avl_node *node, keyfree_t keyfree,
			  valfree_t valfree)
{
	if (node) {
		if (keyfree)
			keyfree(node->key);
		if (valfree)
			valfree(node->value);
		free(node);
	}

	return;
}


static inline int balance_factor(struct avl_node *node)
{
	int left_height, right_height;

	if (!node)
		return 0;

	left_height = node->left ? node->left->height : -1;
	right_height = node->right ? node->right->height : -1;

	return right_height - left_height;
}


#define MAX(a, b) ((a) > (b) ? (a) : (b))
static inline void fix_height(struct avl_node *node)
{
	int left_height, right_height;

	if (!node)
		return;

	left_height = node->left ? node->left->height : -1;
	right_height = node->right ? node->right->height : -1;

	node->height = MAX(left_height, right_height) + 1;

	return;
}
#undef MAX


static struct avl_node *left_rotate(struct avl_node *node)
{
	struct avl_node *right, *right_left;

	if (!node || !node->right)
		goto fail;

	right = node->right;
	right_left = node->right->left;

	right->left = node;
	node->right = right_left;

	fix_height(node);
	fix_height(right);

	return right;
fail:
	printf("Left rotation failed...\n");
	return NULL;
}


static struct avl_node *right_rotate(struct avl_node *node)
{
	struct avl_node *left, *left_right;

	if (!node || !node->left)
		goto fail;

	left = node->left;
	left_right = node->left->right;

	left->right = node;
	node->left = left_right;

	fix_height(node);
	fix_height(left);

	return left;
fail:
	printf("Right rotation failed...\n");
	return NULL;
}


static struct avl_node *rebalance(struct avl_node *node)
{
	if (!node)
		return NULL;

#ifdef AVL_INVARIANT
	if (balance_factor(node) > 1) {
		if (balance_factor(node->right) < 0) {
			node->right = right_rotate(node->right);
			fix_height(node);
		}
		return left_rotate(node);
	} else if (balance_factor(node) < -1) {
		if (balance_factor(node->left) > 0) {
			node->left = left_rotate(node->left);
			fix_height(node);
		}
		return right_rotate(node);
	}
#endif /* AVL_INVARIANT */

	return node;
}


/* Should return NULL on failure */
static struct avl_node *__avl_insert(struct avl_node *node, avl_key key,
				     avl_value value, keycmp_t keycmp)
{
	struct avl_node **child, *new_child;

	if (!node)
		return avl_node_alloc(key, value);
	if (node->key == key)
		return NULL;

	child = keycmp(key, node->key) < 0 ? &(node->left) : &(node->right);
	if (!(new_child = __avl_insert(*child, key, value, keycmp)))
		return NULL;
	*child = new_child;

	fix_height(node);

	return rebalance(node);
}


static struct avl_node *__avl_delete_find_min(struct avl_node *node)
{
	struct avl_node *prev;

	prev = NULL;
	while (node)
		node = (prev = node)->left;

	return prev;
}


static struct avl_node *__avl_delete_remove_min(struct avl_node *node,
						keyfree_t keyfree,
						valfree_t valfree)
{
	struct avl_node *prev, *ret;

	if (!node)
		return NULL;

	prev = NULL;
	while (node)
		node = (prev = node)->left;
	ret = prev->right;

	avl_node_free(prev, keyfree, valfree);

	fix_height(ret);
	return rebalance(ret);
}


/* Should return NULL on failure */
static struct avl_node *__avl_delete(struct avl_node *node, avl_key key,
				     keycmp_t keycmp, keyfree_t keyfree,
				     valfree_t valfree)
{
	int cmp;
	struct avl_node *min;

	if (!node)
		return NULL;

	cmp = keycmp(key, node->key);

	if (cmp < 0) {
		node->left = __avl_delete(node->left, key, keycmp,
					  keyfree, valfree);
		fix_height(node);
		return rebalance(node);
	}
	if (cmp > 0) {
		node->right = __avl_delete(node->right, key, keycmp,
					   keyfree, valfree);
		fix_height(node);
		return rebalance(node);
	}

	min = __avl_delete_find_min(node->right);
	if (!min) {
		struct avl_node *ret;
		ret = node->left;
		avl_node_free(node, keyfree, valfree);
		return ret;
	}
	node->key = min->key;
	node->value = min->value;
	node->right = __avl_delete_remove_min(node->right, keyfree, valfree);

	fix_height(node);
	return rebalance(node);
}


static void avl_node_recursive_free(struct avl_node *node, keyfree_t keyfree,
				    valfree_t valfree)
{
	if (node) {
		avl_node_recursive_free(node->right, keyfree, valfree);
		avl_node_recursive_free(node->left, keyfree, valfree);
	}
	avl_node_free(node, keyfree, valfree);

	return;
}


struct avl_tree *avl_create_tree(keycmp_t keycmp, keyfree_t keyfree,
				 valfree_t valfree)
{
	struct avl_tree *new;

	if (!(new = malloc(sizeof *new))) {
		printf("avl_create_tree: Failed allocation\n");
		return NULL;
	}
	new->root = NULL;
	new->keycmp = keycmp;
	new->keyfree = keyfree;
	new->valfree = valfree;

	return new;
}


static int avl_traverse_recursive(struct avl_node *node,
				  traverse_cb_t traverse_cb, void *cb_data)
{
	int val_left, val_right, val_this;
	if (!node)
		return 0;
	val_right = avl_traverse_recursive(node->right, traverse_cb, cb_data);
	val_this = traverse_cb(node->key, node->value, cb_data);
	val_left = avl_traverse_recursive(node->left, traverse_cb, cb_data);
	return val_left + val_this + val_right;
}


int avl_insert(struct avl_tree *tree, avl_key key, avl_value value)
{
	struct avl_node *new_parent;

	if (!(new_parent = __avl_insert(tree->root, key, value, tree->keycmp)))
		return -1;
	tree->root = new_parent;

	return 0;
}


void avl_delete(struct avl_tree *tree, avl_key key)
{
	struct avl_node *new_parent;

	tree->root = __avl_delete(tree->root, key, tree->keycmp, tree->keyfree,
				  tree->valfree);

	return;
}


int avl_lookup(struct avl_tree *tree, avl_key key, avl_value *value)
{
	struct avl_node *tmp;

	tmp = tree->root;
	while (tmp) {
		int cmp;
		cmp = tree->keycmp(key, tmp->key);
		if (cmp == 0) {
			if (value)
				*value = tmp->value;
			return 0;
		}
		tmp = cmp < 0 ? tmp->left : tmp->right;
	}

	return -1;
}


struct avl_node *avl_get_node(struct avl_tree *tree, avl_key key)
{
	struct avl_node *tmp;

	tmp = tree->root;
	while (tmp) {
		int cmp;
		cmp = tree->keycmp(key, tmp->key);
		if (cmp == 0)
			return tmp;
		tmp = cmp < 0 ? tmp->left : tmp->right;
	}

	return NULL;
}


int avl_traverse(struct avl_tree *tree, traverse_cb_t traverse_cb,
		 void *cb_data)
{
	return avl_traverse_recursive(tree->root, traverse_cb, cb_data);
}


void avl_destroy_tree(struct avl_tree *tree)
{
	avl_node_recursive_free(tree->root, tree->keyfree, tree->valfree);
	free(tree);

	return;
}
