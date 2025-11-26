using System.Collections.Generic;
using System.IO;
using System.Text;

using Glyphborn.Common.IO;
using Glyphborn.Common.Models;

namespace Glyphborn.Common.Compilers
{
	public static class TextCompiler
	{
		public static void WriteTextBin(MapModel map, string path)
		{
			using var fs = File.Create(path);
			using var bw = new BinaryWriterLE(fs);

			int count = map.LocalTexts.Count;
			bw.Write((ushort) count);

			long tableStart = fs.Position;

			// Reserve table space
			for (int i = 0; i < count; i++)
				bw.Write(0u);
			for (int i = 0; i < count; i++)
				bw.Write(0u);

			List<(uint offset, uint length)> entries = new(count);
			
			// Write text blob
			for (int i = 0; i < count; i++)
			{
				string s = map.LocalTexts[i];
				var bytes = Encoding.UTF8.GetBytes(s);

				uint offset = (uint) fs.Position;
				bw.Write(bytes);
				uint length = (uint) bytes.Length;

				entries.Add((offset, length));
			}

			// Patch table
			fs.Position = tableStart;

			foreach (var e in entries)
				bw.Write(e.offset);
			foreach (var e in entries)
				bw.Write(e.length);
		}
	}
}
