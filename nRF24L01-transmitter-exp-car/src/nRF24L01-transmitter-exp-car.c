/*
 ===============================================================================
 Name        : adc-no-interrupt.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-7-11
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
#define CHANNEL 107 /* valor entre 1 e 126 - deve ser o mesmo para TX e RX*/
#define BUFFER_SIZE 32 /* valor entre 1 e 32 - tamanho do buffer */
#define ADDRESS 75 /* valor entre 1 e 255 */

/* variáveis globais */
uint8_t adChannel;
uint8_t selectedChannel = 0;
uint8_t adChannels[3] = { 0, 0, 0 };
uint8_t dataSending[BUFFER_SIZE]; // matriz armazena dados recebidos

/* protótipos */
void adcInit(void);
void buttonsConfig(void);
void interruptButtonsConfig(void);
void sysTickConfig(void);
void delayUs(uint32_t);
void spiInit(void); // configura/inicializa spi
void spiPinsConfig(void); // configura pinos do spi
void spiSend(uint8_t); // envia dado para buffer tx
uint8_t spiRead(void); // lê e retorna dado
void nRF24L01Init(void);
void nRF24L01PinsConfig(void); // configura SS, CE e IRQ
void nRF24L01Send(void);
uint8_t nRF24L01RegisterRead(uint8_t);

/* isr de interrupção externa no port1 - pino é colocado em estado VSS pelo nRF24L01, quando ocorre recepção de dado */
void PIOINT1_IRQHandler(void) {
	static uint8_t adChannel; // armazena canal em que ocorreu a conversão
	if (LPC_GPIO1->MIS & (1 << 10)) { // verifica se flag do pino está setada
		LPC_GPIO1->IC |= (1 << 10); // limpa flag de interrupção do pino
		if (LPC_ADC->GDR & (1 << 31)) {
			adChannel = ((LPC_ADC->GDR & (0b111 << 24)) >> 24); // captura canal em que ocorreu a conversão
			if (!(LPC_ADC->STAT & (adChannel << 8))) { // verifica se não ocorreu overrun no canal
				dataSending[adChannel] = (((LPC_ADC->GDR & (0x3FF << 6)) >> 6)
						/ 5); // captura valor convertido e atribui à matriz de tráfego
			}
		}
		printf("%s\n", "Enviado!");
		/*
		 uint8_t i;
		 for (i = 0; i < BUFFER_SIZE; i++) {
		 printf("%s%u%s%u\n", "Valor pos. ", i, ": ", dataSending[i]);
		 }
		 */
		nRF24L01Send(); // chama função de recepção de dados
	}
}

/* isr de interrupção externa */
void PIOINT2_IRQHandler(void) {
	if (LPC_GPIO2->MIS & (1 << 7)) { // verifica se interrupção ocorreu no pino p2.7
		(dataSending[0]) ? (dataSending[0] = 0) : (dataSending[0] = 1);
	}
	if (LPC_GPIO2->MIS & (1 << 9)) { // verifica se interrupção ocorreu no pino p2.9
		(dataSending[1]) ? (dataSending[1] = 0) : (dataSending[1] = 1);
	}
	if (LPC_GPIO2->MIS & (1 << 8)) { // verifica se interrupção ocorreu no pino p2.8
		(dataSending[2]) ? (dataSending[2] = 0) : (dataSending[2] = 1);
	}
	if (LPC_GPIO2->MIS & (1 << 10)) { // verifica se interrupção ocorreu no pino p2.10
		(dataSending[3]) ? (dataSending[3] = 0) : (dataSending[3] = 1);
	}
	delayUs(177000); // delay para time debouncing
	LPC_GPIO2->IC |= (1 << 7 | 1 << 8 | 1 << 9 | 1 << 10); // limpa flag de interrupção do pino
}

/* isr de interrupção externa */
void PIOINT3_IRQHandler(void) {
	if (LPC_GPIO3->MIS & (1 << 0)) { // verifica se interrupção ocorreu no pino p3.0
		(dataSending[14]) ? (dataSending[14] = 0) : (dataSending[14] = 1);
	}
	if (LPC_GPIO3->MIS & (1 << 4)) { // verifica se interrupção ocorreu no pino p3.4
		(dataSending[15]) ? (dataSending[15] = 0) : (dataSending[15] = 1);
	}
	delayUs(177000); // delay para time debouncing
	LPC_GPIO3->IC |= (1 << 0 | 1 << 4); // limpa flag de interrupção do pino
}

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	sysTickConfig();
	uartInit();
	adcInit();
	spiInit(); // configura spi
	spiPinsConfig(); // configura pinos spi
	buttonsConfig();
	interruptButtonsConfig();
	nRF24L01PinsConfig();
	printf("%s%u\n", "Valor do registro: ", nRF24L01RegisterRead(0x10));
	nRF24L01Init();
	printf("%s%u\n", "Valor do registro: ", nRF24L01RegisterRead(0x10));
