namespace Glyphborn.Common.Models
{
	public class MapEventModel
	{
		public int X { get; set; }
		public int Y { get; set; }
		public int Z { get; set; }

		public ushort Type { get; set; }
		public ushort ScriptId { get; set; }
	}
}
