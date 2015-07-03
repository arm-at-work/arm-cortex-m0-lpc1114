/*
 ===============================================================================
 Name        : blink-test.c
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
#include <NXP/crp.h>

void ledConfig(void);
void motorConfig(void);
void sysTickConfig(void);
void delayUs(uint32_t);
void NRFPinsTest(void);

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	ledConfig();
	motorConfig();
	sysTickConfig();
	NRFPinsTest();
	do {
		LPC_GPIO0->DATA ^= (1 << 3); // inverte estado lógico do pino
		LPC_GPIO3->DATA ^= (1 << 4);
		LPC_GPIO2->DATA ^= (1 << 5);
		delayUs(1000000);
	} while (1);
	return 0;
}

void ledConfig(void) {
	LPC_IOCON->PIO0_3 &= ~(0b11 << 0);
	LPC_GPIO0->DIR |= (1 << 3);
	LPC_GPIO0->DATA &= ~(1 << 3);
}

void motorConfig(void) {
	/* INPUT 1 */
	LPC_IOCON->PIO3_4 &= ~(0b11 << 0);
	LPC_GPIO3->DIR |= (1 << 4);
	LPC_GPIO3->DATA |= (1 << 4);
	/* INPUT 2 */
	LPC_IOCON->PIO2_5 &= ~(0b11 << 0);
	LPC_GPIO2->DIR |= (1 << 5);
	LPC_GPIO2->DATA &= ~(1 << 5);
}

void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; //	valor de reinício para o contador
	SysTick->CTRL |= (1 << 2); // fonte de decremento é c_clk
}

void delayUs(uint32_t us) {
	static uint32_t count;
	SysTick->CTRL |= (1 << 0);
	for (count = 0; count < us; count++) {
		do {
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0);
}

void NRFPinsTest(void) {
	/* p2.10 - CE (quando low permite recepção de dados, quando high apenas exibe) */
	LPC_IOCON->PIO2_10 &= ~(0b111 << 0); // pino como gpio
	LPC_GPIO2->DIR |= (1 << 10); // pino como saída
	LPC_GPIO2->DATA |= (1 << 10); // estado lógico baixo
	/* p2.2 - CSN (quando low espera comando, quando high espera dado) */
	LPC_IOCON->PIO2_2 &= ~(0b111 << 0); // pino como gpio
	LPC_GPIO2->DIR |= (1 << 2); // pino como saída
	LPC_GPIO2->DATA |= (1 << 2); // estado lógico baixo
	/* p0.10 - IRQ */
	LPC_IOCON->SWCLK_PIO0_10 = ((LPC_IOCON->SWCLK_PIO0_10 & ~(0b111 << 0)) | (0b001 << 0)); // pino como gpio
	LPC_GPIO0->DIR |= (1 << 10); // pino como entrada
	LPC_GPIO0->DATA |= (1 << 10); // estado lógico alto
	/* p2.11 - SCK */
	LPC_IOCON->PIO2_11 &= ~(0b111 << 0); // pino como sck
	LPC_GPIO2->DIR |= (1 << 11); // pino como entrada
	LPC_GPIO2->DATA |= (1 << 11); // estado lógico alto
	/* p0.9 - MOSI */
	LPC_IOCON->PIO0_9 &= ~(0b111 << 0); // pino como mosi
	LPC_GPIO0->DIR |= (1 << 9); // pino como entrada
	LPC_GPIO0->DATA |= (1 << 9); // estado lógico alto
	/* p0.8 - MISO */
	LPC_IOCON->PIO0_8 &= ~(0b111 << 0); // pino como miso
	LPC_GPIO0->DIR |= (1 << 8); // pino como entrada
	LPC_GPIO0->DATA |= (1 << 8); // estado lógico alto
}
