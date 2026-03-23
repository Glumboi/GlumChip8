using Raylib_cs;
using System;
using System.Collections.Generic;
using System.Text;

namespace GlumChip8.Core
{
    public class Sound
    {
        private Raylib_cs.Sound _buzzSound;

        public void Init()
        {
            if (!Raylib.IsAudioDeviceReady())
                Raylib.InitAudioDevice();
            if (_buzzSound.Stream.Buffer == IntPtr.Zero)
            {
                _buzzSound = Raylib.LoadSound("./Assets/buzz.wav");
            }
        }

        public void Beep()
        {
            Raylib.PlaySound(_buzzSound);
        }

        ~Sound()
        {
            Raylib.UnloadSound(_buzzSound);
            Raylib.CloseAudioDevice();
        }
    }
}
