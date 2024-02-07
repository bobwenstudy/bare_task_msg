#include <stdint.h>
#include <stddef.h>

#include "bare_common.h"
#include "bare_heap.h"
#include "bare_task.h"
#include "bare_list.h"

static BARE_LIST_HEAD(task_list);

void bare_task_send_msg(struct bare_task* task, struct bare_msg* msg)
{
	__bare_disable_isr();
	bare_list_add_tail(&msg->node, &task->msg_list);
	__bare_enable_isr();
}

void bare_task_delete(struct bare_task* task)
{
	__bare_disable_isr();
	bare_list_del(&task->node);
	__bare_enable_isr();
}

void bare_task_create(struct bare_task* task)
{
	INIT_BARE_LIST_HEAD(&(task->msg_list));
	__bare_disable_isr();
	bare_list_add_tail(&task->node, &task_list);
	__bare_enable_isr();
}

int bare_task_check_empty(void)
{
    struct bare_list_head *p_task_head, *p_msg_head, *p_msg_temp;
    struct bare_task *tmp;

	int is_empty = 1;

	__bare_disable_isr();
	if(!bare_list_empty(&task_list))
	{
		bare_list_for_each(p_task_head, &task_list)
		{
			tmp = bare_list_entry(p_task_head, struct bare_task, node);
			
			if(!bare_list_empty(&(tmp->msg_list)))
			{
				is_empty = 0;
				break;
			}
		}
	}
	__bare_enable_isr();

	return is_empty;
}

void bare_task_polling(void)
{
    struct bare_list_head *p_task_head, *p_msg_head, *p_msg_temp;
    struct bare_task *tmp;

	if(!bare_list_empty(&task_list))
	{
		bare_list_for_each(p_task_head, &task_list)
		{
			tmp = bare_list_entry(p_task_head, struct bare_task, node);
			
			bare_list_for_each_safe(p_msg_head, p_msg_temp, &(tmp->msg_list))
			{
				struct bare_msg *msg = bare_list_entry(p_msg_head, struct bare_msg, node);

				int ret = tmp->func(msg);
				if(ret != BARE_TASK_HDL_CONSUMED)
				{
					break;
				}

				__bare_disable_isr();
				bare_list_del_init(p_msg_head);
				__bare_enable_isr();

				bare_msg_free(msg);
			}
		}
	}
}

void bare_task_init(void)
{
	INIT_BARE_LIST_HEAD(&(task_list));
}