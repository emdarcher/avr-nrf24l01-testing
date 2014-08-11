

#include "my_nrf.h"

volatile uint8_t *data; //for interrupt stuff

void init_nrf_SPI(void){
    
    #ifdef USING_ATTINY26
        
        RF_CTRL_DDR |= (RF_CE | RF_CSN); //setup CE and CSN as outputs
        DDRB |= ((1<<PB1)|(1<<PB2));//set the USI DO and SCK pins as output
        
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
        while((USISR & (1<<USIOIF))==0 ){
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
    write_read_byte_nrf_SPI(W_REGISTER + reg); //R_Register,  set nrf to read mode
                                                //'reg' will be read back
    _delay_us(10);
    write_read_byte_nrf_SPI(Package); //send package to be written to "reg"
    _delay_us(10);
    RF_CTRL_PORT |= RF_CSN; //set CSN back to HIGH
    
}
uint8_t *rw_nrf(uint8_t ReadWrite, uint8_t reg, uint8_t *val, uint8_t antVal){
    
    //READ_BIT or WRITE_BIT, reg register, val package array, # ints in package
    
    if(ReadWrite==WRITE_BIT){
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
    
    int i;
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
    
    #if USING_INT0_IRQ==1
    //init_nrf_INT0_IRQ();
    #endif
    
    #if USING_LED_DEBUG==1
    //init_nrf_led_debug();
    #endif
    
    _delay_ms(100); //allow to reach power down if shut down
    
    uint8_t val[5]; //array of ints to send to *rw_nrf function
    
    //enable auto-ack, EN_AA
    //only works with identical addresses on transmitter and receiver
    //WRITE_BIT (write mode_, EN_AA reg, val 1, 1= len of data ints
    val[0]=0x01;
    rw_nrf(WRITE_BIT,EN_AA, val, 1); 
    
    //setup num of retries and delay
    //0b0010 1111 "2" sets 750uS delay between retries,
    //"F" is num retries, (1-15 now 15)
    val[0]=0x2F;
    rw_nrf(WRITE_BIT,SETUP_RETR, val, 1);
    
    //choose num of data pipes (1-5)
    val[0]=0x01;
    rw_nrf(WRITE_BIT, EN_RXADDR, val, 1); //EN pipe #0
    
    //RF_Address width setup (setup here as 5)
    val[0]=0b00000011;
    rw_nrf(WRITE_BIT,SETUP_AW,val,1);
    
    //RF channel setup, choose freq, 2.4 - 2.527 GHz 1MHz/step
    //setup as 120
    val[0]=120;
    rw_nrf(WRITE_BIT,RF_CH,val,1);
    
    //RF setup - choose pwr and data rate 
    //setup here for 250kbps and 0dBm pwr
    val[0] = ((1<<RF_DR_LOW)|0x06);
    rw_nrf(WRITE_BIT,RF_SETUP, val,1);
    
    //RX RF_Address setup 
    //memcpy(val, "\xDE\xAD\xBE\xEF\x00", 5);
    val[0]=0xDE;val[1]=0xAD;val[2]=0xBE;val[3]=0xEF;val[4]=0x00;
    rw_nrf(WRITE_BIT,RX_ADDR_P0,val,5); //pipe #0 address
    //set TX Address same in this setup.
    rw_nrf(WRITE_BIT,TX_ADDR, val,5);
    
    
    //Payload width
    val[0]=0x03;
    rw_nrf(WRITE_BIT,RX_PW_P0, val,1);
    
    //set for dynamic payload allocation/width whatever, stuff
    //enable it on pipe #0
    //val[0]=(1<<DPL_P0);
    //rw_nrf(WRITE_BIT, DYNPD, val, 1);
    //and enable it globally, and dynamic ack (ability for noack)
    val[0]=((1<<EN_DPL)|(1<<EN_DYN_ACK));
    rw_nrf(WRITE_BIT, FEATURE, val,1);
    //enable it on pipe #0
    val[0]=(1<<DPL_P0);
    rw_nrf(WRITE_BIT, DYNPD, val, 1);
    
    
    
    //CONFIG reg setup = boot nrf and choose rx or tx
    //set as tx, pwr up, and irq not triggered by transmittion failure
    val[0]=((1<<MASK_MAX_RT)|(1<<EN_CRC) |(1<<PWR_UP));
    rw_nrf(WRITE_BIT, CONFIG, val, 1);
                                                
    //device needs 1.5ms delay to reach standby mode (CE=low)
    _delay_ms(100); //why 100ms?
    
    #if USING_INT0_IRQ==1
    sei();//enable interrupts
    #endif
    
}
void transmit_nrf_payload(uint8_t * W_buff){
    
    rw_nrf(READ_BIT, FLUSH_TX, W_buff,0);//sends 0xe1 to flush registry
    rw_nrf(READ_BIT, W_TX_PAYLOAD, W_buff, sizeof(W_buff));//sends data to nrf
    //using READ b/c they are on hightest byte level.
    
    #if USING_INT0_IRQ==1
    sei(); //enable global interrupts
    #endif
    
    _delay_ms(10); //need 10ms delay for some reason
    RF_CTRL_PORT |= (RF_CE); //transmit the data, set CE high to enable
    _delay_us(20); //delay at least 10us!
    RF_CTRL_PORT &= ~(RF_CE);//CE low, stop transmitting
    _delay_ms(10); //delay before proceeding
}
void receive_nrf_payload(void){
    //for when in reciever mode
    //should be made more efficient
    
    #if USING_INT0_IRQ==1
    sei(); //enable global interrupts
    #endif
    
    RF_CTRL_PORT |= RF_CE; //make it listen for data
    _delay_ms(1000); //listen 1s at a time 
    RF_CTRL_PORT &= ~(RF_CE); //CE low, stop listening
    
    #if USING_INT0_IRQ==1
    cli(); //disable global interrupts
    #endif
    
}
void reset_nrf(void){
    
    _delay_us(10);
    RF_CTRL_PORT |= (RF_CSN); //CSN low
    _delay_us(10);
    write_read_byte_nrf_SPI(W_REGISTER + STATUS); //write status to registry
    _delay_us(10);
    write_read_byte_nrf_SPI(0x70); //reset all irqs in STATUS registry
    _delay_us(10);
    RF_CTRL_PORT |= RF_CSN; //CSN IR_High
    
}

void init_nrf_led_debug(void){
    
    //setup output
    LED_DEBUG_DDR |= LED_DEBUG_BIT;
    
    //set high for off
    LED_DEBUG_PORT |= LED_DEBUG_BIT;
    
}

void init_nrf_INT0_IRQ(void){
    #if USING_INT0_IRQ==1
    #ifdef USING_ATTINY26
        
        DDRB &= ~(1<<DDB6); //external interrupt int0, make sure it's input
        
        MCUCR |= (1<<ISC01); //into falling edge pb6
        MCUCR &= ~(1<<ISC00); //falling edge stuff
        
        GIMSK |= (1<<INT0); //enable INT0 interrupt
        
        //sei(); //enable global interrupts, should do later
        
    #endif
    #endif
}


#if USING_INT0_IRQ==1
//ISR for stuff

//triggered when tx succed or rx recieved data
ISR(INT0_vect)
{
    //Mask_Max_rt in config reg has to be set to stop from trigger on failed tx
    
    cli(); //disable global interrupts
    RF_CTRL_PORT &= ~(RF_CE); //CE low, stop sending/listening
    
    
    #if USING_LED_DEBUG==1
    LED_DEBUG_PORT &= ~LED_DEBUG_BIT; //low for on
    _delay_ms(100);
    LED_DEBUG_PORT |= LED_DEBUG_BIT; //high for off
    #endif
    
    //uncomment below if being a reciever
    //data=rw_nrf(R,R_RX_PAYLOAD,data,sizeof(data)); //read out rx msg
    
    reset_nrf(); //reset for further comm
    
    sei(); //enable global interrupts
}


#endif
