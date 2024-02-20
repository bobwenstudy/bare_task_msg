#ifndef _BARE_LIST_H_
#define _BARE_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Includes -----------------------------------------------------------------*/

/** Define -------------------------------------------------------------------*/
#ifndef container_of
#define container_of(ptr, type, member) ((type *)(((const char *)(ptr)) - offsetof(type, member)))
#endif

struct bare_list_head
{
    struct bare_list_head *next, *prev;
};

/** Exported functions -------------------------------------------------------*/
#define BARE_LIST_HEAD_INIT(name)                                                                  \
    {                                                                                              \
        &(name), &(name)                                                                           \
    }
#define BARE_LIST_HEAD(name) struct bare_list_head name = BARE_LIST_HEAD_INIT(name)
void INIT_BARE_LIST_HEAD(struct bare_list_head *list);

void bare_list_add(struct bare_list_head *new, struct bare_list_head *head);
void bare_list_add_tail(struct bare_list_head *new, struct bare_list_head *head);

void bare_list_del(struct bare_list_head *entry);
void bare_list_del_init(struct bare_list_head *entry);

void bare_list_replace(struct bare_list_head *old, struct bare_list_head *new);
void bare_list_replace_init(struct bare_list_head *old, struct bare_list_head *new);

int bare_list_empty(const struct bare_list_head *head);

#define bare_list_entry(ptr, type, member)       container_of(ptr, type, member)
#define bare_list_first_entry(ptr, type, member) bare_list_entry((ptr)->next, type, member)
#define bare_list_for_each(pos, head)            for (pos = (head)->next; pos != (head); pos = (pos)->next)
#define bare_list_for_each_safe(pos, n, head)                                                      \
    for (pos = (head)->next, n = (pos)->next; pos != (head); pos = (n), n = (pos)->next)

#ifdef __cplusplus
}
#endif

#endif /*!< _BARE_LIST_H_ */