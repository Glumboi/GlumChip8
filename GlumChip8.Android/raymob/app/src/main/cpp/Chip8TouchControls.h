#ifndef CHIP_TOUCH_CONTROLS_H
#define CHIP_TOUCH_CONTROLS_H

#include "Chip8Defines.h"
#include "Chip8System.h"
#include "Chip8CPU.h"

void Chip8TouchControls_Render_KeyPad(Word keys[16], int offsetX);
void Chip8TouchControls_Render_CloseRom(int offsetX);
void Chip8TouchControls_Render(Word keys[16], int offsetX);

#endif