/*
 ===============================================================================
 Name        : pwm-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-7-3
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código da memória flash

/* protótipos */
void pwmInit(void); // configura/inicializa pwm
void ledConfig(void); // configura pino de led

/* isr do tmr32b0 */
void TIMER32_0_IRQHandler(void) {
	if (LPC_TMR32B0->IR & (1 << 0)) { // verifica se flag do canal 0 está setada
		LPC_GPIO2->DATA |= (1 << 1); // led aceso
		LPC_TMR32B0->IR |= (1 << 0); // limpa flag de interrupção do canal 0
	}
	if (LPC_TMR32B0->IR & (1 << 1)) {
		LPC_GPIO2->DATA &= ~(1 << 1); // led apagado
		LPC_TMR32B0->IR |= (1 << 1); // limpa flag de interrupção do canal 1
	}
}

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 48mhz, etc
	SystemCoreClockUpdate(); // atualiza valor de clock para a variável 'SystemCoreClock'
	ledConfig(); // configura pino de led
	pwmInit(); // configura/inicializa pwm
	do { // loop infinito
	} while (1);
	return 1;
}

/* configura/inicializa pwm */
void pwmInit(void) {
	/* configurações gerais */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 9); // habilita clock para ct32b0
	/* configurações de operação */
	LPC_TMR32B0->CTCR &= ~(0b11 << 0); // fonte de incremento é pclk_ct32b0
	LPC_TMR32B0->PR = 1; // valor do prescaler
	uint32_t clkPR = (SystemCoreClock / (LPC_TMR32B0->PR + 1));
	LPC_TMR32B0->MR0 = ((clkPR / (1000 / 20)) - 1); // valor do canal 0 de comparação
	LPC_TMR32B0->MR1 = ((clkPR / (1000 / 18)) - 1); // valor do canal 1 de comparação
	LPC_TMR32B0->MCR |= (1 << 1); // reinicia contadores, após coincidência de comparação no canal 0 de comparação
	LPC_TMR32B0->PWMC |= (1 << 0);
	/* configurações de interrupção */
	//NVIC->IPR[18] |= ()
	LPC_TMR32B0->MCR |= (1 << 0 | 1 << 3); // habilita interrupção por comparação nos canais 0 e 1
	NVIC_SetPriority(TIMER_32_0_IRQn, 0); // atribui nível de prioridade de interrupção ao TMR32B0
	NVIC->ISER[0] |= (1 << 18); // habilita interrupção do TMR32B0
	NVIC->ICPR[0] |= (1 << 18); // retira qualquer pendência de interrupção para ct32b0
	/* inicializa tmr32b0 */
	LPC_TMR32B0->TCR |= (1 << 0); // inicializa tmr32b0
}

/* configura pino de led */
void ledConfig(void) {
	LPC_IOCON->PIO2_1 &= ~(0b11 << 0); // pino como GPIO
	LPC_GPIO2->DIR |= (1 << 1); // pino como saída
	LPC_GPIO2->DATA &= ~(1 << 1); // nível lógico alto
}
