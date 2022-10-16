/* 
 * File:   main.c
 * Author: kjw
 *
 * Created on May 6, 2022, 4:38 PM
 */

/****************************
 PRE-PROCESSING DIRECTIVES
 ****************************/

#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include "ws2812b.h"

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


//#define CONTROL_OUTPUT LATAbits.LATA1
//#define CONTROL_OUTPUT_MODE TRISAbits.TRISA1
#define _1S_COUNT 24000000

/****************************
 HELPER FUNCTION PROTOTYPES
 ****************************/
void generalSetup();
//void gpioSetup();

// Changing the array one neopixel at a time to a common color
// Cycling through different colors
void rainbowChase();

// Changing all neopixels to a single color all at once
// Cycling through different colors
void cyclingAll();


/****************************
 GLOBAL VARIABLES
 ****************************/
wsColor testColor = {.r = 0, .g=0 , .b = 0};
wsColor display[5];
float h = 12;

/*
 * 
 */
int main(int argc, char** argv) {
    
    __builtin_disable_interrupts(); // disable interrupts while initializing things
    generalSetup();
    ws2812b_setup();
    __builtin_enable_interrupts(); 
    
//    cyclingAll();
    rainbowChase();
    
    return (EXIT_SUCCESS);
}

/****************************
 HELPER FUNCTIONS
 ****************************/

void generalSetup(){
    
    // Disable Analog inputs on Port A
    ANSELA=0;
    
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
}

void rainbowChase(){
    
    // initialize display color
    h = 0;
    for (int i=0; i < 5; i++){
        display[i] = HSBtoRGB(h, 1, 1);
    }
    ws2812b_setColor(display, 5);
    
    while (1){
        
        // cycling through different colors
        h+=60;
        if (h>=360){
            h = 30;
        }
        for (int i=0; i<5; i++){
            display[i] = HSBtoRGB(h, 1, 1);
            
            // crude delay 
            long t = 0;
            while (t < 300000){
                t++;
            } 
            ws2812b_setColor(display, 5);
        }
    }
}

void cyclingAll(){
    
    // hues to cycle through 
    float hues[] = {0, 60, 120, 180, 240, 300, 358};
    int index = 0;
    
    while(1){
        
        // stock set color
        for (int i=0; i < 5; i++){
            display[i] = HSBtoRGB(hues[index], 1, 1);
        }
        ws2812b_setColor(display, 5);
        index++;
        if (index > 4){index = 0;}
        
        // crude delay
        long t = 0;
        while (t < 40000000){
            t++;
        }
    }
}