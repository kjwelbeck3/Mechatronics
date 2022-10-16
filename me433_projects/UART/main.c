/* 
 * File:   main.c
 * Author: kjw
 *
 * Created on April 5, 2022, 11:23 AM
 */

/****************************
 PRE-PROCESSING DIRECTIVES
 ****************************/

#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>

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

#define HALF_WAVE_PERIOD 12000000
#define NU32_DESIRED_BAUD 230400    // Baudrate for RS232
#define NU32_SYS_FREQ 48000000ul    // 48 million Hz
#define MESSAGE_LENGTH 200
#define DEBOUNCE_COUNT 1200000


/****************************
 HELPER FUNCTION PROTOTYPES
 ****************************/
void _Setup();
void blinkTwice();
void blinkMessage();
void UART1_Setup();
void readUART1(char * string, int maxLength);
void writeUART1(const char * string);

/****************************
 GLOBAL VARIABLES
 ****************************/
int transmit = 0;
int pressCount = 0;
char msg[MESSAGE_LENGTH];

/****************************
 MAIN FUNCTION
 ****************************/
int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    _Setup();
    UART1_Setup();
    
    __builtin_enable_interrupts();

    while (1) {
        blinkMessage();
    }
}


/****************************
 HELPER FUNCTIONS
 ****************************/

void _Setup(){
    
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISBbits.TRISB4 = 1;
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
}
void blinkTwice(){
    if (PORTBbits.RB4){
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = 0;
    }

    while(!PORTBbits.RB4){
        if (_CP0_GET_COUNT() < HALF_WAVE_PERIOD){
            LATAbits.LATA4 = 1;
        }
        else if (_CP0_GET_COUNT() < 2 * HALF_WAVE_PERIOD){
            LATAbits.LATA4 = 0;
        }
        else if (_CP0_GET_COUNT() < 3 * HALF_WAVE_PERIOD){
            LATAbits.LATA4 = 1;
        }
        else if (_CP0_GET_COUNT() < 4 * HALF_WAVE_PERIOD){
            LATAbits.LATA4 = 0;
        }
    }
}

void UART1_Setup(){
  // turn on UART1 without an interrupt
  U1MODEbits.BRGH = 0; // set baud to NU32_DESIRED_BAUD
  U1BRG = ((NU32_SYS_FREQ / NU32_DESIRED_BAUD) / 16) - 1;

  // 8 bit, no parity bit, and 1 stop bit (8N1 setup)
  U1MODEbits.PDSEL = 0;
  U1MODEbits.STSEL = 0;

//  // configure TX & RX pins as output & input pins
  U1STAbits.UTXEN = 1;
  U1STAbits.URXEN = 1; 
  
  // configure 
  U1RXRbits.U1RXR = 0b0001; // Set B6 (pin 15) to U1RX
  RPB7Rbits.RPB7R = 0b0001; // Set B7 (pin 16) to U1TX
  
  
  // hardware flow control: disable RTS and CTS
  U1MODEbits.UEN = 0;

  // enable the uart
  U1MODEbits.ON = 1;

}

// Read from UART1
// block other functions until you get a '\r' or '\n'
// send the pointer to your char array and the number of elements in the array
void readUART1(char * string, int maxLength){
  char data = 0;
  int complete = 0, num_bytes = 0;
  
  // loop until you get a '\r' or '\n'
  while (!complete) {
    if (U1STAbits.URXDA) { // if data is available
      data = U1RXREG;      // read the data
      if ((data == '\n') || (data == '\r')) {
        complete = 1;
      } else {
        string[num_bytes] = data;
        ++num_bytes;
        // roll over if the array is too small
        if (num_bytes >= maxLength) {
          num_bytes = 0;
        }
      }
    }
  }
  // end the string
  string[num_bytes] = '\0';   
}

// Write a character array using UART1
void writeUART1(const char * string){
  while (*string != '\0') {
    while (U1STAbits.UTXBF) {
      ; // wait until tx buffer isn't full
    }
    U1TXREG = *string;
    ++string;
  }
}

void blinkMessage(){
    if (PORTBbits.RB4){
        _CP0_SET_COUNT(0);
        if (transmit){
            transmit = 0;
        }
    }
    else{
        /// DEBOUNCE THEN PRINT TO SERIAL
        if (_CP0_GET_COUNT() > DEBOUNCE_COUNT){
            if (!transmit){
                pressCount++;
                sprintf(msg, "Button pressed! [%d]\r\n", pressCount);
                writeUART1(msg);
                transmit = 1;
            }
        }
    }
}
