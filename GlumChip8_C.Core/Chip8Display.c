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
				if (display->_planes[p][x, y])
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

	x %= CHIP8_SCREEN_WIDTH;
	y %= CHIP8_SCREEN_HEIGHT;

	Word* planeData = display->_planes[plane];
	int idx = y * CHIP8_SCREEN_WIDTH + x;

	bool old = planeData[idx] != 0;
	planeData[idx] ^= pixelOn ? 1 : 0;
	return old && !planeData[idx];
}

void Chip8Display_Scroll(Chip8Display* display, int n, Chip8Display_Scrolling_Direction direction)
{
	if (n <= 0) return;

	switch (direction)
	{
	case Down:
	{
		if (n >= CHIP8_SCREEN_HEIGHT)
		{
			// clearing all rows
			for (int p = 0; p < CHIP8_DISPLAY_GET_NUMBER_OF_PLANES(display); p++)
				for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++)
					for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++)
						display->_planes[p][x, y] = false;
			break;
		}

		for (int p = 0; p < CHIP8_DISPLAY_GET_NUMBER_OF_PLANES(display); p++)
		{
			for (int y = CHIP8_SCREEN_HEIGHT - 1; y >= n; y--)
			{
				for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++)
				{
					display->_planes[p][x, y] = display->_planes[p][x, y - n];
				}
			}
			// Clear the newly empty rows at the top
			for (int y = 0; y < n; y++)
			{
				for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++) display->_planes[p][x, y] = false;
			}
		}
		break;
	}
	case Right:
	{
		if (n >= CHIP8_SCREEN_WIDTH)
		{
			// clearing all columns
			for (int p = 0; p < CHIP8_DISPLAY_GET_NUMBER_OF_PLANES(display); p++)
				for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++)
					for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++)
						display->_planes[p][x, y] = false;
			break;
		}

		for (int p = 0; p < CHIP8_DISPLAY_GET_NUMBER_OF_PLANES(display); p++)
		{
			for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++)
			{
				// start from the last valid index
				for (int x = CHIP8_SCREEN_WIDTH - 1; x >= n; x--)
				{
					display->_planes[p][x, y] = display->_planes[p][x - n, y];
				}
				for (int x = 0; x < n; x++) display->_planes[p][x, y] = false;
			}

		}
		break;
	}
	case Left:
	{
		if (n >= CHIP8_SCREEN_WIDTH)
		{
			// clearing all columns
			for (int p = 0; p < CHIP8_DISPLAY_GET_NUMBER_OF_PLANES(display); p++)
				for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++)
					for (int x = 0; x < CHIP8_SCREEN_WIDTH; x++)
						display->_planes[p][x, y] = false;
			break;
		}

		for (int p = 0; p < CHIP8_DISPLAY_GET_NUMBER_OF_PLANES(display); p++)
		{
			for (int y = 0; y < CHIP8_SCREEN_HEIGHT; y++)
			{
				for (int x = 0; x < CHIP8_SCREEN_WIDTH - n; x++)
				{
					// move pixel from the right (x + n) into current x
					display->_planes[p][x, y] = display->_planes[p][x + n, y];
				}
				for (int x = CHIP8_SCREEN_WIDTH - n; x < CHIP8_SCREEN_WIDTH; x++)
				{
					display->_planes[p][x, y] = false;
				}
			}

		}
		break;
	}
	default:
	{
		break;
	}
	}
}