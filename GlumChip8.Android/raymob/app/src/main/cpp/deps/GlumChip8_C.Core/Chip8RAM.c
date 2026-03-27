#include "Chip8RAM.h"

void Chip8RAM_WriteByte(Chip8RAM* ram, DoubleWord add, Word val)
{

	if (add >= CHIP8_RAM_SZ)
	{
		fprintf(stderr, "Address %d is out of range!\n", add);
		return;
	}

	ram->_data[add] = val;
}

DoubleWord Chip8RAM_ReadByte(Chip8RAM* ram, DoubleWord add)
{
	if (add >= CHIP8_RAM_SZ)
	{
		fprintf(stderr, "Address %d is out of range!\n", add);
		return 0xFFFF;
	}

	return ram->_data[add];
}
