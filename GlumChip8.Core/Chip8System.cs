using Raylib_cs;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks.Sources;

namespace GlumChip8.Core
{
    public class Chip8Native
    {

        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void RunChip8Rom_FromFile(string file);
        // Add these two new methods to match the DLL

        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_CPU_Step();

        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)] // Forces 1-byte bool
        public static extern bool Chip8_CPU_GetRunning();
        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)] // Forces 1-byte bool
        public static extern bool Chip8Display_IsHighRes();

        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)] // Forces 1-byte bool
        public static extern bool Chip8Display_IsPlaneActive(int p);

        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_CPU_SetRunning([MarshalAs(UnmanagedType.I1)] bool v);

        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_Display_Render();

        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Chip8Display_GetPlane(int planeIndex);

        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Init_Chip8Emulator();

        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_Decrement_Timers();

        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_Reset_Rom();
        [DllImport("./lib/GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8Keyboard_UpdateKeys([MarshalAs(UnmanagedType.LPArray, SizeConst = 16)] byte[] keys, int lastPressed);
    }

    public class Chip8System
    {
        public RAM RAM { get => _cpu.RAM; }

        public Keyboard Keyboard { get => _cpu.Keyboard; }

        public Display Display { get => _cpu.Display; }

        public Sound Sound { get => _cpu.Sound; }

        CPU _cpu = new();
        public CPU CPU { get => _cpu; }

        public bool Running { get => Chip8Native.Chip8_CPU_GetRunning(); }

        private double _lastCpuTime;
        private double _lastTimerTime;

        private const double CPU_HZ = 700.0;
        private const double TIMER_HZ = 60.0;

        const int MAX_CYCLES = 20;
        int cycles = 0;

        private RenderTexture2D _renderTexture;

        public Chip8System()
        {
        }

        public void TogglePause()
        {
            Chip8Native.Chip8_CPU_SetRunning(!Chip8Native.Chip8_CPU_GetRunning());
        }

        public void ResetCurrentRom()
        {
            //CPU.ResetCurrentRom();
            Chip8Native.Chip8_Reset_Rom();
        }

        public void SetToDefault()
        {
            //CPU.InitSystemDefault();
            Chip8Native.Init_Chip8Emulator();
        }

        public void Update()
        {


            cycles = 0;

            Chip8Native.Chip8Keyboard_UpdateKeys(Keyboard._keys, Keyboard.LastPressed);

            double now = Raylib.GetTime();
            while (now - _lastCpuTime >= 1.0 / CPU_HZ && cycles < MAX_CYCLES)
            {
                Chip8Native.Chip8_CPU_Step();
                _lastCpuTime += 1.0 / CPU_HZ;
                cycles++;
            }

            while (now - _lastTimerTime >= 1.0 / TIMER_HZ)
            {
                Chip8Native.Chip8_Decrement_Timers();
                _lastTimerTime += 1.0 / TIMER_HZ;
            }


            bool isHighRes = Chip8Native.Chip8Display_IsHighRes();
            int simW = isHighRes ? 128 : 64;
            int simH = isHighRes ? 64 : 32;

            int screenW = Raylib.GetScreenWidth();
            int screenH = Raylib.GetScreenHeight();
            ResizeRenderTexture(screenW, screenH);

            int scaleX = screenW / simW;
            int scaleY = screenH / simH;
            int scale = (int)MathF.Min(scaleX, scaleY);

            Raylib.BeginTextureMode(_renderTexture);
            Raylib.ClearBackground(Color.Black);
            unsafe
            {
                for (int p = 0; p < 2; p++)
                {
                    if (!Chip8Native.Chip8Display_IsPlaneActive(p)) continue;

                    byte* plane = (byte*)Chip8Native.Chip8Display_GetPlane(p);
                    if (plane == null) continue;
                    Color pColor = (p == 0) ? Color.White : Color.Gray;

                    for (int y = 0; y < simH; y++)
                    {
                        for (int x = 0; x < simW; x++)
                        {
                            if (plane[y * 128 + x] != 0)
                            {
                                Raylib.DrawRectangle(x * scale, y * scale, scale, scale, pColor);
                            }
                        }
                    }
                }
            }
            Raylib.EndTextureMode();

        }
        public void LaunchFromFile(string file)
        {
            Chip8Native.Chip8_CPU_SetRunning(false);
            Chip8Native.RunChip8Rom_FromFile(file);
            Chip8Native.Chip8_CPU_SetRunning(true);
            if (_renderTexture.Id == 0)
            {
                _renderTexture = Raylib.LoadRenderTexture(
                    Raylib.GetScreenWidth(),
                    Raylib.GetScreenHeight()
                );
            }
        }

        public void DrawRenderTexture()
        {
            Raylib.DrawTexturePro(_renderTexture.Texture,
                new Rectangle(0, 0, _renderTexture.Texture.Width, -_renderTexture.Texture.Height),
                new Rectangle(0, 0, Raylib.GetScreenWidth(), Raylib.GetScreenHeight()),
                new(0, 0), 0.0f, Raylib_cs.Color.White);
        }

        public void ResizeRenderTexture(int w, int h)
        {
            if (_renderTexture.Id != 0)
                Raylib.UnloadRenderTexture(_renderTexture);

            _renderTexture = Raylib.LoadRenderTexture(w, h);
        }

        ~Chip8System()
        {
            Raylib.UnloadRenderTexture(_renderTexture);
        }
    }

}
