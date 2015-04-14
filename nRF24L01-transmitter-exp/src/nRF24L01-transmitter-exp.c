/*
 ===============================================================================
 Name        : nrf24l01-transmitter-exp.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-4-8
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código à memória flash
#include "uart.h" // implementa configuração e inicialização da uart

/* configurações gerais */
#define CHANNEL 107 /* valor entre 1 e 126 - deve ser o mesmo para TX e RX*/
#define BUFFER_SIZE 8 /* valor entre 1 e 32 - tamanho do buffer */
#define ADDRESS 75 /* valor entre 1 e 255 */

/* variáveis globais */
uint8_t dataSending[BUFFER_SIZE]; // matriz armazena dados recebidos

/* protótipos */
void sysTickConfig(void); // configura systick
void delayUs(unsigned int); // implementa delay em microssegundos
void buttonsConfig(void); // configura pinos dos botões
void adcInit(void); // configura/inicializa o adc
void spiInit(void); // configura/inicializa spi
void spiPinsConfig(void); // configura pinos do spi
void spiSend(uint8_t); // envia dado para buffer tx
uint8_t spiRead(void); // lê e retorna dado
void nRF24L01Init(void);
void nRF24L01PinsConfig(void); // configura SS, CE e IRQ
void nRF24L01Send(void);
uint8_t nRF24L01RegisterRead(uint8_t);

/* isr de interrupção externa no port1 - pino é colocado em estado VSS pelo nRF24L01, quando ocorre recepção de dado */
void PIOINT0_IRQHandler(void) {
	static uint8_t adChannel; // armazena canal em que ocorreu a conversão
	if (LPC_GPIO0->MIS & (1 << 10)) { // verifica se flag do pino está setada
		LPC_GPIO0->IC |= (1 << 10); // limpa flag de interrupção do pino
		if (LPC_ADC->GDR & (1 << 31)) {
			adChannel = ((LPC_ADC->GDR & (0b111 << 24)) >> 24); // captura canal em que ocorreu a conversão
			if (!(LPC_ADC->STAT & (adChannel << 8))) { // verifica se não ocorreu overrun no canal
				dataSending[adChannel] = (((LPC_ADC->GDR & (0x3FF << 6)) >> 6)
						/ 5); // captura valor convertido e atribui à matriz de tráfego
			}
		}
		printf("%s\n", "Enviou!");
		nRF24L01Send(); // chama função de recepção de dados
	}
}

/* isr de interrupções externas p2 */
void PIOINT2_IRQHandler(void) {
	if (LPC_GPIO2->MIS & (1 << 4)) { // verifica se interrupção ocorreu no pino p2.4
		if (dataSending[0]) { // verifica se posição de dado referente ao status do led, está setada
			dataSending[0] = 0; // atribui 1 à posição do led
			printf("%s\n", "Led desligado");
		} else {
			dataSending[0] = 1; // atribui 0 à posição do led
			printf("%s\n", "Led ligado");
		}
		delayUs(177000); // delay para time debouncing
		LPC_GPIO2->IC |= (1 << 4); // limpa flag de interrupção do pino
	}
}

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 48mhz
	SystemCoreClockUpdate(); // atualiza valor de clock para variável SystemCoreClock
	uartInit();
	adcInit(); // configura/inicializa adc
	sysTickConfig(); // configura systick
	buttonsConfig(); // configura pinos dos botões
	spiInit(); // configura spi
	spiPinsConfig(); // configura pinos spi
	//nRF24L01PinsConfig();
	nRF24L01PinsConfig();
	printf("%s%u\n", "Valor do registro: ", nRF24L01RegisterRead(0x10));
	nRF24L01Init();
	printf("%s%u\n", "Valor do registro: ", nRF24L01RegisterRead(0x10));
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
	LPC_IOCON->SCK_LOC = ((LPC_IOCON->SCK_LOC & ~(0b11 << 0)) | (0b01 << 0)); // seleciona pino p0.6, para pino sck0
	LPC_IOCON->PIO2_11 = ((LPC_IOCON->PIO2_11 & ~(0b111 << 0 | 0b11 << 3))
			| (0b001 << 0)); // pino como sck
	/* p0.9 - MOSI */
	LPC_IOCON->PIO0_9 = ((LPC_IOCON->PIO0_9 & ~(0b111 << 0 | 0b11 << 3))
			| (0b001 << 0)); // pino como mosi
	/* p0.8 - MISO */
	LPC_IOCON->PIO0_8 = ((LPC_IOCON->PIO0_8 & ~(0b111 << 0 | 0b11 << 3))
			| (0b001 << 0)); // pino como miso
}

