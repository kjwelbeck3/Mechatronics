#include "currentcontrol.h"


/*************************
 * HELPER FUNCTIONS
*************************/
void currentControl_Startup()
{
    // setup 5 kHZ interrupt on Timer 2 for current control

    T2CONbits.TCKPS = 0b011; // Timer2 prescaler N=8 (1:8)
    PR2 = 1999;              // period = (PR2+1) * N * 12.5 ns = 0.2 ms, 5kHz
    TMR2 = 0;                // initialize TMR2 count to 0
    T2CONbits.TGATE = 0;     // gated accumualtion mode disabled
    IPC2 = 24;               // priority level 6
    IFS0bits.T2IF = 0;       // clear Timer2 interrupt flag
    IEC0bits.T2IE = 1;       // enable Timer2 interrupt

    // setup 20 kHz on Timer3 for PWM with 50% duty cycle

    T3CONbits.TCKPS = 0;    // Timer3 prescaler N=1 (1:1)
    PR3 = 3999;             // period = (PR3+1) * N * 12.5 ns = 50 us, 20 kHz
    TMR3 = 0;               // initialize TMR3 count to 0
    OC1CONbits.OCM = 0b110; // PWM mode without fault pin; other OC1CON bits are defaults
    OC1CONbits.OCTSEL = 1;  // Selecting to use Timer3, instead of Timer2 default
    OC1RS = 2000;           // duty cycle = OC1RS/(PR3+1) = 75%
    OC1R = 2000;            // initialize before turning OC1 on; afterward it is read-only

    // Enable timers and pwm
    T3CONbits.ON = 1;  // turn on Timer3
    OC1CONbits.ON = 1; // turn on OC1
    T2CONbits.ON = 1;  // turn on the timer2

    // digital output for the direction of the motor
    TRISDbits.TRISD8 = 0; // Seting D8 for output (H-Bridge DIR)

}

void set_PWM(int pwm)
{
    PWMDutyCycle = abs(pwm);
    PWMDutyCycle = PWMDutyCycle > 100 ? 100 : PWMDutyCycle;
    motor_direction = pwm > 0 ? 1 : 0;
}

int get_PWM()
{
    int pwm = (motor_direction == 1) ? PWMDutyCycle : PWMDutyCycle * -1;
    return pwm;
}

void setCurrentGains(float p, float i)
{
    P_current_control = p;
    I_current_control = i;
}

float getCurrentP()
{
    return P_current_control;
}

float getCurrentI()
{
    return I_current_control;
}


float getDesiredCurrent()
{
    return desiredCurrent;
}

void setDesiredCurrent(float current){
    desiredCurrent = current;
}

/****************************
 * INTERRUPT SERVICE ROUTINES
*****************************/

void __ISR(_TIMER_2_VECTOR, IPL6SRS) CurrentController(void) // _TIMER_2_VECTOR = 8
{

    // // test
    // OC1RS = 1000;                           // set to 25% duty cycle
    // MOTOR_DIR = !MOTOR_DIR;                 // toggle the motor direction

    // operating mode dependence
    enum mode_t m = get_mode();
    switch (m)
    {
    case IDLE:
    {
        OC1RS = PWMDutyCycle = 0;
        MOTOR_DIR = motor_direction = 0;
        itest_count = 0;
        error_sum = 0;
        break;
    }
    case PWM:
    {
        OC1RS = (int)(PWMDutyCycle / 100.0 * 4000.0);
        MOTOR_DIR = motor_direction;
        break;
    }
    case ITEST:
    {
    
        float refCurrent = 0;

        if (itest_count < 25)
        {
            refCurrent = 200;
        }
        else if (itest_count < 50)
        {
            refCurrent = -200;
        }
        else if (itest_count < 75)
        {
            refCurrent = 200;
        }
        else if (itest_count < 100)
        {
            refCurrent = -200;
        }
        if (itest_count == 99)
        {
            set_mode(IDLE);
        }

        float current_error = refCurrent - INA219_read_current();
        error_sum += current_error;
        error_sum = (error_sum > ERROR_SUM_MAX) ? ERROR_SUM_MAX : error_sum;
        error_sum = (error_sum < -ERROR_SUM_MAX) ? -ERROR_SUM_MAX : error_sum;
        int pi_current = (int)(P_current_control * current_error + I_current_control * error_sum);
        set_PWM(pi_current);
        OC1RS = (int)(PWMDutyCycle / 100.0 * 4000.0);
        MOTOR_DIR = motor_direction;
        

        refCurrentArray[itest_count] = refCurrent;
        int curr = INA219_read_current();
        actCurrentArray[itest_count] = (int) (curr);


        itest_count++;
        if (get_mode() == IDLE){
            itest_count = 0;
            error_sum =0;
        }
        break;
    }
    case HOLD:
        {   
            volatile float curr =  INA219_read_current();
            volatile float current_error = desiredCurrent - curr;
            
            error_sum += current_error;
            error_sum = (error_sum > ERROR_SUM_MAX) ? ERROR_SUM_MAX : error_sum;
            error_sum = (error_sum < -ERROR_SUM_MAX) ? -ERROR_SUM_MAX : error_sum;
            volatile int pi_current = (int)(P_current_control * current_error + I_current_control * error_sum);
            set_PWM(pi_current);
            OC1RS = (int)(PWMDutyCycle / 100.0 * 4000.0);
            MOTOR_DIR = motor_direction;

            break;
        }
    case TRACK:
        {
            volatile float curr =  INA219_read_current();
            volatile float current_error = desiredCurrent - curr;
            
            error_sum += current_error;
            error_sum = (error_sum > ERROR_SUM_MAX) ? ERROR_SUM_MAX : error_sum;
            error_sum = (error_sum < -ERROR_SUM_MAX) ? -ERROR_SUM_MAX : error_sum;
            volatile int pi_current = (int)(P_current_control * current_error + I_current_control * error_sum);
            set_PWM(pi_current);
            OC1RS = (int)(PWMDutyCycle / 100.0 * 4000.0);
            MOTOR_DIR = motor_direction;
            break;
        }

    default:
        {

            break;
        }
    }

    IFS0bits.T2IF = 0; // clear interrupt flag
}

