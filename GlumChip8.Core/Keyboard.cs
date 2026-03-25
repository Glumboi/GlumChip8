using Raylib_cs;
using System.Collections.Generic;

public class Keyboard
{
    public readonly byte[] _keys = new byte[16];
    private int _lastPressed = -1;

    // Make LastPressed read/write for WPF updates
    public int LastPressed
    {
        get => _lastPressed;
        set => _lastPressed = value;
    }

    private readonly Dictionary<Raylib_cs.KeyboardKey, byte> _keyMap = new()
    {
        { KeyboardKey.One, 0x1 }, { KeyboardKey.Two, 0x2 }, { KeyboardKey.Three, 0x3 }, { KeyboardKey.Four, 0xC },
        { KeyboardKey.Q, 0x4 },   { KeyboardKey.W, 0x5 },   { KeyboardKey.E, 0x6 },     { KeyboardKey.R, 0xD },
        { KeyboardKey.A, 0x7 },   { KeyboardKey.S, 0x8 },   { KeyboardKey.D, 0x9 },     { KeyboardKey.F, 0xE },
        { KeyboardKey.Z, 0xA },   { KeyboardKey.X, 0x0 },   { KeyboardKey.C, 0xB },     { KeyboardKey.V, 0xF }
    };

    public void UpdateFromPCKeyBoard()
    {
        _lastPressed = -1;
        foreach (var pair in _keyMap)
        {
            if (Raylib.IsKeyDown(pair.Key))
            {
                _keys[pair.Value] = 1;
                if (_lastPressed == -1) _lastPressed = pair.Value;
            }
            else
            {
                _keys[pair.Value] = 0;
            }
        }
    }
    // Returns last pressed key and resets it
    public int GetLastPressedHexKey()
    {
        return _lastPressed;
    }

    // NEW: WPF key mapping
    public bool TryMapKey(System.Windows.Input.Key wpfKey, out byte chipKey)
    {
        chipKey = 0;
        switch (wpfKey)
        {
            case System.Windows.Input.Key.D1: chipKey = 0x1; break;
            case System.Windows.Input.Key.D2: chipKey = 0x2; break;
            case System.Windows.Input.Key.D3: chipKey = 0x3; break;
            case System.Windows.Input.Key.D4: chipKey = 0xC; break;
            case System.Windows.Input.Key.Q: chipKey = 0x4; break;
            case System.Windows.Input.Key.W: chipKey = 0x5; break;
            case System.Windows.Input.Key.E: chipKey = 0x6; break;
            case System.Windows.Input.Key.R: chipKey = 0xD; break;
            case System.Windows.Input.Key.A: chipKey = 0x7; break;
            case System.Windows.Input.Key.S: chipKey = 0x8; break;
            case System.Windows.Input.Key.D: chipKey = 0x9; break;
            case System.Windows.Input.Key.F: chipKey = 0xE; break;
            case System.Windows.Input.Key.Z: chipKey = 0xA; break;
            case System.Windows.Input.Key.X: chipKey = 0x0; break;
            case System.Windows.Input.Key.C: chipKey = 0xB; break;
            case System.Windows.Input.Key.V: chipKey = 0xF; break;
            default: return false;
        }
        return true;
    }
}