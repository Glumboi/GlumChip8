#ifndef CHIP8_CPU_H
#define CHIP8_CPU_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Chip8System.h"
#include "Chip8RAM.h"
#include "Chip8Display.h"
#include "Chip8Keyboard.h"

typedef struct _chip8CPU
{
	Word _registers[CHIP8_REG_CNT];
	Word _flagRegisters[CHIP8_REG_CNT];

	DoubleWord _vI, _PC;
	size_t _programLen;

	DoubleWord _stack[CHIP8_STACK_CNT];
	Word _SP, _DT, _ST;
	Word _activePlanes;

	bool _running;

	Chip8RAM* _ram;
} Chip8CPU;

Chip8CPU Chip8CPU_Create();
void Chip8CPU_Push(Chip8CPU* cpu, DoubleWord val);
DoubleWord Chip8CPU_Pop(Chip8CPU* cpu);
void Chip8CPU_WriteRegister(Chip8CPU* cpu, Word targetReg, Word val);
Word Chip8CPU_ReadRegister(Chip8CPU* cpu, Word targetReg);
void Chip8CPU_LoadProgram(Chip8CPU* cpu, Word* program, size_t programSz);
DoubleWord Chip8CPU_Step(Chip8CPU* cpu);
void Chip8CPU_Jump(Chip8CPU* cpu, DoubleWord loc);
void Chip8CPU_ResetRegisters(Chip8CPU* cpu);
void Chip8CPU_ResetCurrentRom(Chip8CPU* cpu);

void Chip8CPU_ExecuteCurrent(Chip8CPU* cpu);

#endif