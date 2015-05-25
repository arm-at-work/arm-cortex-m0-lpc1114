/*
 ===============================================================================
 Name        : spi-5110-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-5-19
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código à memória flash
#include "characters.h" // importa arquivos com caracteres

#define LCD_COMMAND 0 // instrução para envio de comando
#define LCD_DATA 1 // instrução para envio de dado

#define GLCD_X 84
#define GLCD_Y 48

/* protótipos */
void spiInit(void); // configura/inicializa spi
void spiPinsConfig(void); // configura pinos do spi
void spiSend(unsigned char); // envia dado para buffer tx
void sysTickConfig(void); // configura systick
void delayUs(unsigned int); // implementa delay em microssegundos
void glcdInit(void);
void glcdPutString(char *characters);
void glcdPutChar(char character);
void glcdClear(void);
void glcdSend(uint8_t, unsigned char); // envia dado para glcd

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 48mhz
	SystemCoreClockUpdate(); // atualiza valor de clock para variável SystemCoreClock
	spiInit(); // configura spi
	spiPinsConfig(); // configura pinos spi
	sysTickConfig(); // configura systick
	glcdInit(); // configura/inicaliza glcd
	glcdClear();
	glcdPutString("Mallong testando. Era só aquilo Oo");
	do { // loop infinito
	} while (1);
	return 1;
}

/* configura/inicializa spi */
void spiInit(void) {
	/* configurações gerais */
	LPC_SYSCON->PRESETCTRL |= (1 << 0); // escreve 1 para os sinais de reset
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 11); // habilita clock para ssp0
	LPC_SYSCON->SSP0CLKDIV = 1; // pclk_ssp0 = c_clk / 2 (se não setado, spi não funciona)
	/* configurações de operação */
	LPC_SSP0->CR0 |= (0 << 7 | 0 << 6 | 0b00 << 4 | 0b111 << 0); // dado de 8 bits; formato spi de frame; cpol = 0; cpha = 0
	LPC_SSP0->CR1 |= (1 << 1 | 0 << 2); // habilita spi ;modo master
	LPC_SSP0->CPSR |= (0b10 << 0); // prescaler do divisor de clock (se não setado, spi não funciona)
}

/* configura pinos do spi */
void spiPinsConfig(void) {
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 6); // habilita clock para pinos gpio
	/* p0.6 - SCK */
	LPC_IOCON->SCK_LOC = ((LPC_IOCON->SCK_LOC & ~(0b11 << 0)) | (0b10 << 0)); // seleciona pino p2.11, para pino sck0
	LPC_IOCON->PIO0_6 = ((LPC_IOCON->PIO0_6 & ~(0b11 << 0 | 0b11 << 3))
			| (0b10 << 0)); // desabilita pull-up e pull-down (importante, senão não funciona); pino como sck
	/* p0.9 - MOSI/DIN */
	LPC_IOCON->PIO0_9 = ((LPC_IOCON->PIO0_9 & ~(0b11 << 0 | 0b11 << 3))
			| (0b01 << 0)); // pino como mosi
	/* p0.2 - CE (quando low permite recepção de dados, quando high apenas exibe) */
	LPC_IOCON->PIO0_2 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO0->DIR |= (1 << 2); // pino como saída
	LPC_GPIO0->DATA |= (1 << 2); // estado lógico alto
	/* p3.0 - DC (quando low espera comando, quando high espera dado) */
	LPC_IOCON->PIO3_0 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO3->DIR |= (1 << 0); // pino como saída
	LPC_GPIO3->DATA &= ~(1 << 0); // estado lógico baixo
	/* p0.8 - RST (quando low, reseta o glcd) */
	LPC_IOCON->PIO0_8 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO0->DIR |= (1 << 8); // pino como saída
	LPC_GPIO0->DATA |= (1 << 8); // estado lógico alto
}

/** envia dados */
void spiSend(unsigned char data) {
	do { // aguarda fim da transferência
	} while (!(LPC_SSP0->SR & (1 << 4 | 1 << 1)));
	LPC_SSP0->DR = data; // envia um dado ou instrução
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

/** envia instrução, comando, dado ao glcd */
void glcdSend(uint8_t dataOrCommand, unsigned char data) {
	if (dataOrCommand == 1) { // verifica se envio sera de dado ou de comando
		LPC_GPIO3->DATA |= (1 << 0); // estado lógico 1 - instrução para envio de dado
	} else {
		LPC_GPIO3->DATA &= ~(1 << 0); // estado lógico 0 - instrução para envio de comando
	}
	LPC_GPIO0->DATA &= ~(1 << 2); // estado lógico baixo
	spiSend(data); // envia dado ou comando para o slave
	LPC_GPIO0->DATA |= (1 << 2); // estado lógico alto
}

/** coloca caractere no glcd */
void glcdPutChar(char character) {
	unsigned int index;
	glcdSend(LCD_DATA, 0x00); // envia instrução ao glcd, informando que será passado um dado
	for (index = 0; index < 5; index++) {
		glcdSend(LCD_DATA, ASCII[character - 0x20][index]);
	}
	glcdSend(LCD_DATA, 0x00);
}

/** limpa tela do glcd */
void glcdClear(void) {
	unsigned int index;
	for (index = 0; index < GLCD_X * GLCD_Y / 8; index++) {
		glcdSend(LCD_DATA, 0x00);
	}
}

void glcdPutString(char *characters) {
	while (*characters) {
		glcdPutChar(*characters++);
	}
}

void glcdInit(void) {
	LPC_GPIO0->DATA &= ~(1 << 8);
	LPC_GPIO0->DATA |= (1 << 8); // alterna pino RST
	glcdSend(LCD_COMMAND, 0x21);  // comandos extendidos.
	glcdSend(LCD_COMMAND, 0xBF);  // seta contrast do glcd (Vop)
	glcdSend(LCD_COMMAND, 0x04);  // seta coeficiente de temperatura.
	glcdSend(LCD_COMMAND, 0x13);  // seta modo bias de 1:48.
	glcdSend(LCD_COMMAND, 0x20);  // comandos básicos
	glcdSend(LCD_COMMAND, 0x0C);  // glcd no modo normal
}