//	printf("%s%u\n", "Valor do clock: ", SystemCoreClock);
	nRF24L01Send();
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
	LPC_SSP0->CPSR |= (0b100 << 0); // prescaler do divisor de clock
}

/* configura pinos do spi */
void spiPinsConfig(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 6); // habilita clock para pinos gpio
	/* p2.11 - SCK */
	LPC_IOCON->SCK_LOC = ((LPC_IOCON->SCK_LOC & ~(0b11 << 0)) | (0b01 << 0)); // seleciona pino p2.11, para pino sck0
	LPC_IOCON->PIO2_11 = ((LPC_IOCON->PIO2_11 & ~(0b11 << 0 | 0b11 << 3))
			| (0b01 << 0)); // pino como sck
	/* p0.9 - MOSI */
	LPC_IOCON->PIO0_9 = ((LPC_IOCON->PIO0_9 & ~(0b11 << 0 | 0b11 << 3))
			| (0b01 << 0)); // pino como mosi
	/* p0.8 - MISO */
	LPC_IOCON->PIO0_8 = ((LPC_IOCON->PIO0_8 & ~(0b11 << 0 | 0b11 << 3))
			| (0b01 << 0)); // pino como miso
}

/* configura interrupção para pino IRQ */
void nRF24L01PinsConfig(void) {
	/* p1.8 - CE (quando low permite recepção de dados, quando high apenas exibe) */
	LPC_IOCON->PIO1_8 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO1->DIR |= (1 << 8); // pino como saída
	LPC_GPIO1->DATA &= ~(1 << 8); // estado lógico baixo
	/* p0.2 - CSN (quando low espera comando, quando high espera dado) */
	LPC_IOCON->PIO0_2 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO0->DIR |= (1 << 2); // pino como saída
	LPC_GPIO0->DATA |= (1 << 2); // estado lógico baixo
	/* p1.10 - IRQ */
	LPC_IOCON->PIO1_10 = ((LPC_IOCON->PIO1_10 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pino como gpio; habilita pull-up
	LPC_GPIO1->DIR &= ~(1 << 10); // pino como entrada
	LPC_GPIO1->DATA |= (1 << 10); // estado lógico alto
	LPC_GPIO1->IE |= (1 << 10); // habilita interrupção para o pino
	LPC_GPIO1->IS &= ~(1 << 10); // interrupção por mudança de borda
	LPC_GPIO1->IBE &= ~(1 << 10); // interrupção será em borda específica
	LPC_GPIO1->IEV &= ~(1 << 10); // interrupção será por borda de descida
	NVIC_SetPriority(EINT1_IRQn, 1); // nível 1 de prioridade de interrupção externa no port1
	NVIC_EnableIRQ(EINT1_IRQn); // habilita interrupções no port1
	NVIC_ClearPendingIRQ(EINT1_IRQn); // retira qualquer pendência de interrupção no port1
}

/* envia dados */
void spiSend(uint8_t data) {
	do { // aguarda fim da transferência
	} while (!(LPC_SSP0->SR & (1 << 4 | 1 << 1)));
	LPC_SSP0->DR = data; // envia um dado ou instrução
}

/* recebe dados */
uint8_t spiRead(void) {
	do { // aguarda fim da transferência
	} while (!(LPC_SSP0->SR & (1 << 4 | 1 << 1)));
	LPC_SSP0->DR = 0x00; // envia um dado ou instrução
	return LPC_SSP0->DR; // retorna valor do registrador
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
	LPC_ADC->CR |= (1 << 0 | 1 << 8 | 1 << 16); // habilita canal; clkdiv = 1 (pclk_adc / 2); modo burst
}

/* configura pinos dos botões da direita */
void buttonsConfig(void) {
	/* DIREITA - p2.9(R1), p2.7(L1), p3.0(Borb. right), p3.4(Azul), p2.10(Borb. left), p2.8(Roxo) */
	LPC_IOCON->PIO2_9 = ((LPC_IOCON->PIO2_9 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_IOCON->PIO2_7 = ((LPC_IOCON->PIO2_7 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_IOCON->PIO3_0 = ((LPC_IOCON->PIO3_0 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_IOCON->PIO3_4 = ((LPC_IOCON->PIO3_4 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_IOCON->PIO2_10 = ((LPC_IOCON->PIO2_10 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_IOCON->PIO2_8 = ((LPC_IOCON->PIO2_8 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_GPIO2->DIR &= ~(1 << 10 | 1 << 9 | 1 << 8 | 1 << 7); // pinos como entrada
	LPC_GPIO2->DATA |= (1 << 10 | 1 << 9 | 1 << 8 | 1 << 7); // estado lógico alto
	LPC_GPIO3->DIR &= ~(1 << 0 | 1 << 4); // pinos como entrada
	LPC_GPIO3->DATA |= (1 << 0 | 1 << 4); // estado lógico alto
}

/* configura interrupções dos botões */
void interruptButtonsConfig(void) {
	/* configurações de interrupção p2 */
	LPC_GPIO2->IE |= (1 << 10 | 1 << 9 | 1 << 8 | 1 << 7); // habilita interrupção nos pinos
	LPC_GPIO2->IS &= ~(1 << 10 | 1 << 9 | 1 << 8 | 1 << 7); // interrupção por mudança de borda
	LPC_GPIO2->IBE &= ~(1 << 10 | 1 << 9 | 1 << 8 | 1 << 7); // interrupção será em borda específica
	LPC_GPIO2->IEV &= ~(1 << 10 | 1 << 9 | 1 << 8 | 1 << 7); // interrupção será por borda de descida
	NVIC_EnableIRQ(EINT2_IRQn); // habilita interrupções no port2
	NVIC_SetPriority(EINT2_IRQn, 1); // atribui nível de prioridade às interrupções no port2
	NVIC_ClearPendingIRQ(EINT2_IRQn); // retira qualquer pendência de interrupção no port2
	/* configurações de interrupção p1 */
	LPC_GPIO3->IE |= (1 << 0 | 1 << 4); // habilita interrupção nos pinos
	LPC_GPIO3->IS &= ~(1 << 0 | 1 << 4); // interrupção por mudança de borda
	LPC_GPIO3->IBE &= ~(1 << 0 | 1 << 4); // interrupção será em borda específica
	LPC_GPIO3->IEV &= ~(1 << 0 | 1 << 4); // interrupção será por borda de descida
	NVIC_EnableIRQ(EINT3_IRQn); // habilita interrupções no port3
	NVIC_SetPriority(EINT3_IRQn, 1); // atribui nível de prioridade às interrupções no port3
	NVIC_ClearPendingIRQ(EINT3_IRQn); // retira qualquer pendência de interrupção no port3
}

/* configura systick */
void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; // valor de reinício para o contador
	SysTick->CTRL |= (1 << 2); // fonte de decremento é c_clk
}

/* implementa delay em microssegundos */
void delayUs(uint32_t us) {
	static uint32_t count; // contador de 'zeramentos'
	SysTick->CTRL |= (1 << 0); // inicializa contador
	for (count = 0; count < us; count++) { // aguarda até que valor do contador se iguale à valor recebido via parâmetro
		do { // aguarda até que flag de zeramento esteja setada
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0); // para contador
}

/* configura/inicializa nRF24L01 */
void nRF24L01Init(void) {
	//RX_ADDR_P0 - configura endereço de recepção PIPE0
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x2A);
	spiSend(ADDRESS);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//TX_ADDR - configura endereço de transmissão
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x30);
	spiSend(ADDRESS);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//EN_AA - habilita autoACK no PIPE0
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x21);
	spiSend(0x01);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//EN_RXADDR - ativa o PIPE0
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x22);
	spiSend(0x01);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//SETUP_AW - define o endereço com tamanho de 5 Bytes
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x23);
	spiSend(0x03);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//SETUP_RETR - configura para nao retransmitir pacotes
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x24);
	spiSend(0x00);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//RF_CH - define o canal do modulo (TX e RX devem ser iguais)
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	//spiSend(0x05);
	spiSend(0x05);
	spiSend(CHANNEL);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//RF_SETUP - ativa LNA, taxa em 250K, e maxima potencia 0dbm
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x26);
	spiSend(0b00100110);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//STATUS - reseta o resgistrador STATUS
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x27);
	spiSend(0x70);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//RX_PW_P0 - tamanho do buffer PIPE0
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x31);
	spiSend(BUFFER_SIZE);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//CONFIG - coloca em modo de recepção, e define CRC de 2 Bytes
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x20);
	spiSend(0x0F);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//tempo para sair do modo standby entrar em modo de recepçao
	LPC_GPIO1->DATA |= (1 << 8); // CE estado lógico alto
	delayUs(15);
	LPC_GPIO1->DATA &= ~(1 << 8); // CE estado lógico baixo
}

void nRF24L01Send(void) {
	/* STATUS - limpa registrador */
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x27);
	spiSend(0b01110000); // limpa flags de recepção, transmissão, max e habilita CRC
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	/* FLUSH_TX - limpa o buffer */
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0xE1);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	/* W_TX_PAYLOAD - envia os dados para o buffer */
	static uint8_t i;
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0xA0);
	for (i = 0; i < BUFFER_SIZE; i++)
		spiSend(dataSending[i]); // coloca dado no buffer tx
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	//CONFIG - transmission mode
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x20);
	spiSend(0x0E);
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico alto

	/* pulso para transmitir os dados */
	LPC_GPIO1->DATA |= (1 << 8); // CE estado lógico alto
	delayUs(15);
	LPC_GPIO1->DATA &= ~(1 << 8); // CE estado lógico baixo

	delayUs(5000);
}

/* testa leitura de registrador do nRF24L01+ */
uint8_t nRF24L01RegisterRead(uint8_t reg) {
	LPC_GPIO0->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(reg);
	reg = spiRead();
	LPC_GPIO0->DATA |= (1 << 2); // CSN estado lógico baixo
	return reg;
}
