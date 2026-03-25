#include "Chip8CPU.h"

Chip8CPU Chip8CPU_Create()
{
	Chip8CPU ret = { 0 };
	ret._activePlanes = CHIP8_DEFAULT_ACTIVE_PLANES;

	return ret;
}

void Chip8CPU_Push(Chip8CPU* cpu, DoubleWord val)
{
	if (cpu->_SP >= 16)
	{
		fprintf(stderr, "Stack overflowed!\n");
		return;
	}
	cpu->_stack[cpu->_SP++] = val;
}

DoubleWord Chip8CPU_Pop(Chip8CPU* cpu)
{
	if (cpu->_SP == 0)
	{
		fprintf(stderr, "Stack underflowed!\n");
		return 0xFFFF;
	}
	return cpu->_stack[--cpu->_SP];
}

void Chip8CPU_WriteRegister(Chip8CPU* cpu, Word targetReg, Word val)
{
	if (targetReg >= CHIP8_REG_CNT)
	{
		fprintf(stderr, "Target Register out of range!\n");
		return;
	}

	cpu->_registers[targetReg] = val;
}

Word Chip8CPU_ReadRegister(Chip8CPU* cpu, Word targetReg)
{
	if (targetReg >= CHIP8_REG_CNT)
	{
		fprintf(stderr, "Target Register out of range!\n");
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
	DoubleWord ret = (Chip8RAM_ReadByte(cpu->_ram, cpu->_PC) << 8) | Chip8RAM_ReadByte(cpu->_ram, cpu->_PC + 1);
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
	//_keyboard = new();
}

void Chip8CPU_ExecuteCurrent(Chip8CPU* cpu)
{
	for (size_t i = 0; i < CHIP8_CYCLES; i++)
	{
		if (cpu->_PC >= CHIP8_PROGRAM_START + cpu->_programLen) break;

		DoubleWord opcode = Chip8CPU_Step(cpu);
		fprintf(stdout, "Executing opcode %04X\n", opcode);
		switch (opcode & 0xF000)
		{
		case 0x1000: Chip8CPU_Jump(cpu, opcode); break;
		case 0x0000:
		{
			switch (opcode)
			{
			case 0x00E0: // CLS: Clear screen
				Chip8Display_Clear(g_chip8SystemInstance._display, 0);
				break;

			case 0x00EE: // RET: Return from subroutine
				cpu->_PC = Chip8CPU_Pop(cpu);
				break;

			case 0x00FB: // SCR: Scroll right 4 pixels
				Chip8Display_Scroll(g_chip8SystemInstance._display, 4, 2);
				break;

			case 0x00FC: // SCL: Scroll left 4 pixels
				Chip8Display_Scroll(g_chip8SystemInstance._display, 4, 1);
				break;

			case 0x00FD: // EXIT: Terminate interpreter
				//InitSystemDefault();
				break;

			case 0x00FE: // LOW: Disable high-res (64x32)
				Chip8Display_SetResolution(g_chip8SystemInstance._display, 64, 32);
				break;
			case 0x00FF: // HIGH: Enable high-res (128x64)
				Chip8Display_SetResolution(g_chip8SystemInstance._display, 128, 64);
				break;
			default:
				// Handle 00CN (Scroll down N pixels)
				if ((opcode & 0x00F0) == 0x00C0)
				{
					Word n = opcode & 0x000F;
					Chip8Display_Scroll(g_chip8SystemInstance._display, n, 0);
				}
				else
				{
					fprintf(stderr, "Unknown 0x0 opcode {opcode:%04x}", opcode);
				}
				break;
			}
			break;
		}
		case 0x2000: // call subroutine
		{
			Chip8CPU_Push(cpu, cpu->_PC);
			Chip8CPU_Jump(cpu, opcode);
			break;
		}
		case 0x3000:
		{
			Word x = ((opcode & 0x0F00) >> 8);
			Word n = Chip8CPU_ReadRegister(cpu, x);
			if (n == (opcode & 0x00FF))
			{
				cpu->_PC += 2; // skip next instruction
			}
			break;
		}
		case 0x4000:
		{
			Word x = ((opcode & 0x0F00) >> 8);
			Word n = Chip8CPU_ReadRegister(cpu, x);
			if (n != (opcode & 0x00FF))
			{
				cpu->_PC += 2;
			}
			break;
		}
		case 0x5000:
		{
			Word x = ((opcode & 0x0F00) >> 8);
			Word y = ((opcode & 0x00F0) >> 4);
			if ((opcode & 0x000F) == 0x0000)
			{
				if (Chip8CPU_ReadRegister(cpu, x) == Chip8CPU_ReadRegister(cpu, y))
				{
					cpu->_PC += 2;
				}
			}
			else if ((opcode & 0x000F) == 0x0002)
			{
				if (x <= y)
				{
					for (int j = x; j <= y; j++)
					{
						Chip8RAM_WriteByte(cpu->_ram, cpu->_vI + (j - x), cpu->_registers[j]);
					}
				}
				else
				{
					for (int j = x; j >= y; j--)
					{
						Chip8RAM_WriteByte(cpu->_ram, cpu->_vI + (x - j), cpu->_registers[j]);
					}
				}
			}
			else if ((opcode & 0x000F) == 0x0003)
			{
				if (x <= y)
				{
					for (int j = x; j <= y; j++)
					{
						cpu->_registers[j] = Chip8RAM_ReadByte(cpu->_ram, (cpu->_vI + (j - x)));
					}
				}
				else
				{
					for (int j = x; j >= y; j--)
					{
						cpu->_registers[j] = Chip8RAM_ReadByte(cpu->_ram, (cpu->_vI + (x - j)));
					}
				}
			}
			break;
		}
		case 0x6000:
		{
			Word x = ((opcode & 0x0F00) >> 8);
			Word nn = (opcode & 0x00FF);
			Chip8CPU_WriteRegister(cpu, x, nn);
			break;
		}
		case 0x7000:
		{
			Word x = ((opcode & 0x0F00) >> 8);
			Word nn = (opcode & 0x00FF);
			Chip8CPU_WriteRegister(cpu, x, (Chip8CPU_ReadRegister(cpu, x) + nn));
			break;
		}

		case 0x8000:
		{
			Word x = ((opcode & 0x0F00) >> 8);
			Word y = ((opcode & 0x00F0) >> 4);
			switch (opcode & 0x000F)
			{
			case 0x0000: Chip8CPU_WriteRegister(cpu, x, Chip8CPU_ReadRegister(cpu, y)); break;
			case 0x0001: Chip8CPU_WriteRegister(cpu, x, (Chip8CPU_ReadRegister(cpu, x) | Chip8CPU_ReadRegister(cpu, y))); break;
			case 0x0002: Chip8CPU_WriteRegister(cpu, x, (Chip8CPU_ReadRegister(cpu, x) & Chip8CPU_ReadRegister(cpu, y))); break;
			case 0x0003: Chip8CPU_WriteRegister(cpu, x, (Chip8CPU_ReadRegister(cpu, x) ^ Chip8CPU_ReadRegister(cpu, y))); break;
			case 0x0004:
			{
				int sum = Chip8CPU_ReadRegister(cpu, x) + Chip8CPU_ReadRegister(cpu, y);
				Chip8CPU_WriteRegister(cpu, x, (sum & 0xFF));
				Chip8CPU_WriteRegister(cpu, 0xF, (sum > 255 ? 1 : 0));
				break;
			}
			case 0x0005:
			{
				Word valX = Chip8CPU_ReadRegister(cpu, x);
				Word valY = Chip8CPU_ReadRegister(cpu, y);

				// VF is set to 1 if NOT a borrow (X >= Y)
				Word borrowFlag = (valX >= valY ? 1 : 0);

				Chip8CPU_WriteRegister(cpu, x, (valX - valY));
				Chip8CPU_WriteRegister(cpu, 0xF, borrowFlag);
				break;
			}
			case 0x0006:
			{
				Word valX = Chip8CPU_ReadRegister(cpu, x);
				// Save the least significant bitinto VF before shifting
				Word lsb = (valX & 0x01);
				//Perform the right shift
				Chip8CPU_WriteRegister(cpu, x, valX >> 1);
				// Update VF with the dropped bit
				Chip8CPU_WriteRegister(cpu, 0xF, lsb);
				break;
			}
			case 0x0007:
			{
				Word valX = Chip8CPU_ReadRegister(cpu, x);
				Word valY = Chip8CPU_ReadRegister(cpu, y);

				// VF is set to 1 if Vy >= Vx (No Borrow)
				Word borrowFlag = (valY >= valX ? 1 : 0);
				//perform the subtraction: Vy - Vx
				Chip8CPU_WriteRegister(cpu, x, (valY - valX));
				//Update the flag register
				Chip8CPU_WriteRegister(cpu, 0xF, borrowFlag);
				break;
			}
			case 0x000E:
			{
				cpu->_registers[x] = (cpu->_registers[x] << 1);
				break;
			}
			default:
				fprintf(stderr, "Unknown 0x0 opcode {opcode:%04x}", opcode);
				break;
			}
			break;
		case 0x9000:
		{
			Word x = ((opcode & 0x0F00) >> 8);
			Word y = ((opcode & 0x00F0) >> 4);
			if ((opcode & 0x000F) == 0x0000)
			{
				if (Chip8CPU_ReadRegister(cpu, x) != Chip8CPU_ReadRegister(cpu, y))
				{
					cpu->_PC += 2;
				}
			}
			break;
		}
		case 0xA000:
		{
			cpu->_vI = (opcode & 0x0FFF);
			break;
		}
		case 0xB000:
		{
			Chip8CPU_Jump(cpu, ((opcode & 0x0FFF) + Chip8CPU_ReadRegister(cpu, 0)));
			break;
		}
		case 0xC000:
		{
			Word x = ((opcode & 0x0F00) >> 8);
			Word nn = (opcode & 0x00FF);
			// TODO: implement random
			Word randomByte = 0; //_rng.Next(0, 256);
			Chip8CPU_WriteRegister(cpu, x, (randomByte & nn));
			break;
		}
		case 0xD000:
		{
			Word x = Chip8CPU_ReadRegister(cpu, (opcode & 0x0F00) >> 8);
			Word y = Chip8CPU_ReadRegister(cpu, (opcode & 0x00F0) >> 4);
			Word height = opcode & 0x000F;
			if (height == 0) height = 16;

			// Allocate sprite safely
			Word* spriteData = malloc(sizeof(Word) * height);
			if (!spriteData)
				break;

			// Load sprite bytes from memory
			for (int i = 0; i < height; i++)
				spriteData[i] = Chip8RAM_ReadByte(cpu->_ram, cpu->_vI + i);

			bool collision = false;
			// Draw on all active planes
			for (int plane = 0; plane < 2; plane++)
			{
				if ((cpu->_activePlanes & (1 << plane)) != 0)
					collision |= Chip8Display_DrawSprite(g_chip8SystemInstance._display, x, y, spriteData, height, plane);
			}

			cpu->_registers[0xF] = collision ? 1 : 0;

			free(spriteData);

			Chip8Display_Render(g_chip8SystemInstance._display, cpu->_activePlanes);
			break;
		}
		case 0xE000:
		{
			Word x = ((opcode & 0x0F00) >> 8);
			switch (opcode & 0x00FF)
			{
			case 0x009E:
				// skip next instruction if key with the value of Vx is pressed
				//if (cpu->_keyboard.IsKeyPressed(ReadRegister(x)))
			{
				cpu->_PC += 2;
				break;
			}
			break;
			case 0x00A1:
				// skip next instruction if key with the value of Vx is not pressed
				//if (!cpu->_keyboard.IsKeyPressed(ReadRegister(x)))
			{
				cpu->_PC += 2;
				break;
			}
			break;
			default:
				fprintf(stderr, "Unknown 0x0 opcode {opcode:%04x}", opcode);

				break;
			}
			break;
		}

		}
		}
	}
}

