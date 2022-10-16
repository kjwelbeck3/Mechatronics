#ifndef UTILITIES_H_
#define UTILITIES_H_

enum mode_t{
    IDLE,
    PWM,
    ITEST,
    HOLD,
    TRACK
};

enum mode_t get_mode();
void set_mode(enum mode_t);

// enum mode_t _mode;
// _mode = IDLE;

static enum mode_t _mode = 0;



// int get_mode();
// void set_mode(int);


// static int _mode = 0;

#endif