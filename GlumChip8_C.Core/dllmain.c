#include "Chip8System.h"

#include "Chip8CPU.h"
#include "Chip8Display.h"
#include "Chip8RAM.h"
#include "Chip8Keyboard.h"
#include <time.h>

EXPORT_FN void Init_Chip8Emulator(void)
{
	srand((unsigned int)time(NULL));

	static Chip8CPU cpu;
	cpu = Chip8CPU_Create();

	static Chip8Display display;
	display = Chip8Display_Create();

	static Chip8RAM ram;
	memset(&ram, 0, sizeof(ram));
	cpu._ram = &ram;

	cpu._activePlanes = 0b01;

	// In your Init function
	static Chip8Keyboard keyboard = { 0 };
	Chip8CPU_ResetRegisters(&cpu);
	Chip8System_Init(&cpu, &display, &ram, &keyboard);
}

EXPORT_FN void Chip8Keyboard_UpdateKeys(unsigned char keys[16], int lastPressed)
{
	for (int i = 0; i < 16; i++)
		g_chip8SystemInstance._keyboard->_keys[i] = keys[i];

	g_chip8SystemInstance._keyboard->_lastPressed = lastPressed;
}

EXPORT_FN void Chip8_Reset_Rom(void)
{
	Chip8CPU_ResetRegisters(g_chip8SystemInstance._cpu);
	g_chip8SystemInstance._cpu->_activePlanes = 0b01;
	Chip8Display_Clear(g_chip8SystemInstance._display, 0);
	Chip8Display_Clear(g_chip8SystemInstance._display,1);
}

EXPORT_FN void Chip8_CPU_Step(void)
{
	Chip8CPU_ExecuteCurrent(g_chip8SystemInstance._cpu);
}

EXPORT_FN bool Chip8_CPU_GetRunning(void)
{
	if (!g_chip8SystemInstance._cpu)return false;
	return g_chip8SystemInstance._cpu->_running;
}

EXPORT_FN void Chip8_CPU_SetRunning(bool v)
{
	if (!g_chip8SystemInstance._cpu) return;
	g_chip8SystemInstance._cpu->_running = v;
}

// Returns pointer to plane 0 or 1
// Store pixels as bytes: 0 = off, 1 = on
EXPORT_FN Word* Chip8Display_GetPlane(int plane)
{
	if (!g_chip8SystemInstance._display) return NULL;        // null check
	if (plane < 0 || plane > 1) return NULL;

	return g_chip8SystemInstance._display->_planes[plane];   // must be flattened: Word _planes[2][128*64]
}

EXPORT_FN void RunChip8Rom_FromFile(const char* file)
{
	Init_Chip8Emulator();

	if (!file) return;
	FILE* f = fopen(file, "rb");
	if (!f) return;

	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	rewind(f);

	Word* rombuff = malloc(len);
	if (!rombuff) return;
	fread(rombuff, 1, len, f);
	fclose(f);

	Chip8CPU_LoadProgram(g_chip8SystemInstance._cpu, rombuff, len);

	free(rombuff);
}