#ifndef TIMER_H
#define TIMER_H

void timer_setup(unsigned short frequency);

void timer_shutdown();

unsigned long timer_get();

void timer_reset(unsigned short frequency);

void hlt(void);

#endif
