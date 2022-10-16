/* 
 * File:   main.c
 * Author: kjw
 *
 * Created on May 6, 2022, 11:46 AM
 */

/****************************
 PRE-PROCESSING DIRECTIVES
 ****************************/

#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include "i2c_master_noint.h"
#include "ssd1306.h"
#include "font.h"

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

#define _1S_DELAY_COUNT 24000000.0
#define GREEN_LED LATAbits.LATA4
#define GREEN_LED_MODE TRISAbits.TRISA4

#define LETTER_WIDTH 5
#define LETTER_HEIGHT 8
#define SCREEN_X_LIMIT 128
#define MESSAGE_LEN 200


/****************************
 HELPER FUNCTION PROTOTYPES
 ****************************/
void generalSetup();
void i2cSetup();
void heartbeatSetup();
void heartbeat();
void blinkPixel(unsigned char x, unsigned char y);


void drawLetter(unsigned char x, unsigned char y, unsigned char letter);
void drawMessage(unsigned char x, unsigned char y, char * message);

/****************************
 GLOBAL VARIABLES
 ****************************/
float fps = 0;
char message[MESSAGE_LEN];

/*
 * 
 */
int main(int argc, char** argv) {
    
    __builtin_disable_interrupts(); // disable interrupts while initializing things
    
    ANSELB=0;
    generalSetup();
    heartbeatSetup();
    i2cSetup();
    ssd1306_setup();
    
    __builtin_enable_interrupts();
    
    
    
    while(1){
        _CP0_SET_COUNT(0);
//        heartbeat();
//        blinkPixel(5,30);
//        drawLetter(5, 15, 'A' );
//        drawMessage(0, 0, "Hello! Hello! Hello! Hello! Hello! Hello! Hello! Hello! Hello! Hello!Hello!Hello!Hello!Hello!Hello!Hello!");
        sprintf(message, "fps = %10.6f", fps);
        drawMessage(0, 24, message);
        fps = _1S_DELAY_COUNT/_CP0_GET_COUNT();
            
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
    GREEN_LED_MODE = 0;
    GREEN_LED = 1;
}

void i2cSetup(){
    i2c_master_setup(); // initialize I2C1 (used for comm w/ IMU)
}

void heartbeat(){
    static long startTime;
    startTime = _CP0_GET_COUNT();
    
    //BLOCKING 
    while ((_CP0_GET_COUNT() - startTime) < _1S_DELAY_COUNT){
        GREEN_LED = !GREEN_LED;
    }
//     //NON-BLOCKING
//    if ((_CP0_GET_COUNT() - startTime) < _1S_DELAY_COUNT){
//        GREEN_LED = !GREEN_LED;
//    }
}

void blinkPixel(unsigned char x, unsigned char y){
    static unsigned char color = 0;
    color = !color;
    ssd1306_drawPixel(x, y, color);
    ssd1306_update();
}

void drawLetter(unsigned char x, unsigned char y, unsigned char letter){
    int fontIndex = letter-0x20; 
    for (int i=0; i<LETTER_WIDTH; i++){
        for (int j=0; j<LETTER_HEIGHT; j++){
            ssd1306_drawPixel(x+i, y+j, ((ASCII[fontIndex][i] & (1<<j))>>j));
        }
    }
//    ssd1306_update();    
}

void drawMessage(unsigned char x, unsigned char y, char * message){
    unsigned char nextX = x;
    unsigned char nextY = y;
    int i=0;
    while(message[i]){
        if (nextX+5 > (SCREEN_X_LIMIT-1)){
            nextX = 0;
            nextY = nextY+8;
        }
        drawLetter(nextX, nextY, message[i]);
        i++;
        nextX += 5;
    }
    ssd1306_update();
}