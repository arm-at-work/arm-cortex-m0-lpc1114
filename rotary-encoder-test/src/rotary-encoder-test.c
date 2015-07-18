/*
 ===============================================================================
 Name        : rotary-encoder-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Copyright   : MIT
 Created at  : 2015-7-14
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código à memória flash

/* protótipos */
void rotaryEncoderPinsConfig(void);
void rotaryEncoderIntConfig(void);
void uartInit(void);
void uartSend(unsigned char);
void sysTickConfig(void);
void delayUs(unsigned int);

struct {
	uint8_t left :1;
	uint8_t right :1;
	char index :8;
} direction;

char encDir = 0;

/* isr de interrupções externas do port 1 */
void PIOINT1_IRQHandler(void) {
	/*
	 if (LPC_GPIO1->MIS & (1 << 9)) { // verifica se flag do pino está setada
	 if (!(LPC_GPIO3->DATA & (1 << 5))) { // verifica movimento à direita
	 encDir -= 1;
	 printf("%s%d\n", "mov. dir - pos: ", encDir);
	 }
	 }
	 delayUs(250000);
	 LPC_GPIO1->IC |= (1 << 9); // limpa flag de interrupção do pino
	 */
	printf("%s\n", "esquerda");
	delayUs(77000);
	LPC_GPIO1->IC |= (1 << 9); // limpa flag de interrupção do pino
}

/* isr de interrupções externas do port 3 */
void PIOINT3_IRQHandler(void) {
	/*
	 if (LPC_GPIO3->MIS & (1 << 5)) { // verifica se flag do pino está setada
	 if (!(LPC_GPIO1->DATA & (1 << 9))) { // verifica movimento à esquerda
	 encDir += 1;
	 printf("%s\n", "mov. left - pos: ", encDir);
	 }
	 }
	 delayUs(250000);
	 LPC_GPIO3->IC |= (1 << 5); // limpa flag de interrupção do pino
	 */
	printf("%s\n", "direita");
	delayUs(77000);
	LPC_GPIO3->IC |= (1 << 5); // limpa flag de interrupção do pino
}

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 48mhz, etc
	SystemCoreClockUpdate(); // atualiza valor de clock da cpu, para a variável SystemCoreClock
	rotaryEncoderPinsConfig(); // configura pinos do rotary encoder
	//rotaryEncoderIntConfig(); // configura interrupção nos pinos do rotary encoder
	uartInit(); // configura/inicializa a uart
	do { // loop infinito
		if (!(LPC_GPIO1->DATA & (1 << 9))) {
			printf("%s\n", "esquerda");
			delayUs(77000);
		}
		if (!(LPC_GPIO3->DATA & (1 << 5))) {
			printf("%s\n", "direita");
			delayUs(77000);
		}
	} while (1);
	return 1;
}

