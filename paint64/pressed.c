#include "driver.h"
#include <stdio.h>
#include <stdint.h>

enum {
    buttonUP = 'I',
    buttonDOWN = 'K',
    buttonLEFT = 'J',
    buttonRIGHT = 'L',
    buttonX = 'U',
    buttonY = 'O',
    buttonA = 'P',
    zoneA = 'A',
    zoneB = 'B',
    zoneC = 'C',
    zoneD = 'D',
    zoneBegin = '0',
    zoneEnd = '9',
    zoneBegin2 = 'A',
    zoneEnd2 = 'F',
};

typedef struct {
    uint8_t key[2];
    uint8_t key_count;
} KeyPressStatus;

uint8_t isInZone(KeyPressStatus keyStatus) {
    int pressedTwoKeys = keyStatus.key_count == 2;
    int withinZone = zoneA <= keyStatus.key[0] && keyStatus.key[0] <= zoneD;
    int withinZoneRange = (zoneBegin <= keyStatus.key[1] && keyStatus.key[1] <= zoneEnd) ||
        (zoneBegin2 <= keyStatus.key[1] && keyStatus.key[1] <= zoneEnd2);
    if (withinZone) {
        return pressedTwoKeys ? withinZoneRange : 1;
    }
    return 0;
}

uint8_t isOtherButtons(KeyPressStatus keyStatus) {
    if (keyStatus.key_count != 1) {
        return 0;
    }
    switch (keyStatus.key[0]) {
        case buttonUP:
        case buttonDOWN:
        case buttonLEFT:
        case buttonRIGHT:
        case buttonX:
        case buttonY:
        case buttonA:
            return 1;
        default:
            return 0;
    }
}

uint8_t checkValidKey(KeyPressStatus keyStatus) {
    if (keyStatus.key_count == 1) {
        return isOtherButtons(keyStatus);
    }
    else if (keyStatus.key_count == 2) {
        return isInZone(keyStatus);
    }
    return 0;
}

int main(void) {
    while(1) {
        uint8_t maxKeyID = 255;
        KeyPressStatus keyStatus;
        keyStatus.key_count = 0;
        
        while (keyStatus.key_count < 2) {
            // Detect which key pressed and store it in keyStatus
            for (uint8_t i = 0; i < maxKeyID; i++) {
                if (is_key_pressed(i)) {
                    keyStatus.key[keyStatus.key_count++] = i;
                    break;
                }
            }
            DLY_ms(100);
            if (isOtherButtons(keyStatus)) {
                break;
            }
            if (!isInZone(keyStatus)) {
                break;
            }
        }
        if (checkValidKey(keyStatus)) {
            // print pressed keys
            printf("Pressed keys: ");
            for (int i = 0; i < keyStatus.key_count; i++) {
                printf("%c", (char) keyStatus.key[i]);
            }
            printf("\n");
            DLY_ms(100);
        }
    }
}
