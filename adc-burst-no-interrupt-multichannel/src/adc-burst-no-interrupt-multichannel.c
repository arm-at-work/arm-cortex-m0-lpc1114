/*
 ===============================================================================
 Name        : adc-no-interrupt.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-7-1
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código à memória flash
#include "uart.h"

/* variáveis globais */
static uint8_t adChannel;

/* protótipos */
void adcInit(void);

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	uartInit();
	adcInit();
	do { // loop infinito
		if (LPC_ADC->GDR & (1 << 31)) {
			adChannel = ((LPC_ADC->GDR & (0b111 << 24)) >> 24); // pega canal em que ocorreu conversão
			if (!(LPC_ADC->STAT & (adChannel << 8))) { // verifica se ocorreu overrun no canal
				printf("%s%u%s%u\n", "Valor convertido do canal ", adChannel,
						": ", ((LPC_ADC->GDR & (0x3FF << 6)) >> 6));
			} else {
				printf("%s\n", "Ocorreu overun - valor descartado.");
			}
		}
	} while (1);
	return 1;
}

/* configura/inicializa adc */
void adcInit(void) {
	/* configurações gerais */
	LPC_SYSCON->PDRUNCFG &= ~(1 << 4); // limpando-se o bit, liga-se o periférico adc
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 13); // atribui-se clock ao adc
	/* canal 0 */
	LPC_IOCON->R_PIO0_11 = ((LPC_IOCON->R_PIO0_11
			& ~(0b11 << 0 | 0b11 << 3 | 1 << 7)) | (0b10 << 0)); // pino como ad3; desabilita pull-up e pull-down; seta admode
	/* canal 1 */
	LPC_IOCON->R_PIO1_0 = ((LPC_IOCON->R_PIO1_0
			& ~(0b11 << 0 | 0b11 << 3 | 1 << 7)) | (0b10 << 0)); // pino como ad3; desabilita pull-up e pull-down; seta admode
	/* canal 2 */
	LPC_IOCON->R_PIO1_1 = ((LPC_IOCON->R_PIO1_1
			& ~(0b11 << 0 | 0b11 << 3 | 1 << 7)) | (0b10 << 0)); // pino como ad3; desabilita pull-up e pull-down; seta admode
	/* configurações de operação */
	LPC_ADC->CR |= (1 << 0 | 1 << 1 | 1 << 2 | 1 << 8 | 1 << 16); // habilita canal; clkdiv = 1 (pclk_adc / 2); modo burst
}
