#ifndef CHIP8_DEFINES_H
#define CHIP8_DEFINES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <raylib.h>

typedef uint8_t Word;
typedef uint16_t DoubleWord;

#define CHIP8_REG_CNT 16
#define CHIP8_STACK_CNT 16
#define CHIP8_CYCLES 10
#define CHIP8_DEFAULT_ACTIVE_PLANES 0b01

#define CHIP8_RAM_SZ 65536
#define CHIP8_PROGRAM_START 0x200

#define CHIP8_MEM_START 0
#define CHIP8_MEM_RESERVED_END 0x1FF
#define CHIP8_MEM_RAM_END 0xFFF

#define CHIP8_SCREEN_WIDTH 128
#define CHIP8_SCREEN_HEIGHT 64

// Corrected macros for flattened 1D arrays
#define CHIP8_DISPLAY_GET_NUMBER_OF_PLANES(display) 2
#define CHIP8_DISPLAY_GET_NUMBER_OF_ELEMENTS(display) (CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT * CHIP8_DISPLAY_GET_NUMBER_OF_PLANES(display))
#define CHIP8_DISPLAY_GET_WIDTH(display) CHIP8_SCREEN_WIDTH
#define CHIP8_DISPLAY_GET_HEIGHT(display) CHIP8_SCREEN_HEIGHT

#define CHIP8_DISPLAY_COLOR WHITE

#endif