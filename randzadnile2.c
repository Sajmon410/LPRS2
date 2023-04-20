#include <stdint.h>
#include "system.h"
#include <stdio.h>

#define pio_p32 ((volatile uint32_t *)PIO_BASE)
#define timer_p32 ((volatile uint32_t *)(TIMER_BASE + 0x60))
#define digits_p32 ((volatile uint32_t *)(DIGITS_BASE + 0x60))

#define SEGM0 0
#define SEGM3 1
#define SEGM2 2
#define SEGM1 3
#define SEGM_PACK 4

#define MAGIC 0
#define START 1
#define MODULO 2

typedef struct{

	uint32_t led_unpacked[8];
	uint32_t sw_unpacked[8];

	unsigned led_packed : 8;
	unsigned : 24;

	unsigned sw_packed : 8;
	unsigned : 24;

}bf_pio;

#define pio (*((volatile bf_pio *)PIO_BASE))
uint8_t odd = 0;
uint8_t even = 0;

void timer_isr(){
	static uint8_t cnt = 0;
	static uint8_t parity = 0;

	parity = pio.sw_unpacked[0] + pio.sw_unpacked[1] + pio.sw_unpacked[2] + pio.sw_unpacked[3];

		if(parity%2 == 0){
		pio.led_packed = pio.sw_packed & 0x0f;
		
		even += 1;

		if((even & 0x0f) == 0x0A){
			even += 6;
		}
		if((even & 0xf0) == 0xA0){
			even += 96;
		}

		digits_p32[SEGM2] = even;
		digits_p32[SEGM3] = even >> 4;

	}else
	{
		pio.led_packed = 0b00010000 + (pio.sw_packed & 0x0f);

		odd += 1;

		if((odd & 0x0f) == 0x0A){
			odd += 6;
		}
		if((odd & 0xf0) == 0xA0){
			odd += 96;
		}

		digits_p32[SEGM0] = odd;
		digits_p32[SEGM1] = odd >> 4;
	}
	}


int main() {

	alt_ic_isr_register(
		TIMER_IRQ_INTERRUPT_CONTROLLER_ID,
		TIMER_IRQ,
		timer_isr,
		NULL,
		NULL
	);

	timer_p32[MODULO] = 12000000*5;
	timer_p32[START] = 0;
	digits_p32[SEGM_PACK] = 0;
	
	while(1){

		if(pio.sw_unpacked[7] == 1){
		odd = 0;
		even = 0;

		digits_p32[SEGM2] = even;
		digits_p32[SEGM3] = even >> 4;

		digits_p32[SEGM0] = odd;
		digits_p32[SEGM1] = odd >> 4;

	}

	}

	return 0;
}
