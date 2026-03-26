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
	int targetWidth = display->_isHighRes ? 128 : 64;
	int targetHeight = display->_isHighRes ? 64 : 32;

	int startX = x % targetWidth;
	int startY = y % targetHeight;
	if (startX < 0) startX += targetWidth;
	if (startY < 0) startY += targetHeight;

	for (int row = 0; row < height; row++)
	{
		if (startY + row >= targetHeight) break;

		Word spriteRow = sprite[row];
		for (int col = 0; col < 8; col++)
		{
			if (startX + col >= targetWidth) break;
			if ((spriteRow & (0x80 >> col)) != 0)
			{
				// Pass 'true' because the sprite data indicates a pixel should be drawn here
				if (Chip8Display_XorPixel(display, startX + col, startY + row, true, plane))
					collision = true;
			}
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

	int targetWidth = display->_isHighRes ? 128 : 64;
	int targetHeight = display->_isHighRes ? 64 : 32;
	int startX = x % targetWidth;
	int startY = y % targetHeight;
	if (startX < 0) startX += targetWidth;
	if (startY < 0) startY += targetHeight;

	for (int row = 0; row < 16; row++)
	{
		if (startY + row >= targetHeight) break; // CLIP
		Word rowBits = ((sprite[row * 2] << 8) | sprite[row * 2 + 1]);

		for (int col = 0; col < 16; col++)
		{
			if (startX + col >= targetWidth) break; // CLIP
			if ((rowBits & (0x8000 >> col)) != 0)
			{
				if (Chip8Display_XorPixel(display, startX + col, startY + row, true, plane))
					collision = true;
			}
		}
	}
	return collision;
}

void Chip8Display_Render(Chip8Display* display, Word activePlanes)
{
	// Get actual Raylib window dimensions
	float windowW = (float)GetScreenWidth();
	float windowH = (float)GetScreenHeight();
	if (windowW <= 0 || windowH <= 0) return;

	// Logical dimensions (how many "pixels" the game thinks it has)
	int simW = display->_isHighRes ? 128 : 64;
	int simH = display->_isHighRes ? 64 : 32;

	// Scale factors to stretch those pixels to the full window
	float scaleX = windowW / (float)simW;
	float scaleY = windowH / (float)simH;

	for (int p = 0; p < 2; p++)
	{
		if ((activePlanes & (1 << p)) == 0) continue;

		for (int y = 0; y < simH; y++)
		{
			for (int x = 0; x < simW; x++)
			{
				// CRITICAL: Always use 128 as the stride for the memory index
				// because your display->_planes[p] is a 128x64 array.
				if (display->_planes[p][y * 128 + x])
				{
					DrawRectangleV(
						(Vector2)
					{
						x* scaleX, y* scaleY
					},
						(Vector2)
					{
						scaleX, scaleY
					},
						CHIP8_DISPLAY_COLOR
					);
				}
			}
		}
	}
}



bool Chip8Display_XorPixel(Chip8Display* display, int x, int y, bool pixelOn, int plane)
{
	if (!display || plane < 0 || plane > 1 || !pixelOn) return false;

	// Logical bounds check
	int limitW = display->_isHighRes ? 128 : 64;
	int limitH = display->_isHighRes ? 64 : 32;
	if (x < 0 || x >= limitW || y < 0 || y >= limitH) return false;

	int idx = y * 128 + x;
	Word* planeData = display->_planes[plane];

	bool oldPixel = (planeData[idx] != 0);
	planeData[idx] ^= 1;

	return oldPixel && (planeData[idx] == 0);
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
		else if (direction == Up)
		{
			for (int y = 0; y < height - n; y++)
			{
				memcpy(&plane[y * width], &plane[(y + n) * width], width);
			}
			memset(&plane[(height - n) * width], 0, n * width);
		}

	}
}