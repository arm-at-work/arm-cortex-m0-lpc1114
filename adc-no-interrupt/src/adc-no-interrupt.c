/*
 ===============================================================================
 Name        : adc-no-interrupt.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-6-15
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código à memória flash
#include "uart.h"

/* protótipos */
void adcInit(void);

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	uartInit();
	adcInit();
	do { // loop infinito
		if (LPC_ADC->GDR & (1 << 31)) {
			if (!(LPC_ADC->STAT & (3 << 8))) {
				printf("%s%u\n", "Valor convertido: ",
						((LPC_ADC->GDR & (0x3FF << 6)) >> 6));
			} else {
				printf("%s\n", "Ocorreu overun - valor descartado.");
			}
			LPC_ADC->CR |= (1 << 24); // solicita-se nova conversão no canal
		}
	} while (1);
	return 1;
}

/* configura/inicializa adc */
void adcInit(void) {
	/* configurações gerais */
	LPC_SYSCON->PDRUNCFG &= ~(1 << 4); // limpando-se o bit, liga-se o periférico adc
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 13); // atribui-se clock ao adc
	LPC_IOCON->R_PIO1_2 = ((LPC_IOCON->R_PIO1_2
			& ~(0b11 << 0 | 0b11 << 3 | 1 << 7)) | (0b10 << 0)); // pino como ad3; desabilita pull-up e pull-down; seta admode
	/* configurações de operação */
	LPC_ADC->CR |= (1 << 3 | 1 << 8 | 1 << 24); // habilita canal 3; clkdiv = 1 (pclk_adc / 2); realiza conversão imediatamente
}
