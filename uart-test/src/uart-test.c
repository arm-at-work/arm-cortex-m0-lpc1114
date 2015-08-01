/*
 ===============================================================================
 Name        : uart-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-7-26
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código à memória flash

/* protótipos */
void uartInit(void);
void uartSend(unsigned char data);
void sysTickConfig(void);
void delayUs(unsigned int us);
void ledConfig(void);

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 48mhz
	SystemCoreClockUpdate(); // atualiza valor de clock, para a variável SystemCoreClock
	sysTickConfig(); // inicializa systick
	ledConfig(); // configura pinos de led
	uartInit(); // configura/inicializa uart
	do {
		delayUs(500000);
		LPC_GPIO0->DATA ^= (1 << 7); // inverte estado lógico do pino
		printf("%s\n", "teste");
	} while (1);
	return 1;
}

/* configura/inicializa uart */
void uartInit(void) {
	/* configurações gerais */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12); // habilita clock para a uart
	LPC_SYSCON->UARTCLKDIV = 2; // pclk_uart = c_clk / 2
	LPC_IOCON->PIO1_6 = ((LPC_IOCON->PIO1_6 & ~(0b11 << 0)) | (0b01 << 0)); // pino como rx da uart
	LPC_IOCON->PIO1_7 = ((LPC_IOCON->PIO1_7 & ~(0b11 << 0)) | (0b01 << 0)); // pino como tx da uart
	/* configurações de operação */
	LPC_UART->LCR |= (1 << 7); // seta dlab - permite acesso aos divisores do brg
	LPC_UART->DLL = ((SystemCoreClock / LPC_SYSCON->UARTCLKDIV) / (16 * 9600));
	LPC_UART->DLM = 0;
	LPC_UART->FDR = (1 << 4); // seta mulval
	LPC_UART->LCR &= ~(1 << 7); // limpa dlab - bloqueia acesso aos divisores do brg
	/* configurações de pacote */
	LPC_UART->LCR |= (0b11 << 0); // dado trafegado será de 8 bits
	LPC_UART->LCR &= ~(1 << 2 | 1 << 3); // 1 bit de parada; sem bit de paridade
	LPC_UART->FCR |= (1 << 0 | 1 << 1 | 1 << 2); // ativa e reinicia fifos da uart
	LPC_UART->THR = 0; // limpa buffer tx
	LPC_UART->RBR; // limpa buffer rx
	LPC_UART->FCR &= ~(0b11 << 6); // dma habilitado - nível de disparo de 1 dado
}

/* envia dado para buffer tx */
void uartSend(unsigned char data) {
	do { // aguarda até que buffer tx possa receber novo dado
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

/* configura systick */
void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1;
	SysTick->CTRL |= (1 << 2);
}

/* implementa delay em microssegundos */
void delayUs(unsigned int us) {
	static unsigned int counter;
	SysTick->CTRL |= (1 << 0);
	for (counter = 0; counter < us; counter++) {
		do {
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0);
}

/* configura pino de led */
void ledConfig(void) {
	LPC_IOCON->PIO0_7 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO0->DIR |= (1 << 7); // pino como saída
	LPC_GPIO0->DATA &= ~(1 << 7); // estado lógico 0
}
