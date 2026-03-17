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
        }
    }
}
