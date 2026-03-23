using Raylib_cs;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks.Sources;

namespace GlumChip8.Core
{
    public class Chip8System
    {
        public RAM RAM { get => _cpu.RAM; }

        public Keyboard Keyboard { get => _cpu.Keyboard; }

        public Display Display { get => _cpu.Display; }

        public Sound Sound { get => _cpu.Sound; }

        CPU _cpu = new();
        public CPU CPU { get => _cpu; }
        public bool Running { get => _cpu.Running; }

        public Chip8System()
        {

        }

        public void TogglePause()
        {
            CPU.SetRunning(!Running);
        }

        public void ResetCurrentRom()
        {
            CPU.ResetCurrentRom();
        }

        public void SetToDefault()
        {
            CPU.InitSystemDefault();
        }

        public void Update()
        {
            if (!Running)
                return;
            Keyboard.UpdateFromPCKeyBoard();
            CPU.ExecuteCurrent();
            Display.Render();
        }

        public IntPtr InitWindow(int w, int h, ReadOnlySpan<char> title)
        {
            Raylib.SetConfigFlags(ConfigFlags.UndecoratedWindow);
            Raylib.InitWindow(w, h, title.ToString());
            Raylib.SetTargetFPS(60);
            unsafe
            {
                return (nint)Raylib.GetWindowHandle();
            }
        }

        public IntPtr LaunchFromFile(string file)
        {
            var b = File.ReadAllBytes(file);
            CPU.InitSystemDefault();
            CPU.LoadProgram(Path.GetFileNameWithoutExtension(file), b);
            CPU.SetRunning(true);
            // current handle
            unsafe
            {
                return InitWindow(640, 320, "RaylibInternal");
            }

        }
    }
}
