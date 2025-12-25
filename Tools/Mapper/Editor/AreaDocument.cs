using System.Collections.Generic;

using Glyphborn.Mapper.Tiles;

namespace Glyphborn.Mapper.Editor
{
	public sealed class AreaDocument
	{
		// --- Shared tilesets (same structure MapDocument already uses) ---
		public List<Tileset> Tilesets { get; } = new List<Tileset>();

		// --- Map Matrix ---
		public int Width { get; }
		public int Height { get; }

		public MapDocument?[,] Maps { get; }

		// Optional editor metadata
		public string Name { get; set; } = "New Area";

		public AreaDocument(int width, int height)
		{
			Width = width;
			Height = height;
			Maps = new MapDocument?[width, height];
		}

		public MapDocument? GetMap(int x, int y)
		{
			if (x < 0 || y < 0 || x >= Width || y >= Height)
				return null;

			return Maps[x, y];
		}

		public void SetMap(int x, int y, MapDocument map)
		{
			if (x < 0 || y < 0 || x >= Width || y >= Height)
				return;

			Maps[x, y] = map;
		}
	}
}
