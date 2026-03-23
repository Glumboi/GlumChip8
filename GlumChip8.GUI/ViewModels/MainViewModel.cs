using GalaSoft.MvvmLight.Command;
using GlumChip8.Core;
using GlumChip8.GUI.Core;
using Microsoft.Win32;
using Raylib_cs;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;
using Wpf.Ui.Controls;

namespace GlumChip8.GUI.ViewModels
{
    public class MainViewModel : ViewModelBase
    {
        nint _wpfParentHandle;
        public nint WPFParentHandle { get => _wpfParentHandle; set => SetProperty(ref _wpfParentHandle, value); }

        private string _currentRomPath = "No Rom loaded!";

        public string CurrentRomPath { get => _currentRomPath; set => SetProperty(ref _currentRomPath, value); }

        public MainViewModel()
        {
            CreateCommands();
            LoadChip8Roms(Directory.GetCurrentDirectory());
        }

        void LoadChip8Roms(string load)
        {
            if (!Directory.Exists(load)) return;
            foreach (var item in Directory.GetFiles(load))
            {
                if (Path.GetExtension(item) == ".ch8")
                    RomCollection.Add(Path.GetFileNameWithoutExtension(item));
            }
        }

        private ICommand _pauseRomCommand;

        public ICommand PauseRomCommand { get => _pauseRomCommand; set => SetProperty(ref _pauseRomCommand, value); }

        private void CreatePauseRomCommand()
        {
            PauseRomCommand = new RelayCommand(TogglePause);
        }

        private void TogglePause()
        {
            Chip8.TogglePause();
            OnPropertyChanged(nameof(PauseRomSymbol));
        }

        private ICommand _openRomCommand;

        public ICommand OpenRomCommand { get => _openRomCommand; set => SetProperty(ref _openRomCommand, value); }

        private void CreateOpenRomCommand()
        {
            OpenRomCommand = new RelayCommand(OpenRom);
        }

        private void OpenRom()
        {
            var of = new OpenFileDialog();
            of.Title = "Please select the ROM file you want to load!";
            of.Filter = "Chip8 ROM files (*.ch8)|*.ch8|All files (*.*)|*.*";
            of.InitialDirectory = Directory.GetCurrentDirectory();
            if ((bool)of.ShowDialog())
            {
                // Validate if rom
                CurrentRomPath = of.FileName;
                var ext = Path.GetExtension(CurrentRomPath);
                if (String.Equals(ext, ".ch8", StringComparison.OrdinalIgnoreCase) && File.Exists(CurrentRomPath))
                {
                    var raylibHandle = Chip8.LaunchFromFile(CurrentRomPath);
                    OnPropertyChanged(nameof(PauseRomSymbol));
                    ShowCollection = false;
                    RaylibHost = new Chip8RaylibHost(raylibHandle, Chip8);
                }
            }
        }

        public void CreateCommands()
        {
            CreateOpenRomCommand();
            CreatePauseRomCommand();
        }

        public SymbolRegular PauseRomSymbol
        {
            get
            {
                if (Chip8.Running)
                {
                    return SymbolRegular.Pause48;
                }
                else
                {
                    return SymbolRegular.Play48;
                }
            }
        }

        // Chip8 emulator related
        private Chip8System _chip8 = new();
        public Chip8System Chip8 { get => _chip8; set => SetProperty(ref _chip8, value); }

        private Chip8RaylibHost _raylibHost;
        public Chip8RaylibHost RaylibHost { get => _raylibHost; set => SetProperty(ref _raylibHost, value); }

        private bool _showCollection = true;
        public bool ShowCollection
        {
            get => _showCollection;
            set => SetProperty(ref _showCollection, value);
        }

        // Your existing collection
        public ObservableCollection<string> RomCollection { get; set; } = new() { };
    }
}