#ifndef  CHIP8_RAM_H
#define CHIP8_RAM_H

#include "Chip8Defines.h"

typedef struct _chip8RAM
{
	DoubleWord _data[CHIP8_RAM_SZ];
} Chip8RAM;

void Chip8RAM_WriteByte(Chip8RAM* ram, DoubleWord add, Word val);
DoubleWord Chip8RAM_ReadByte(Chip8RAM* ram, DoubleWord add);

#endif 
