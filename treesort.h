#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct rb_node {
    uintptr_t color;
    struct rb_node *left, *right;
    char *value;
} __attribute__((aligned(sizeof(long))));

typedef struct __node {
    struct rb_node RBnode;
    struct list_head *list;
} node_t;

struct cmap_internal {
    struct rb_node *head;

    /* Properties */
    size_t key_size, element_size, size;
    struct rb_node *it_end, *it_most, *it_least;
    int (*comparator)(void *, void *);
};
typedef enum { CMAP_RED = 0, CMAP_BLACK } color_t;

#define rb_parent(r) ((struct rb_node *) (((r)->color & ~7)))
#define rb_color(r) ((color_t) (r)->color & 1)

#define rb_set_parent(r, p)                         \
    do {                                            \
        (r)->color = rb_color(r) | (uintptr_t) (p); \
    } while (0)
#define rb_set_red(r)     \
    do {                  \
        (r)->color &= ~1; \
    } while (0)
#define rb_set_black(r)  \
    do {                 \
        (r)->color |= 1; \
    } while (0)

#define rb_is_red(r) (!rb_color(r))
#define rb_is_black(r) (rb_color(r))

#if defined(__GNUC__) || defined(__clang__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

enum { _CMP_LESS = -1, _CMP_EQUAL = 0, _CMP_GREATER = 1 };

/* string comparison */
static inline int cmap_cmp_str(void *arg0, void *arg1)
{
    char *a = (char *) arg0, *b = (char *) arg1;
    int result = strcmp(a, b);
    return result < 0 ? _CMP_LESS : result > 0 ? _CMP_GREATER : _CMP_EQUAL;
}

/* Store access to the head node, as well as the first and last nodes.
 * Keep track of all aspects of the tree. All cmap functions require a pointer
 * to this struct.
 */
typedef struct cmap_internal *cmap_t;

#define cmap_init(key_type, element_type, __func) \
    cmap_new(sizeof(key_type), sizeof(element_type), __func)

/*
 * RB tree implement function
 */
node_t *list_make_node(struct list_head *head);
node_t *cmap_create_node(node_t *node);
void tree_free(struct rb_node *node);
struct rb_node *cmap_rotate_left(cmap_t obj, struct rb_node *node);
struct rb_node *cmap_rotate_right(cmap_t obj, struct rb_node *node);
void cmap_l_l(cmap_t obj,
              struct rb_node *node UNUSED,
              struct rb_node *parent UNUSED,
              struct rb_node *grandparent,
              struct rb_node *uncle UNUSED);
void cmap_l_r(cmap_t obj,
              struct rb_node *node,
              struct rb_node *parent,
              struct rb_node *grandparent,
              struct rb_node *uncle);
void cmap_r_r(cmap_t obj,
              struct rb_node *node UNUSED,
              struct rb_node *parent UNUSED,
              struct rb_node *grandparent,
              struct rb_node *uncle UNUSED);
void cmap_r_l(cmap_t obj,
              struct rb_node *node,
              struct rb_node *parent,
              struct rb_node *grandparent,
              struct rb_node *uncle);
void cmap_fix_colors(cmap_t obj, struct rb_node *node);
void cmap_calibrate(cmap_t obj);
cmap_t cmap_new(size_t s1, size_t s2, int (*cmp)(void *, void *));
bool cmap_insert(cmap_t obj, node_t *node, void *value);
node_t *cmap_first(cmap_t obj);
node_t *cmap_next(node_t *node);
void tree_sort(struct list_head *head);
bool list_is_ordered(node_t *list);
void print_level_order(struct rb_node *root);