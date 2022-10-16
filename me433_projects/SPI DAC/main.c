/* 
 * File:   main.c
 * Author: kjw
 *
 * Created on April 21, 2022, 2:00 PM
 */

/****************************
 PRE-PROCESSING DIRECTIVES
 ****************************/

#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include "spi.h"
#include <math.h>

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


#define SINE_FREQ 2000
#define TRIANGLE_FREQ 1000
#define WAVEFORM_LENGTH 50

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/****************************
 HELPER FUNCTION PROTOTYPES
 ****************************/
void _Setup();
unsigned short gen_voltageMsg(char A_or_B, unsigned char voltage);
void genSineWave(float frequency, float amplitude);
void genTriangleWave(float frequency, float amplitude);
void sendVoltageChar(unsigned short);

/****************************
 GLOBAL VARIABLES
 ****************************/

unsigned char waveform_sine[] = {128, 159, 189, 215, 236, 249, 255, 253, 243, 
226, 203, 175, 144, 111, 80, 52, 29, 12, 2, 0, 6, 19, 40, 66, 96, 127, 159, 
189, 215, 236, 249, 255, 253, 243, 226, 203, 175, 144, 111, 80, 52, 29, 12, 2, 
0, 6, 19, 40, 66, 96};

unsigned char waveform_triangle[] = {0, 10, 20, 30, 40, 51, 61, 71, 81, 91, 102,
112, 122, 132, 142, 153, 163, 173, 183, 193, 204, 214, 224, 234, 244, 255, 244, 
234, 224, 214, 204, 193, 183, 173, 163, 152, 142, 132, 122, 112, 101, 91, 81, 
71, 61, 50, 40, 30, 20, 10};

/*
 * 
 */
int main(int argc, char** argv) {
    
    __builtin_disable_interrupts(); // disable interrupts while initializing things

    _Setup();
    initSPI();
//    genSineWave(2000, 128);
//    genTriangleWave(2000, 255);
    
    __builtin_enable_interrupts();
    
    unsigned char i = 0;
    
    while(1){

        sendVoltageChar(gen_voltageMsg(0b0, waveform_sine[i]));
        sendVoltageChar(gen_voltageMsg(0b1, waveform_triangle[i]));
        
        
        i++;
        if (i >= WAVEFORM_LENGTH){
            i = 0;
        }
        _CP0_SET_COUNT(0);
        while(_CP0_GET_COUNT()< 24000000/WAVEFORM_LENGTH ){;}
       
    }

    return (EXIT_SUCCESS);
}


/****************************
 HELPER FUNCTIONS
 ****************************/

void _Setup(){
    
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // Turning off Analog precedence
    ANSELB = 0;
    
    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

}

unsigned short gen_voltageMsg(char A_or_B, unsigned char voltage){
    
    unsigned short voltageMsg = 0;
    voltageMsg = voltageMsg|(A_or_B << 15);
    
    unsigned char buff_ga_shdwn = 0b111;
    voltageMsg = voltageMsg|(buff_ga_shdwn << 12);
    
    voltageMsg = voltageMsg|(voltage << 4);
    
    return voltageMsg;  
}

//void genSineWave(float freq, float amplitude){ 
//    float dt = (1.0/freq)/WAVEFORM_LENGTH;
//    for (int i=0; i< WAVEFORM_LENGTH; i++){
//        waveform_sine[i] = amplitude*sin(dt*i*freq*2*M_PI);  
//    }
//}
//
//void genTriangleWave(float freq, float amplitude){
//    float dt = (1.0/freq)/WAVEFORM_LENGTH;
//    float grad = amplitude*2*freq;
//    for (int i=0; i< WAVEFORM_LENGTH; i++){
//        waveform_triangle[i] = MIN(dt*i*grad, amplitude*2-dt*i*grad);
//    }
//}

void sendVoltageChar(unsigned short voltage){
        LATBbits.LATB9 = 0;  
        spi_io(voltage>>8);
        spi_io(voltage);
        LATBbits.LATB9 = 1; 
}