/* configura pinos do rotary encoder */
void rotaryEncoderPinsConfig(void) {
	/* pino p1.9 (como A) */
	LPC_IOCON->PIO1_9 = ((LPC_IOCON->PIO1_9 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // habilita pull-up; pino como gpio
	LPC_GPIO1->DIR &= ~(1 << 9); // pino como entrada
	LPC_GPIO1->DATA |= (1 << 9); // estado lógico alto
	/* pino p3.5 (como B) */
	LPC_IOCON->PIO3_5 = ((LPC_IOCON->PIO3_5 & ~(0b11 << 3 | 0b11 << 0))
			| (0b10 << 3)); // habilita pull-up; pino como gpio
	LPC_GPIO3->DIR &= ~(1 << 5); // pino como entrada
	LPC_GPIO3->DATA |= (1 << 5); // estado lógico alto
}

/* configura interrupção dos pinos do rotary encoder */
void rotaryEncoderIntConfig(void) {
	/* port1 */
	LPC_GPIO1->IE |= (1 << 9); // habilita interrupção para o pino
	LPC_GPIO1->IS &= ~(1 << 9); // interrupção ocorrerá por mudança de borda no pino
	LPC_GPIO1->IBE &= ~(1 << 9); // evento de interrupção não ocorrerá nas duas bordas
	LPC_GPIO1->IEV &= ~(1 << 9); // evento de interrupção ocorrerá somente na borda da descida
	NVIC_SetPriority(EINT1_IRQn, 2); // nível 2 de prioridade de interrupção externa no port 1
	NVIC_ClearPendingIRQ(EINT1_IRQn); // retira qualquer pendência de interrupção externa no port 1
	NVIC_EnableIRQ(EINT1_IRQn); // habilita interrupções externas no port1
	/* port3 */
	LPC_GPIO3->IE |= (1 << 5); // habilita interrupção para o pino
	LPC_GPIO3->IS &= ~(1 << 5); // interrupção ocorrerá por mudança de borda no pino
	LPC_GPIO3->IBE &= ~(1 << 5); // evento de interrupção não ocorrerá nas duas bordas
	LPC_GPIO3->IEV &= ~(1 << 5); // evento de interrupção ocorrerá somente na borda da descida
	NVIC_SetPriority(EINT3_IRQn, 2); // nível 2 de prioridade de interrupção no port 3
	NVIC_ClearPendingIRQ(EINT3_IRQn); // retira qualquer pendência de interrupção no port3
	NVIC_EnableIRQ(EINT3_IRQn); // habilita interrupções externas no port3
}

/* configura/inicializa a uart */
void uartInit(void) {
	/* configurações gerais */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12); // habilita clock para periférico uart
	LPC_SYSCON->UARTCLKDIV = 2; // pclk_uart = c_clk / 2
	LPC_IOCON->PIO1_6 = ((LPC_IOCON->PIO1_6 & ~(0b11 << 0)) | (0b01 << 0)); // pino como rx da uart
	LPC_IOCON->PIO1_7 = ((LPC_IOCON->PIO1_7 & ~(0b11 << 0)) | (0b01 << 0)); // pino como tx da uart
	/* configurações do brg */
	LPC_UART->LCR |= (1 << 7); // seta dlab - permite acesso aos divisores principais do brg
	LPC_UART->DLL = ((SystemCoreClock / LPC_SYSCON->UARTCLKDIV) / (16 * 9600)); // lsb do brg
	LPC_UART->DLM = 0;
	LPC_UART->FDR |= (1 << 4); // fractional divider - cálculo: ((pclk_uart / (16 * 9600 * DLL)) -1); - segundo valor encontrado, procurar na tabela do fabricante
	LPC_UART->LCR &= ~(1 << 7); // limpa dlab - bloqueia acesso aos divisores principais do brg
	/* configurações do pacote */
	LPC_UART->LCR |= (0b11 << 0); // dado trafegado será de 8 bits
	LPC_UART->LCR &= ~(1 << 2 | 1 << 3); // 1 bit de parada; sem bit de paridade
	LPC_UART->FCR |= (1 << 0 | 1 << 1 | 1 << 2); // ativa e reinicia fifos
	LPC_UART->THR = 0; // limpa buffer tx
	LPC_UART->RBR; // limpa buffer rx
	LPC_UART->FCR &= ~(0b11 << 6); // dma habilitado - nível de disparo de 1 dado
	/* configurações de interrupção */
	LPC_UART->IER |= (1 << 0); // habilita interrupção por rbr
	NVIC_SetPriority(UART_IRQn, 2); // nível 2 de prioridade de interrupção na uart
	NVIC_ClearPendingIRQ(UART_IRQn); // retira qualquer pendência de interrupção na uart
	NVIC_EnableIRQ(UART_IRQn); // habilita interrupções na uart
}

/* envia dado para buffer tx */
void uartSend(unsigned char data) {
	do { // aguarda até que buffer tx possa receber novos dados
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

/* configura sysTick */
void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; // atribui valor de reinício ao contador
	SysTick->CTRL |= (1 << 2); // fonte de decremento é c_clk
}

/* implementa delay em us */
void delayUs(unsigned int us) {
	unsigned int count; // contador de interrupções
	SysTick->CTRL |= (1 << 0); // habilita contador
	for (count = 0; count < us; count++) { // aguarda até que valor do contador se iguale ao valor recebido
		do { // aguarda até que flag de interrupção seja setada
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0); // para contador
}
