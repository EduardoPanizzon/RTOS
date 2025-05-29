#ifndef IO_H
#define	IO_H

#include <stdint.h> 
#include <xc.h>
#include "config.h"

void adc_init();
uint16_t adc_read();

void ext_int_init(uint8_t int_pin, uint8_t edge);

#endif	/* IO_H */

