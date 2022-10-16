## Mechatronics I

This course focused on :

- general C programming; 
- Microchip PIC microcontroller architecture and C programming specific to the PIC (e.g., using the PIC's peripherals, such as analog inputs, digital I/O, counters/timers, comm ports, etc.);
- interfacing the PIC to sensors and actuators, some theory of sensor and actuator operation, and interface circuitry and signal processing.

The final set of weekly mini projects was a microcontroller-based motor position controller, with a menu interface to specify desired trajectories and PID control parameters. 

The source code for this is contained in the `me333_MotorPositionControl` directory.

The following videos demonstrate the menu interface, the generated response plots to step and cubic reference trajectories, and the motor in action tracking a cubic motion profile. 

<video src="/home/kjw/winter22/ME333/HW/HW8_03012022/demo/HW10_demo.mp4"></video>



<video src="/home/kjw/winter22/ME333/HW/HW8_03012022/demo/HW10_cubic.mp4"></video>





<video src="/home/kjw/winter22/ME333/HW/HW8_03012022/demo/HW10_step.mp4"></video>





## Mechatronics II (Advanced)

The `me433_projects` directory contains a collection of more advanced mini projects.

It covered among others:

- designing, soldering and debugging printed circuit boards
- using advanced digital communication protocols like SPI and I2C to control and interface with other devices (IMUs, OLED displays, other MCUs, etc.)
- building a small mobile robot