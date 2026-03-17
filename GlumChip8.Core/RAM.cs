using System;
using System.Collections.Generic;
using System.Text;

namespace GlumChip8.Core
{
    public enum MEMORY_SEGMENTS
    {
        START = 0,
        RESERVED_END = 0x1FF,
        ROM_DATA_START = 0x200,
        RAM_END = 0xFFF,
    }

    public class RAM
    {
        const int MEMORY_SIZE = 4096;
        UInt16[] _data = new UInt16[MEMORY_SIZE];

        public void WriteByte(UInt16 address, byte value)
        {
            if (address >= MEMORY_SIZE)
            {
                throw new ArgumentOutOfRangeException($"Address {address} is out of bounds!");
            }
            _data[address] = value;
        }

        public UInt16 ReadByte(UInt16 address)
        {
            if (address >= MEMORY_SIZE)
            {
                throw new ArgumentOutOfRangeException($"Address {address} is out of bounds!");
            }
            return _data[address];
        }
    }
}
