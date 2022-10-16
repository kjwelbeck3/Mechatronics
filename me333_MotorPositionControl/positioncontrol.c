#include "positioncontrol.h"
#include "currentcontrol.h"
#include <stdio.h>

/*************************
 * HELPER FUNCTIONS
*************************/

void zero_encoder_count()
{
  WriteUART2("b");
  set_encoder_flag(0);
}

int request_encoder_position()
{
  WriteUART2("a"); // asking for position
  while (!get_encoder_flag()){;}
  set_encoder_flag(0);
  int pos = get_encoder_count();
  return pos;
}

void positionControl_Startup()
{
    // setup 200 HZ interrupt on Timer 4 for position control

    T4CONbits.TCKPS = 0b110; // Timer4 prescaler N=64 (1:64)
    PR4 = 6249;              // period = (PR2+1) * N * 12.5 ns = 5 ms, 200 Hz
    TMR4 = 0;                // initialize TMR4 count to 0
    T4CONbits.TGATE = 0;     // gated accumualtion mode disabled
    IPC4 = 20;               // priority level 5
    IFS0bits.T4IF = 0;       // clear Timer4 interrupt flag
    IEC0bits.T4IE = 1;       // enable Timer4 interrupt

    // Enable timers and pwm
    T4CONbits.ON = 1;  // turn on the timer4
}


void setPositionGains(float p, float i, float d)
{
    P_position_control = p;
    I_position_control = i;
    D_position_control = d;
}

float getPositionP()
{
    return P_position_control;
}

float getPositionI()
{
    return I_position_control;
}

float getPositionD()
{
    return D_position_control;
}

void setDesiredAngle(int angle)
{
    prev_angle = 0;
    desired_angle = angle;
    angle_error_sum = 0;    
}

int getDesiredAngle()
{
    return desired_angle;
}

/*************************
 * INTERRUPT SERVICE ROUTINES
*************************/
void __ISR(_TIMER_4_VECTOR, IPL5SOFT) PositionController(void) // _TIMER_4_VECTOR = 16
{

    // // Test - toggle LED2
    // NU32_LED1 = !NU32_LED1;

    // Position Control

    if (get_mode() == HOLD)
    {
        static int hold_count = 0;
        encCount = request_encoder_position();

        curr_angle = 360.0/(334*4) * encCount;
        angle_error = desired_angle - curr_angle;
        angle_error_sum += angle_error;
        angle_rate = (prev_angle - curr_angle)/DT;
        // angle_rate = (curr_angle - prev_angle)/DT;
        // angle_rate = (curr_angle - prev_angle);
        prev_angle = curr_angle;

        angle_error_sum = angle_error_sum > ANGLE_ERROR_SUM_MAX ? ANGLE_ERROR_SUM_MAX : angle_error_sum;
        angle_error_sum = angle_error_sum < -ANGLE_ERROR_SUM_MAX ? -ANGLE_ERROR_SUM_MAX : angle_error_sum;
        
        dCurrent = P_position_control * angle_error + I_position_control * angle_error_sum + D_position_control * angle_rate;
        setDesiredCurrent(dCurrent);
 
        actPositionArray[hold_count] = curr_angle;
        refPositionArray[hold_count] = desired_angle;  //  ##############
        
        hold_count++;

        if (hold_count == (MAX_REF_TRAJ_LENGTH-1))
        {
            set_mode(IDLE);
            hold_count = 0;
        }

    }
    else if (get_mode() == TRACK)
    {
        
        static int track_idx = 0;
        desired_angle = referenceTrajectory[track_idx];
        encCount = request_encoder_position();

        curr_angle = 360.0/(334*4) * encCount;
        angle_error = desired_angle - curr_angle;
        angle_error_sum += angle_error;
        angle_rate = (prev_angle - curr_angle)/DT;

        prev_angle = curr_angle;

        angle_error_sum = angle_error_sum > ANGLE_ERROR_SUM_MAX ? ANGLE_ERROR_SUM_MAX : angle_error_sum;
        angle_error_sum = angle_error_sum < -ANGLE_ERROR_SUM_MAX ? -ANGLE_ERROR_SUM_MAX : angle_error_sum;
        
        dCurrent = P_position_control * angle_error + I_position_control * angle_error_sum + D_position_control * angle_rate;
        setDesiredCurrent(dCurrent);

        actPositionArray[track_idx] = curr_angle;
        refPositionArray[track_idx] = desired_angle;

        track_idx++;

        if (track_idx == referenceTrajectoryLength){
            set_mode(HOLD);
            track_idx = 0;

        }
    }

    IFS0bits.T4IF = 0; // clear interrupt flag
}