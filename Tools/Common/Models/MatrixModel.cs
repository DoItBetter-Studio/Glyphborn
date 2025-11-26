using System.Collections.Generic;

namespace Glyphborn.Common.Models
{
	public class MatrixModel
	{
		public class Entry
		{
			public uint MapId { get; set; }
			public int GridX { get; set; }
			public int GridY { get; set; }

			public string Name { get; set; } = string.Empty;
			public string AreaType { get; set; } = "Plains";
			public string Icon { get; set; } = "default";
			public string MusicDay { get; set; } = "none";
			public string MusicNight { get; set; } = "none";
			public string Weather { get; set; } = "clear";

			public ushort Flags { get; set; }
		}

		public List<Entry> Entries { get; } = new();
	}
}
