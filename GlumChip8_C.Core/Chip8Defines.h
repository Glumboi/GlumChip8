#ifndef CHIP8_DEFINES_H
#define CHIP8_DEFINES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef uint8_t Word;
typedef uint16_t DoubleWord;

#define CHIP8_REG_CNT 0xF
#define CHIP8_STACK_CNT 16
#define CHIP8_CYCLES 10
#define CHIP8_DEFAULT_ACTIVE_PLANES 0b01

#define CHIP8_RAM_SZ 65536
#define CHIP8_PROGRAM_START 0x200

#define MEM_START 0
#define MEM_RESERVED_END 0x1FF
#define MEM_ROM_DATA_START 0x200
#define MEM_RAM_END = 0xFFF

#endif