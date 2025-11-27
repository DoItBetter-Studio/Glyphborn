using Glyphborn.Common.Models;

namespace MapEditor.ViewModels
{
	public class MainWindowViewModel
	{
		public string StatusText { get; set; } = "Ready";

		public MapViewModel? CurrentMap { get; set; }
		public TilePaletteViewModel TilePalette { get; }

		public TileDatabase TileDatabase { get; }

		public MainWindowViewModel()
		{
			// TODO: adjust path as needed (relative to exe or config)
			TileDatabase = TileDatabase.Load("tiledefs.json");

			TilePalette = new TilePaletteViewModel(TileDatabase);

			// TEMP: create a dummy map to see something
			var model = new MapModel();

			CurrentMap = new MapViewModel(model);
		}
	}
}
