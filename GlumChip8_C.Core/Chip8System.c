#include "Chip8System.h"
Chip8System g_chip8SystemInstance = { 0 };

void Chip8System_Init(Chip8CPU* cpu, Chip8Display* display, Chip8RAM* ram)
{
    g_chip8SystemInstance._cpu = cpu;
    g_chip8SystemInstance._ram = ram;

    // Make display persistent
    g_chip8SystemInstance._display = display;
}
