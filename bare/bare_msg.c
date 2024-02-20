#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "bare_common.h"
#include "bare_heap.h"
#include "bare_msg.h"

void *bare_msg_alloc_len(uint16_t id, uint16_t const param_len)
{
    __bare_disable_isr();
    struct bare_msg *msg = (struct bare_msg *)bare_heap_malloc(sizeof(struct bare_msg) + param_len);
    __bare_enable_isr();

    if (msg == NULL)
    {
        return NULL;
    }

    memset(msg, 0, sizeof(struct bare_msg) + param_len);

    msg->id = id;
    msg->param_len = param_len;

    return msg;
}

void *bare_msg_alloc(uint16_t id, uint16_t const param_len, void *param)
{
    struct bare_msg *msg = bare_msg_alloc_len(id, param_len);

    if (msg == NULL)
    {
        return NULL;
    }

    memcpy(msg->param, param, param_len);

    return msg;
}

void bare_msg_free(struct bare_msg *msg)
{
    __bare_disable_isr();
    bare_heap_free(msg);
    __bare_enable_isr();
}
