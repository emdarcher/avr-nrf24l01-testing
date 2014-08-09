//lib based on parts of this blog post:
// http://gizmosnack.blogspot.com/2013/04/tutorial-nrf24l01-and-avr.html


#ifndef MY_NRF_H
#define MY_NRF_H

#include <string.h>

#define RF_CTRL_DDR DDRB
#define RF_CTRL_PORT PORTB
#define RF_CSN  (1<<4)
#define RF_CE   (1<<5)
#define RF_IRQ  (1<<6)

#define WRITE_BIT 1
#define READ_BIT 0

#define USING_ATTINY26
//#define USING_ATTINYx5
//#define USING_ATMEGAxx8

#define USING_INT0_IRQ 1

//prototypes

void init_nrf_SPI(void);
uint8_t write_read_byte_nrf_SPI(uint8_t cData);
uint8_t get_nrf_reg(uint8_t reg);
void write_to_nrf(uint8_t reg, uint8_t Package);
uint8_t *rw_nrf(uint8_t ReadWrite, uint8_t reg, uint8_t *val, uint8_t antVal);
void init_nrf(void);
void transmit_nrf_payload(uint8_t * W_buff);
void receive_nrf_payload(void);
void reset_nrf(void);

void init_nrf_INT0_IRQ(void);


#endif
