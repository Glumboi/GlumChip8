#ifndef CHIP8_KEYBOARD_H
#define CHIP8_KEYBOARD_H

#include "Chip8Defines.h"
#include <stdbool.h>

typedef struct _chip8Keyboard
{
    Word _keys[16];
    int _lastPressed;
} Chip8Keyboard;

void Chip8Keyboard_Update(Chip8Keyboard* keyboard);
#endif