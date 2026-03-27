#include "Chip8CPU.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Chip8CPU Chip8CPU_Create()
{
	Chip8CPU ret = { 0 };
	ret._activePlanes = CHIP8_DEFAULT_ACTIVE_PLANES;
	return ret;
}

void Chip8CPU_Push(Chip8CPU* cpu, DoubleWord val)
{
	if (cpu->_SP >= CHIP8_STACK_CNT)
	{
		fprintf(stderr, "Stack overflowed!\n");
		cpu->_running = false;
		return;
	}
	cpu->_stack[cpu->_SP++] = val;
}

DoubleWord Chip8CPU_Pop(Chip8CPU* cpu)
{
	if (cpu->_SP == 0)
	{
		fprintf(stderr, "Stack underflowed!\n");
		cpu->_running = false;
		return 0xFFFF;
	}
	return cpu->_stack[--cpu->_SP];
}

void Chip8CPU_WriteRegister(Chip8CPU* cpu, Word targetReg, Word val)
{
	if (targetReg >= CHIP8_REG_CNT)
	{
		fprintf(stderr, "Target Register out of range!\n");
		cpu->_running = false;
		return;
	}
	cpu->_registers[targetReg] = val;
}

Word Chip8CPU_ReadRegister(Chip8CPU* cpu, Word targetReg)
{
	if (targetReg >= CHIP8_REG_CNT)
	{
		fprintf(stderr, "Target Register out of range!\n");
		cpu->_running = false;
		return 0xFF;
	}
	return cpu->_registers[targetReg];
}

void Chip8CPU_LoadProgram(Chip8CPU* cpu, Word* program, size_t programSz)
{
	for (size_t i = 0; i < programSz; i++)
		Chip8RAM_WriteByte(cpu->_ram, CHIP8_PROGRAM_START + i, program[i]);
	cpu->_programLen = programSz;
}

DoubleWord Chip8CPU_Step(Chip8CPU* cpu)
{
	DoubleWord ret = (Chip8RAM_ReadByte(cpu->_ram, cpu->_PC) << 8) |
		Chip8RAM_ReadByte(cpu->_ram, cpu->_PC + 1);
	cpu->_PC += 2;
	return ret;
}

void Chip8CPU_Jump(Chip8CPU* cpu, DoubleWord opcode)
{
	cpu->_PC = opcode & 0x0FFF;
}

void Chip8CPU_ResetRegisters(Chip8CPU* cpu)
{
	memset(cpu->_registers, 0, sizeof(cpu->_registers));
	cpu->_vI = 0;
	cpu->_PC = CHIP8_PROGRAM_START;
	cpu->_SP = 0;
	cpu->_DT = 0;
	cpu->_ST = 0;
}

void Chip8CPU_ResetCurrentRom(Chip8CPU* cpu)
{
	Chip8CPU_ResetRegisters(cpu);
	Chip8Display_Clear(g_chip8SystemInstance._display, 0);
}

