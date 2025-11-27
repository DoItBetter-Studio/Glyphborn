using System.Collections.ObjectModel;

using Glyphborn.Common.Models;
using Glyphborn.Common.ViewModels;

namespace TileDefEditor.ViewModels
{
    public class TileDefListViewModel : BaseViewModel
    {
		public ObservableCollection<TileDefViewModel> Tiles { get; } = new();

		private TileDefViewModel? _selectedTile;
		public TileDefViewModel? SelectedTile
		{
			get => _selectedTile;
			set => SetProperty(ref _selectedTile, value);
		}

		private readonly MainViewModel _main;

		public TileDefListViewModel(MainViewModel main)
		{
			_main = main;
		}

		public void Reload(TileDatabase db)
		{
			Tiles.Clear();
			foreach (var t in db.Tiles)
				Tiles.Add(new TileDefViewModel(t));
		}
    }
}
