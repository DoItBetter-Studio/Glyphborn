using System.Collections.Generic;
using System.Drawing;

namespace Glyphborn.Mapper.Editor
{
	public static class TextureLoader
	{
		public static readonly Dictionary<string, Bitmap> _cache = new();

		public static Bitmap Load(string path)
		{
			if (_cache.TryGetValue(path, out var bmp))
				return bmp;

			var loaded = new Bitmap(path);
			_cache[path] = loaded;
			return loaded;
		}

		public static void Clear()
		{
			foreach (var bmp in _cache.Values)
				bmp.Dispose();

			_cache.Clear();
		}
	}
}
