

#include <stdint.h>
#include "system.h"
#include <stdio.h>

#define pio_p32 ((volatile uint32_t *)PIO_BASE)
#define digits_p32 ((volatile uint32_t *)DIGITS_BASE)
#define timer_p32 ((volatile uint32_t *)TIMER_BASE)

#define CNT 0
#define MODULO 1
#define START 2

#define SEGM0 0
#define SEGM1 1
#define SEGM2 2
#define SEGM3 3
#define SEGM_PACK 4

typedef struct
{

	uint32_t led_unpacked[8];
	uint32_t sw_unpacked[8];

	unsigned led_packed : 8;
	unsigned : 24;

	unsigned sw_packed : 8;
	unsigned : 24;

} bf_pio;

#define pio (*(volatile bf_pio *)PIO_BASE)

#define LEFT 0
#define RIGHT 1


void timer_isr()
{
	static uint8_t cnt = 1;
	static uint8_t dir = LEFT;
	static uint8_t pause = 0;

	if ((pio.sw_packed & 0x30) == 0x00)
	{
		timer_p32[MODULO] = 1200000;
		pause++;
		if((pause & 0x0f) == 0x0A){
			pause += 6;
		}
		if((pause & 0xf0) == 0xA0){
			pause += 96;
		}
		digits_p32[SEGM0] = pause;
		digits_p32[SEGM1] = pause >> 4;
	}
	else
	{
	timer_p32[MODULO] = 12000000 >> (((~pio.sw_packed) & 0x30) >> 4);
		if (dir == LEFT)
		{
			if (cnt < (pio.sw_packed & 0x07))
			{
				pio.led_unpacked[cnt] = 1;
				cnt++;
			}
			else
			{
				pio.led_unpacked[cnt] = 1;
				cnt++;
				dir = RIGHT;
			}
		}
		if (dir == RIGHT)
		{
			if (cnt > 1)
			{
				pio.led_unpacked[cnt] = 0;
				cnt--;
			}
			else
			{
				pio.led_unpacked[cnt] = 0;
				cnt--;
				dir = LEFT;
			}
		}
	}
}

int main()
{

	alt_ic_isr_register(
		TIMER_IRQ_INTERRUPT_CONTROLLER_ID,
		TIMER_IRQ,
		timer_isr,
		NULL,
		NULL);

	timer_p32[MODULO] = 12000000;
	timer_p32[START] = 0;
	pio.led_packed = 00000001;
	digits_p32[SEGM_PACK] = 0;

	while (1)
	{
	}

	return 0;
}
