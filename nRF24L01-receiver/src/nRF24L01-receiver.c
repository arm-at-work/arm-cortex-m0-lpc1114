/*
 ===============================================================================
 Name        : nRF24L01-receiver.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-7-8
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código à memória flash
#include "uart.h"

/* configurações gerais */
#define CHANNEL 107 // canal 1 a 126
#define ADDRESS 75 // endereço RX 1 a 255
#define BUFFER_SIZE 10 // tamanho do buffer 1 a 32

/* propriedades */
uint8_t dataReceiving[BUFFER_SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // dado a ser enviado para o outro nRF24L01

/* protótipos */
void ledsConfig(void); // configura leds para teste de tráfego
void sysTickConfig(void);
void delayUs(unsigned int us); // systick
void spiInit(void);
void spiPinsConfig(void);
void spiSend(uint8_t data);
uint8_t spiRead(); // spi
void nRF24L01PinsConfig(void); // configura pinos para o nRF24L01
void nRF24L01Init(void); // inicializa nRF24L01
void nRF24L01Receive(void); // envia dados
uint8_t nRF24L01RegisterRead(uint8_t reg); // testa leitura de registradores do CI

/* isr de interrupção externa no port1 - pino é colocado em estado VSS pelo nRF24L01, quando ocorre recepção de dado */
void PIOINT1_IRQHandler(void) {
	if (LPC_GPIO1->MIS & (1 << 0)) { // verifica se flag do pino está setada
		if (dataReceiving[0] == 'd') {
			LPC_GPIO2->DATA &= ~(1 << 10); // estado lógico baixo
			//printf("%s\n", "Apagando led");
		} else if (dataReceiving[0] == 'l') {
			LPC_GPIO2->DATA |= (1 << 10); // estado lógico alto
			//printf("%s\n", "Acendendo led");
		} else {
			//printf("%s%u\n", "dado recebido, porem, fora do range - valor: ",dataReceiving[0]);
		}
		LPC_GPIO1->IC |= (1 << 0); // limpa flag de interrupção do pino
		nRF24L01Receive();
	}
}

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 100mhz, pclk = 25mhz, etc
	SystemCoreClockUpdate(); // atualiza valor de clock para a variável SystemCoreClock
	uartInit(); // configura/inicializa a uart3
	ledsConfig(); // configura leds
	sysTickConfig(); // configura systick
	spiInit(); // configura/inicializa spi
	spiPinsConfig(); // configura pinos do spi
	nRF24L01PinsConfig(); // configura pinos para o nRF24L01
	//printf("%s%u\n", "Valor do registro: ", nRF24L01RegisterRead(0x0A));
	nRF24L01Init(); // configura/inicializa nRF24L01
	//printf("%s%u\n", "Valor do registro: ", nRF24L01RegisterRead(0x0A));
	nRF24L01Receive();
	do { // loop infinito
	} while (1);
	return 1;
}

/* configura systick */
void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; // valor de reinício de systick
	SysTick->CTRL |= (1 << 2); // habilita interrupções; fonte de decremento é c_clk
}

/* implementa delay em microsegundos */
void delayUs(unsigned int us) {
	static uint32_t count; // zera contador de interrupções do systick
	SysTick->CTRL |= (1 << 0); // habilita contador do systick
	for (count = 0; count < us; count++) {
		do { // aguarda até que contador de interrupções se iguale ao valor recebido via parâmetro
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0); // para contador do systick
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
	LPC_SSP0->CPSR |= (0b100 << 0); // prescaler do divisor de clock
}

/* configura pinos do spi */
void spiPinsConfig(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 6); // habilita clock para pinos gpio
	/* p0.6 - SCK */
	LPC_IOCON->SCK_LOC = ((LPC_IOCON->SCK_LOC & ~(0b11 << 0)) | (0b10 << 0)); // seleciona pino p0.6, para pino sck0
	LPC_IOCON->PIO0_6 = ((LPC_IOCON->PIO0_6 & ~(0b11 << 0 | 0b11 << 3))
			| (0b10 << 0)); // pino como sck
	/* p0.9 - MOSI */
	LPC_IOCON->PIO0_9 = ((LPC_IOCON->PIO0_9 & ~(0b11 << 0 | 0b11 << 3))
			| (0b01 << 0)); // pino como mosi
	/* p0.8 - MISO */
	LPC_IOCON->PIO0_8 = ((LPC_IOCON->PIO0_8 & ~(0b11 << 0 | 0b11 << 3))
			| (0b01 << 0)); // pino como miso
}

