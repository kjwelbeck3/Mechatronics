#ifndef POSITIONCONTROL_H_
#define POSITIONCONTROL_H_


/*************************
 * PRE PROCESSOR DIRECTIVES
*************************/

#include "NU32.h"
#include "utilities.h"
#include "encoder.h"
#include "currentcontrol.h"

/*************************
 * CONSTANTS
*************************/

#define ANGLE_ERROR_SUM_MAX 10
#define DT 0.005
#define MAX_REF_TRAJ_LENGTH 2000
#define PBUFF_SIZE 200

/*************************
 * PRIVATE GLOBAL VARIABLES
*************************/
static volatile float P_position_control = 30.0; 
static volatile float I_position_control = 5;
static volatile float D_position_control = 8.0;

static volatile int desired_angle = 0;
static volatile float angle_error_sum = 0;
static volatile float prev_angle = 0;
static char pbuffer[PBUFF_SIZE];

static volatile int encCount;
static volatile float curr_angle;
static volatile float angle_error;
static volatile float angle_rate;
static volatile float dCurrent;

/*************************
 * PUBLIC GLOBAL VARIABLES
*************************/
float referenceTrajectory[MAX_REF_TRAJ_LENGTH];
volatile int referenceTrajectoryLength;
float actPositionArray[MAX_REF_TRAJ_LENGTH];
float refPositionArray[MAX_REF_TRAJ_LENGTH];



/*************************
 * HELPER FUNCTION PROTOTYPES
*************************/

void positionControl_Startup();

void setPositionGains(float p, float i, float d);
float getPositionP();
float getPositionI();
float getPositionD();

void setDesiredAngle(int angle);
int getDesiredAngle();




#endif