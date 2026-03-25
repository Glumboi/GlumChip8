using GlumChip8.Core;
using Raylib_cs;
using System.Runtime.InteropServices;

namespace GlumChip8.Console
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

        [DllImport("GlumChip8_C.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Chip8Keyboard_UpdateKeys([MarshalAs(UnmanagedType.LPArray, SizeConst = 16)] bool[] keys, int lastPressed);
    }

    internal class Program
    {
        static Keyboard kb = new();
        static void Main(string[] args)
        {
            Chip8Native.RunChip8Rom_FromFile(@"F:\Github\GlumChip8\TestRoms\6-keypad.ch8");
            Raylib.InitWindow(800, 600, "TEST");
            float scaleX = (float)Raylib.GetScreenWidth() / 128;
            float scaleY = (float)Raylib.GetScreenHeight() / 64;

            while (!Raylib.WindowShouldClose())
            {
                Raylib.BeginDrawing();
                Raylib.ClearBackground(Color.Black);
                kb.UpdateFromPCKeyBoard();
                Chip8Native.Chip8Keyboard_UpdateKeys(kb._keys, kb.LastPressed);
                Chip8Native.Chip8_CPU_Step();
                IntPtr planePtr = Chip8Native.Chip8Display_GetPlane(0);
                if (planePtr == IntPtr.Zero)
                {
                    System.Console.WriteLine("Error: plane pointer is null!");
                }
                else
                {
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
                Raylib.EndDrawing();
            }
        }
    }
}
