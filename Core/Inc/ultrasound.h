#ifndef ULTRASOUND_H
#define ULTRASOUND_H

#include "main.h"

void enable_ultrasound();
void disable_ultrasound();
float get_read_cm();
uint32_t get_read_us();

#endif
