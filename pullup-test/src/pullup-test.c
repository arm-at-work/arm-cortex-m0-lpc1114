/*
 ===============================================================================
 Name        : pullup-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-7-26
 Created at  : 2015-7-26 2015-6-272015-5-102015-05-12
 Description : Neste exemplo é demonstrada utilização dos resistores pull-up, para
 alternar o estado lógico do led. O time-deboucing é implementado através de função
 de delay gerado pelo SysTick (System Tick Timer).
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código à memória flash

/* protótipos */
void buttonsConfig(void); // configura pinos de botões
void ledConfig(void); // configura pino de led
void sysTickConfig(void); // configura systick
void delayUs(unsigned int); // implementa delay em microssegundos

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 48mhz, etc.
	SystemCoreClockUpdate(); // atualiza valor de c_clk, para variável SystemCoreClock
	sysTickConfig(); // configura systick
	ledConfig(); // configura pino de led
	buttonsConfig(); // configura pinos de botões
	do { // loop infinito
		if (!(LPC_GPIO1->DATA & (1 << 8))) {
			LPC_GPIO2->DATA ^= (1 << 1); // estado lógico alto
			delayUs(250000); // delay de 250ms
		}
		if (!(LPC_GPIO1->DATA & (1 << 10))) {
			LPC_GPIO2->DATA ^= (1 << 7); // estado lógico alto
			delayUs(250000); // delay de 250ms
		}
		if (!(LPC_GPIO2->DATA & (1 << 4))) {
			LPC_GPIO1->DATA ^= (1 << 4); // estado lógico alto
			delayUs(250000); // delay de 250ms
		}
		if (!(LPC_GPIO0->DATA & (1 << 7))) {
			LPC_GPIO2->DATA ^= (1 << 10); // estado lógico alto
			delayUs(250000); // delay de 250ms
		}
		if (!(LPC_GPIO3->DATA & (1 << 4))) {
			LPC_GPIO2->DATA ^= (1 << 8); // estado lógico alto
			delayUs(250000); // delay de 250ms
		}
		if (!(LPC_GPIO2->DATA & (1 << 5))) {
			LPC_GPIO2->DATA ^= (1 << 2); // estado lógico alto
			delayUs(250000); // delay de 250ms
		}
	} while (1);
	return 1;
}

/* configura pinos de botões */
void buttonsConfig(void) {
	/* p0.7 - (D 2º) */
	LPC_IOCON->PIO0_7 = ((LPC_IOCON->PIO0_7 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_GPIO0->DIR &= ~(1 << 7); // pino como entrada
	LPC_GPIO0->DATA |= (1 << 7); // estado lógico alto
	/* p1.8 (botão esquerdo) - p1.10 (botão encoder)*/
	LPC_IOCON->PIO1_10 = ((LPC_IOCON->PIO1_10 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_IOCON->PIO1_8 = ((LPC_IOCON->PIO1_8 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_GPIO1->DIR &= ~(1 << 8 | 1 << 10); // pino como entrada
	LPC_GPIO1->DATA |= (1 << 8 | 1 << 10); // estado lógico alto
	/* p2.4 (D 1º), p2.5 (D 4º) */
	LPC_IOCON->PIO2_4 = ((LPC_IOCON->PIO2_4 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_IOCON->PIO2_5 = ((LPC_IOCON->PIO2_5 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_GPIO2->DIR &= ~(1 << 4 | 1 << 5); // pinos como entrada
	LPC_GPIO2->DATA |= (1 << 4 | 1 << 5); // estado lógico alto
	/* p3.4 (D 3º) */
	LPC_IOCON->PIO3_4 = ((LPC_IOCON->PIO3_4 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_GPIO3->DIR &= ~(1 << 4); // pino como entrada
	LPC_GPIO3->DATA |= (1 << 4); // estado lógico alto
}

/* configura pino de led */
void ledConfig(void) {
	/* p1.4 (3º) */
	LPC_IOCON->PIO1_4 &= ~(0b11 << 0); // pino como GPIO
	LPC_GPIO1->DIR |= (1 << 4); // pinos como saída
	LPC_GPIO1->DATA &= ~(1 << 4); // estado lógico alto
	/* p2.1 (1º), p2.2 (6º), p2.7 (2º), p2.8 (5º), p2.10 (4º) */
	LPC_IOCON->PIO2_1 &= ~(0b11 << 0); // pino como GPIO
	LPC_IOCON->PIO2_2 &= ~(0b11 << 0); // pino como GPIO
	LPC_IOCON->PIO2_7 &= ~(0b11 << 0); // pino como GPIO
	LPC_IOCON->PIO2_8 &= ~(0b11 << 0); // pino como GPIO
	LPC_IOCON->PIO2_10 &= ~(0b11 << 0); // pino como GPIO
	LPC_GPIO2->DIR |= (1 << 1 | 1 << 2 | 1 << 7 | 1 << 8 | 1 << 10); // pino como saída
	LPC_GPIO2->DATA &= ~(1 << 1 | 1 << 2 | 1 << 7 | 1 << 8 | 1 << 10); // estado lógico alto
}

/* configura systick */
void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; // valor de reinício para o contador
	SysTick->CTRL |= (1 << 2); // fonte de decremento é c_clk
}

/* implementa delay em microssegundos */
void delayUs(unsigned int us) {
	unsigned int counter; // contador de interrupções
	SysTick->CTRL |= (1 << 0); // habilita contador
	for (counter = 0; counter < us; counter++) { // aguarda até que valor do contador se iguale ao valor recebido via parâmetro
		do { // aguarda até que flag de interrupção esteja setada
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0); // para contador
}
