using System;
using System.IO;
using System.Text;

using Glyphborn.Common.IO;
using Glyphborn.Common.Models;

namespace Glyphborn.Common.Compilers
{
	public static class MatrixCompiler
	{
		public static void WriteMatrixBin(MatrixModel matrix, string path)
		{
			using var fs = File.Create(path);
			using var bw = new BinaryWriterLE(fs);

			bw.Write((ushort) matrix.Entries.Count);

			foreach (var e in matrix.Entries)
			{
				// name[32]
				WriteFixedString(bw, e.Name, 32);

				// area_type[16]
				WriteFixedString(bw, e.AreaType, 16);

				// icon[32]
				WriteFixedString(bw, e.Icon, 32);

				// music_day[32]
				WriteFixedString(bw, e.MusicDay, 32);

				// music_night[32]
				WriteFixedString(bw, e.MusicNight, 32);

				// weather[16]
				WriteFixedString(bw, e.Weather, 16);

				bw.Write((ushort) e.MapId);
				bw.Write((ushort) e.Flags);
			}
		}

		private static void WriteFixedString(BinaryWriterLE bw, string s, int max)
		{
			var bytes = Encoding.ASCII.GetBytes(s ?? "");
			if (bytes.Length > max)
				throw new Exception($"String too large: \"{s}\" max={max}");

			bw.Write(bytes);

			// padding
			int remaining = max - bytes.Length;
			for (int i = 0; i < remaining; i++)
				bw.Write((byte) 0);
		}
	}
}
