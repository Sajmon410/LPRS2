#include <stdint.h>
#include "system.h"
#include <stdio.h>

// Timer defs
#define TIMER_CNT 0
#define TIMER_MODULO 1
#define TIMER_CTRL_STATUS 2
#define TIMER_MAGIC 3
#define TIMER_RESET_FLAG 0
#define TIMER_PAUSE_FLAG 1
#define TIMER_WRAP_FLAG 2
#define TIMER_WRAPPED_FLAG 3
#define TIMER_RESET (TIMER_RESET_FLAG + 4)
#define TIMER_PAUSE (TIMER_PAUSE_FLAG + 4)
#define TIMER_WRAP (TIMER_WRAP_FLAG + 4)
#define TIMER_WRAPPED (TIMER_WRAPPED_FLAG + 4)

// Display defs
#define SEGM_0 0
#define SEGM_1 1
#define SEGM_2 2
#define SEGM_3 3
#define SEGM_PACK 4

// Pointer defs
#define pio_p32 ((volatile uint32_t *)PIO_BASE)
#define timer_p32 ((volatile uint32_t *)TIMER_BASE)
#define digits_p32 ((volatile uint32_t *)DIGITS_BASE)

// Bitfield
typedef struct
{

	uint32_t led_unpacked[8];
	uint32_t sw_unpacked[8];

	unsigned led_packed : 8;
	unsigned sw_packed : 8;
	unsigned : 16;

	uint32_t babadeda[15];

} bf_pio;

// Bitfield pointer
#define pio (*((volatile bf_pio *)PIO_BASE))

// Function that is called when IRQ registered
static void timer_isr()
{
	// Flag to change direction, maybe can go without it just use left and right but fuck it
	static uint8_t flag = 0;
	static uint8_t left = 0;
	static uint8_t right = 0;

	// Check for SW1 and SW0 and update modulo accordingly
	if (pio.sw_unpacked[0] == 0 && pio.sw_unpacked[1] == 0)
	{
		timer_p32[TIMER_MODULO] = 24000000;
	}
	else if (pio.sw_unpacked[0] == 0 && pio.sw_unpacked[1] == 1)
	{
		timer_p32[TIMER_MODULO] = 12000000;
	}
	else if (pio.sw_unpacked[0] == 1 && pio.sw_unpacked[1] == 0)
	{
		timer_p32[TIMER_MODULO] = 6000000;
	}
	else if (pio.sw_unpacked[0] == 1 && pio.sw_unpacked[1] == 1)
	{
		timer_p32[TIMER_MODULO] = 3000000;
	}

	// If flag is 1 we led register to the left, otherwise we shift it to the right
	if (flag == 1)
	{
		pio.led_packed <<= 1;
	}
	else if (flag == 0)
	{
		pio.led_packed >>= 1;
	}

	// If we hit the least significant bit, change the flag to start shifting in the other direction
	// Also increment right variable, check when the 4 least significant bits equal A, and add 6, to avoid displaying it as a hexadec value
	if (pio.led_packed == 0x01)
	{
		flag = 1;
		right++;
		if ((right & 0x0f) == 0x0A)
		{
			right += 6;
		}
	}
	// Same thing
	else if (pio.led_packed == 0x80)
	{
		flag = 0;
		left++;
		if ((left & 0x0f) == 0x0A)
		{
			left += 6;
		}
	}

	// Check for reset switch and reset all values if pressed
	if (pio.sw_unpacked[7] == 1)
	{

		pio.led_packed = 0x80;
		left = 0;
		right = 0;
		flag = 0;
	}

	// Write right and left values to display
	digits_p32[SEGM_0] = left & 0x0f;
	digits_p32[SEGM_1] = left >> 4;

	digits_p32[SEGM_2] = right & 0x0f;
	digits_p32[SEGM_3] = right >> 4;
}

int main()
{
	// Register timer to use timer_isr function at every wrap
	alt_ic_isr_register(
		TIMER_IRQ_INTERRUPT_CONTROLLER_ID,
		TIMER_IRQ,
		timer_isr,
		NULL,
		NULL);

	// Init values, start timer, everything else can be handled inside timer_isr
	pio.led_packed = 0x80;
	digits_p32[SEGM_PACK] = 0x00;
	timer_p32[TIMER_MODULO] = 24000000;
	timer_p32[TIMER_CTRL_STATUS] = 0;

	while (1)
	{
	}

	return 0;
}
