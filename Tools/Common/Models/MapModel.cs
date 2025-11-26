using System.Collections.Generic;

namespace Glyphborn.Common.Models
{
	public class MapModel
	{
		public const int SizeX = 32;
		public const int SizeY = 32;
		public const int SizeZ = 32;

		public string Name { get; set; } = "NewMap";

		public TileId[,,] Tiles { get; } = new TileId[SizeX, SizeY, SizeZ];

		public byte[,,] Collision { get; } = new byte[SizeX, SizeY, SizeZ];

		public List<MapEventModel> Events { get; } = new();
		public List<string> LocalTexts { get; } = new();
		public List<MapScriptStub> Scripts { get; } = new();
	}
}
