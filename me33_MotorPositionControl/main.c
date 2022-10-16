#include <stdio.h>
#include "NU32.h"           // config bits, constants, funcs for startup and UART
#include "encoder.h"
#include "utilities.h"
#include "ina219.h"
#include "currentcontrol.h"
#include "positioncontrol.h"
#include <string.h>


/*************************
 * CONSTANTS
*************************/

#define BUF_SIZE 200
#define MESSAGE_LENGTH 100

/*************************
 * HELPER FUNCTION PROTOTYPES
*************************/
static int request_encoder_position();  
static void zero_encoder_count();
static void request_mode_to_buffer();   // To report current mode
void send_itest_data();                 // Sending ITEST array data back for visualization 
void accept_trajectory();               // Recieving trajectory from python script into array to reference later
void send_track_data();                 // Send TRACK arrays data back for visualization
void send_hold_data();                  // Send HOLD arrays data back for visualization

/*************************
 * PRIVATE GLOBAL VARIABLES
*************************/
static char buffer[BUF_SIZE];


/*************************
 * MAIN FUNCTION
*************************/
int main()
{
  
  NU32_Startup();         // cache on, min flash wait, interrupts on, LED/button init, UART init
  NU32_LED1 = 1;          // turn off the LEDs
  NU32_LED2 = 1;
  set_mode(IDLE);         // Setting start up mode to IDLE
 
  // disabling interrupts to initialize modules or peripherals
  __builtin_disable_interrupts();
  UART2_Startup();
  INA219_Startup();
  currentControl_Startup();
  positionControl_Startup();
  __builtin_enable_interrupts();

  while (1)
  {
    NU32_ReadUART3(buffer, BUF_SIZE);   // we expect the next character to be a menu command
    NU32_LED2 = 1;                      // clear the error LED
    switch (buffer[0])
    {


    case 'b':
    {
      float current = INA219_read_current();
      sprintf(buffer, "%f\r\n", current);
      NU32_WriteUART3(buffer);
      break;
    } 

    case 'c':
    {
      int count = request_encoder_position();
      sprintf(buffer, "%d\r\n", count);
      NU32_WriteUART3(buffer);
      break;
    }

     case 'd':
     {
      int count = request_encoder_position();
      double degs = 360.0/(334*4) * count;
      sprintf(buffer, "%f\r\n", degs);
      NU32_WriteUART3(buffer);
      break;
    }

    case 'e':
     {
      zero_encoder_count();
      int count = request_encoder_position();
      sprintf(buffer, "%d\r\n", count);
      NU32_WriteUART3(buffer);
      break;
    }

    case 'f':
     {
      int pwm = 0;
      NU32_ReadUART3(buffer, BUF_SIZE);
      sscanf(buffer, "%d", &pwm);
      set_PWM(pwm);
      
      // Returning for confirmation
      int set_pwm = get_PWM();
      sprintf(buffer, "%d\r\n", set_pwm);
      NU32_WriteUART3(buffer);
      set_mode(PWM);
      break;
    }

    case 'g':
     {
      float p=0.0, i=0.0;
      NU32_ReadUART3(buffer, BUF_SIZE);
      sscanf(buffer, "%f", &p);
      NU32_ReadUART3(buffer, BUF_SIZE);
      sscanf(buffer, "%f", &i);
      setCurrentGains(p, i);

      // Returning for confirmation
      float _p = getCurrentP();     
      sprintf(buffer, "%f\r\n", _p);
      NU32_WriteUART3(buffer);

      float _i = getCurrentI();
      sprintf(buffer, "%f\r\n", _i);
      NU32_WriteUART3(buffer);

      break;
    }

    case 'h':
     {
      float _p = getCurrentP();     
      sprintf(buffer, "%f\r\n", _p);
      NU32_WriteUART3(buffer);

      float _i = getCurrentI();
      sprintf(buffer, "%f\r\n", _i);
      NU32_WriteUART3(buffer);
      break;
    }

    case 'i':
     {
      float p=0.0, i=0.0, d=0.0;
      NU32_ReadUART3(buffer, BUF_SIZE);
      sscanf(buffer, "%f", &p);
      NU32_ReadUART3(buffer, BUF_SIZE);
      sscanf(buffer, "%f", &i);
      NU32_ReadUART3(buffer, BUF_SIZE);
      sscanf(buffer, "%f", &d);
      setPositionGains(p, i, d); 

      // Returning for confirmation
      float _p = getPositionP();     
      sprintf(buffer, "%f\r\n", _p);
      NU32_WriteUART3(buffer);

      float _i = getPositionI();
      sprintf(buffer, "%f\r\n", _i);
      NU32_WriteUART3(buffer);

      float _d = getPositionD();
      sprintf(buffer, "%f\r\n", _d);
      NU32_WriteUART3(buffer);

      break;
    }

    case 'j':
     {
      float _p = getPositionP();     
      sprintf(buffer, "%f\r\n", _p);
      NU32_WriteUART3(buffer);

      float _i = getPositionI();
      sprintf(buffer, "%f\r\n", _i);
      NU32_WriteUART3(buffer);

      float _d = getPositionD();
      sprintf(buffer, "%f\r\n", _d);
      NU32_WriteUART3(buffer);

      break;
    }
    
    case 'k':
     {
      set_mode(ITEST);
      while (get_mode()==ITEST){;}
      send_itest_data();
      break;
    }

    case 'l':
    {
      static int hold_count = 0;
      if (hold_count == 0){
        zero_encoder_count();
      }
      int angle=0;
      NU32_ReadUART3(buffer, BUF_SIZE);
      sscanf(buffer, "%d", &angle);
      setDesiredAngle(angle);
      
      int setAng = getDesiredAngle();
      sprintf(buffer, "%d\r\n", setAng);
      NU32_WriteUART3(buffer);
      
      hold_count++;
      set_mode(HOLD);
      while (get_mode()==HOLD){;}
      send_hold_data();

      break;
    }

    case 'm':
    {
      accept_trajectory();
      break;
    }

    // case 's':
    // {
    //   int desAng = getDesiredAngle();
    //   sprintf(buffer, "%f\r\n", desAng);
    //   NU32_WriteUART3(buffer);

    //   break;
    // }

    case 'n':
    {
      accept_trajectory();
      break;
    }

    case 'o':
    {
      set_mode(TRACK);
      while (get_mode()==TRACK){;}
      send_track_data();
      break;
    }

    case 'p':
     {
      set_mode(IDLE);
      set_PWM(0);

      // Returning for confirmation
      int set_pwm = get_PWM();
      sprintf(buffer, "%d\r\n", set_pwm);
      NU32_WriteUART3(buffer);
      break;
    }

    case 'r':
     {
      request_mode_to_buffer();
      NU32_WriteUART3(buffer);
      break;
    }

    case 'q':
    {
      // handle q for quit. Later you may want to return to IDLE mode here.
      break;
    }
    default:
    {
      NU32_LED2 = 0; // turn on LED2 to indicate an error
      break;
    }
    }
  }
  return 0;
}


