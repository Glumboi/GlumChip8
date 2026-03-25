#ifndef CHIP8_DISPLAY_H
#define CHIP8_DISPLAY_H

#include "Chip8Defines.h"

typedef enum
{
	Down = 0,
	Left,
	Right,
	Up
} Chip8Display_Scrolling_Direction;

typedef struct _chip8Display
{
	Word _planes[2][CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT]; // Flattened 1D arrays
	bool _isHighRes;
} Chip8Display;

Chip8Display Chip8Display_Create();
void Chip8Display_Clear(Chip8Display* display, int plane);
void Chip8Display_SetResolution(Chip8Display* display, int w, int h);
bool Chip8Display_DrawSprite(Chip8Display* display, int x, int y, Word* sprite, size_t spriteLen, int plane);
bool Chip8Display_DrawSprite16(Chip8Display* display, int x, int y, Word* sprite, size_t spriteLen, int plane);
void Chip8Display_Render(Chip8Display* display, Word activePlanes);
bool Chip8Display_XorPixel(Chip8Display* display, int x, int y, bool pixelOn, int plane);
void Chip8Display_Scroll(Chip8Display* display, int n, Chip8Display_Scrolling_Direction direction);

#endif 
