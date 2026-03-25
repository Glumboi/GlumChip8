#include "Chip8Display.h"

Chip8Display Chip8Display_Create()
{
	Chip8Display display = { 0 };
	display._isHighRes = false;

	// Clear both planes
	Chip8Display_Clear(&display, 0);
	Chip8Display_Clear(&display, 1);

	return display;
}

void Chip8Display_Clear(Chip8Display* display, int plane)
{
	if (!display || plane < 0 || plane > 1) return;
	for (int i = 0; i < CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT; i++)
		display->_planes[plane][i] = 0;
}

void Chip8Display_SetResolution(Chip8Display* display, int w, int h)
{
	display->_isHighRes = (w == 128);
	Chip8Display_Clear(display, 0);
	Chip8Display_Clear(display, 1);
}

bool Chip8Display_DrawSprite(Chip8Display* display, int x, int y, Word* sprite, int height, int plane)
{
	bool collision = false;
	for (int row = 0; row < height; row++)
	{
		Word spriteRow = sprite[row];
		for (int col = 0; col < 8; col++)
		{
			if ((spriteRow & (0x80 >> col)) != 0)
				collision |= Chip8Display_XorPixel(display, x + col, y + row, true, plane);
		}
	}
	return collision;
}

bool Chip8Display_DrawSprite16(Chip8Display* display, int x, int y, Word* sprite, size_t spriteLen, int plane)
{
	bool collision = false;
	if (spriteLen != 32)
	{
		fprintf(stderr, "16x16 sprite must be exactly 32 bytes(2 bytes per row).\n");
		return collision;
	}

	for (int row = 0; row < 16; row++)
	{
		// Each row uses 2 bytes: high byte then low byte
		Word rowBits = ((sprite[row * 2] << 8) | sprite[row * 2 + 1]);

		for (int col = 0; col < 16; col++)
		{
			bool pixelOn = (rowBits & (1 << (15 - col))) != 0; // MSB = leftmost
			if (pixelOn)
			{
				collision |= Chip8Display_XorPixel(display, x + col, y + row, true, plane);
			}
		}
	}

	return collision;
}

void Chip8Display_Render(Chip8Display* display, Word activePlanes)
{
	int screenW = GetScreenWidth();
	int screenH = GetScreenHeight();

	if (screenW == 0 || screenH == 0) return;

	float scaleX = (float)screenW / CHIP8_SCREEN_WIDTH;
	float scaleY = (float)screenH / CHIP8_SCREEN_HEIGHT;

	for (int p = 0; p < CHIP8_DISPLAY_GET_NUMBER_OF_PLANES(display); p++)
	{
		// Only render active planes
		if ((activePlanes & (1 << p)) == 0) continue;

		for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++)
		{
			for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++)
			{
				if (display->_planes[p][y * CHIP8_SCREEN_WIDTH + x]) 
				{
					DrawRectangleV((Vector2) { x* scaleX, y* scaleY },
						(Vector2)
					{
						scaleX, scaleY
					},
						CHIP8_DISPLAY_COLOR);
				}
			}
		}
	}
}

bool Chip8Display_XorPixel(Chip8Display* display, int x, int y, bool pixelOn, int plane)
{
	if (!display || plane < 0 || plane > 1) return false;

	// 1. Determine actual logical bounds based on current resolution
	int targetWidth = display->_isHighRes ? 128 : 64;
	int targetHeight = display->_isHighRes ? 64 : 32;

	// 2. Wrap the starting coordinates
	x %= targetWidth;
	y %= targetHeight;

	// Ensure positive results for wrapping
	if (x < 0) x += targetWidth;
	if (y < 0) y += targetHeight;

	bool collision = false;
	Word* planeData = display->_planes[plane];

	if (!display->_isHighRes)
	{
		// Low-res 64x32 mode: We scale 1 pixel into a 2x2 block 
		// in our 128x64 internal buffer to keep memory consistent.
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				int tx = (x * 2) + i;
				int ty = (y * 2) + j;

				int idx = ty * 128 + tx;
				bool oldPixel = planeData[idx] != 0;
				planeData[idx] ^= (pixelOn ? 1 : 0);

				// If any of the 4 sub-pixels were turned off, it's a collision
				if (oldPixel && !planeData[idx]) collision = true;
			}
		}
	}
	else
	{
		// High-res 128x64 mode: Direct mapping
		int idx = y * 128 + x;
		bool oldPixel = planeData[idx] != 0;
		planeData[idx] ^= (pixelOn ? 1 : 0);
		collision = (oldPixel && !planeData[idx]);
	}

	return collision;
}

void Chip8Display_Scroll(Chip8Display* display, int n, Chip8Display_Scrolling_Direction direction)
{
	if (n <= 0 || !display) return;

	int width = 128; // Always use the full width for internal buffer math
	int height = 64;

	for (int p = 0; p < 2; p++)
	{
		Word* plane = display->_planes[p];

		if (direction == Down)
		{
			for (int y = height - 1; y >= n; y--)
			{
				memcpy(&plane[y * width], &plane[(y - n) * width], width);
			}
			memset(plane, 0, n * width);
		}
		else if (direction == Right)
		{
			for (int y = 0; y < height; y++)
			{
				memmove(&plane[y * width + n], &plane[y * width], width - n);
				memset(&plane[y * width], 0, n);
			}
		}
		else if (direction == Left)
		{
			for (int y = 0; y < height; y++)
			{
				memmove(&plane[y * width], &plane[y * width + n], width - n);
				memset(&plane[y * width + (width - n)], 0, n);
			}
		}
	}
}