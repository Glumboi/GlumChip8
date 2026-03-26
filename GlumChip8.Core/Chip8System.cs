using Raylib_cs;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks.Sources;

namespace GlumChip8.Core
{
    public class Chip8Native
    {

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void RunChip8Rom_FromFile(string file);
        // Add these two new methods to match the DLL

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_CPU_Step();

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)] // Forces 1-byte bool
        public static extern bool Chip8_CPU_GetRunning();
        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)] // Forces 1-byte bool
        public static extern bool Chip8Display_IsHighRes();

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)] // Forces 1-byte bool
        public static extern bool Chip8Display_IsPlaneActive(int p);

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_CPU_SetRunning([MarshalAs(UnmanagedType.I1)] bool v);

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_Display_Render();

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Chip8Display_GetPlane(int planeIndex);

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Init_Chip8Emulator();

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_Decrement_Timers();

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_Reset_Rom();
        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
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

            float screenW = Raylib.GetScreenWidth();
            float screenH = Raylib.GetScreenHeight();

            float scaleX = screenW / simW;
            float scaleY = screenH / simH;

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
                                Raylib.DrawRectangle(
                                    (int)(x * scaleX),
                                    (int)(y * scaleY),
                                    (int)Math.Ceiling(scaleX),
                                    (int)Math.Ceiling(scaleY),
                                    (p == 0) ? Color.White : Color.Gray
                                );
                            }
                        }
                    }
                }
            }
        }
        public void LaunchFromFile(string file)
        {
            Chip8Native.Chip8_CPU_SetRunning(false);
            Chip8Native.RunChip8Rom_FromFile(file);
            Chip8Native.Chip8_CPU_SetRunning(true);
        }

    }

}