/* envia dados */
void spiSend(uint8_t data) {
	LPC_SSP0->DR = data; // envia um dado ou instrução
	do { // aguarda fim da transferência
	} while (!(LPC_SSP0->SR & (1 << 4 | 1 << 1)));
}

/* recebe dados */
uint8_t spiRead() {
	LPC_SSP0->DR = 0xFF; // envia um dado ou instrução
	do { // aguarda fim da transferência
	} while (!(LPC_SSP0->SR & (1 << 4 | 1 << 1)));
	return LPC_SSP0->DR; // retorna valor do registrador
}

/* configura leds */
void ledsConfig(void) {
	LPC_IOCON->PIO2_10 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO2->DIR |= (1 << 10); // pino como saída
	LPC_GPIO2->DATA &= ~(1 << 10); // estado lógico baixo
}

/* configura interrupção para pino IRQ */
void nRF24L01PinsConfig(void) {
	/* p0.2 - CE (quando low permite recepção de dados, quando high apenas exibe) */
	LPC_IOCON->PIO0_2 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO0->DIR |= (1 << 2); // pino como saída
	LPC_GPIO0->DATA &= ~(1 << 2); // CE estado lógico baixo
	/* p3.0 - CSN (quando low espera comando, quando high espera dado) */
	LPC_IOCON->PIO3_0 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO3->DIR |= (1 << 0); // pino como saída
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto
	/* p1.0 - IRQ */
	LPC_IOCON->R_PIO1_0 = ((LPC_IOCON->R_PIO1_0
			& ~(0b11 << 3 | 0b11 << 0 | 1 << 10))
			| (1 << 7 | 0b10 << 3 | 0b01 << 0)); // pino como gpio; habilita pull-up
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

/* configura/inicializa nRF24L01 */
void nRF24L01Init() {
	//RX_ADDR_P0 - configura endereço de recepção PIPE0
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x2A);
	spiSend(ADDRESS);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto

	/* SETUP_RETR - configura para nao retransmitir pacotes */
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x24);
	spiSend(0x00);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto

	/* RF_CH - define o canal do modulo (TX e RX devem ser iguais) */
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x05);
	spiSend(CHANNEL);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto

	/* RF_SETUP - ativa LNA, taxa em 250K, e maxima potencia 0dbm */
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x26);
	spiSend(0b00100110);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto

	/* STATUS - reseta o resgistrador */
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x27);
	spiSend(0b01110000); // limpa flags
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto

	//RX_PW_P0 - tamanho do buffer PIPE0
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x31);
	spiSend(BUFFER_SIZE);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto

	//CONFIG - coloca em modo de recepção, e define CRC de 2 Bytes
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x20);
	spiSend(0x0F);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto

	LPC_GPIO0->DATA |= (1 << 2); // CE estado lógico alto
	delayUs(15);
}

void nRF24L01Receive(void) {
	//STATUS - limpa registrador
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x27);
	spiSend(0x70); // limpa flags de recepção, transmissão, max e habilita CRC
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto

	// LIMPA FLUSH_RX
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0xE2);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto

	delayUs(15);

	//R_RX_PAYLOAD - receive data from FIFO RX buffer
	uint8_t i;
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x61);
	for (i = 0; i < BUFFER_SIZE; i++) {
		dataReceiving[i] = spiRead();
	}
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto

	delayUs(15);
}

/* testa leitura de registrador do nRF24L01+ */
uint8_t nRF24L01RegisterRead(uint8_t reg) {
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(reg); // envia endereço do registrador para leitura
	reg = spiRead(); // lê valor do registrador anteriormente selecionado
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico alto
	return reg; // retorna valor lido do registrador
}
