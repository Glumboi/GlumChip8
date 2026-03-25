#ifndef CHIP8_SYSTEM_H
#define CHIP8_SYSTEM_H

#include "HelperDefines.h"
#include "Chip8Defines.h"

// Forward declarations to avoid pulling heavy headers (like raylib.h) into every
// translation unit that includes this header. Concrete headers should be
// included in the C files that need the full definitions.
typedef struct _chip8Display Chip8Display;
typedef struct _chip8Keyboard Chip8Keyboard;
typedef struct _chip8RAM Chip8RAM;
typedef struct _chip8Sound Chip8Sound;

// Forward declaration to avoid circular include with Chip8CPU.h
typedef struct _chip8CPU Chip8CPU;

typedef struct
{
	Chip8Display* _display;
	Chip8RAM* _ram;
	Chip8CPU* _cpu;
} Chip8System;

extern Chip8System g_chip8SystemInstance;
bool Chip8System_Init(Chip8CPU* cpu, Chip8Display* display, Chip8RAM* ram);

#endif	