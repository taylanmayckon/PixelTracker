#ifndef STRUCTS_H
#define STRUCTS_H
#include <stdint.h>


typedef struct {
    int red;
    int green;
    int blue;
} LEDs;

typedef struct{
    int a;
    int b;
} BUZZERS;

typedef struct {
    int a;
    int b;
} BUTTONS;

typedef struct {
    int x_pin;
    int y_pin;
    int button;
    volatile uint16_t vrx_value;
    volatile uint16_t vry_value;
} JOY;

#endif