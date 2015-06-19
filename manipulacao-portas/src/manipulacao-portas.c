/*
 ===============================================================================
 Name        : manipulacao-portas.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Copyright   : MIT
 Created at  : 2015-6-15
 Description : Neste exemplo demonstro como manipular pinos GPIO nos PORTs, bem como
 demonstro uso dos principais operadores de uso corriqueiro na programação em C, para
 microcontroladores ARM Cortex-M0. No exemplo implemento uma forma provisória de delay,
 para demonstrar o funcionamento do operador de inversão. Esse delay não é um bom padrão
 e em exemplos seguintes estarei demonstrando a implementação de delays com o SysTick
 (System Tick Timer) e com o RIT (Repetitive Interrupt).
 ===============================================================================
 */
#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

/* protótipos */
void ledConfig(void); // configura pino de led

int main(void) {
	SystemInit(); // inicializa systema - c_clk = 100mhz, pclk = 25mhz, etc
	ledConfig(); // configura pino de led
	unsigned int i = 0;
	do { // loop infinito
		for (i = 0; i < 0xFFFFF; ++i) // delay provisório para teste do operador de inversão
			;
		LPC_GPIO0->DATA ^= (1 << 7); // inverte estado lógico do pino
	} while (1);
	return 1;
}

/* configura pino de led */
void ledConfig(void) {
	LPC_IOCON->PIO0_7 &= ~(0b11 << 0); // pino como GPIO
	LPC_GPIO0->DIR |= (1 << 7); // pino como saída
	LPC_GPIO0->DATA |= (1 << 7); // nível lógico alto
}
