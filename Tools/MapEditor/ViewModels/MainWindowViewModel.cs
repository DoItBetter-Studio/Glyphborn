using Glyphborn.Common.Models;

namespace MapEditor.ViewModels
{
	public class MainWindowViewModel
	{
		public string StatusText { get; set; } = "Ready";

		public MapViewModel? CurrentMap { get; set; }
		public TilePaletteViewModel TilePalette { get; }

		public MainWindowViewModel()
		{
			// TODO: adjust path as needed (relative to exe or config)
			var db = TileDatabase.Load("tiledefs.json");

			TilePalette = new TilePaletteViewModel(db);

			// TEMP: create a dummy map to see something
			var model = new MapModel();

			for (int z = 0; z < 8; z++)
			for (int x = 0; x < 8; x++)
				model.Tiles[x, 0, z] = new TileId((ushort) ((x + z) % 8 + 1));

			CurrentMap = new MapViewModel(model);
		}
	}
}