void Chip8CPU_ExecuteCurrent(Chip8CPU* cpu)
{
	//for (size_t cycle = 0; cycle < CHIP8_CYCLES; cycle++)
	//{
		if (!cpu->_running) return;
		if (cpu->_PC >= CHIP8_PROGRAM_START + cpu->_programLen) return;

		DoubleWord opcode = Chip8CPU_Step(cpu);

		switch (opcode & 0xF000)
		{
		case 0x0000:
			switch (opcode)
			{
			case 0x00E0: // CLS
				for (int p = 0; p < 2; p++)
				{
					if ((cpu->_activePlanes & (1 << p)) != 0)
					{
						Chip8Display_Clear(g_chip8SystemInstance._display, p);
					}
				}
				break;
			case 0x00EE: // RET
				cpu->_PC = Chip8CPU_Pop(cpu);
				break;
			case 0x00FB: // SCR (scroll right 4)
				Chip8Display_Scroll(g_chip8SystemInstance._display, 4, Right);
				break;
			case 0x00FC: // SCL (scroll left 4)
				Chip8Display_Scroll(g_chip8SystemInstance._display, 4, Left);
				break;
			case 0x00FD: // EXIT
				Chip8CPU_ResetRegisters(cpu);
				break;
			case 0x00FE: // LOW
				Chip8Display_SetResolution(g_chip8SystemInstance._display, 64, 32);
				break;
			case 0x00FF: // HIGH
				Chip8Display_SetResolution(g_chip8SystemInstance._display, 128, 64);
				break;
			default:
				if ((opcode & 0x00F0) == 0x00C0)
				{ // Scroll Down
					Chip8Display_Scroll(g_chip8SystemInstance._display, opcode & 0x000F, Down);
				}
				else if ((opcode & 0x00F0) == 0x00D0)
				{ // Scroll Up (XO-CHIP)
					Chip8Display_Scroll(g_chip8SystemInstance._display, opcode & 0x000F, Up);
				}
				else
				{
					fprintf(stderr, "Unknown 0x0 opcode %04X\n", opcode);
				}
				break;
			}
			break;

		case 0x1000: Chip8CPU_Jump(cpu, opcode); break;
		case 0x2000: Chip8CPU_Push(cpu, cpu->_PC); Chip8CPU_Jump(cpu, opcode); break;
		case 0x3000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			if (Chip8CPU_ReadRegister(cpu, x) == (opcode & 0x00FF))
				cpu->_PC += 2;
			break;
		}
		case 0x4000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			if (Chip8CPU_ReadRegister(cpu, x) != (opcode & 0x00FF))
				cpu->_PC += 2;
			break;
		}
		case 0x5000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			Word y = (opcode & 0x00F0) >> 4;
			switch (opcode & 0x000F)
			{
			case 0x0: // Skip if VX == VY
				if (Chip8CPU_ReadRegister(cpu, x) == Chip8CPU_ReadRegister(cpu, y)) cpu->_PC += 2;
				break;
			case 0x2: // Save VX through VY to I (XO-CHIP)
				for (int i = x, offset = 0; i <= y; i++, offset++)
					Chip8RAM_WriteByte(cpu->_ram, cpu->_vI + offset, cpu->_registers[i]);
				break;
			case 0x3: // Load VX through VY from I (XO-CHIP)
				for (int i = x, offset = 0; i <= y; i++, offset++)
					cpu->_registers[i] = Chip8RAM_ReadByte(cpu->_ram, cpu->_vI + offset);
				break;
			}
			break;
		}
		case 0x6000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			Chip8CPU_WriteRegister(cpu, x, opcode & 0x00FF);
			break;
		}
		case 0x7000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			Chip8CPU_WriteRegister(cpu, x,
				(Chip8CPU_ReadRegister(cpu, x) + (opcode & 0x00FF)) & 0xFF);
			break;
		}
		case 0x8000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			Word y = (opcode & 0x00F0) >> 4;
			switch (opcode & 0x000F)
			{
			case 0x0: Chip8CPU_WriteRegister(cpu, x, Chip8CPU_ReadRegister(cpu, y));
				break;
			case 0x1: Chip8CPU_WriteRegister(cpu, x,
				Chip8CPU_ReadRegister(cpu, x) | Chip8CPU_ReadRegister(cpu, y));
				break;
			case 0x2: Chip8CPU_WriteRegister(cpu, x,
				Chip8CPU_ReadRegister(cpu, x) & Chip8CPU_ReadRegister(cpu, y));
				break;
			case 0x3: Chip8CPU_WriteRegister(cpu, x,
				Chip8CPU_ReadRegister(cpu, x) ^ Chip8CPU_ReadRegister(cpu, y));
				break;
			case 0x4:
			{
				int sum = Chip8CPU_ReadRegister(cpu, x) + Chip8CPU_ReadRegister(cpu, y);
				Chip8CPU_WriteRegister(cpu, x, sum & 0xFF);
				Chip8CPU_WriteRegister(cpu, 0xF, sum > 0xFF ? 1 : 0);
				break;
			}
			case 0x5:
			{
				Word vx = Chip8CPU_ReadRegister(cpu, x);
				Word vy = Chip8CPU_ReadRegister(cpu, y);
				Chip8CPU_WriteRegister(cpu, x, (vx - vy) & 0xFF);
				Chip8CPU_WriteRegister(cpu, 0xF, vx >= vy ? 1 : 0);
				break;
			}
			case 0x6: // Shift Right
			{
				Word vx = Chip8CPU_ReadRegister(cpu, x);
				Chip8CPU_WriteRegister(cpu, 0xF, vx & 0x01);
				Chip8CPU_WriteRegister(cpu, x, vx >> 1);
				break;
			}
			case 0x7:
			{
				Word vx = Chip8CPU_ReadRegister(cpu, x);
				Word vy = Chip8CPU_ReadRegister(cpu, y);
				Chip8CPU_WriteRegister(cpu, x, (vy - vx) & 0xFF);
				Chip8CPU_WriteRegister(cpu, 0xF, vy >= vx ? 1 : 0);
				break;
			}
			case 0xE: // Shift Left
			{
				Word vx = Chip8CPU_ReadRegister(cpu, x);
				Chip8CPU_WriteRegister(cpu, 0xF, (vx & 0x80) >> 7);
				Chip8CPU_WriteRegister(cpu, x, (vx << 1) & 0xFF);
				break;
			}
			default:
				fprintf(stderr, "Unknown 0x8 opcode %04X\n", opcode);
				break;
			}
			break;
		}
		case 0x9000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			Word y = (opcode & 0x00F0) >> 4;
			if ((opcode & 0x000F) == 0 && Chip8CPU_ReadRegister(cpu, x) != Chip8CPU_ReadRegister(cpu, y))
				cpu->_PC += 2;
			break;
		}
		case 0xA000:
		{
			cpu->_vI = opcode & 0x0FFF;
			break;
		}
		case 0xB000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			if (0)//(cpu->_quirks.bnnnUsesVx)
				cpu->_PC = (opcode & 0x0FFF) + cpu->_registers[x];
			else
				cpu->_PC = (opcode & 0x0FFF) + cpu->_registers[0];
			break;
		}
		case 0xC000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			Word nn = opcode & 0x00FF;
			Word rnd = rand() % 256;
			Chip8CPU_WriteRegister(cpu, x, rnd & nn);
			break;
		}
		case 0xD000:
		{
			Word vx = Chip8CPU_ReadRegister(cpu, (opcode & 0x0F00) >> 8);
			Word vy = Chip8CPU_ReadRegister(cpu, (opcode & 0x00F0) >> 4);
			Word n = opcode & 0x000F;
			bool collision = false;

			// Quirk: XO-CHIP/SCHIP usually wraps X and Y start positions
			vx %= (g_chip8SystemInstance._display->_isHighRes ? 128 : 64);
			vy %= (g_chip8SystemInstance._display->_isHighRes ? 64 : 32);

			if (n == 0)
			{ // 16x16 Sprite
				Word sprite16[32];
				for (int j = 0; j < 32; j++)
					sprite16[j] = Chip8RAM_ReadByte(cpu->_ram, cpu->_vI + j);

				for (int p = 0; p < 2; p++)
				{
					if (cpu->_activePlanes & (1 << p))
						collision |= Chip8Display_DrawSprite16(g_chip8SystemInstance._display, vx, vy, sprite16, sizeof(sprite16) / sizeof(sprite16[0]), p);
				}
			}
			else
			{ // 8xN Sprite
				Word sprite8[16];
				for (int j = 0; j < n; j++)
					sprite8[j] = Chip8RAM_ReadByte(cpu->_ram, cpu->_vI + j);

				for (int p = 0; p < 2; p++)
				{
					if (cpu->_activePlanes & (1 << p))
						collision |= Chip8Display_DrawSprite(g_chip8SystemInstance._display, vx, vy, sprite8, n, p);
				}
			}
			cpu->_registers[0xF] = collision ? 1 : 0;
			break;
		}

		case 0xE000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			Word key = Chip8CPU_ReadRegister(cpu, x);
			if (key >= 16) break;
			switch (opcode & 0x00FF)
			{
			case 0x9E:
				if (g_chip8SystemInstance._keyboard->_keys[key])
					cpu->_PC += 2;
				break;
			case 0xA1:
				if (!g_chip8SystemInstance._keyboard->_keys[key])
					cpu->_PC += 2;
				break;
			}
			break;
		}
		case 0xF000:
		{
			Word x = (opcode & 0x0F00) >> 8;
			switch (opcode & 0x00FF)
			{
			case 0x00:
			{
				DoubleWord address = (Chip8RAM_ReadByte(cpu->_ram, cpu->_PC) << 8) |
					Chip8RAM_ReadByte(cpu->_ram, cpu->_PC + 1);
				cpu->_PC += 2;
				cpu->_PC = address;
				break;
			}
			case 0x07:
			{

				Chip8CPU_WriteRegister(cpu, x, cpu->_DT);
				break;
			}
			case 0x0A:
			{
				if (g_chip8SystemInstance._keyboard->_lastPressed == -1)
					cpu->_PC -= 2; // repeat instruction
				else
				{
					Chip8CPU_WriteRegister(cpu, x, g_chip8SystemInstance._keyboard->_lastPressed);
					g_chip8SystemInstance._keyboard->_lastPressed = -1;
				}
				break;
			}
			case 0x15:
			{
				cpu->_DT = Chip8CPU_ReadRegister(cpu, x);
				break;
			}
			case 0x18:
			{
				cpu->_ST = Chip8CPU_ReadRegister(cpu, x);
				break;
			}
			case 0x1E:
			{
				Word val = Chip8CPU_ReadRegister(cpu, x);
				if (cpu->_vI + val > 0xFFF)
				{
					cpu->_registers[0xF] = 1;
				}
				else
				{
					cpu->_registers[0xF] = 0;
				}

				cpu->_vI += val;
				break;
			}
			case 0x29:
			{
				cpu->_vI = 0x50 + (Chip8CPU_ReadRegister(cpu, x) * 5);
				break;
			}
			case 0x33:
			{
				Chip8RAM_WriteByte(cpu->_ram, cpu->_vI, Chip8CPU_ReadRegister(cpu, x) / 100);
				Chip8RAM_WriteByte(cpu->_ram, cpu->_vI + 1, (Chip8CPU_ReadRegister(cpu, x) / 10) % 10);
				Chip8RAM_WriteByte(cpu->_ram, cpu->_vI + 2, Chip8CPU_ReadRegister(cpu, x) % 10);
				break;
			}
			case 0x55:
			{
				for (int j = 0; j <= x; j++)
					Chip8RAM_WriteByte(cpu->_ram, cpu->_vI + j, Chip8CPU_ReadRegister(cpu, j));
				// TODO: Implement quirks properly
				//cpu->_vI += x + 1;
				break;
			}
			case 0x65: // Read registers V0 through VX from memory starting at I
			{
				for (int j = 0; j <= x; j++)
					Chip8CPU_WriteRegister(cpu, j, Chip8RAM_ReadByte(cpu->_ram, cpu->_vI + j));
				// TODO: Implement quirks properly
				//cpu->_vI += x + 1;
				break;
			}
			case 0x75:
			{
				for (int j = 0; j <= x; j++)
					cpu->_flagRegisters[j] = cpu->_registers[j];
				break;
			}
			case 0x85:
			{
				for (int j = 0; j <= x; j++)
					cpu->_registers[j] = cpu->_flagRegisters[j];
				break;
			}
			case 0xF1:
			{
				break;
			}
			case 0xF2: // load audio pattern (W.I.P)
			{
				break;
			}
			default:
			{
				fprintf(stderr, "Unknown 0xF opcode %04X\n", opcode);
				break;
			}
			}
			break;
		}
		default:
		{
			fprintf(stderr, "Unknown opcode: %04X\n", opcode);
			break;
		}
		}
	//}
}