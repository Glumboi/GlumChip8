#include "Chip8Keyboard.h"
#include <raylib.h>

// Map Raylib keys to Chip-8 hex values (0x0 - 0xF)
// This matches your C# dictionary layout
static const int RAY_TO_CHIP_MAP[16] = {
	KEY_X,     // 0x0
	KEY_ONE,   // 0x1
	KEY_TWO,   // 0x2
	KEY_THREE, // 0x3
	KEY_Q,     // 0x4
	KEY_W,     // 0x5
	KEY_E,     // 0x6
	KEY_A,     // 0x7
	KEY_S,     // 0x8
	KEY_D,     // 0x9
	KEY_Z,     // 0xA
	KEY_C,     // 0xB
	KEY_FOUR,  // 0xC
	KEY_R,     // 0xD
	KEY_F,     // 0xE
	KEY_V      // 0xF
};

void Chip8Keyboard_Update(Chip8Keyboard* keyboard)
{
	if (!keyboard) return;

	keyboard->_lastPressed = -1;

	for (int i = 0; i < 16; i++)
	{
		keyboard->_keys[i] = IsKeyDown(RAY_TO_CHIP_MAP[i]);

		if (IsKeyPressed(RAY_TO_CHIP_MAP[i]))
		{
			keyboard->_lastPressed = i;
		}
	}
}
