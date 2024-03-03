#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        q_release_element(entry);
    }
    free(head);
    return;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;

    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add(&new->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;

    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add_tail(&new->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head == head->prev)
        return NULL;

    element_t *temp = list_first_entry(head, element_t, list);
    if (sp) {
        memcpy(sp, temp->value, bufsize);
        *(sp + bufsize - 1) = '\0';
    }
    list_del(&temp->list);

    return temp;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head)
        return NULL;

    element_t *temp = list_last_entry(head, element_t, list);
    if (sp) {
        memcpy(sp, temp->value, bufsize);
        *(sp + bufsize - 1) = '\0';
    }
    list_del(&temp->list);

    return temp;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return -1;

    int size = 0;
    struct list_head *this;
    list_for_each (this, head)
        ++size;
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head)
        return false;

    struct list_head *slow, *fast;
    for (slow = head->next, fast = head->next;
         fast != head && fast->next != head;
         fast = fast->next->next, slow = slow->next)
        ;

    list_del_init(slow);
    q_release_element(list_entry(slow, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;

    element_t *entry, *safe;
    bool duplicating = false;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (&safe->list == head) {
            break;
        }

        if (!strcmp(entry->value, safe->value)) {
            list_del(&entry->list);
            q_release_element(entry);
            duplicating = true;
        } else if (duplicating) {
            list_del(&entry->list);
            q_release_element(entry);
            duplicating = false;
        }
    }
    if (duplicating) {
        list_del(&entry->list);
        q_release_element(entry);
    }


    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head)
        return;

    struct list_head *this, *temp;
    for (this = head, temp = head;
         temp->next != head && temp->next->next != head;
         temp = temp->next->next) {
        this = temp->next;
        list_move(this->next, temp);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;
    struct list_head **sec_last = &head->prev->prev;
    while (*sec_last != head) {
        list_move_tail(*sec_last, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || k <= 1)
        return;

    int size = q_size(head);
    struct list_head *this = head->next;
    while (size >= k) {
        for (size_t i = 0; i < k; i++) {
            this = this->next;
        }

        LIST_HEAD(temp);

        list_cut_position(&temp, head, this->prev);
        q_reverse(&temp);
        list_splice_tail_init(&temp, head);

        size -= k;
    }
    if (size > 0) {
        for (size_t i = 0; i < size - 1; i++) {
            this = this->next;
        }
        LIST_HEAD(temp);

        list_cut_position(&temp, head, this);
        list_splice_tail_init(&temp, head);
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
