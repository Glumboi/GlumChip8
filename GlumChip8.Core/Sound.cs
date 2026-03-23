using Raylib_cs;
using System;

namespace GlumChip8.Core
{
    public class Sound
    {
        private Raylib_cs.Sound _buzzSound;
        private byte[] _patternBuffer = new byte[16]; // XO-CHIP pattern buffer
        private double _frequency = 4000;             // pitch for playback

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
            if (_buzzSound.Stream.Buffer != IntPtr.Zero)
                Raylib.PlaySound(_buzzSound);
        }

        public void LoadPattern(byte[] pattern)
        {
            if (pattern.Length != 16)
                throw new ArgumentException("Audio pattern must be 16 bytes.");
            _patternBuffer = pattern;
        }

        public void SetFrequency(double freq)
        {
            _frequency = freq;
            // Adjust playback logic if you implement pattern-based sound
        }

        ~Sound()
        {
            if (_buzzSound.Stream.Buffer != IntPtr.Zero)
                Raylib.UnloadSound(_buzzSound);
            if (Raylib.IsAudioDeviceReady())
                Raylib.CloseAudioDevice();
        }
    }
}