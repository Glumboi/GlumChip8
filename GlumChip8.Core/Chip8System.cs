using Raylib_cs;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks.Sources;

namespace GlumChip8.Core
{
    internal class Chip8Native
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
        public static extern void Chip8_CPU_SetRunning([MarshalAs(UnmanagedType.I1)] bool v);

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_Display_Render();

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Chip8Display_GetPlane(int planeIndex);

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Init_Chip8Emulator();

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8_Reset_Rom();
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
            //Keyboard.UpdateFromPCKeyBoard();
            //CPU.ExecuteCurrent();
            Chip8Native.Chip8_CPU_Step();
            IntPtr planePtr = Chip8Native.Chip8Display_GetPlane(0);
            if (planePtr == IntPtr.Zero)
            {
                System.Console.WriteLine("Error: plane pointer is null!");
            }
            else
            {
                float scaleX = (float)Raylib.GetScreenWidth() / 128;
                float scaleY = (float)Raylib.GetScreenHeight() / 64;

                unsafe
                {
                    byte* plane = (byte*)planePtr;
                    for (int y = 0; y < 64; y++)
                    {
                        for (int x = 0; x < 128; x++)
                        {
                            if (plane[y * 128 + x] != 0)
                                Raylib.DrawRectangle(
                                    (int)(x * scaleX),
                                    (int)(y * scaleY),
                                    (int)scaleX,
                                    (int)scaleY,
                                    Color.White
                                );
                        }
                    }
                }
            }
            Display.Render();
        }

        public void LaunchFromFile(string file)
        {
            Chip8Native.RunChip8Rom_FromFile(file);
            Chip8Native.Chip8_CPU_SetRunning(true);
            //var b = File.ReadAllBytes(file);
            //CPU.InitSystemDefault();
            //CPU.LoadProgram(Path.GetFileNameWithoutExtension(file), b);
            //CPU.SetRunning(true);
        }

    }

}
