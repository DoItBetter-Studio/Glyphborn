using System.Collections.Generic;
using System.Windows.Media;
using System.Windows.Media.Imaging;

using Glyphborn.Common.Models;
using Glyphborn.Common.Util;

namespace MapEditor.Render
{
	public class MapRenderer
	{
		private readonly SoftwareRenderSurface _surface;
		private readonly TileDatabase _db;

		public int TileSize = 32;

		// Cache: tile type -> pixel buffer (TileSize*TileSize) in 0xAARRGGBB
		private readonly Dictionary<ushort, uint[]?> _textureCache = new();

		public MapRenderer(SoftwareRenderSurface surface, TileDatabase db)
		{
			_surface = surface;
			_db = db;
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

					uint[]? tex = GetTextureForTile(tile);

					if (tex != null)
					{
						DrawTexture2D(x, z, tex);
					}
					else
					{
						uint color = ColorFromTileId(tile);
						DrawTile2D(x, z, color);
					}
				}
			}
		}

		private uint[]? GetTextureForTile(TileId tile)
		{
			ushort type = tile.Type;

			if (_textureCache.TryGetValue(type, out var cached))
				return cached;

			var def = _db.GetById(type);
			if (def == null || string.IsNullOrEmpty(def.TexturePath))
			{
				_textureCache[type] = null;
				return null;
			}

			try
			{
				var bi = new BitmapImage();
				bi.BeginInit();
				bi.CacheOption = BitmapCacheOption.OnLoad;
				// Use DecodePixelWidth/Height to scale when loading
				bi.DecodePixelWidth = TileSize;
				bi.DecodePixelHeight = TileSize;
				bi.UriSource = new System.Uri(PathUtil.Normalize(def.TexturePath), System.UriKind.RelativeOrAbsolute);
				bi.EndInit();
				bi.Freeze();

				// Ensure BGRA32
				var formatted = new FormatConvertedBitmap(bi, PixelFormats.Bgra32, null, 0);
				int stride = TileSize * 4;
				byte[] pixels = new byte[TileSize * stride];
				formatted.CopyPixels(pixels, stride, 0);

				var buf = new uint[TileSize * TileSize];
				for (int i = 0; i < buf.Length; i++)
				{
					int idx = i * 4;
					byte b = pixels[idx + 0];
					byte g = pixels[idx + 1];
					byte r = pixels[idx + 2];
					byte a = pixels[idx + 3];
					buf[i] = ((uint) a << 24) | ((uint) r << 16) | ((uint) g << 8) | b;
				}

				_textureCache[type] = buf;
				return buf;
			}
			catch
			{
				_textureCache[type] = null;
				return null;
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

		private void DrawTexture2D(int x, int z, uint[] tex)
		{
			int screenX = x * TileSize;
			int screenY = z * TileSize;

			for (int yy = 0; yy < TileSize; yy++)
			{
				for (int xx = 0; xx < TileSize; xx++)
				{
					uint c = tex[yy * TileSize + xx];
					_surface.DrawPixel(screenX + xx, screenY + yy, c);
				}
			}
		}

		private uint ColorFromTileId(TileId tile)
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
