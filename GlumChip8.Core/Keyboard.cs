using Raylib_cs;
using System;
using System.Collections.Generic;
using System.Text;

namespace GlumChip8.Core
{
    public class Keyboard
    {
        private readonly bool[] _keys = new bool[16];

        public void PressKey(int key)
        {
            if (key < 0 || key > 15)
                throw new ArgumentOutOfRangeException(nameof(key), "Key must be between 0 and 15.");
            _keys[key] = true;
        }

        public void ReleaseKey(int key)
        {
            if (key < 0 || key > 15)
                throw new ArgumentOutOfRangeException(nameof(key), "Key must be between 0 and 15.");
            _keys[key] = false;
        }

        public bool IsKeyPressed(int key)
        {
            if (key < 0 || key > 15)
                throw new ArgumentOutOfRangeException(nameof(key), "Key must be between 0 and 15.");
            return _keys[key];
        }

        public void UpdateFromPCKeyBoard()
        {
            // reset all keys first
            for (int i = 0; i < 16; i++)
                _keys[i] = false;
            // TODO: implement fully
            if (Raylib.IsKeyDown(KeyboardKey.One))
                _keys[0x1] = true;
            if (Raylib.IsKeyDown(KeyboardKey.Two))
                _keys[0x2] = true;
            if (Raylib.IsKeyDown(KeyboardKey.Three))
                _keys[0x3] = true;
            if (Raylib.IsKeyDown(KeyboardKey.Four)) 
                _keys[0xC] = true;

            if (Raylib.IsKeyDown(KeyboardKey.Q))
                _keys[0x4] = true;
            if (Raylib.IsKeyDown(KeyboardKey.W))
                _keys[0x5] = true;
            if (Raylib.IsKeyDown(KeyboardKey.E)) 
                _keys[0x6] = true;
            if (Raylib.IsKeyDown(KeyboardKey.R))
                _keys[0xD] = true;

            if (Raylib.IsKeyDown(KeyboardKey.A))
                _keys[0x7] = true;
            if (Raylib.IsKeyDown(KeyboardKey.S)) 
                _keys[0x8] = true;
            if (Raylib.IsKeyDown(KeyboardKey.D))
                _keys[0x9] = true;
            if (Raylib.IsKeyDown(KeyboardKey.F))
                _keys[0xE] = true;

            if (Raylib.IsKeyDown(KeyboardKey.Z))
                _keys[0xA] = true;
            if (Raylib.IsKeyDown(KeyboardKey.X))
                _keys[0x0] = true;
            if (Raylib.IsKeyDown(KeyboardKey.C))
                _keys[0xB] = true;
            if (Raylib.IsKeyDown(KeyboardKey.V))
                _keys[0xF] = true;
        }
    }
}
