using System.IO;

using Glyphborn.Common.IO;
using Glyphborn.Common.Models;

namespace Glyphborn.Common.Compilers
{
	public static class EventsCompiler
	{
		public static void WriteEventsBin(MapModel map, string path)
		{
			using var fs = File.Create(path);
			using var bw = new BinaryWriterLE(fs);

			bw.Write((ushort)map.Events.Count);

			foreach (var e in map.Events)
			{
				bw.Write((ushort) e.X);
				bw.Write((ushort) e.Y);
				bw.Write((ushort) e.Z);
				bw.Write((ushort) e.Type);
				bw.Write((ushort) e.ScriptId);
			}
		}
	}
}
