/*
 ===============================================================================
 Name        : toRes-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-4-4
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

/* protótipos */
void ledsConfig(void);
void sysTickConfig(void);
void delayUs(unsigned int);

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	ledsConfig();
	sysTickConfig();
	do {
		delayUs(500000);
		LPC_GPIO2->DATA ^= (1 << 10 | 1 << 9 | 1 << 5); // estado lógico baixo
	} while (1);
	return 1;
}

/* configura pinos de leds */
void ledsConfig(void) {
	/* p2.10, p2.9 e p2.5 */
	LPC_IOCON->PIO2_10 &= ~(0b11 << 0); // pino como gpio
	LPC_IOCON->PIO2_9 &= ~(0b11 << 0); // pino como gpio
	LPC_IOCON->PIO2_5 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO2->DIR |= (1 << 10 | 1 << 9 | 1 << 5); // pinos como saída
	LPC_GPIO2->DATA &= ~(1 << 10 | 1 << 9 | 1 << 5); // estado lógico baixo
}

/* configura systick */
void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1;
	SysTick->CTRL |= (1 << 2);
}

/* implementa delay em microssegundos */
void delayUs(unsigned int us) {
	unsigned int count;
	SysTick->CTRL |= (1 << 0);
	for (count = 0; count < us; count++) {
		do {
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0);
}
