#include "rbtree.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* https://en.wikipedia.org/wiki/Red%E2%80%93black_tree
 * Definition of a red-black tree
 * A red-black tree is a binary search tree with the following properties:
 * 	1- Every node is either red or black.
 * 	2- Root is always black.
 * 	3- Every leaf (NULL) is black.
 * 	4- If a node is red, then both its children are black.
 * 	5- Every simple path from a node to leaf contains the same number of
 * 		black nodes.
 * 	6- New nodes are always red.
 *	7- No path can have two consecutive red nodes a(red)->b(red)
 *
 * If the tree violates these constraints at any point, we rebalace:
 * 	1- if the node has a black aunt, we rotate around its parent
 * 	2- if the node has a red aunt, we color-flip
 */

/* bit flags for each type of violation, -- used for experiments -- */
/* clang-format off */
typedef enum {
    RB_VALID                = 0x00, /* 00000000: no violations, clean state. */
    RB_INVALID_COLOR        = 0x01, /* 00000001: rule 1 - invalid color. */
    RB_RED_ROOT             = 0x02, /* 00000010: rule 2 - root is red. */
    RB_NULL_NOT_BLACK       = 0x04, /* 00000100: rule 3 - leaf (NULL) nodes must be black. */
    RB_RED_CHILD_OF_RED     = 0x08, /* 00001000: rule 4/7 - red node has a red child. */
    RB_UNEQUAL_BLACK_PATHS  = 0x10  /* 00010000: rule 5 - black nodes in all paths are unequal. */
} rb_violation_t;
/* clang-format on */

typedef uint32_t rb_validation_t;
/* if rb_validation_t is RB_RED_ROOT | RB_RED_CHILD_OF_RED
 * x =  00000010 | 00001000
 * x = 00001010
 */

/* holds rotation types  */
typedef enum {
	LEFT,
	RIGHT
} rotation_t;

/* tree node definition */
struct node_t {
	void   *key;	/* key/value */
	node_t *parent; /* parent*/
	node_t *left;	/* left subtree */
	node_t *right;	/* right subtree */
	color_t color;	/* either red or black, always black for null or tree
					   root node */
};

/* forward declarations */
/* clang-format off */
static void rb_insert_node(node_t *root, node_t *newnode);
static void rb_validate_tree(node_t *root, rb_validation_t *violations);
static void rb_validate_tree_recursive(node_t *root, rb_validation_t *violations, int *black_h);
static color_t rb_get_uncle_color(node_t *n);
static void rb_color_flip(node_t *root);
static void rb_rotate(node_t *node, rotation_t dir);
static void rb_rebalance(node_t *node);
static bool rb_validate_black_height(node_t *root, int black_height);
static int rb_get_black_height(node_t *root);
/* clang-format on  */

node_t *
create_node(void *val)
{
	if (val == NULL) return NULL;

	node_t *n = (node_t *)malloc(sizeof(node_t));
	if (n == NULL) return NULL;

	n->key	  = val;
	n->parent = n->right = n->left = NULL;

	return n;
}

void
insert_node(node_t **root, void *val)
{
	node_t *newnode = create_node(val);
	assert(newnode);

	if (*root == NULL) {
		/* first node becomes root and must be black */
		newnode->color = BLACK;
		*root		   = newnode;
		return;
	}
	rb_insert_node(*root, newnode);
	/* validate tree after insertion */

	/* assume tree is valid, why not */
    rb_validation_t violations = RB_VALID;
	rb_validate_tree(*root, &violations);
	
    if (violations == RB_VALID) {
        printf("tree is a valid rb tree\n");
        return;
    }
    
    printf("tree validation failed.\n");
    
    /* Check each violation flag */
    if (violations & RB_RED_ROOT) {
        printf("- root is red (violates property 2)\n");
    }
    
    if (violations & RB_RED_CHILD_OF_RED) {
        printf("- found red node with red child (violates properties 4/7)\n");
    }
    
    if (violations & RB_UNEQUAL_BLACK_PATHS) {
        printf("- paths have different number of black nodes (violates property 5)\n");
    }
    
    if (violations & RB_INVALID_COLOR) {
        printf("- found node with invalid color (violates property 1)\n");
    }
    
    if (violations & RB_NULL_NOT_BLACK) {
        printf("- found null leaf that isn't black (violates property 3)\n");
    }

	/* the fix should be on the newly node that was inserted */
	/* 	so instead of passing the root to the below functions, we should pass
	 * the newnode */
	
}

