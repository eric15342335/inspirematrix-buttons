#include "system_mac.h"

#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_KEYS 6

CGKeyCode pressedKeys[MAX_KEYS];
int pressedKeyCount = 0;
pthread_mutex_t keyMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread;

void addKey(CGKeyCode keyCode) {
    for (int i = 0; i < pressedKeyCount; i++) {
        if (pressedKeys[i] == keyCode)
            return; // Key already in array
    }
    if (pressedKeyCount < MAX_KEYS) {
        pressedKeys[pressedKeyCount++] = keyCode;
    }
}

void removeKey(CGKeyCode keyCode) {
    for (int i = 0; i < pressedKeyCount; i++) {
        if (pressedKeys[i] == keyCode) {
            // Move last element to this position and decrease count
            pressedKeys[i] = pressedKeys[--pressedKeyCount];
            return;
        }
    }
}

bool is_key_pressed(CGKeyCode keyCode) {
    bool pressed = false;
    pthread_mutex_lock(&keyMutex);
    for (int i = 0; i < pressedKeyCount; i++) {
        if (pressedKeys[i] == keyCode) {
            pressed = true;
            break;
        }
    }
    pthread_mutex_unlock(&keyMutex);
    return pressed;
}

CGEventRef keyboardCallback(
    CGEventTapProxy proxy, CGEventType type, CGEventRef event, void * refcon) {
    (void)proxy;
    (void)refcon;
    if (type != kCGEventKeyDown && type != kCGEventKeyUp) {
        return event;
    }

    CGKeyCode keyCode =
        (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

    pthread_mutex_lock(&keyMutex);
    if (type == kCGEventKeyDown) {
        addKey(keyCode);
    }
    else if (type == kCGEventKeyUp) {
        removeKey(keyCode);
    }
    pthread_mutex_unlock(&keyMutex);

    return event;
}

void * eventTapThread(void * arg) {
    (void)arg;
    CFMachPortRef eventTap;
    CFRunLoopSourceRef runLoopSource;

    eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0,
        kCGEventMaskForAllEvents, keyboardCallback, NULL);

    if (!eventTap) {
        fprintf(stderr, "Failed to create event tap\n");
        return NULL;
    }

    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);

    CFRunLoopRun();

    return NULL;
}

void pthread_init() { pthread_create(&thread, NULL, eventTapThread, NULL); }
