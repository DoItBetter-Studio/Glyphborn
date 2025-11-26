using System.IO;

using Glyphborn.Common.IO;
using Glyphborn.Common.Models;

namespace Glyphborn.Common.Compilers
{
	public static class MapCompiler
	{
		public static void WriteMapBin(MapModel model, string path)
		{
			using var fs = File.Create(path);
			using var bw = new BinaryWriterLE(fs);

			bw.WriteAscii("GBG0");
			bw.Write((ushort) 1);
			bw.Write((byte) MapModel.SizeX);
			bw.Write((byte) MapModel.SizeY);
			bw.Write((byte) MapModel.SizeZ);
			bw.Write((uint) 0);
		}
	}
}
