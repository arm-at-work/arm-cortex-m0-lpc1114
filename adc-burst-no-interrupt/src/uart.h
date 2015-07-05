/*
 * uart.h
 *
 *  Created at: 19/02/2015
 *      Author: Thiago Mallon <thiagomallon@gmail.com>
 */

#ifndef UART_H_
#define UART_H_

/* configura/inicializa a uart */
void uartInit(void) {
	/* configurações gerais */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 12); // habilita clock para periférico uart
	LPC_SYSCON->UARTCLKDIV = 2; // pclk_uart = c_clk / 2
	LPC_IOCON->PIO1_6 = ((LPC_IOCON->PIO1_6 & ~(0b11 << 0 | 0b11 << 3))
			| (0b01 << 0)); // desabilita pull-down; pino como rx
	LPC_IOCON->PIO1_7 = ((LPC_IOCON->PIO1_7 & ~(0b11 << 0 | 0b11 << 3))
			| (0b01 << 0)); // desabilita pull-down; pino como tx
	/* configurações do brg */
	LPC_UART->LCR |= (1 << 7); // seta dlab - permite acesso aos divisores do brg
	LPC_UART->DLL = ((SystemCoreClock / LPC_SYSCON->UARTCLKDIV) / (16 * 9600)); // lsb do divisor principal
	LPC_UART->DLM = 0; // msb do divisor principal
	LPC_UART->FDR |= (1 << 4); // divisor fracional
	LPC_UART->LCR &= ~(1 << 7); // limpa dlab - bloqueia acesso aos divisores do brg
	/* configurações do pacote */
	LPC_UART->LCR |= (0b11 << 0); // dado trafegado será de 8 bits
	LPC_UART->LCR &= ~(1 << 2 | 1 << 3); // 1 bit de parada; sem bit de paridade
	LPC_UART->FCR |= (1 << 0 | 1 << 1 | 1 << 2); // ativa e reinicia fifos
	LPC_UART->THR = 0; // limpa buffer tx
	LPC_UART->RBR; // limpa buffer rx
	LPC_UART->FCR &= ~(0b11 << 6); // dma habilitado - nível de disparo de 1 dado
	/* configurações de interrupção */
	LPC_UART->IER |= (1 << 0); // habilita interrupção por rbr
	NVIC_SetPriority(UART_IRQn, 2); // nível 2 de prioridade de interrupção
	NVIC_EnableIRQ(UART_IRQn); // habilita interrupções na uart
	NVIC_ClearPendingIRQ(UART_IRQn); // retira qualquer pendência de interrupção na uart
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

#endif /* UART_H_ */
