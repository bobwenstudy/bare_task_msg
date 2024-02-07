#ifndef _BARE_COMMON_H_
#define _BARE_COMMON_H_


/** Includes -----------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>


/** Define -------------------------------------------------------------------*/


/** Exported functions -------------------------------------------------------*/

typedef int bt_base_t;
extern bt_base_t bare_hw_interrupt_disable(void);
extern void bare_hw_interrupt_enable(bt_base_t level);

#define __bare_disable_isr() bt_base_t _bare_isr_level = bare_hw_interrupt_disable()
#define __bare_enable_isr() bare_hw_interrupt_enable(_bare_isr_level)

#endif /*!< _BARE_COMMON_H_ */
