/**
 * @file           inkvs.h
 * @author         Injoinic
 * @version        v0.0.1
 * @date           2022-12-29
 * @copyright      Copyright(c) 2022 Injoinic Technology
 * @brief          Injoinic Key Value Storage(INKVS)
 *
 */
#ifndef _BARE_TASK_H_
#define _BARE_TASK_H_


/** Includes -----------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>

#include "bare_msg.h"


/** Define -------------------------------------------------------------------*/
enum bare_task_hdl_result
{
    BARE_TASK_HDL_CONSUMED = 0, ///< consumed, msg and ext are freed by the kernel
    BARE_TASK_HDL_SAVED,        ///< not consumed, will be pushed in the saved queue
};

typedef int (*bare_task_func_t)(struct bare_msg *msg);

typedef struct bare_task
{
    struct bare_list_head node;

    struct bare_list_head msg_list;

    bare_task_func_t func;
} bare_task_t;


/** Exported functions -------------------------------------------------------*/

void bare_task_send_msg(struct bare_task* task, struct bare_msg* msg);

void bare_task_delete(struct bare_task* task);

void bare_task_create(struct bare_task* task);

void bare_task_polling(void);

int bare_task_check_empty(void);

void bare_task_init(void);

#endif /*!< _BARE_TASK_H_ */