/*************************
 * HELPER FUNCTION
*************************/
int request_encoder_position()
{
  WriteUART2("a"); // asking for position
  while (!get_encoder_flag()){;}
  set_encoder_flag(0);
  int pos = get_encoder_count();
  return pos;
}

void zero_encoder_count()
{
  WriteUART2("b");
  set_encoder_flag(0);
}

void request_mode_to_buffer()
{
  enum mode_t m = get_mode();
      switch (m)
      {
        case IDLE: {sprintf(buffer,"Current mode: IDLE\r\n"); break; } 
        case PWM: {sprintf(buffer,"Current mode: PWM\r\n"); break; } 
        case ITEST: {sprintf(buffer,"Current mode: ITEST\r\n"); break; } 
        case HOLD: {sprintf(buffer,"Current mode: HOLD\r\n"); break; } 
        case TRACK: {sprintf(buffer,"Current mode: TRACK\r\n"); break; } 
        default: {sprintf(buffer,"Unresolved mode\r\n"); break; }
      }
}

void send_itest_data()
{
  sprintf(buffer, "%d\n\r", NUM_DATA_POINTS);
  NU32_WriteUART3(buffer);

  for (int i =0; i < NUM_DATA_POINTS; i++)
  {
      sprintf(buffer, "%d %d\n\r",refCurrentArray[i], actCurrentArray[i]); 
      NU32_WriteUART3(buffer);

  }
}

void accept_trajectory()
{  
  NU32_ReadUART3(buffer, BUF_SIZE);
  sscanf(buffer, "%d", &referenceTrajectoryLength);
  
  for (int i = 0; i < referenceTrajectoryLength; i++)
  {
    float val = 0;
    NU32_ReadUART3(buffer, BUF_SIZE);
    sscanf(buffer, "%f", &val);
    referenceTrajectory[i] = val; 
  }
}

void send_track_data()
{
  
  sprintf(buffer, "%d\n\r", referenceTrajectoryLength);
  NU32_WriteUART3(buffer);

  for (int i =0; i < referenceTrajectoryLength; i++)
  {
      sprintf(buffer, "%f %f\n\r", refPositionArray[i] ,actPositionArray[i]); 
      NU32_WriteUART3(buffer);

  }
}

void send_hold_data()
{
  sprintf(buffer, "%d\n\r", MAX_REF_TRAJ_LENGTH);
  NU32_WriteUART3(buffer);

  for (int i =0; i < MAX_REF_TRAJ_LENGTH; i++)
  {
      sprintf(buffer, "%f %f\n\r", refPositionArray[i] ,actPositionArray[i]); 
      NU32_WriteUART3(buffer);

  }
}