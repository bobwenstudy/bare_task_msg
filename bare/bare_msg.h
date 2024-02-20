#ifndef _BARE_MSG_H_
#define _BARE_MSG_H_

/** Includes -----------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>

#include "bare_list.h"

/** Define -------------------------------------------------------------------*/
typedef struct bare_msg
{
    struct bare_list_head node;

    uint16_t id;
    uint16_t param_len;
    uint8_t param[]; ///< Parameter embedded struct. Must be word-aligned.
} bare_msg_t;

/** Exported functions -------------------------------------------------------*/
void *bare_msg_alloc_len(uint16_t id, uint16_t const param_len);
void *bare_msg_alloc(uint16_t id, uint16_t const param_len, void *param);
void bare_msg_free(struct bare_msg *msg);

#endif /*!< _BARE_MSG_H_ */
