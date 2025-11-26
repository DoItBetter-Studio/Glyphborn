using System.Collections.ObjectModel;

using Glyphborn.Common.Models;
using Glyphborn.Common.ViewModels;

using MapEditor.Services;

namespace MapEditor.ViewModels
{
	public class TilePaletteViewModel : BaseViewModel
	{
		public ObservableCollection<TileDefViewModel> Tiles { get; } = new();

		private TileDefViewModel? _selected;
		public TileDefViewModel? SelectedTile
		{
			get => _selected;
			set
			{
				if (SetProperty(ref _selected, value))
					OnSelectedChanged();
			}
		}

		public TilePaletteViewModel(TileDatabase db)
		{
			LoadDatabase(db);
		}

		public void LoadDatabase(TileDatabase db)
		{
			Tiles.Clear();
			foreach (var t in db.Tiles)
				Tiles.Add(new TileDefViewModel(t));
		}

		private void OnSelectedChanged()
		{
			foreach (var tile in Tiles)
				tile.IsSelected = (tile == SelectedTile);

			EditorState.SelectedTile = SelectedTile?.Def;
		}
	}
}
