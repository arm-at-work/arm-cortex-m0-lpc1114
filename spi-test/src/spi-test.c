/*
 ===============================================================================
 Name        : spi-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-4-30
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código à memória flash
#include "uart.h" // implementa configuração e inicialização da uart
#include "nRF24L01.h" // implementa configurações do nRF24L01+

/* protótipos */
void spiInit(void); // configura/inicializa spi
void spiPinsConfig(void); // configura pinos do spi
void irqConfig(void); // configura interrupção para pino IRQ
void spiSend(unsigned char); // envia dado para buffer tx
uint8_t spiRead(unsigned char); // lê e retorna dado
void sysTickConfig(void); // configura systick
void delayUs(unsigned int); // implementa delay em microssegundos
void nRF24L01Test(uint8_t); // implementa teste de leitura de registradores do nRF24L01

/* isr de interrupção externa no port1 - pino é colocado em estado VSS pelo nRF24L01, quando ocorre recepção de dado */
void PIOINT1_IRQHandler(void) {
	if (LPC_GPIO1->MIS & (1 << 0)) { // verifica se flag do pino está setada
		nRF24L01ReceiveData(); // chama função de recepção de dados
		LPC_GPIO1->IC |= (1 << 0); // limpa flag de interrupção do pino
	}
}

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 48mhz
	SystemCoreClockUpdate(); // atualiza valor de clock para variável SystemCoreClock
	uartInit();
	spiInit(); // configura spi
	spiPinsConfig(); // configura pinos spi
	sysTickConfig(); // configura systick
	// nRF24L01Test(0x10);
	nRF24L01Init();
	// nRF24L01Test(0x10); // testa atribuição de valor ao registrador do nRF24L01+
	nRF24L01ReceiveData(); // inicia recebimento de dados
	do { // loop infinito
	} while (1);
	return 1;
}

/* configura/inicializa spi */
void spiInit(void) {
	/* configurações gerais */
	LPC_SYSCON->PRESETCTRL |= (1 << 0); // escreve 1 para os sinais de reset
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 11); // habilita clock para ssp0
	LPC_SYSCON->SSP0CLKDIV = 1; // pclk_ssp0 = c_clk / 2
	/* configurações de operação */
	LPC_SSP0->CR0 |= (0 << 7 | 0 << 6 | 0b00 << 4 | 0b111 << 0); // dado de 8 bits; formato spi de frame; cpol = 0; cpha = 0
	LPC_SSP0->CR1 |= (1 << 1 | 0 << 2); // habilita spi; modo master
	LPC_SSP0->CPSR |= (0b10 << 0); // prescaler do divisor de clock
}

/* configura pinos do spi */
void spiPinsConfig(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 6); // habilita clock para pinos gpio
	/* p0.6 - SCK */
	LPC_IOCON->SCK_LOC = ((LPC_IOCON->SCK_LOC & ~(0b11 << 0)) | (0b10 << 0)); // seleciona pino p2.11, para pino sck0
	LPC_IOCON->PIO0_6 = ((LPC_IOCON->PIO0_6 & ~(0b11 << 0 | 0b11 << 3))
			| (0b10 << 0)); // pino como sck
	/* p0.9 - MOSI */
	LPC_IOCON->PIO0_9 = ((LPC_IOCON->PIO0_9 & ~(0b11 << 0 | 0b11 << 3))
			| (0b01 << 0)); // pino como mosi
	/* p0.8 - MISO */
	LPC_IOCON->PIO0_8 = ((LPC_IOCON->PIO0_8 & ~(0b11 << 0 | 0b11 << 3))
			| (0b01 << 0)); // pino como miso
	/* p0.2 - CE (quando low permite recepção de dados, quando high apenas exibe) */
	LPC_IOCON->PIO0_2 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO0->DIR |= (1 << 2); // pino como saída
	LPC_GPIO0->DATA &= ~(1 << 2); // estado lógico baixo
	/* p3.0 - CSN (quando low espera comando, quando high espera dado) */
	LPC_IOCON->PIO3_0 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO3->DIR |= (1 << 0); // pino como saída
	LPC_GPIO3->DATA |= (1 << 0); // estado lógico baixo
}

/* configura interrupção para pino IRQ */
void irqConfig(void) {
	/* p1.0 - IRQ */
	LPC_IOCON->R_PIO1_0 = ((LPC_IOCON->R_PIO1_0 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3 | 0b01 << 0)); // pino como gpio; habilita pull-up
	LPC_GPIO1->DIR &= ~(1 << 0); // pino como entrada
	LPC_GPIO1->DATA |= (1 << 0); // estado lógico alto
	LPC_GPIO1->IE |= (1 << 0); // habilita interrupção para o pino
	LPC_GPIO1->IS &= ~(1 << 0); // interrupção por mudança de borda
	LPC_GPIO1->IBE &= ~(1 << 0); // interrupção será em borda específica
	LPC_GPIO1->IEV &= ~(1 << 0); // interrupção será por borda de descida
	NVIC_SetPriority(EINT1_IRQn, 1); // nível 1 de prioridade de interrupção externa no port1
	NVIC_EnableIRQ(EINT1_IRQn); // habilita interrupções no port1
	NVIC_ClearPendingIRQ(EINT1_IRQn); // retira qualquer pendência de interrupção no port1
}

/* envia dados */
void spiSend(unsigned char data) {
	do { // aguarda fim da transferência
	} while (!(LPC_SSP0->SR & (1 << 4 | 1 << 1)));
	LPC_SSP0->DR = data; // envia um dado ou instrução
}

/* recebe dados */
uint8_t spiRead(unsigned char data) {
	do { // aguarda fim da transferência
	} while (!(LPC_SSP0->SR & (1 << 4 | 1 << 1)));
	LPC_SSP0->DR = data; // envia um dado ou instrução
	return LPC_SSP0->DR; // retorna valor do registrador
}

/* configura systick */
void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; // atribui valor de reinício para contador
	SysTick->CTRL |= (1 << 2); // fonte de decremento é c_clk
}

/* implementa delay em microssegundos */
void delayUs(unsigned int us) {
	unsigned int count; // contador de interrupção
	SysTick->CTRL |= (1 << 0); // habilita contador
	for (count = 0; count < us; count++) { // aguarda até que valor do contador se iguale ao valor recebido via parâmetro
		do { // aguarda até que flag de interrupção seja setada
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0); // para contador
}