void
delete_node(node_t *root, void *val)
{
}
void
search(node_t *n, void *query_key)
{
}
void
range_search(node_t *n, node_t **out_list, void *query_key)
{
}

/* this function does a simple bst insertion, the caller then validate the tree
 * after insertion */
static void
rb_insert_node(node_t *root, node_t *newnode)
{
	node_t *current = root;
	node_t *parent	= NULL;

	/* find insertion point */
	while (current != NULL) {
		parent = current;
		if (newnode->key < current->key)
			current = current->left;
		else if (newnode->key > current->key)
			current = current->right;
		else
			return;
	}

	/* set parent relationship */
	newnode->parent = parent;

	/* insert node in correct position */
	if (newnode->key < parent->key) {
		parent->left = newnode;
	} else {
		parent->right = newnode;
	}

	/* RB tree insertion rules:
	 * 1. new node is red
	 * 2. fix RB properties if violated (this happens in the caller fucntion
	 * after this routeine retruns)
	 */
	newnode->color = RED;
}


static int
rb_get_black_height(node_t *root)
{
	node_t *curr = root;
	int black_height = 0;
	while (curr->left) {
		if (!IS_RED(curr)) black_height++;
		curr = curr->left;
	}
	return black_height;
}

static bool
rb_validate_black_height(node_t *root, int black_height)
{
	if (!root) return black_height == 0;
	if (!IS_RED(root)) black_height--;
	return rb_validate_black_height(root->left, black_height) &&
		   rb_validate_black_height(root->right, black_height);
}

static void
rb_validate_tree(node_t *root, rb_validation_t *violations)
{
	/* assume rb tree is valid because why not  */
	*violations = RB_VALID;

	/* empty tree is valid */
	if (!root) {
		return;
	}

	/* vrify root has no parent */
	if (root->parent != NULL) {
		printf("root node has parent pointer!\n");
		assert(0);
	}

	/*get black hegiht */
	int				black_height = rb_get_black_height(root);

	/* root should be black */
    if (IS_RED(root)) {
        *violations &= ~RB_VALID;
        *violations |= RB_RED_ROOT;
    }
	
	/* check all other properties recursively */
	rb_validate_tree_recursive(root, violations, &black_height);
}

/* dfs with preorder traversal  */
static void
rb_validate_tree_recursive(node_t *root, rb_validation_t *violations, int *black_h)
{
	if (!root) {
        /* If black height is not zero at leaf, tree is invalid */
        if (*black_h != 0) {
            *violations &= ~RB_VALID;
            *violations |= RB_UNEQUAL_BLACK_PATHS;
        }
        return;
    }

	/* if invalid colors */
    if (!IS_RED(root) && !IS_BLACK(root)) {
        *violations &= ~RB_VALID;
        *violations |= RB_INVALID_COLOR;
    }

	if (IS_BLACK(root)) {
		 /* if ndoe is not red, decrement black height */
		(*black_h)--;
	}

	/* check if a red node has non-black childrens */
	/* this should also check if there is a path with two consecutive red nodes */
	if (IS_RED(root)){
		/* note null nodes are consdiered black, so */
		/* 	if a red node has no childrens it's still valid  */
		if (root->left && IS_RED(root->left) ||
			root->right && IS_RED(root->right)){
			 *violations &= ~RB_VALID;
             *violations |= RB_RED_CHILD_OF_RED;
		}
	}
	
    /* save current black_h since we need same value for both paths */
    int current_black_h = *black_h;
    rb_validate_tree_recursive(root->left, violations, black_h);
    
    /* restore black_h for right path */
    *black_h = current_black_h;
    rb_validate_tree_recursive(root->right, violations, black_h);
	
}

/* returns the color of the aunt,
   the return value determines what fix is needed */
static color_t
rb_get_uncle_color(node_t *n)
{
	if (n->parent == NULL || n->parent->parent == NULL) return -1;
	node_t *grandparent = n->parent->parent;
	if (grandparent->left == n->parent) {
		/* null aunt is a black node */
		return grandparent->right == NULL ? BLACK : grandparent->right->color;
	}
	/* null nodes are considered black */
	return grandparent->left == NULL ? BLACK : grandparent->left->color;
}

static void
rb_color_flip(node_t *root)
{
}
static void
rb_rotate(node_t *node, rotation_t dir)
{
}

static void
rb_rebalance(node_t *node)
{
	color_t c = rb_get_uncle_color(node);
	if (c == -1) return;
	if (c == RED) {
		rb_color_flip(node);
		return;
	}
	rb_rotate(node->parent, LEFT);
}