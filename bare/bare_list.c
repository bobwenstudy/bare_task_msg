#include <stdint.h>
#include <stddef.h>
#include "bare_heap.h"
#include "bare_task.h"
#include "bare_list.h"

#define BARE_LIST_POISON1 NULL
#define BARE_LIST_POISON2 NULL

void INIT_BARE_LIST_HEAD(struct bare_list_head *list)
{
    list->next = list;
    list->prev = list;
}

static void __list_add(struct bare_list_head *new, struct bare_list_head *prev,
                       struct bare_list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void bare_list_add(struct bare_list_head *new, struct bare_list_head *head)
{
    __list_add(new, head, head->next);
}

void bare_list_add_tail(struct bare_list_head *new, struct bare_list_head *head)
{
    __list_add(new, head->prev, head);
}

static void __list_del(struct bare_list_head *prev, struct bare_list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

void bare_list_del(struct bare_list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = BARE_LIST_POISON1;
    entry->prev = BARE_LIST_POISON2;
}

void bare_list_del_init(struct bare_list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_BARE_LIST_HEAD(entry);
}

void bare_list_replace(struct bare_list_head *old, struct bare_list_head *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

void bare_list_replace_init(struct bare_list_head *old, struct bare_list_head *new)
{
    bare_list_replace(old, new);
    INIT_BARE_LIST_HEAD(old);
}

int bare_list_empty(const struct bare_list_head *head)
{
    return head->next == head ? 1 : 0;
}
