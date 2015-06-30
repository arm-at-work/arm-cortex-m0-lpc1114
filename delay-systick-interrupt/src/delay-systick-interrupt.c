/*
 ===============================================================================
 Name        : delay-systick-interrupt.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-6-27
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código da memória flash

unsigned int sysCounter;

/* protótipos */
void ledConfig(void); // configura pino de led
void sysTickConfig(void); // configura systick
void delayUs(uint32_t); // implementa delay em microssegundos

/* isr do systick */
void SysTick_Handler(void) {
	sysCounter++;
}

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 48mhz, etc.
	SystemCoreClockUpdate(); // atualiza valor de c_clk, para variável SystemCoreClock
	ledConfig(); // configura pino de led
	sysTickConfig(); // configura systick
	do { // loop infinito
		delayUs(500000);
		LPC_GPIO2->DATA ^= (1 << 1); // inverte estado lógico do pino
	} while (1);

	return 1;
}

/* configura pino de led */
void ledConfig(void) {
	/* pino p2.1 */
	LPC_IOCON->PIO2_1 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO2->DIR |= (1 << 1); // pino como saída
	LPC_GPIO2->DATA &= ~(1 << 1); // estado lógico baixo
}

/* configura systick */
void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; // valor de reinício para o contador
	NVIC_SetPriority(SysTick_IRQn, 0); // prioridade alta para interrupções no systick
	NVIC_ClearPendingIRQ(SysTick_IRQn); // retira pendência de interrupção
	SysTick->CTRL |= (1 << 1 | 1 << 2); // fonte de decremento é c_clk
}

/* implementa delay em microssegundos */
void delayUs(uint32_t us) {
	sysCounter = 0; // zera contador
	SysTick->CTRL |= (1 << 0); // habilita contador
	do { // aguarda até que flag de interrupção esteja setada
	} while (sysCounter < us);
	SysTick->CTRL &= ~(1 << 0); // para contador
}
