using System.IO;

using Glyphborn.Common.IO;
using Glyphborn.Common.Models;

namespace Glyphborn.Common.Compilers
{
	public static class CollisionCompiler
	{
		public static void WriteCollisionBin(MapModel map, string path)
		{
			using var fs = File.Create(path);
			using var bw = new BinaryWriterLE(fs);

			bw.WriteAscii("GBC0");
			bw.Write((ushort) 1);
			bw.Write((byte) MapModel.SizeX);
			bw.Write((byte) MapModel.SizeY);
			bw.Write((byte) MapModel.SizeZ);
			bw.Write((uint) 0);

			for (int y = 0; y < MapModel.SizeY; y++)
				for (int z = 0; z < MapModel.SizeZ; z++)
					for (int x = 0; x < MapModel.SizeX; x++)
						bw.Write(map.Collision[x, y, z]);
		}
	}
}
