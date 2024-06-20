/*
 * C implementation for C++ std::map using red-black tree.
 *
 * Any data type can be stored in a cmap, just like std::map.
 * A cmap instance requires the specification of two file types.
 *   1. the key;
 *   2. what data type the tree node will store;
 *
 * It will also require a comparison function to sort the tree.
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "treesort.h"

/*
 * RB tree implement function
 */
node_t *list_make_node(struct list_head *list)
{
    node_t *node = malloc(sizeof(node_t));
    node->list = list;
    node->RBnode.value =
        ((element_t *) list_entry(list, element_t, list))->value;
    cmap_create_node(node);
    return node;
}

node_t *cmap_create_node(node_t *node)
{
    /* Setup the pointers */
    node->RBnode.left = node->RBnode.right = NULL;
    rb_set_parent(&(node->RBnode), NULL);

    /* Set the color to black by default */
    rb_set_red(&(node->RBnode));

    return node;
}

void tree_free(struct rb_node *node)
{
    if (!node)
        return;

    tree_free(node->left);
    tree_free(node->right);
    free(container_of(node, node_t, RBnode));
}
struct rb_node *cmap_rotate_left(cmap_t obj, struct rb_node *node)
{
    struct rb_node *r = node->right, *rl = r->left, *up = rb_parent(node);

    /* Adjust */
    rb_set_parent(r, up);
    r->left = node;

    node->right = rl;
    rb_set_parent(node, r);

    if (node->right)
        rb_set_parent(node->right, node);

    if (up) {
        if (up->right == node)
            up->right = r;
        else
            up->left = r;
    }

    if (node == obj->head)
        obj->head = r;

    return r;
}
struct rb_node *cmap_rotate_right(cmap_t obj, struct rb_node *node)
{
    struct rb_node *l = node->left, *lr = l->right, *up = rb_parent(node);

    rb_set_parent(l, up);
    l->right = node;

    node->left = lr;
    rb_set_parent(node, l);

    if (node->left)
        rb_set_parent(node->left, node);

    if (up) {
        if (up->right == node)
            up->right = l;
        else
            up->left = l;
    }

    if (node == obj->head)
        obj->head = l;

    return l;
}
void cmap_l_l(cmap_t obj,
              struct rb_node *node UNUSED,
              struct rb_node *parent UNUSED,
              struct rb_node *grandparent,
              struct rb_node *uncle UNUSED)
{
    /* Rotate to the right according to grandparent */
    grandparent = cmap_rotate_right(obj, grandparent);

    /* Swap grandparent and uncle's colors */
    color_t c1 = rb_color(grandparent), c2 = rb_color(grandparent->right);

    if (c1 == CMAP_RED)
        rb_set_red(grandparent->right);
    else
        rb_set_black(grandparent->right);

    if (c2 == CMAP_RED)
        rb_set_red(grandparent);
    else
        rb_set_black(grandparent);
}
void cmap_l_r(cmap_t obj,
              struct rb_node *node,
              struct rb_node *parent,
              struct rb_node *grandparent,
              struct rb_node *uncle)
{
    /* Rotate to the left according to parent */
    parent = cmap_rotate_left(obj, parent);

    /* Refigure out the identity */
    node = parent->left;
    grandparent = rb_parent(parent);
    uncle =
        (grandparent->left == parent) ? grandparent->right : grandparent->left;

    /* Apply left-left case */
    cmap_l_l(obj, node, parent, grandparent, uncle);
}
void cmap_r_r(cmap_t obj,
              struct rb_node *node UNUSED,
              struct rb_node *parent UNUSED,
              struct rb_node *grandparent,
              struct rb_node *uncle UNUSED)
{
    /* Rotate to the left according to grandparent */
    grandparent = cmap_rotate_left(obj, grandparent);

    /* Swap grandparent and uncle's colors */
    color_t c1 = rb_color(grandparent), c2 = rb_color(grandparent->left);

    if (c1 == CMAP_RED)
        rb_set_red(grandparent->left);
    else
        rb_set_black(grandparent->left);

    if (c2 == CMAP_RED)
        rb_set_red(grandparent);
    else
        rb_set_black(grandparent);
}
void cmap_r_l(cmap_t obj,
              struct rb_node *node,
              struct rb_node *parent,
              struct rb_node *grandparent,
              struct rb_node *uncle)
{
    /* Rotate to the right according to parent */
    parent = cmap_rotate_right(obj, parent);

    /* Refigure out the identity */
    node = parent->right;
    grandparent = rb_parent(parent);
    uncle =
        (grandparent->left == parent) ? grandparent->right : grandparent->left;

