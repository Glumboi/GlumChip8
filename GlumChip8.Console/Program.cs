using GlumChip8.Core;

namespace GlumChip8.Console
{
    internal class Program
    {
        static void Main(string[] args)
        {
            CPU cpu = new();
            const string file = @"";
            if (!File.Exists(file))
            {
                System.Console.WriteLine($"File {file} does not exist.");
                return;
            }
            var b = File.ReadAllBytes(file);
            cpu.LoadProgram(Path.GetFileNameWithoutExtension(file), b);
            cpu.RunLoadedProgram();
            System.Console.WriteLine("Program executed, register state: ");
            System.Console.WriteLine(cpu.GetRegisterState());
        }
    }
}
