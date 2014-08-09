

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
    init_nrf();
    
    LED_DEBUG_PORT &= ~LED_DEBUG_BIT;
    _delay_ms(1000);
    LED_DEBUG_PORT |= LED_DEBUG_BIT;
    
    W_buffer[0] = 64;
    W_buffer[1] = 32;
    W_buffer[2] = '\0';
    
    while(1){
        
        transmit_nrf_payload(W_buffer);
        _delay_ms(50);//wait a bit
        
    }
    return 0;
}