    /* Apply right-right case */
    cmap_r_r(obj, node, parent, grandparent, uncle);
}
void cmap_fix_colors(cmap_t obj, struct rb_node *node)
{
    /* If root, set the color to black */
    if (node == obj->head) {
        rb_set_black(node);
        return;
    }

    /* If node's parent is black or node is root, back out. */
    if (rb_is_black(rb_parent(node)) && rb_parent(node) != obj->head)
        return;

    /* Find out the identity */
    struct rb_node *parent = rb_parent(node), *grandparent = rb_parent(parent),
                   *uncle;

    if (!rb_parent(parent))
        return;

    /* Find out the uncle */
    if (grandparent->left == parent)
        uncle = grandparent->right;
    else
        uncle = grandparent->left;

    if (uncle && rb_is_red(uncle)) {
        /* If the uncle is red, change color of parent and uncle to black */
        rb_set_black(uncle);
        rb_set_black(parent);

        /* Change color of grandparent to red. */
        rb_set_red(grandparent);

        /* Call this on the grandparent */
        cmap_fix_colors(obj, grandparent);
    } else if (!uncle || rb_is_black(uncle)) {
        /* If the uncle is black. */
        if (parent == grandparent->left && node == parent->left)
            cmap_l_l(obj, node, parent, grandparent, uncle);
        else if (parent == grandparent->left && node == parent->right)
            cmap_l_r(obj, node, parent, grandparent, uncle);
        else if (parent == grandparent->right && node == parent->left)
            cmap_r_l(obj, node, parent, grandparent, uncle);
        else if (parent == grandparent->right && node == parent->right)
            cmap_r_r(obj, node, parent, grandparent, uncle);
    }
}
void cmap_calibrate(cmap_t obj)
{
    if (!obj->head) {
        obj->it_least = obj->it_most = NULL;
        return;
    }

    /* Recompute it_least and it_most */
    obj->it_least = obj->it_most = obj->head;

    while (obj->it_least->left)
        obj->it_least = obj->it_least->left;

    while (obj->it_most->right)
        obj->it_most = obj->it_most->right;
}
cmap_t cmap_new(size_t s1, size_t s2, int (*cmp)(void *, void *))
{
    cmap_t obj = malloc(sizeof(struct cmap_internal));

    obj->head = NULL;

    obj->key_size = s1;
    obj->element_size = s2;
    obj->size = 0;

    obj->comparator = cmp;

    obj->it_end = malloc(sizeof(struct rb_node));
    obj->it_least = malloc(sizeof(struct rb_node));
    obj->it_most = malloc(sizeof(struct rb_node));

    obj->it_end->left = obj->it_end->right = NULL;
    obj->it_least->left = obj->it_least->right = NULL;
    obj->it_most->left = obj->it_most->right = NULL;

    return obj;
}

bool cmap_insert(cmap_t obj, node_t *node, void *value)
{
    cmap_create_node(node);

    obj->size++;

    if (!obj->head) {
        /* Just insert the node in as the new head. */
        obj->head = &node->RBnode;
        rb_set_black(obj->head);

        /* Calibrate the tree to properly assign pointers. */
        cmap_calibrate(obj);
        return true;
    }

    /* Traverse the tree until we hit the end or find a side that is NULL */
    for (struct rb_node *cur = obj->head;;) {
        int res = obj->comparator(node->RBnode.value, cur->value);
        if (!res) /* If the key matches something else, don't insert */
            assert(0 && "not support repetitive value");

        if (res < 0) {
            if (!cur->left) {
                cur->left = &(node->RBnode);
                rb_set_parent(&(node->RBnode), cur);
                cmap_fix_colors(obj, &(node->RBnode));
                break;
            }
            cur = cur->left;
        } else {
            if (!cur->right) {
                cur->right = &(node->RBnode);
                rb_set_parent(&(node->RBnode), cur);
                cmap_fix_colors(obj, &(node->RBnode));
                break;
            }
            cur = cur->right;
        }
    }

    cmap_calibrate(obj);
    return true;
}


// void print_level_order(struct rb_node* root)
// {
//     int h = height(root);
//     for (int i = 1; i <= h; i++)
//         print_current_level(root, i);
//     printf("\n");
// }

node_t *cmap_first(cmap_t obj)
{
    struct rb_node *n = obj->head;
    if (!n)
        return NULL;

    while (n->left)
        n = n->left;
    return (node_t *) container_of(n, node_t, RBnode);
}
node_t *cmap_next(node_t *node)
{
    if (!node)
        return NULL;

    if (node->RBnode.right) {
        node = (node_t *) container_of(node->RBnode.right, node_t, RBnode);
        while (node->RBnode.left)
            node = (node_t *) container_of(node->RBnode.left, node_t, RBnode);
        return node;
    }

    struct rb_node *parent;
    while ((parent = rb_parent(&(node->RBnode))) &&
           &(node->RBnode) == parent->right)
        node = (node_t *) container_of(parent, node_t, RBnode);

    return parent ? (node_t *) container_of(parent, node_t, RBnode) : NULL;
}


void tree_sort(struct list_head *head)
{
    cmap_t map = cmap_new(sizeof(long), sizeof(NULL), cmap_cmp_str);
    struct list_head *list;
    list_for_each (list, head) {
        node_t *node = list_make_node(list);
        cmap_insert(map, node, NULL);
    }

    node_t *node = cmap_first(map);
    if (!node) {
        free(map);
        return;
    }

    for (node_t *next = cmap_next(node); next; next = cmap_next(next)) {
        list_del(next->list);
        list_add(next->list, node->list);
        node = next;
    }

    tree_free(map->head);
    free(map);
}