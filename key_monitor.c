#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

#define MAX_KEYS 256

CGKeyCode pressedKeys[MAX_KEYS];
int pressedKeyCount = 0;
pthread_mutex_t keyMutex = PTHREAD_MUTEX_INITIALIZER;

void addKey(CGKeyCode keyCode) {
    for (int i = 0; i < pressedKeyCount; i++) {
        if (pressedKeys[i] == keyCode) return; // Key already in array
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

CGEventRef keyboardCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    if (type != kCGEventKeyDown && type != kCGEventKeyUp) {
        return event;
    }

    CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

    pthread_mutex_lock(&keyMutex);
    if (type == kCGEventKeyDown) {
        addKey(keyCode);
    } else if (type == kCGEventKeyUp) {
        removeKey(keyCode);
    }
    pthread_mutex_unlock(&keyMutex);

    return event;
}

void* eventTapThread(void* arg) {
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

int main() {
    pthread_t thread;

    if (pthread_create(&thread, NULL, eventTapThread, NULL) != 0) {
        fprintf(stderr, "Failed to create event tap thread\n");
        return 1;
    }

    printf("Monitoring key presses. Press Ctrl+C to exit.\n");

    while (1) {
        pthread_mutex_lock(&keyMutex);
        if (pressedKeyCount > 0) {
            printf("Currently pressed keys: ");
            for (int i = 0; i < pressedKeyCount; i++) {
                printf("%d ", pressedKeys[i]);
            }
            printf("\n");
        }
        pthread_mutex_unlock(&keyMutex);

        usleep(50000);  // Sleep for 50ms
    }

    return 0;
}