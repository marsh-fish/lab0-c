#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/**
 * q_shuffle() - Shuffle elements of queue
 * @head: header of queue
 *
 * No effect if queue is NULL or empty. If there has only one element, do
 * nothing.
 */
void q_shuffle(struct list_head *head);


/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
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

/*
 * New an element for s,
 * It will allocate memory for s
 * Return null if allocation failed.
 */
element_t *new_element(char *s)
{
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return NULL;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return NULL;
    }
    return new;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = new_element(s);
    if (!new)
        return false;
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = new_element(s);
    if (!new)
        return false;
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
    return q_remove_head(head->prev->prev, sp, bufsize);
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

/* Merge two sorted list */
void merge_two_sorted(struct list_head *head_cut,
                      struct list_head *head,
                      bool descend)
{
    struct list_head *this = head->next;
    while (!list_empty(head_cut) && this != head) {
        element_t *entry1 = list_first_entry(head_cut, element_t, list);
        element_t *entry2 = list_entry(this, element_t, list);
        if (strcmp(entry1->value, entry2->value) * (descend ? -1 : 1) > 0) {
            this = this->next;
        } else
            list_move_tail(&entry1->list, this);
    }
    list_splice_tail_init(head_cut, head);
};


/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;


    struct list_head *slow, *fast;
    for (slow = head, fast = head;
         fast->next != head && fast->next->next != head;
         fast = fast->next->next, slow = slow->next)
        ;

    LIST_HEAD(temp);
    list_cut_position(&temp, head, slow);
    q_sort(&temp, descend);
    q_sort(head, descend);

    merge_two_sorted(&temp, head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head)
        return 0;

    struct list_head *this = head->next;

    while (this->next != head) {
        element_t *entry1 = list_entry(this, element_t, list);
        element_t *entry2 = list_entry(this->next, element_t, list);
        if (strcmp(entry1->value, entry2->value) < 0) {
            this = this->next;
        } else {
            list_del(&entry2->list);
            q_release_element(entry2);
        }
    }

    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head)
        return 0;

    struct list_head *this = head->prev;

    while (this->prev != head) {
        element_t *entry1 = list_entry(this, element_t, list);
        element_t *entry2 = list_entry(this->prev, element_t, list);
        if (strcmp(entry1->value, entry2->value) < 0) {
            this = this->prev;
        } else {
            list_del(&entry2->list);
            q_release_element(entry2);
        }
    }

    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head)
        return 0;

    if (q_size(head) < 2)
        return q_size(head);

    struct list_head *this = list_first_entry(head, queue_contex_t, chain)->q;
    queue_contex_t *entry, *safe;
    bool first_time = true;
    list_for_each_entry_safe (entry, safe, head, chain) {
        if (first_time) {
            first_time = false;
            continue;
        }
        merge_two_sorted(entry->q, this, descend);
    }

    return q_size(this);
}

static inline void swap(struct list_head *a, struct list_head *b)
{
    element_t *a_entry = list_entry(a, element_t, list);
    element_t *b_entry = list_entry(b, element_t, list);
    char *tmp = b_entry->value;
    b_entry->value = a_entry->value;
    a_entry->value = tmp;
}

/* Shuffle elements of queue */
void q_shuffle(struct list_head *head)
{
    if (!head || list_is_singular(head))
        return;

    int len = q_size(head) - 1;
    struct list_head *new;

    for (new = head->prev; new != head &&len; new = new->prev, len--) {
        int r = rand() % len;
        struct list_head *old = head->next;
        while (r--)
            old = old->next;
        swap(old, new);
    }
}
