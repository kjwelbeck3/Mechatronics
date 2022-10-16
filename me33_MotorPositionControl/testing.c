// #include "NU32.h"
// #include "encoder.h"
// #include "ina219.h"
// #include <stdio.h>

// #define DELAY_TIME 200000000
// #define MESSAGE_LENGTH 100
// void delay();

// int main(){

//     char msg[MESSAGE_LENGTH];

//     NU32_Startup();
//     UART2_Startup();
//     INA219_Startup();


//     while(1)
//     {
//         WriteUART2("a"); // asking for position
//         while (!get_encoder_flag()){
//             //  NU32_WriteUART3("in focus");
//             ;
//         }
//         set_encoder_flag(0);
//         int pos = get_encoder_count();
//         sprintf(msg, "encoder position: %d\r\n", pos);
//         NU32_WriteUART3(msg);

//         float current = INA219_read_current();
//         sprintf(msg, "current: %f\r\n", current);
//         NU32_WriteUART3(msg);

//         delay();
//     }


// }

// void delay(){
//     _CP0_SET_COUNT(0); 
//     while (_CP0_GET_COUNT() < DELAY_TIME){;}
// }
