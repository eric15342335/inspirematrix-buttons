#ifndef SYSTEM_MAC_H
#define SYSTEM_MAC_H

#include <stdbool.h>
#include <pthread.h>
#include <ApplicationServices/ApplicationServices.h>

#define A_Key 0
#define S_Key 1
#define D_Key 2
#define W_Key 13
#define F_Key 3

pthread_t thread;

#define MAX_KEYS 6
CGKeyCode pressedKeys[MAX_KEYS];
int pressedKeyCount_;

void pthread_init();

bool is_key_pressed(CGKeyCode keyCode);

#endif