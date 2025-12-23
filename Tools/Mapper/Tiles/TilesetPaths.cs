using System;
using System.IO;

namespace Glyphborn.Mapper.Tiles
{
	static class TilesetPaths
	{
		public static string Root => Path.Combine(AppContext.BaseDirectory, "../..", "assets", "tilesets");

		public static string Regional => Path.Combine(Root, "regional");
		public static string Local => Path.Combine(Root, "local");
		public static string Interior => Path.Combine(Root, "interior");
	}
}
