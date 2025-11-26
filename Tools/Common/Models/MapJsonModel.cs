using System.Collections.Generic;

namespace Glyphborn.Common.Models
{
	public class MapJsonModel
	{
		public string Name { get; set; } = "NewMap";

		public List<MapJsonTile> Tiles { get; set; } = new();
		public List<MapJsonCollision> Collisions { get; set; } = new();
		public List<MapEventModel> Events { get; set; } = new();
		public List<string> Text { get; set; } = new();
		public List<MapScriptStub> Scripts { get; set; } = new();
	}

	public class MapJsonTile
	{
		public int X { get; set; }
		public int Y { get; set; }
		public int Z { get; set; }
		public ushort Id { get; set; }
	}

	public class MapJsonCollision
	{
		public int X { get; set; }
		public int Y { get; set; }
		public int Z { get; set; }
		public byte Value { get; set; }
	}
}
