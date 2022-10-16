/* 
 * File:   main.c
 * Author: kjw
 *
 * Created on May 5, 2022, 11:22 AM
 */


/****************************
 PRE-PROCESSING DIRECTIVES
 ****************************/

#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include "i2c_master_noint.h"


// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use fast frc oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = OFF // primary osc disabled
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt value
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz fast rc internal oscillator
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0x0909 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations


#define HEARTBEAT_FREQUENCY 12000000
#define HEARTBEAT_LED LATAbits.LATA4
#define HEARTBEAT_LED_MODE TRISAbits.TRISA4

#define READ_ADDRESS 0b01000001
#define WRITE_ADDRESS 0b01000000
#define _7BIT_ADDRESS 0b0100000

#define I2C_IODIR 0x00
#define I2C_OLAT 0x0A
#define I2C_GPIO 0x09

#define LOW 0

/****************************
 HELPER FUNCTION PROTOTYPES
 ****************************/
void generalSetup();
void IOExpanderSetup();
void heartbeatSetup();
void heartbeat();

unsigned char readButton();
void LED_On();
void LED_Off();
unsigned char readRegister(unsigned char address, unsigned char _register);
void setRegister(unsigned char address, unsigned char _register, unsigned char value);


/****************************
 GLOBAL VARIABLES
 ****************************/


/*
 * 
 */
int main(int argc, char** argv) {
    
    __builtin_disable_interrupts(); // disable interrupts while initializing things
    
    ANSELB=0;
    generalSetup();
    heartbeatSetup();
    IOExpanderSetup();
    
    __builtin_enable_interrupts();
    
    while(1){
        heartbeat();
//        LED_On();
//        LED_Off();
        unsigned char gpios = readButton(); 
        if (readButton() == LOW)
        
        {
            LED_On();
        }
        else {
            LED_Off();
        }
    }
    return (EXIT_SUCCESS);
}


/****************************
 HELPER FUNCTIONS
 ****************************/

void generalSetup(){
    
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
}

void heartbeatSetup(){
    HEARTBEAT_LED_MODE = 0;
    HEARTBEAT_LED = 1;
    _CP0_SET_COUNT(0); 
}

void heartbeat(){
    
    // BLOCKING
    while(_CP0_GET_COUNT() < HEARTBEAT_FREQUENCY){;}
    HEARTBEAT_LED = !HEARTBEAT_LED;
    _CP0_SET_COUNT(0);
  
//    // NONBLOCKING
//    if(_CP0_GET_COUNT() > HEARTBEAT_FREQUENCY){
//    HEARTBEAT_LED = !HEARTBEAT_LED;
//    _CP0_SET_COUNT(0);
//    }
    
}

void IOExpanderSetup(){
    
// Set PIC to be master on I2C bus
    i2c_master_setup();
    
// Set the LED(I07) to Output
    i2c_master_start();
    i2c_master_send(WRITE_ADDRESS);
    i2c_master_send(I2C_IODIR);
    i2c_master_send(0x7F);
    i2c_master_stop();

// LED Output: Low by default
}

unsigned char readButton(){
//    i2c_master_start();
//    i2c_master_send(WRITE_ADDRESS);
//    i2c_master_send(I2C_GPIO);
//    i2c_master_restart();
//    
//    i2c_master_send(READ_ADDRESS);
//    unsigned char _gpio = i2c_master_recv();
//    i2c_master_ack(1);
//    i2c_master_stop();
    return readRegister(_7BIT_ADDRESS, I2C_GPIO) & 0b1;
//    return _gpio & 0b1;
    
}

void LED_On(){
//    // Set the LED Output (OL7) to 1
//    i2c_master_start();
//    i2c_master_send(WRITE_ADDRESS);
//    i2c_master_send(I2C_OLAT);
//    i2c_master_send(0b10000000);
//    i2c_master_stop();
    
    setRegister(_7BIT_ADDRESS, I2C_OLAT, 0b10000000);
    
}

void LED_Off(){
//    // Set the LED Output (OL7) to 0
//    i2c_master_start();
//    i2c_master_send(WRITE_ADDRESS);
//    i2c_master_send(I2C_OLAT);
//    i2c_master_send(0);
//    i2c_master_stop();
    
    setRegister(_7BIT_ADDRESS, I2C_OLAT, 0);

}

void setRegister(unsigned char address, unsigned char _register, unsigned char value){
    
    unsigned char _8bitWriteAddress = (address << 1);
    i2c_master_start();
    i2c_master_send(_8bitWriteAddress);
    i2c_master_send(_register);
    i2c_master_send(value);
    i2c_master_stop();
}

unsigned char readRegister(unsigned char address, unsigned char _register){
    
    unsigned char _8bitWriteAddress = (address << 1);
    unsigned char _8bitReadAddress = ((address << 1) | 0b1);
    i2c_master_start();
    i2c_master_send(_8bitWriteAddress);
    i2c_master_send(_register);
    i2c_master_restart();
    
    i2c_master_send(_8bitReadAddress);
    unsigned char regVal = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    
    return regVal;
}