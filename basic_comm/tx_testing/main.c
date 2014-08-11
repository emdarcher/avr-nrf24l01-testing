

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include "my_nrf.h"



int main(void)
{
    uint8_t W_buffer[32];
    
    init_nrf_SPI();
    init_nrf_led_debug();
    init_nrf_INT0_IRQ();
    init_nrf();
    
    LED_DEBUG_PORT &= ~LED_DEBUG_BIT;
    _delay_ms(1000);
    LED_DEBUG_PORT |= LED_DEBUG_BIT;
    
    DDRA |= (1<<DDA0);
    PORTA |= (1<<PA0);
    
    W_buffer[0] = 64;
    W_buffer[1] = 32;
    W_buffer[2] = '\0';
    
    while(1){
        transmit_nrf_payload(W_buffer);
        uint8_t tempreg_status = get_nrf_reg(STATUS);
        /*
        if((tempreg_status & (1<<TX_DS)) !=0){
            reset_nrf();
            LED_DEBUG_PORT &= ~LED_DEBUG_BIT; //low for on
    _delay_ms(100);
    LED_DEBUG_PORT |= LED_DEBUG_BIT; //high for off
    
        }*/
        if((tempreg_status & (1<<4)) != 0){
            transmit_nrf_payload(W_buffer);
            PORTA &= ~(1<<PA0);
            //uint8_t tempreg = get_nrf_reg(STATUS);
            tempreg_status |= (1<<4);
            write_to_nrf(STATUS, tempreg_status);
            //_delay_ms(50);
        }
        /*if((get_nrf_reg(STATUS) & (1<<TX_DS)) !=0){
            reset_nrf();
            LED_DEBUG_PORT &= ~LED_DEBUG_BIT; //low for on
    _delay_ms(100);
    LED_DEBUG_PORT |= LED_DEBUG_BIT; //high for off
    
        }*/
        
        PORTA |= (1<<PA0);
        _delay_ms(50);//wait a bit
        
    }
    return 0;
}
