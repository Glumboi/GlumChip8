using Raylib_cs;
using System;
using System.Collections.Generic;
using System.Reflection.Metadata.Ecma335;
using System.Runtime.InteropServices;
using System.Text;

namespace GlumChip8.Core
{
    public class CPU
    {
        // V0-F
        byte[] _registers = new byte[16];
        UInt16 _vI = 0; // I register

        UInt16 _PC = (ushort)MEMORY_SEGMENTS.ROM_DATA_START; // Program counter
        public UInt16 ProgramCounter { get => _PC; }
        int _programLength = 0;

        UInt16[] _stack = new UInt16[16];
        byte _SP = 0; // Stack pointer
        byte _dt = 0, _st = 0; // delay timer and sound timer

        const int CYCLES = 10;

        RAM _ram = new();
        public RAM RAM { get => _ram; }

        Keyboard _keyboard = new();
        public Keyboard Keyboard { get => _keyboard; }

        Display _display = new();
        public Display Display { get => _display; }

        Sound _sound = new();
        public Sound Sound { get => _sound; }

        string _romTitle = "[NO GAME]";
        public string RomTitle { get => _romTitle; }

        bool _running = false;

        public bool Running { get => _running; }

        public CPU()
        {
            _sound.Init();
        }

        public void Push(UInt16 value)
        {
            if (_SP >= 16)
                throw new StackOverflowException();

            _stack[_SP++] = value;
        }

        public UInt16 Pop()
        {
            if (_SP == 0)
                throw new InvalidOperationException();

            return _stack[--_SP];
        }

        public void WriteRegister(int reg, byte val)
        {
            if (reg > 0xF)
            {
                throw new ArgumentOutOfRangeException($"Register v{reg} may not be used by any program!");
            }

            _registers[reg] = val;
        }

        public byte ReadRegister(int reg)
        {
            if (reg > 0xF)
            {
                throw new ArgumentOutOfRangeException($"v{reg} is not a valid register!");
            }

            return _registers[reg];
        }

        public void LoadProgram(string title, byte[] program)
        {
            if (program.Length > 0xFFF - 0x200)
            {
                throw new ArgumentException("Program is too large to fit in memory!");
            }

            for (int i = 0; i < program.Length; i++)
            {
                _ram.WriteByte((UInt16)(0x200 + i), program[i]);
            }
            _programLength = program.Length;
            _romTitle = title;
        }

        public UInt16 Step()
        {
            ushort opcode = (ushort)((_ram.ReadByte(_PC) << 8) | _ram.ReadByte((ushort)(_PC + 1)));
            _PC += 2;
            return opcode;
        }

        public void Jump(UInt16 opcode)
        {
            _PC = (ushort)(opcode & 0x0FFF);
        }

        private void ResetRegisters()
        {
            for (int i = 0; i < _registers.Length; i++)
            {
                _registers[i] = 0;
            }
            _vI = 0;
            _PC = (ushort)MEMORY_SEGMENTS.ROM_DATA_START;
            _SP = 0;
            _dt = 0;
            _st = 0;
        }

        public void ResetCurrentRom()
        {
            ResetRegisters();
            _display.Clear();
            _keyboard = new();
        }

        public void InitSystemDefault()
        {
            ResetRegisters();
            _display.Clear();
            _keyboard = new();
            _running = false;
            _programLength = 0;
        }

