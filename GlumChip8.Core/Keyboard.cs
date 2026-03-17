using Raylib_cs;
using System;
using System.Collections.Generic;
using System.Text;

namespace GlumChip8.Core
{
    public class Keyboard
    {
        private readonly bool[] _keys = new bool[16];

        // map raylib keys to chip8 values
        private readonly Dictionary<KeyboardKey, byte> _keyMap = new()
        {
            { KeyboardKey.One, 0x1 }, { KeyboardKey.Two, 0x2 }, { KeyboardKey.Three, 0x3 }, { KeyboardKey.Four, 0xC },
            { KeyboardKey.Q, 0x4 },   { KeyboardKey.W, 0x5 },   { KeyboardKey.E, 0x6 },     { KeyboardKey.R, 0xD },
            { KeyboardKey.A, 0x7 },   { KeyboardKey.S, 0x8 },   { KeyboardKey.D, 0x9 },     { KeyboardKey.F, 0xE },
            { KeyboardKey.Z, 0xA },   { KeyboardKey.X, 0x0 },   { KeyboardKey.C, 0xB },     { KeyboardKey.V, 0xF }
        };

        public bool IsKeyPressed(int key) => _keys[key];

        public int GetLastPressedHexKey()
        {
            KeyboardKey key = (KeyboardKey)Raylib.GetKeyPressed();
            if (_keyMap.TryGetValue(key, out byte hexValue))
            {
                return hexValue;
            }
            return -1;
        }

        public void UpdateFromPCKeyBoard()
        {
            foreach (var pair in _keyMap)
            {
                _keys[pair.Value] = Raylib.IsKeyDown(pair.Key);
            }
        }
    }
}
