/*
 ===============================================================================
 Name        : adc-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-4-24
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código da memória flash

/* protótipos */
void adcInit(void); // configura/inicializa adc
void uartInit(void); // configura/inicializa uart
void uartSend(unsigned char data); // envia dado para buffer tx

/* isr do adc */
void ADC_IRQHandler(void) {
	uint8_t channel = ((LPC_ADC->GDR & (0b111 << 24)) >> 24); // captura canal de conversão completada
	if (!(LPC_ADC->STAT & (channel << 8))) { // verifica se não ocorreu overrun no canal
		printf("%s%u%s%u\n", "Conversão completada no canal ", channel,
				". Valor convertido: ", ((LPC_ADC->GDR & (0x3FF << 6)) >> 6));
	}
}

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 48mhz, etc.
	SystemCoreClockUpdate(); // atualiza valor de c_clk na variável SystemCoreClock
	uartInit(); // configura/inicializa uart
	adcInit(); // configura/inicializa adc
	do { // loop infinito
	} while (1);
	return 1;
}

/* configura/inicializa uart */
void uartInit(void) {
	/* configurações gerais */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12); // habilita clock para uart
	LPC_SYSCON->UARTCLKDIV = 2; // pclk_uart = c_clk / 2
	LPC_IOCON->PIO1_6 |= (0b01 << 0); // pino como rx da uart
	LPC_IOCON->PIO1_7 |= (0b01 << 0); // pino como tx da uart
	/* configurações de operação */
	LPC_UART->LCR |= (1 << 7); // seta dlab - habilita acesso aos divisores principais
	LPC_UART->DLL = ((SystemCoreClock / LPC_SYSCON->UARTCLKDIV) / (16 * 9600)); // lsb do divisor principal do brg
	LPC_UART->DLM = 0; // msb do divisor principal do brg
	LPC_UART->FDR |= (1 << 4); // divisor fracional do brg - cálculo: ((pclk_uart / (16 * 9600 * dll)) - 1);
	LPC_UART->LCR &= ~(1 << 7); // limpa dlab - bloqueia acesso aos divisores principais do brg
	/* configurações do pacote */
	LPC_UART->LCR |= (0b11 << 0); // dado trafegado será de 8 bits
	LPC_UART->LCR &= ~(1 << 2 | 1 << 3); // 1 bit de parada; sem bit de paridade
	LPC_UART->FCR |= (1 << 0 | 1 << 1 | 1 << 2); // ativa e reinicia fifos
	LPC_UART->THR = 0; // limpa buffer tx
	LPC_UART->RBR; // limpa buffer rx
	LPC_UART->FCR &= ~(0b11 << 6); // dma habilitado - nível de disparo de 1 dado
	LPC_UART->IER |= (1 << 0); // habilita interrupção por rbr
	/* configurações de interrupção */
	NVIC_SetPriority(UART_IRQn, 1); // nível 1 de prioridade de interrupção
	NVIC->ISER[0] |= (1 << 21); // habilita interrupções na uart
	NVIC->ICPR[0] |= (1 << 21); // retira qualquer pendência de interrupção da uart
}

/* envia dado para buffer tx */
void uartSend(unsigned char data) {
	do {		// aguarda até que buffer tx possa receber novos dados
	} while (!(LPC_UART->LSR & (1 << 5)));
	LPC_UART->THR = data; // coloca novo dado no buffer tx
}

/* print's imprimem na uart */
int __sys_write(int iFileHandler, char *pcBuffer, int iLength) {
	unsigned int i;
	for (i = 0; i < iLength; i++) {
		uartSend(pcBuffer[i]);
	}
	return iLength;
}

/* configura/inicializa adc */
void adcInit(void) {
	/* configurações gerais */
	LPC_SYSCON->PDRUNCFG &= ~(1 << 4); // liga adc
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 13); // habilita clock para adc
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
	LPC_ADC->CR |= (1 << 0 | 1 << 1 | 1 << 2 | 1 << 8 | 1 << 16); // habilita canal 3; clkdiv = 1 (pclk_adc / 2); modo burst
	/* configurações de interrupção */
	LPC_ADC->INTEN |= (1 << 3); // habilita interrupção por conversão completada no canal 3
	NVIC_EnableIRQ(ADC_IRQn); // habilita interrupção no ADC
	NVIC_SetPriority(ADC_IRQn, 1); // nível 1 de prioridade de interrupção
	NVIC_ClearPendingIRQ(ADC_IRQn); // retira qualquer pendência de interrupção do adc
}

