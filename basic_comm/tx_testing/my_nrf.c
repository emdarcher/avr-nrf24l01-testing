

#include "my_nrf.h"

void init_nrf_SPI(void){
    
    #ifdef USING_ATTINY26
        
        RF_CTRL_DDR |= (RF_CE | RF_CSN); //setup CE and CSN as outputs
        DDRB |= ((1<<PB1)|(1<<PB2))//set the USI DO and SCK pins as output
        
        DDRB &= ~(1<<PB0); //set USI DI for input
        PORTB |= (1<<PB0); //is this pullup needed???
        
        //USI stuff
        USICR |= ((1<<USIWM0)|(1<<USICS1)|(1<<USICLK));
        
        RF_CTRL_PORT |= (RF_CSN); //set CSN high
        RF_CTRL_PORT &= ~(RF_CE); //set CE LOW
        
    #endif
    
}

uint8_t write_read_byte_nrf_SPI(uint8_t cData){
    
    #ifdef USING_ATTINY26
        //load into data reg
        USIDR = cData;
        
        USISR |= (1<<USIOIF); //clear flag to be able to recieve new data
        
        //wait for complete transmission
        while(!(USISI & (1<<USIOIF)) ){
            USICR |= (1<<USITC);  //Toggle SCK and count 4bit cnt 0-15,
                                    //USIOIF will be set when it overflows
        }
        
        return USIDR;
    #endif
}

uint8_t get_nrf_reg(uint8_t reg){
    
    _delay_us(10); //wait since last command
    RF_CTRL_PORT &= ~(RF_CSN); //CSN low, nrf listening
    _delay_us(10);
    write_read_byte_nrf_SPI(R_REGISTER + reg); //R_Register,  set nrf to read mode
                                                //'reg' will be read back
    _delay_us(10);
    reg = write_read_byte_nrf_SPI(NOP); //send NOP (dummy byte) to recieve
                                        //1st byte in "reg"
    RF_CTRL_PORT |= RF_CSN; //set CSN back to HIGH
    return reg; //return the byte in reg
    
}
void write_to_nrf(uint8_t reg, uint8_t Package){
    
    _delay_us(10); //make sure last cmd was a bit ago
    RF_CTRL_PORT &= ~(RF_CSN); //CSN low, nrf listening
    _delay_us(10);
    write_read_byte_nrf_SPI(R_REGISTER + reg); //R_Register,  set nrf to read mode
                                                //'reg' will be read back
    _delay_us(10);
    write_read_byte_nrf_SPI(Package); //send package to be written to "reg"
    _delay_us(10);
    RF_CTRL_PORT |= RF_CSN; //set CSN back to HIGH
    
}
uint8_t *rw_nrf(uint8_t ReadWrite, uint8_t reg, uint8_t *val, uint8_t antVal){
    
    //READ_BIT or WRITE_BIT, reg register, val package array, # ints in package
    
    if(ReadWrite){
    //if WRITE_BIT=1, then we write, other wise nothin' b/c READ_BIT=0
        reg = W_REGISTER + reg; //add write bit to reg
    }
    
    //Create array to return at end
    //static is needed to return an array
    static uint8_t ret[32];
    
    _delay_us(10); //wait before stuff
    RF_CTRL_PORT &= ~(RF_CSN); //CSN low, nrf listening
    _delay_us(10);
    write_read_byte_nrf_SPI(reg); //set nrf to read or write mode 
    _delay_us(10);
    
    int i
    for(i=0;i<antVal;i++){
        if((!(ReadWrite)) && (reg != W_TX_PAYLOAD)){ //want to read?
                                                    //you can't add write bit.
            ret[i]= write_read_byte_nrf_SPI(NOP); //send dummy so we can rx data
            _delay_us(10);
        } else {
            write_read_byte_nrf_SPI(val[i]); //send cmds to nrf 1 at a time
            _delay_us(10);
        }
    }
    RF_CTRL_PORT |= (RF_CSN); //CSN high to make nrf do nothing
    
    return ret; //return the array thingy
}
void init_nrf(void){
    
    
    
}
void transmit_nrf_payload(uint8_t * W_buff){
    
    
    
}
void receive_nrf_payload(void){
    
    
    
}
void reset_nrf(void){
    
    
    
}

void init_nrf_INT0_IRQ(void){
    
    
    
}
