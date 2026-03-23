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
            int screenW = Raylib.GetScreenWidth();
            int screenH = Raylib.GetScreenHeight();

            // Prevent division by zero if window is minimized
            if (screenW == 0 || screenH == 0) return;

            float scaleX = (float)screenW / WIDTH;
            float scaleY = (float)screenH / HEIGHT;

            for (int y = 0; y < HEIGHT; y++)
            {
                for (int x = 0; x < WIDTH; x++)
                {
                    if (_pixels[x, y])
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

    }
}
