/*
 * nRF24L01.h
 *
 *  Created at: 30/01/2015
 *      Author: Lilian Massuda
 */

#ifndef NRF24L01_H_
#define NRF24L01_H_

/* configurações gerais */
#define CHANNEL 107 // 1 a 126
#define ENDTX 75  // 1 a 255
#define ENDRX 75  // 1 a 255
#define BUFFER 10 // tamanho do dado trafegado

/* registradores */
#define RX_ADDR_P0 		0x2A
#define TX_ADDR 		0x30
#define EN_AA 			0X21
#define EN_RXADDR 		0x22
#define SETUP_AW 		0x23
#define SETUP_RETR 		0x24
#define RF_CH 			0x05
#define RF_SETUP 		0x26
#define STATUS 			0X27
#define RX_PW_P0 		0x31
#define CONFIG 			0x20
#define W_TX_PAYLOAD  	0xA0
#define FLUSH_TX      	0xE1
#define FLUSH_RX		0xE2

/* variáveis globais */
uint8_t receivingData[BUFFER] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // matriz armazena dados recebidos

/* protótipos */
void delayUs(unsigned int);
void spiSend(unsigned char); // envia dado para buffer tx
uint8_t spiRead(unsigned char); // envia dado para buffer tx

/* configura/inicializa nRF24L01 */
void nRF24L01Init() {
	// RX_ADDR_P0 - configura endere�o de recep��o PIPE0
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(RX_ADDR_P0);
	spiSend(ENDRX);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	/* não necessário */
	// TX_ADDR - configura endere�o de transmiss�o
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(TX_ADDR);
	spiSend(ENDTX);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	spiSend(0xC2);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	// SETUP_RETR - configura para nao retransmitir pacotes
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(SETUP_RETR);
	spiSend(0x00);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	// EN_AA - habilita autoACK no PIPE0
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(EN_AA);
	spiSend(0x01);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo
	/* fim do não necessário */

	// EN_RXADDR - ativa o PIPE0
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(EN_RXADDR);
	spiSend(0x01);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	// SETUP_AW - define o endere�o com tamanho de 5 Bytes
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(SETUP_AW);
	spiSend(0x03);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	// RF_CH - define o canal do modulo (TX e RX devem ser iguais)
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(RF_CH);
	spiSend(CHANNEL);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	// RF_SETUP - ativa LNA, taxa em 250K, e maxima potencia 0dbm
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(RF_SETUP);
	spiSend(0b00100110);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	// STATUS - reseta o resgistrador STATUS
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(STATUS);
	spiSend(0x70);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	// RX_PW_P0 - tamanho do buffer PIPE0
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(RX_PW_P0);
	spiSend(10);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	// CONFIG - coloca em modo de recep��o, e define CRC de 2 Bytes
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(CONFIG);
	spiSend(0x0F);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	// tempo para sair do modo standby entrar em modo de recep�ao
	LPC_GPIO0->DATA |= (1 << 2); // CE estado lógico alto
	delayUs(10);
}

/* recebe dado */
void nRF24L01ReceiveData() {
	uint8_t i;
	// STATUS - clear register
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x27);
	spiSend(0x70);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	//R_RX_PAYLOAD - receive data from FIFO RX buffer
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0x61);
	for (i = 0; i < BUFFER; i++) {
		receivingData[i] = spiRead(0xFF);
	}
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	// LIMPA FLUSH_RX
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(0xE2);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo

	delayUs(10);
}

/* testa leitura de registrador do nRF24L01+ */
void nRF24L01Test(uint8_t reg) {
	delayUs(10);
	LPC_GPIO3->DATA &= ~(1 << 0); // CSN estado lógico baixo
	spiSend(reg);
	reg = spiRead(0x00);
	LPC_GPIO3->DATA |= (1 << 0); // CSN estado lógico baixo
	printf("%s%u\n", "Valor lido: ", reg);
}

#endif /* NRF24L01_H_ */
