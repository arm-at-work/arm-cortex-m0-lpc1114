/*
 ===============================================================================
 Name        : delay-systick.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-4-17
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

void sysTickConfig(void); // configura systick
void delayUs(unsigned int); // implementa delay em microssegundos
void ledConfig(void); // configura pino de led

int main(void) {
	SystemInit(); // inicializa systema - c_clk = 100mhz, pclk = 25mhz, etc
	SystemCoreClockUpdate(); // atualiza valor de c_clk na variável SystemCoreClock
	sysTickConfig(); // configura systick
	ledConfig(); // configura pino de led
	/* Initialize GPIO (sets up clock) */
	do { // loop infinito
		delayUs(500000);
		LPC_GPIO1->DATA ^= (1 << 8 | 1 << 4);
//		LPC_GPIO2->DATA ^= (1 << 1 | 1 << 2 | 1 << 7 | 1 << 8 | 1 << 10); // estado lógico alto
		LPC_GPIO2->DATA ^= (1 << 2 | 1 << 7 | 1 << 8 | 1 << 10); // estado lógico alto
	} while (1);
	return 1;
}

/* configura systick */
void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; // valor de reinício do contador
	SysTick->CTRL |= (1 << 2);
}

/* implementa delay em microssegundos */
void delayUs(unsigned int us) {
	static unsigned int counter; // contador de interrupções
	SysTick->CTRL |= (1 << 0); // habilita contador
	for (counter = 0; counter < us; counter++) { // aguarda até que valor do contador se iguale ao valor recebido via parâmetro
		do { // aguarda até que flag de interrupção seja setada
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0); // para contador
}

/* configura pino de led */
void ledConfig(void) {
	/* p1.4 (3º) */
	LPC_IOCON->PIO1_4 &= ~(0b11 << 0); // pino como GPIO
	LPC_IOCON->PIO1_8 &= ~(0b11 << 0); // pino como GPIO
	LPC_GPIO1->DIR |= (1 << 8 | 1 << 4); // pinos como saída
	LPC_GPIO1->DATA |= (1 << 8 | 1 << 4); // estado lógico alto
	/* p2.1 (1º), p2.2 (6º), p2.7 (2º), p2.8 (5º), p2.10 (4º) */
	LPC_IOCON->PIO2_2 &= ~(0b11 << 0); // pino como GPIO
	LPC_IOCON->PIO2_7 &= ~(0b11 << 0); // pino como GPIO
	LPC_IOCON->PIO2_8 &= ~(0b11 << 0); // pino como GPIO
	LPC_IOCON->PIO2_10 &= ~(0b11 << 0); // pino como GPIO
	LPC_GPIO2->DIR |= (1 << 2 | 1 << 7 | 1 << 8 | 1 << 10); // pino como saída
	LPC_GPIO2->DATA |= (1 << 2 | 1 << 7 | 1 << 8 | 1 << 10); // estado lógico alto
}