        public void ExecuteCurrent()
        {
            for (int i = 0; i < CYCLES; i++)
            {
                if (_PC >= 0x200 + _programLength) break;

                UInt16 opcode = Step();

                switch (opcode & 0xF000)
                {
                    case 0x1000: Jump(opcode); break;
                    case 0x0000:
                        {
                            switch (opcode)
                            {
                                case 0x00E0:
                                    _display.Clear();
                                    break;
                                case 0x00EE:
                                    _PC = Pop();
                                    break;
                                default:
                                    Console.WriteLine($"Unknown 0x0 opcode {opcode:X4}");
                                    break;
                            }
                            break;
                        }
                    case 0x2000: // call subroutine
                        {
                            Push(_PC);
                            Jump(opcode);
                            break;
                        }
                    case 0x3000:
                        {
                            byte x = (byte)((opcode & 0x0F00) >> 8);
                            byte n = ReadRegister(x);
                            if (n == (opcode & 0x00FF))
                            {
                                _PC += 2; // skip next instruction
                            }
                            break;
                        }
                    case 0x4000:
                        {
                            byte x = (byte)((opcode & 0x0F00) >> 8);
                            byte n = ReadRegister(x);
                            if (n != (opcode & 0x00FF))
                            {
                                _PC += 2;
                            }
                            break;
                        }
                    case 0x5000:
                        {
                            byte x = (byte)((opcode & 0x0F00) >> 8);
                            byte y = (byte)((opcode & 0x00F0) >> 4);
                            if ((opcode & 0x000F) == 0x0000)
                            {
                                if (ReadRegister(x) == ReadRegister(y))
                                {
                                    _PC += 2;
                                }
                            }
                            break;
                        }
                    case 0x6000:
                        {
                            byte x = (byte)((opcode & 0x0F00) >> 8);
                            byte nn = (byte)(opcode & 0x00FF);
                            WriteRegister(x, nn);
                            break;
                        }
                    case 0x7000:
                        {
                            byte x = (byte)((opcode & 0x0F00) >> 8);
                            byte nn = (byte)(opcode & 0x00FF);
                            WriteRegister(x, (byte)(ReadRegister(x) + nn));
                            break;
                        }
                    case 0x8000:
                        {
                            byte x = (byte)((opcode & 0x0F00) >> 8);
                            byte y = (byte)((opcode & 0x00F0) >> 4);
                            switch (opcode & 0x000F)
                            {
                                case 0x0000: WriteRegister(x, ReadRegister(y)); break;
                                case 0x0001: WriteRegister(x, (byte)(ReadRegister(x) | ReadRegister(y))); break;
                                case 0x0002: WriteRegister(x, (byte)(ReadRegister(x) & ReadRegister(y))); break;
                                case 0x0003: WriteRegister(x, (byte)(ReadRegister(x) ^ ReadRegister(y))); break;
                                case 0x0004:
                                    {
                                        int sum = ReadRegister(x) + ReadRegister(y);
                                        WriteRegister(x, (byte)(sum & 0xFF));
                                        WriteRegister(0xF, (byte)(sum > 255 ? 1 : 0));
                                        break;
                                    }
                                default:
                                    Console.WriteLine($"Unknown opcode {opcode:X4}");
                                    break;
                            }
                            break;
                        }
                    case 0x9000:
                        {
                            byte x = (byte)((opcode & 0x0F00) >> 8);
                            byte y = (byte)((opcode & 0x00F0) >> 4);
                            if ((opcode & 0x000F) == 0x0000)
                            {
                                if (ReadRegister(x) != ReadRegister(y))
                                {
                                    _PC += 2;
                                }
                            }
                            break;
                        }
                    case 0xA000:
                        {
                            _vI = (ushort)(opcode & 0x0FFF);
                            break;
                        }
                    case 0xB000:
                        {
                            Jump((ushort)((opcode & 0x0FFF) + ReadRegister(0)));
                            break;
                        }
                    case 0xC000:
                        {
                            byte x = (byte)((opcode & 0x0F00) >> 8);
                            byte nn = (byte)(opcode & 0x00FF);
                            byte randomByte = (byte)(new Random().Next(0, 256));
                            WriteRegister(x, (byte)(randomByte & nn));
                            break;
                        }
                    case 0xD000:
                        {
                            // draw sprite at (Vx, Vy) with width 8 pixels and height N pixels
                            // set VF to 1 if any pixels are flipped from set to unset when the sprite is drawn, and 0 otherwise
                            byte x = ReadRegister((byte)((opcode & 0x0F00) >> 8));
                            byte y = ReadRegister((byte)((opcode & 0x00F0) >> 4));
                            byte height = (byte)(opcode & 0x000F);
                            byte[] sprite = new byte[height];

                            for (int j = 0; j < height; j++)
                            {
                                sprite[j] = (byte)_ram.ReadByte((ushort)(_vI + j));
                            }

                            _registers[0xF] = (byte)(_display.DrawSprite(x, y, sprite) ? 1 : 0);
                            break;
                        }
                    case 0xE000:
                        {
                            byte x = (byte)((opcode & 0x0F00) >> 8);
                            switch (opcode & 0x00FF)
                            {
                                case 0x009E:
                                    // skip next instruction if key with the value of Vx is pressed
                                    if (_keyboard.IsKeyPressed(ReadRegister(x)))
                                    {
                                        _PC += 2;
                                        break;
                                    }
                                    break;
                                case 0x00A1:
                                    // skip next instruction if key with the value of Vx is not pressed
                                    if (!_keyboard.IsKeyPressed(ReadRegister(x)))
                                    {
                                        _PC += 2;
                                        break;
                                    }
                                    break;
                                default:
                                    Console.WriteLine($"Unknown opcode {opcode:X4}");
                                    break;
                            }
                            break;
                        }
                    case 0xF000:
                        {
                            byte x = (byte)((opcode & 0x0F00) >> 8);
                            switch (opcode & 0x00FF)
                            {
                                case 0x0007:
                                    // set Vx = delay timer value
                                    WriteRegister(x, _dt);
                                    break;
                                case 0x000A: // wait for key press
                                    bool keyPressed = false;
                                    for (int key = 0; key < 16; key++)
                                    {
                                        if (_keyboard.IsKeyPressed(key))
                                        {
                                            WriteRegister(x, (byte)key);
                                            keyPressed = true;
                                            break;
                                        }
                                    }
                                    if (!keyPressed)
                                    {
                                        _PC -= 2; // stay on this instruction until key pressed
                                    }
                                    break;
                                case 0x0015:
                                    // set delay timer = Vx
                                    _dt = ReadRegister(x);
                                    break;
                                case 0x0018:
                                    // set sound timer = Vx
                                    _st = ReadRegister(x);
                                    break;
                                case 0x001E:
                                    // set I = I + Vx
                                    _vI = (ushort)(_vI + ReadRegister(x));
                                    break;
                                case 0x0029:
                                    // set I = location of sprite for digit Vx
                                    byte digit = ReadRegister(x);
                                    _vI = (ushort)(0x050 + (digit * 5));
                                    break;
                                case 0x0033:
                                    // store BCD representation of Vx in memory locations I, I+1, and I+2
                                    byte value = ReadRegister(x);

                                    _ram.WriteByte(_vI, (byte)(value / 100));
                                    _ram.WriteByte((ushort)(_vI + 1), (byte)((value / 10) % 10));
                                    _ram.WriteByte((ushort)(_vI + 2), (byte)(value % 10));
                                    break;
                                case 0x0055:
                                    // store registers V0 through Vx in memory starting at location I
                                    for (int j = 0; j <= x; j++)
                                    {
                                        _ram.WriteByte((ushort)(_vI + j), ReadRegister(j));
                                    }

                                    break;
                                case 0x0065:
                                    // read registers V0 through Vx from memory starting at location I
                                    for (int j = 0; j <= x; j++)
                                    {
                                        WriteRegister(j, (byte)_ram.ReadByte((ushort)(_vI + j)));
                                    }
                                    break;
                                default:
                                    Console.WriteLine($"Unknown opcode {opcode:X4}");
                                    break;
                            }
                            break;
                        }
                    default:
                        {

                            System.Console.WriteLine($"Opcode {opcode:X4} not implemented yet! skipping execution.");
                            break;
                        }
                }
            }
            if (_st != 0)
            {
                _sound.Beep();
                _st--;
            }
        }

        public void RunLoadedProgram()
        {
            if (!Raylib.IsWindowReady()) throw new Exception("Need to initialize a raylib window before running a ROM!");
            _running = true;
            while (!Raylib.WindowShouldClose())
            {
                if (!_running)
                {
                    continue;
                } // pausing

                _keyboard.UpdateFromPCKeyBoard();
                ExecuteCurrent();
                _display.Render();
            }
            Raylib.CloseWindow();
        }

        public string GetRegisterState()
        {
            StringBuilder stringBuilder = new StringBuilder();
            for (int i = 0; i < _registers.Length; i++)
            {
                stringBuilder.Append($"v{i:X}: {_registers[i]:X2} ");
            }
            return stringBuilder.ToString();
        }

        public void SetRunning(bool val)
        {
            _running = val;
        }
    }
}
