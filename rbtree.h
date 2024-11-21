#ifndef RBTREE_H
#define RBTREE_H

typedef enum {
	RED = 1,
	BLACK
} color_t;

#define SET_COLOR(n, c)                                                   \
	do {                                                                  \
		((n)->color = c)                                                  \
	} while (0)
#define GET_COLOR(n) ((n)->color)
#define IS_RED(n)	 ((n)->color == RED)
#define IS_BLACK(n)	 ((n)->color == BLACK)

/* forward declartions  */
typedef struct node_t node_t;

/* clang-format off */
node_t *create_node(void *val); /* initializes a node, all new nodes are RED initially */
void insert_node(node_t **root, void *val); /* inserts a node, internlly it does santiy checkig and rebalance tree if needed */
void delete_node(node_t *root, void *val); /* inserts a node, internlly it does santiy checkig and rebalance tree if needed */
void search(node_t *n, void *query_key); /* search for a node */
void range_search(node_t *n, node_t **out_list, void *query_key); /* range search for a given query */
/* clang-format on */
#endif
