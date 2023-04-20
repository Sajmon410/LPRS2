

#include <stdint.h>
#include "system.h"
#include <stdio.h>
#include "sys/alt_irq.h"
#include <unistd.h>

#define pio_p32 ((volatile uint32_t*)PIO_BASE)
#define digits_p32 ((volatile uint32_t*)DIGITS_BASE)
#define timer_p32 ((volatile uint32_t*)(TIMER_BASE+0x20))

#define PIO_LEDS 16
#define PIO_SW 17

#define TIMER_CNT 0
#define TIMER_MAGIC 1
#define TIMER_STATUS 2
#define TIMER_MODULO 3

typedef struct{
	unsigned right1 : 4;
	unsigned right10 : 4;
	unsigned left1 : 4;
	unsigned left10 : 4;
}bf_digits;

typedef struct{
	unsigned led_packed : 8;
	unsigned : 24;
	unsigned speed : 2;
	unsigned sw_packed : 5;
	unsigned reset : 1;
	unsigned : 24;
}bf_pio;

typedef enum{
	TO_RIGHT,
	TO_LEFT
}state_t;

state_t state = TO_RIGHT;

#define digits (*((volatile bf_digits*)(DIGITS_BASE+0x10)))
#define pio (*((volatile bf_pio*)(PIO_BASE+0x40)))

static uint8_t leds = 0b10000000;
static uint8_t left_cnt = 0;
static uint8_t right_cnt = 0;

static void timer_isr ( void * context ) {
	//Transfer function
	switch(state){
		case TO_RIGHT:
			if(leds&1){
				leds <<= 1;
				state = TO_LEFT;
				right_cnt++;
			}else{
				leds >>= 1;
			}
			break;
		case TO_LEFT:
			if(leds>>7 & 1){
				leds >>= 1;
				state = TO_RIGHT;
				left_cnt++;
			}else{
				leds <<= 1;
			}
			break;
	}
	
	
	
	//Output
	pio.led_packed = leds;
	printf("left: 0x%08x\n",left_cnt);
	printf("right: 0x%08x\n",right_cnt);
	
	if((left_cnt&0xf) == 0xa){
		left_cnt += 6;
	}
	
	if((right_cnt&0xf) == 0xa){
		right_cnt += 6;
	}
	
	digits.left1 = left_cnt&0xf;
	digits.left10 = left_cnt>>4 & 0xf;
	digits.right1 = right_cnt&0xf;
	digits.right10 = right_cnt>>4 & 0xf;
	
}

void init(){
	timer_p32[TIMER_STATUS] &= 1;
	pio.led_packed = 0b10000000;
	digits.left1 = 0;
	digits.left10 = 0;
	digits.right1 = 0;
	digits.right10 = 0;
	leds = 0b10000000;
	left_cnt = 0;
	right_cnt = 0;
}

int main() {

	init();

#if 0
	printf("0x%08x",timer_p32[TIMER_MAGIC]);
	digits_p32[0] = 0;
	digits_p32[1] = 1;
	digits_p32[2] = 2;
	digits_p32[3] = 3;
	digits.unpacked_segm[0] = 0;
	digits.unpacked_segm[1] = 1;
	digits.unpacked_segm[2] = 2;
	digits.unpacked_segm[3] = 3;
#endif

	alt_ic_isr_register (
		TIMER_IRQ_INTERRUPT_CONTROLLER_ID , // alt_u32 ic_id
		TIMER_IRQ , // alt_u32 irq
		timer_isr , // alt_isr_func isr
		NULL , // void * isr_context
		NULL // void * flags
	);
	
	timer_p32[TIMER_MODULO] = 12000000;
	timer_p32[TIMER_STATUS] = 0;
	

	while(1){
		///////////////////
		// Read inputs.

		///////////////////
		// Calculate state.
		
		if(pio.reset){
			init();
		}else{
			timer_p32[TIMER_STATUS] &= 0b1110;
		}

		///////////////////
		// Write outputs.
		
		timer_p32[TIMER_MODULO] = (12000000<<1)>>pio.speed;
		
		///////////////////
		// Other things to do.
		
	}

	return 0;
}