/* configura interrupção para pino IRQ */
void nRF24L01PinsConfig(void) {
	/* p2.10 - CE (quando low permite recepção de dados, quando high apenas exibe) */
	LPC_IOCON->PIO2_10 &= ~(0b111 << 0); // pino como gpio
	LPC_GPIO2->DIR |= (1 << 10); // pino como saída
	LPC_GPIO2->DATA &= ~(1 << 10); // estado lógico baixo
	/* p2.2 - CSN (quando low espera comando, quando high espera dado) */
	LPC_IOCON->PIO2_2 &= ~(0b111 << 0); // pino como gpio
	LPC_GPIO2->DIR |= (1 << 2); // pino como saída
	LPC_GPIO2->DATA |= (1 << 2); // estado lógico alto
	/* p0.10 - IRQ */
	LPC_IOCON->SWCLK_PIO0_10 = ((LPC_IOCON->SWCLK_PIO0_10
			& ~(0b111 << 0 | 0b11 << 3)) | (0b001 << 0 | 0b10 << 3)); // pino como gpio; habilita pull-up
	LPC_GPIO0->DIR &= ~(1 << 10); // pino como entrada
	LPC_GPIO0->DATA |= (1 << 10); // estado lógico alto
	LPC_GPIO0->IE |= (1 << 10); // habilita interrupção para o pino
	LPC_GPIO0->IS &= ~(1 << 10); // interrupção por mudança de borda
	LPC_GPIO0->IBE &= ~(1 << 10); // interrupção será em borda específica
	LPC_GPIO0->IEV &= ~(1 << 10); // interrupção será por borda de descida
	NVIC_SetPriority(EINT0_IRQn, 1); // nível 1 de prioridade de interrupção externa no port1
	NVIC_EnableIRQ(EINT0_IRQn); // habilita interrupções no port1
	NVIC_ClearPendingIRQ(EINT0_IRQn); // retira qualquer pendência de interrupção no port1
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

/* configura pinos de botões */
void buttonsConfig(void) {
	/* p2.4 (D 1º), p2.5 (D 4º) */
	LPC_IOCON->PIO2_4 = ((LPC_IOCON->PIO2_4 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_IOCON->PIO2_5 = ((LPC_IOCON->PIO2_5 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // pull-up ativo; pino como gpio
	LPC_GPIO2->DIR &= ~(1 << 4 | 1 << 5); // pinos como entrada
	LPC_GPIO2->DATA |= (1 << 4 | 1 << 5); // estado lógico alto
	/* configurações de interrupção p2 */
	LPC_GPIO2->IE |= (1 << 4 | 1 << 5); // habilita interrupção nos pinos
	LPC_GPIO2->IS &= ~(1 << 4 | 1 << 5); // interrupção nos pinos será por borda
	LPC_GPIO2->IBE &= ~(1 << 4 | 1 << 5); // interrupção será em borda específica
	LPC_GPIO2->IEV &= ~(1 << 4 | 1 << 5); // interrupção por borda de descida
	NVIC_EnableIRQ(EINT2_IRQn); // habilita interrupções no port2
	NVIC_SetPriority(EINT2_IRQn, 1); // atribui nível de prioridade 2 às interrupções no port2
	NVIC_ClearPendingIRQ(EINT2_IRQn); // retira qualquer pendência de interrupção no port2
}

/* configura/inicializa ADC */
void adcInit(void) {
	/* configurações gerais */
	LPC_SYSCON->PDRUNCFG &= ~(1 << 4); // limpando-se o bit, liga-se o periférico adc
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 13); // atribui-se clock ao adc
	LPC_IOCON->R_PIO1_2 = ((LPC_IOCON->R_PIO1_2
			& ~(0b11 << 0 | 0b11 << 3 | 1 << 7)) | (0b10 << 0)); // pino como ad3; desabilita pull-up e pull-down; seta admode
	/* configurações de operação */
	LPC_ADC->CR |= (1 << 3 | 1 << 8 | 1 << 16); // habilita canal 3; clkdiv = 1 (pclk_adc / 2); solicita conversão
}

/* configura/inicializa nRF24L01 */
void nRF24L01Init(void) {
	//RX_ADDR_P0 - configura endereço de recepção PIPE0
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x2A);
	spiSend(ADDRESS);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//TX_ADDR - configura endereço de transmissão
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x30);
	spiSend(ADDRESS);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//EN_AA - habilita autoACK no PIPE0
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x21);
	spiSend(0x01);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//EN_RXADDR - ativa o PIPE0
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x22);
	spiSend(0x01);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//SETUP_AW - define o endereço com tamanho de 5 Bytes
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x23);
	spiSend(0x03);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//SETUP_RETR - configura para nao retransmitir pacotes
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x24);
	spiSend(0x00);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//RF_CH - define o canal do modulo (TX e RX devem ser iguais)
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	//spiSend(0x05);
	spiSend(0x05);
	spiSend(CHANNEL);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//RF_SETUP - ativa LNA, taxa em 250K, e maxima potencia 0dbm
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x26);
	spiSend(0b00100110);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//STATUS - reseta o resgistrador STATUS
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x27);
	spiSend(0x70);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//RX_PW_P0 - tamanho do buffer PIPE0
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x31);
	spiSend(BUFFER_SIZE);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//CONFIG - coloca em modo de recepção, e define CRC de 2 Bytes
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x20);
	spiSend(0x0F);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//tempo para sair do modo standby entrar em modo de recepçao
	LPC_GPIO2->DATA |= (1 << 10); // CE estado lógico alto
	delayUs(15);
	LPC_GPIO2->DATA &= ~(1 << 10); // CE estado lógico baixo
}

void nRF24L01Send(void) {
	/* STATUS - limpa registrador */
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x27);
	spiSend(0b01110000); // limpa flags de recepção, transmissão, max e habilita CRC
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	/* FLUSH_TX - limpa o buffer */
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0xE1);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	/* W_TX_PAYLOAD - envia os dados para o buffer */
	static uint8_t i;
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0xA0);
	for (i = 0; i < BUFFER_SIZE; i++)
		spiSend(dataSending[i]); // coloca dado no buffer tx
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	//CONFIG - transmission mode
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(0x20);
	spiSend(0x0E);
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico alto

	/* pulso para transmitir os dados */
	LPC_GPIO2->DATA |= (1 << 10); // CE estado lógico alto
	delayUs(15);
	LPC_GPIO2->DATA &= ~(1 << 10); // CE estado lógico baixo

	delayUs(5000);
}

/* testa leitura de registrador do nRF24L01+ */
uint8_t nRF24L01RegisterRead(uint8_t reg) {
	LPC_GPIO2->DATA &= ~(1 << 2); // CSN estado lógico baixo
	spiSend(reg);
	reg = spiRead();
	LPC_GPIO2->DATA |= (1 << 2); // CSN estado lógico baixo
	return reg;
}
