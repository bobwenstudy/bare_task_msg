#include <stdio.h>

#include "bare_common.h"
#include "bare_task.h"
#include "bare_heap.h"

bt_base_t bare_hw_interrupt_disable(void)
{
    return 0;
}
void bare_hw_interrupt_enable(bt_base_t level)
{
}

struct bare_task user_task1;
struct bare_task user_task2;

static int test_msg_consumed_user_task1 = 0;
static int test_msg_consumed_user_task2 = 0;

int user_task1_func(struct bare_msg *msg)
{
    printf("user_task1(), id: 0x%x, len: %d\n", msg->id, msg->param_len);
    for (int i = 0; i < msg->param_len; i++)
    {
        printf("0x%x:", ((uint8_t *)msg->param)[i]);
    }
    if (msg->param_len)
    {
        printf("\n");
    }

    if (test_msg_consumed_user_task1 == 0)
    {
        test_msg_consumed_user_task1 = 1;
        printf("BARE_TASK_HDL_SAVED\n");
        return BARE_TASK_HDL_SAVED;
    }

    return BARE_TASK_HDL_CONSUMED;
}

void user_task1_test(void)
{
    struct bare_task *p_task = &user_task1;

    p_task->func = user_task1_func;
    bare_task_create(p_task);

    bare_task_send_msg(p_task, bare_msg_alloc(1, 0, NULL));
    bare_task_send_msg(p_task, bare_msg_alloc(2, 0, NULL));
    uint8_t data[10];
    for (int i = 0; i < sizeof(data); i++)
    {
        data[i] = i;
    }
    bare_task_send_msg(p_task, bare_msg_alloc(0x10, sizeof(data), data));
    for (int i = 0; i < sizeof(data); i++)
    {
        data[i] = i + 0x10;
    }
    bare_task_send_msg(p_task, bare_msg_alloc(0x11, sizeof(data), data));
}

int user_task2_func(struct bare_msg *msg)
{
    printf("user_task2(), id: 0x%x, len: %d\n", msg->id, msg->param_len);
    for (int i = 0; i < msg->param_len; i++)
    {
        printf("0x%x:", ((uint8_t *)msg->param)[i]);
    }
    if (msg->param_len)
    {
        printf("\n");
    }

    if (test_msg_consumed_user_task2 == 0 && msg->id > 0x10)
    {
        test_msg_consumed_user_task2 = 1;
        printf("BARE_TASK_HDL_SAVED\n");
        return BARE_TASK_HDL_SAVED;
    }

    return BARE_TASK_HDL_CONSUMED;
}

void user_task2_test(void)
{
    struct bare_task *p_task = &user_task2;

    p_task->func = user_task2_func;
    bare_task_create(p_task);

    bare_task_send_msg(p_task, bare_msg_alloc(8, 0, NULL));
    bare_task_send_msg(p_task, bare_msg_alloc(9, 0, NULL));
    uint8_t data[20];
    for (int i = 0; i < sizeof(data); i++)
    {
        data[i] = i;
    }
    bare_task_send_msg(p_task, bare_msg_alloc(0x50, sizeof(data), data));
    for (int i = 0; i < sizeof(data); i++)
    {
        data[i] = i + 0x10;
    }
    bare_task_send_msg(p_task, bare_msg_alloc(0x51, sizeof(data), data));

#define TEST_ALLOC_LEN 100
    struct bare_msg *msg = bare_msg_alloc_len(0x51, TEST_ALLOC_LEN);
    for (int i = 0; i < TEST_ALLOC_LEN; i++)
    {
        msg->param[i] = i + 0x50;
    }
    bare_task_send_msg(p_task, msg);
}

#define HEAD_SIZE 0x1000
static uint8_t user_heap[HEAD_SIZE];
int main(void)
{
    bare_heap_init((uint32_t *)user_heap, HEAD_SIZE);
    printf("Heap Remain Size: 0x%x\n", bare_heap_get_remain_size());

    bare_task_init();

    user_task1_test();
    user_task2_test();

    // Test task delete
    // bare_task_delete(&user_task1);
    // bare_task_delete(&user_task2);

    printf("Task Start Work!\n");
    while (1)
    {
        bare_task_polling();

        if (bare_task_check_empty())
        {
            printf("Task End Work!\n");
            printf("Heap Remain Size: 0x%x\n", bare_heap_get_remain_size());
            if (!bare_heap_check_empty(HEAD_SIZE))
            {
                printf("Something Error!\n");
            }
            break;
        }
    }
    return 0;
}