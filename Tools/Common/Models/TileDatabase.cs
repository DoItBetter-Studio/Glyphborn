using System.Collections.Generic;
using System.Linq;

using Glyphborn.Common.Util;

namespace Glyphborn.Common.Models
{
	public class TileDatabase
	{
		public List<TileDef> Tiles { get; set; } = new();

		public TileDef? GetById(ushort id) => Tiles.FirstOrDefault(t => t.Id == id);

		public static TileDatabase Load(string path) => JsonUtil.Load<TileDatabase>(path);

		public void Save(string path) => JsonUtil.Save(path, this);
	}
}
