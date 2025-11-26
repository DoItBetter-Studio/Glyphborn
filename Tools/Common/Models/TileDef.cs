namespace Glyphborn.Common.Models
{
	public class TileDef
	{
		public ushort Id { get; set; }
		public string Name { get; set; } = string.Empty;

		public TileCategory Category { get; set; }

		public bool IsSolid { get; set; }
		public bool IsWall { get; set; }
		public bool IsFloor { get; set; }
		public bool IsRoof { get; set; }

		public bool IsInterior { get; set; }
		public bool BlocksLight { get; set; }

		public int RenderHeight { get; set; } = 1;
		public int MaxVariants { get; set; } = 1;

		public string? ModelPath { get; set; }
		public string? TexturePath { get; set; }
	}
}
