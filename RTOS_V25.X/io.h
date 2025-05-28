#ifndef IO_H
#define	IO_H

#include <stdint.h> // Adiciona esta linha para os tipos uint8_t e uint16_t
#include <xc.h>
#include "config.h"

void pwm_init(uint8_t channel);
void pwm_set_duty(uint8_t channel, uint16_t duty);

void adc_init();
uint16_t adc_read(uint8_t channel);

void ext_int_init(uint8_t int_pin, uint8_t edge);

#endif	/* IO_H */

