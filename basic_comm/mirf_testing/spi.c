/*
    Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

    $Id$
*/

#include "spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__)
#define PORT_SPI    PORTB
#define DDR_SPI     DDRB
#define DD_MISO     DDB4
#define DD_MOSI     DDB3
#define DD_SS       DDB2
#define DD_SCK      DDB5
#endif

#if defined(__AVR_ATtiny26__)

#define PORT_SPI PORTB
#define DDR_SPI DDRB
#define DD_MISO DDB0
#define DD_MOSI DDB1
#define DD_SS   DDB4
#define DD_SCK  DDB2

#endif



void spi_init()
// Initialize pins for spi communication
{
    #if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__)
    DDR_SPI &= ~((1<<DD_MOSI)|(1<<DD_MISO)|(1<<DD_SS)|(1<<DD_SCK));
    // Define the following pins as output
    DDR_SPI |= ((1<<DD_MOSI)|(1<<DD_SS)|(1<<DD_SCK));

    
    SPCR = ((1<<SPE)|               // SPI Enable
            (0<<SPIE)|              // SPI Interupt Enable
            (0<<DORD)|              // Data Order (0:MSB first / 1:LSB first)
            (1<<MSTR)|              // Master/Slave select   
            (0<<SPR1)|(1<<SPR0)|    // SPI Clock Rate
            (0<<CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
            (0<<CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)

    SPSR = (1<<SPI2X);              // Double Clock Rate
    #endif
    
    #if defined(__AVR_ATtiny26__)
    
    //#ifdef USING_ATTINY26
        
        //RF_CTRL_DDR |= (RF_CE | RF_CSN); //setup CE and CSN as outputs
        
        DDRB |= ((1<<PB1)|(1<<PB2));//set the USI DO and SCK pins as output
        
        DDRB &= ~(1<<PB0); //set USI DI for input
        PORTB |= (1<<PB0); //is this pullup needed???
        
        //USI stuff
        USICR |= ((1<<USIWM0)|(1<<USICS1)|(1<<USICLK));
        
        //RF_CTRL_PORT |= (RF_CSN); //set CSN high
        //RF_CTRL_PORT &= ~(RF_CE); //set CE LOW
        
    //#endif
    
    #endif
    
    
}

void spi_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len)
// Shift full array through target device
{
       uint8_t i;      
       for (i = 0; i < len; i++) {
             #if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__)
             SPDR = dataout[i];
             while((SPSR & (1<<SPIF))==0);
             datain[i] = SPDR;
             #endif
             
            #if defined(__AVR_ATtiny26__)
            //USIDR = cData;
            USIDR = dataout[i];
            USISR |= (1<<USIOIF); //clear flag to be able to recieve new data
        
            //wait for complete transmission
            while((USISR & (1<<USIOIF))==0 ){
                USICR |= (1<<USITC);  //Toggle SCK and count 4bit cnt 0-15,
                                    //USIOIF will be set when it overflows
            }
            datain[i] = USIDR;
            #endif
       }
}

void spi_transmit_sync (uint8_t * dataout, uint8_t len)
// Shift full array to target device without receiving any byte
{
       uint8_t i;      
       for (i = 0; i < len; i++) {
            #if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__)
             SPDR = dataout[i];
             while((SPSR & (1<<SPIF))==0);
            #endif
            
            #if defined(__AVR_ATtiny26__)
            USIDR = dataout[i];
            USISR |= (1<<USIOIF); //clear flag to be able to recieve new data
            //wait for complete transmission
            while((USISR & (1<<USIOIF))==0 ){
                USICR |= (1<<USITC);  //Toggle SCK and count 4bit cnt 0-15,
                                    //USIOIF will be set when it overflows
            }
            
            #endif
       }
}

uint8_t spi_fast_shift (uint8_t data)
// Clocks only one byte to target device and returns the received one
{
    #if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__)
    SPDR = data;
    while((SPSR & (1<<SPIF))==0);
    return SPDR;
    #endif
    
    #if defined(__AVR_ATtiny26__)
    USIDR = data;
            USISR |= (1<<USIOIF); //clear flag to be able to recieve new data
            //wait for complete transmission
            while((USISR & (1<<USIOIF))==0 ){
                USICR |= (1<<USITC);  //Toggle SCK and count 4bit cnt 0-15,
                                    //USIOIF will be set when it overflows
            }
            return USIDR;
    #endif
}

