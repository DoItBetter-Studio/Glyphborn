using Glyphborn.Common.Models;

namespace MapEditor.Render
{
	public class MapRenderer
	{
		private readonly SoftwareRenderSurface _surface;

		public int TileSize = 16;

		public MapRenderer(SoftwareRenderSurface surface)
		{
			_surface = surface;
		}

		public void Render(MapModel map, int floor, int cameraDir)
		{
			_surface.Clear(0xFF000000);

			for (int z = 0; z < MapModel.SizeZ; z++)
			{
				for (int x = 0; x < MapModel.SizeX; x++)
				{
					var tile = map.Tiles[x, floor, z];
					if (tile.Value == 0)
						continue;

					// temporary coloring
					uint color = ColorFromTildId(tile);

					DrawTile2D(x, z, color);
				}
			}
		}

		private void DrawTile2D(int x, int z, uint color)
		{
			int screenX = x * TileSize;
			int screenY = z * TileSize;

			for (int yy = 0; yy < TileSize; yy++)
			{
				for (int xx = 0; xx < TileSize; xx++)
				{
					_surface.DrawPixel(screenX + xx, screenY + yy, color);
				}
			}
		}

		private uint ColorFromTildId(TileId tile)
		{
			uint baseCode = tile.Type;

			// quick pseudo-color
			byte r = (byte) ((baseCode * 53) % 200 + 55);
			byte g = (byte) ((baseCode * 97) % 200 + 55);
			byte b = (byte) ((baseCode * 11) % 200 + 55);

			return 0xFF000000u | ((uint) r << 16) | ((uint) g << 8) | b;
		}
	}
}
