#ifndef CURRENTCONTROL_H_
#define CURRENTCONTROL_H_


/*************************
 * PRE_PROCESSOR DIRECTIVES
*************************/
#include "NU32.h"
#include "utilities.h"
#include "ina219.h"
#include <stdio.h>
#include <string.h>
// #include "positioncontrol.h"


/*************************
 * CONSTANTS
*************************/
#define MOTOR_DIR LATDbits.LATD8
#define NUM_DATA_POINTS 100
#define ERROR_SUM_MAX 25


/*************************
 * PRIVATE GLOBAL VARIABLES
*************************/
static volatile int PWMDutyCycle = 0;
static volatile int motor_direction = 0;
static float P_current_control = 0.0; //0.05, 0.02;
static float I_current_control = 1.0; //q0.8, 0.8;
static volatile float desiredCurrent = 0;

static int itest_count = 0;
static volatile float error_sum = 0;

static char cbuff[100];


/*************************
 * PUBLIC GLOBAL VARIABLES
*************************/
int refCurrentArray[NUM_DATA_POINTS];
int actCurrentArray[NUM_DATA_POINTS];


/*************************
 * HELPER FUNCTION PROTOTYPES
*************************/
void currentControl_Startup();
void set_PWM(int);
int get_PWM();
void setCurrentGains(float p, float i);
float getCurrentP();
float getCurrentI();
float getDesiredCurrent();
void setDesiredCurrent(float current);


#endif