

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

//#include "my_nrf.h"
#include "mirf.h"

#define LED_DEBUG_DDR DDRB
#define LED_DEBUG_PORT PORTB
#define LED_DEBUG_BIT (1<<3);


uint8_t address[5] = { 0xDE,0xAD,0xBE,0xEF,0x00};

void init_nrf_led_debug(void){
    
    //setup output
    LED_DEBUG_DDR |= LED_DEBUG_BIT;
    
    //set high for off
    LED_DEBUG_PORT |= LED_DEBUG_BIT;
    
}

int main(void)
{
    uint8_t W_buffer[32];
    uint8_t inbuffer[32];
    //init_nrf_SPI();
    init_nrf_led_debug();
    //init_nrf_INT0_IRQ();
    //init_nrf();
    
    mirf_init();
    _delay_ms(50);
    sei();
    
    mirf_config();
    
    mirf_set_TADDR(address);
    mirf_set_RADDR(address);
    
    
    LED_DEBUG_PORT &= ~LED_DEBUG_BIT;
    
    _delay_ms(1000);
    LED_DEBUG_PORT |= LED_DEBUG_BIT;
    
    W_buffer[0] = 64;
    W_buffer[1] = 32;
    W_buffer[2] = '\0';
    
    while(1){
        //PTX=0;
        LED_DEBUG_PORT &= ~LED_DEBUG_BIT;
        //transmit_nrf_payload(W_buffer);
        mirf_send(W_buffer,3);
        //_delay_ms(50);//wait a bit
        //while (!mirf_data_ready());
        //mirf_get_data(inbuffer);
        _delay_ms(100);//wait a bit
         LED_DEBUG_PORT |= LED_DEBUG_BIT;
    }
    return 0;
    
}
