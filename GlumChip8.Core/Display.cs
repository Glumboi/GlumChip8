using Raylib_cs;
using System;
using System.Collections.Generic;
using System.Numerics;
using System.Text;

namespace GlumChip8.Core
{
    public class Display
    {

        // Assuming 128×64 high-res mode
        public const int SCREEN_WIDTH = 128;
        public const int SCREEN_HEIGHT = 64;

        // 2 planes (XO-CHIP standard)
        public bool[][,] _planes = new bool[2][,]
        {
    new bool[SCREEN_WIDTH, SCREEN_HEIGHT], // Plane 0
    new bool[SCREEN_WIDTH, SCREEN_HEIGHT]  // Plane 1
        };

        private bool _isHighRes = false; // Toggle with 00FF / 00FE

        public void Clear(int plane = 0)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {
                for (int y = 0; y < SCREEN_HEIGHT; y++)
                {
                    _planes[plane][x, y] = false;
                }
            }
        }

        public bool DrawSprite(int x, int y, byte[] sprite, int plane = 0)
        {
            bool collision = false;
            for (int row = 0; row < sprite.Length; row++)
            {
                byte spriteRow = sprite[row];
                for (int col = 0; col < 8; col++)
                {
                    if ((spriteRow & (0x80 >> col)) != 0)
                    {
                        collision |= XorPixel(x + col, y + row, true, plane);
                    }
                }
            }
            return collision;
        }

        public bool DrawSprite16(int x, int y, byte[] sprite, int plane = 0)
        {
            if (sprite.Length != 32)
                throw new ArgumentException("16x16 sprite must be exactly 32 bytes (2 bytes per row).");

            bool collision = false;

            for (int row = 0; row < 16; row++)
            {
                // Each row uses 2 bytes: high byte then low byte
                ushort rowBits = (ushort)((sprite[row * 2] << 8) | sprite[row * 2 + 1]);

                for (int col = 0; col < 16; col++)
                {
                    bool pixelOn = (rowBits & (1 << (15 - col))) != 0; // MSB = leftmost
                    if (pixelOn)
                    {
                        collision |= XorPixel(x + col, y + row, true, plane);
                    }
                }
            }

            return collision;
        }

        public void Render(int plane = 0)
        {
            int screenW = Raylib.GetScreenWidth();
            int screenH = Raylib.GetScreenHeight();

            // Prevent division by zero if window is minimized
            if (screenW == 0 || screenH == 0) return;

            float scaleX = (float)screenW / SCREEN_WIDTH;
            float scaleY = (float)screenH / SCREEN_HEIGHT;

            for (int y = 0; y < SCREEN_HEIGHT; y++)
            {
                for (int x = 0; x < SCREEN_WIDTH; x++)
                {
                    if (_planes[plane][x, y])
                    {
                        Raylib.DrawRectangleV(
                            new System.Numerics.Vector2(x * scaleX, y * scaleY),
                            new System.Numerics.Vector2(scaleX, scaleY),
                            Color.White
                        );
                    }
                }
            }
        }

        public bool XorPixel(int x, int y, bool pixelOn, int plane = 0)
        {
            if (!_isHighRes)
            {
                // Low-res 64x32 -> scale to 128x64
                bool collision = false;

                int baseX = x * 2;
                int baseY = y * 2;

                for (int i = 0; i < 2; i++)
                {
                    for (int j = 0; j < 2; j++)
                    {
                        int targetX = baseX + i;
                        int targetY = baseY + j;

                        if (targetX >= SCREEN_WIDTH || targetY >= SCREEN_HEIGHT)
                            continue; // Skip out-of-bounds

                        bool oldPixel = _planes[plane][targetX, targetY];
                        _planes[plane][targetX, targetY] = oldPixel ^ pixelOn;
                        if (oldPixel && !(_planes[plane][targetX, targetY]))
                            collision = true;
                    }
                }

                return collision;
            }

            // High-res mode 128x64
            x %= SCREEN_WIDTH;
            y %= SCREEN_HEIGHT;
            bool oldHigh = _planes[plane][x, y];
            _planes[plane][x, y] = oldHigh ^ pixelOn;
            return oldHigh && !(_planes[plane][x, y]);
        }

        public void ScrollDown(int n)
        {
            for (int p = 0; p < _planes.Length; p++)
            {
                for (int y = SCREEN_HEIGHT - 1; y >= n; y--)
                {
                    for (int x = 0; x < SCREEN_WIDTH; x++)
                    {
                        _planes[p][x, y] = _planes[p][x, y - n];
                    }
                }
                // Clear the newly empty rows at the top
                for (int y = 0; y < n; y++)
                {
                    for (int x = 0; x < SCREEN_WIDTH; x++) _planes[p][x, y] = false;
                }
            }
        }

        public void ScrollRight(int n)
        {
            for (int p = 0; p < _planes.Length; p++)
            {
                for (int y = 0; y < SCREEN_HEIGHT; y++)
                {
                    for (int x = SCREEN_WIDTH - 1; x >= n; x--)
                    {
                        _planes[p][x, y] = _planes[p][x - n, y];
                    }
                    for (int x = 0; x < n; x++) _planes[p][x, y] = false;
                }
            }
        }

        public void ScrollLeft(int n)
        {
            for (int p = 0; p < _planes.Length; p++)
            {
                for (int y = 0; y < SCREEN_HEIGHT; y++)
                {
                    for (int x = 0; x < SCREEN_WIDTH - n; x++)
                    {
                        _planes[p][x, y] = _planes[p][x + n, y];
                    }
                    for (int x = SCREEN_WIDTH - n; x < SCREEN_WIDTH; x++) _planes[p][x, y] = false;
                }
            }
        }

        public void SetResolution(int width, int height)
        {
            _isHighRes = (width == 128);
            Clear(0);
            Clear(1);
        }
    }
}
