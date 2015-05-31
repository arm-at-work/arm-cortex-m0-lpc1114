/*
 ===============================================================================
 Name        : external-interrupt-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-5-27
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código da memória flash

/* protótipos */
void buttonsConfig(void); // configura pinos de botões
void buttonsInterruptConfig(void); // configura interrupção dos pinos
void ledConfig(void); // configura pino de led
void sysTickConfig(void); // configura systick
void delayUs(unsigned int); // implementa delay em microssegundos

/* isr de interrupções externas p0 */
void PIOINT0_IRQHandler(void) {
	if (LPC_GPIO0->MIS & (1 << 7)) { // verifica se interrupção ocorreu no pino p2.4
		LPC_GPIO2->DATA ^= (1 << 10); // estado lógico alto
	}
	delayUs(150000); // delay de 150ms
	LPC_GPIO0->IC |= (1 << 7); // limpa flag de interrupção do pino
}

/* isr de interrupções externas p1 */
void PIOINT1_IRQHandler(void) {
	if (LPC_GPIO1->MIS & (1 << 8)) { // verifica se interrupção ocorreu no pino p2.4
		LPC_GPIO2->DATA ^= (1 << 1); // estado lógico alto
	}
	if (LPC_GPIO1->MIS & (1 << 10)) { // verifica se interrupção ocorreu no pino p2.4
		LPC_GPIO2->DATA ^= (1 << 7); // estado lógico alto
	}
	delayUs(150000); // delay de 150ms
	LPC_GPIO1->IC |= (1 << 8 | 1 << 10); // limpa flag de interrupção do pino
}

/* isr de interrupções externas p2 */
void PIOINT2_IRQHandler(void) {
	if (LPC_GPIO2->MIS & (1 << 4)) { // verifica se interrupção ocorreu no pino p2.4
		LPC_GPIO1->DATA ^= (1 << 4); // estado lógico alto
	}
	if (LPC_GPIO2->MIS & (1 << 5)) { // verifica se interrupção ocorreu no pino p2.4
		LPC_GPIO2->DATA ^= (1 << 2); // estado lógico alto
	}
	delayUs(150000); // delay de 150ms
	LPC_GPIO2->IC |= (1 << 4 | 1 << 5); // limpa flag de interrupção do pino
}

/* isr de interrupções externas */
void PIOINT3_IRQHandler(void) {
	if (LPC_GPIO3->MIS & (1 << 4)) { // verifica se interrupção ocorreu no pino p2.4
		LPC_GPIO2->DATA ^= (1 << 8); // estado lógico alto
	}
	delayUs(150000); // delay de 150ms
	LPC_GPIO3->IC |= (1 << 4); // limpa flag de interrupção do pino
}

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 48mhz, etc.
	buttonsConfig(); // configura pinos de botões
	buttonsInterruptConfig(); // configura interrupções dos pinos
	ledConfig(); // configura pino de led
	sysTickConfig(); // configura systick
	do { // loop infinito
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

/* configura pinos de botões */
void buttonsInterruptConfig(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 6);
	/* configurações de interrupção p0 */
	LPC_GPIO0->IE |= (1 << 7); // habilita interrupção nos pinos
	LPC_GPIO0->IS &= ~(1 << 7); // interrupção nos pinos será por borda
	LPC_GPIO0->IBE &= ~(1 << 7); // interrupção será em borda específica
	LPC_GPIO0->IEV &= ~(1 << 7); // interrupção por borda de descida
	NVIC_EnableIRQ(EINT0_IRQn); // habilita interrupções no port0
	NVIC_SetPriority(EINT0_IRQn, 2); // atribui nível de prioridade 2 às interrupções no port0
	NVIC_ClearPendingIRQ(EINT0_IRQn); // retira qualquer pendência de interrupção no port0
	/* configurações de interrupção p0 */
	LPC_GPIO1->IE |= (1 << 8 | 1 << 10); // habilita interrupção nos pinos
	LPC_GPIO1->IS &= ~(1 << 8 | 1 << 10); // interrupção nos pinos será por borda
	LPC_GPIO1->IBE &= ~(1 << 8 | 1 << 10); // interrupção será em borda específica
	LPC_GPIO1->IEV &= ~(1 << 8 | 1 << 10); // interrupção por borda de descida
	NVIC_EnableIRQ(EINT1_IRQn); // habilita interrupções no port1
	NVIC_SetPriority(EINT1_IRQn, 2); // atribui nível de prioridade 2 às interrupções no port1
	NVIC_ClearPendingIRQ(EINT1_IRQn); // retira qualquer pendência de interrupção no port1
	/* configurações de interrupção p2 */
	LPC_GPIO2->IE |= (1 << 4 | 1 << 5); // habilita interrupção nos pinos
	LPC_GPIO2->IS &= ~(1 << 4 | 1 << 5); // interrupção nos pinos será por borda
	LPC_GPIO2->IBE &= ~(1 << 4 | 1 << 5); // interrupção será em borda específica
	LPC_GPIO2->IEV &= ~(1 << 4 | 1 << 5); // interrupção por borda de descida
	NVIC_EnableIRQ(EINT2_IRQn); // habilita interrupções no port2
	NVIC_SetPriority(EINT2_IRQn, 2); // atribui nível de prioridade 2 às interrupções no port2
	NVIC_ClearPendingIRQ(EINT2_IRQn); // retira qualquer pendência de interrupção no port2
	/* configurações de interrupção p3 */
	LPC_GPIO3->IE |= (1 << 4); // habilita interrupção nos pinos
	LPC_GPIO3->IS &= ~(1 << 4); // interrupção nos pinos será por borda
	LPC_GPIO3->IBE &= ~(1 << 4); // interrupção será em borda específica
	LPC_GPIO3->IEV &= ~(1 << 4); // interrupção por borda de descida
	NVIC_EnableIRQ(EINT3_IRQn); // habilita interrupções no port3
	NVIC_SetPriority(EINT3_IRQn, 2); // atribui nível de prioridade 2 às interrupções no port3
	NVIC_ClearPendingIRQ(EINT3_IRQn); // retira qualquer pendência de interrupção no port3
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
	unsigned int count; // contador de interrupções
	SysTick->CTRL |= (1 << 0); // inicializa contador
	for (count = 0; count < us; count++) {
		do {
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0); // para contador
}
