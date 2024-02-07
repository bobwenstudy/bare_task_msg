#ifndef _BARE_HEAP_H_
#define _BARE_HEAP_H_


/** Includes -----------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>


/** Define -------------------------------------------------------------------*/


/** Exported functions -------------------------------------------------------*/
void *bare_heap_malloc (uint32_t xWantedSize);
void bare_heap_free (void *pv);
void bare_heap_init(uint32_t *heap, uint32_t size);

uint32_t bare_heap_get_remain_size(void);
int bare_heap_check_empty(uint32_t size);

#endif /*!< _BARE_HEAP_H_ */
