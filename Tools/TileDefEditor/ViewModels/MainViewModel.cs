using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

using Glyphborn.Common.Models;
using Glyphborn.Common.ViewModels;

using TileDefEditor.Services;

namespace TileDefEditor.ViewModels
{
    public class MainViewModel : BaseViewModel
    {
		public TileDatabase Database { get; private set; } = new();
		public TileDefListViewModel TileList { get; }
		public TileDefViewModel? SelectedTile
		{
			get => TileList.SelectedTile;
		}

		private readonly TileDefDocumentService _docService;

		// Commands
		public ICommand NewCommand { get; }
		public ICommand OpenCommand { get; }
		public ICommand SaveCommand { get; }
		public ICommand SaveAsCommand { get; }
		public ICommand ExitCommand { get; }

		public MainViewModel()
		{
			_docService = new TileDefDocumentService();

			TileList = new TileDefListViewModel(this);

			NewCommand = new RelayCommand(_ => NewFile());
			OpenCommand = new RelayCommand(_ => OpenFile());
			SaveCommand = new RelayCommand(_ => SaveFile());
			SaveAsCommand = new RelayCommand(_ => SaveFileAs());
			ExitCommand = new RelayCommand(_ => Application.Current.Shutdown());
		}

		private void NewFile()
		{
			Database = new TileDatabase();
			TileList.Reload(Database);
		}

		private void OpenFile()
		{
			string? file = _docService.OpenDialog();
			if (file == null) return;

			Database = TileDatabase.Load(file);
			TileList.Reload(Database);
			_docService.CurrentPath = file;
		}

		private void SaveFile()
		{
			if (_docService.CurrentPath == null)
			{
				SaveFileAs();
				return;
			}

			Database.Save(_docService.CurrentPath);
		}

		private void SaveFileAs()
		{
			string? file = _docService.SaveDialog();
			if (file == null) return;

			_docService.CurrentPath = file;
			Database.Save(file);
		}
	}
}
