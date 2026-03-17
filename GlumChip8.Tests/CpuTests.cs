using GlumChip8.Core;

namespace GlumChip8.Tests
{
    public class CpuTests
    {
        CPU chip8Cpu;

        [SetUp]
        public void Setup()
        {
            chip8Cpu = new();
        }

        [Test]
        public void Test_PushAndPop()
        {
            chip8Cpu.Push(200);
            var value = chip8Cpu.Pop();
            Assert.That(value, Is.EqualTo(200));
        }

        [Test]
        public void Test_RegisterWriteAndRead()
        {
            chip8Cpu.WriteRegister(0, 123);
            var value = chip8Cpu.ReadRegister(0);
            Assert.That(value, Is.EqualTo(123));
        }

        [Test]
        public void Test_Jump()
        {
            chip8Cpu.Jump(0x13F3);
            Assert.That(chip8Cpu.ProgramCounter, Is.EqualTo(0x3F3));
        }
    }
}
