using System;
using System.Collections.Generic;
using System.Text;
using Raylib_cs;

namespace GlumChip8.Core
{
    public class Display
    {

        public const int WIDTH = 64;
        public const int HEIGHT = 32;
        private readonly bool[,] _pixels = new bool[WIDTH, HEIGHT];
        public void Clear()
        {
            for (int x = 0; x < WIDTH; x++)
            {
                for (int y = 0; y < HEIGHT; y++)
                {
                    _pixels[x, y] = false;
                }
            }
        }

        public bool DrawSprite(int x, int y, byte[] sprite)
        {
            bool collision = false;
            for (int row = 0; row < sprite.Length; row++)
            {
                byte spriteRow = sprite[row];
                for (int col = 0; col < 8; col++)
                {
                    if ((spriteRow & (0x80 >> col)) != 0)
                    {
                        int pixelX = (x + col) % WIDTH;
                        int pixelY = (y + row) % HEIGHT;
                        if (_pixels[pixelX, pixelY])
                        {
                            collision = true;
                        }
                        _pixels[pixelX, pixelY] ^= true;
                    }
                }
            }
            return collision;
        }

        public void Render()
        {
            for (int x = 0; x < WIDTH; x++)
            {
                for (int y = 0; y < HEIGHT; y++)
                {
                    // Draw as rectangle for better visibility
                    if (_pixels[x, y])
                    {
                        Raylib.DrawRectangle(x * 10, y * 10, 10, 10, Raylib_cs.Color.White);
                    }
                    else
                    {
                        Raylib.DrawRectangle(x * 10, y * 10, 10, 10, Raylib_cs.Color.Black);
                    }
                }
            }
        }

    }
}
