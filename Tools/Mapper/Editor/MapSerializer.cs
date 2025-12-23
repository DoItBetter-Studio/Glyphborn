using System;
using System.IO;
using System.Linq;
using System.Text;

using Glyphborn.Mapper.Tiles;

namespace Glyphborn.Mapper.Editor
{
	internal static class MapSerializer
	{
		public static void SaveBinary(MapDocument doc, string path)
		{
			using (var fs = new FileStream(path, FileMode.Create))
			using (var bw = new BinaryWriter(fs))
			{
				// Write tileset count
				bw.Write((byte) doc.Tilesets.Count);

				// Write tileset paths (relative to Assets/)
				foreach (var tileset in doc.Tilesets)
				{
					string relativePath = $"{tileset.Type.ToString().ToLower()}/{tileset.Name}.gbts";
					byte[] pathBytes = Encoding.UTF8.GetBytes(relativePath);
					bw.Write((ushort) pathBytes.Length);
					bw.Write(pathBytes);
				}

				for (int layers = 0; layers < MapDocument.LAYERS; layers++)
				{
					for (int y = 0; y < MapDocument.HEIGHT; y++)
					{
						for (int x = 0; x < MapDocument.WIDTH; x++)
						{
							var tile = doc.Tiles[layers][y][x];

							// Pack tileset (2 bits) + tileId (14 bits) into ushort
							ushort packed = (ushort) ((tile.Tileset << 14) | tile.TileId);
							bw.Write(packed);
						}
					}
				}

				for (int layer = 0; layer < MapDocument.LAYERS; layer++)
				{
					for (int y = 0; y < MapDocument.HEIGHT; y++)
					{
						for (int x = 0; x < MapDocument.WIDTH; x++)
						{
							bw.Write((byte) 0);
						}
					}
				}
			}

			doc.IsDirty = false;
		}

		public static MapDocument LoadBinary(string path)
		{
			using (var fs = new FileStream(path, FileMode.Open))
			using (var br = new BinaryReader(fs))
			{
				var doc = new MapDocument();

				byte tilesetCount = br.ReadByte();

				for (byte i = 0; i < tilesetCount; i++)
				{
					byte[] bytes = br.ReadBytes(br.ReadUInt16());
					string tilesetName = Encoding.UTF8.GetString(bytes).Trim('\0');

					Tileset tilset = TilesetSerializer.LoadBinary(tilesetName);
					doc.Tilesets.Add(tilset);
				}

				for (int layer = 0; layer < MapDocument.LAYERS; layer++)
				{
					for (int y = 0; y < MapDocument.HEIGHT; y++)
					{
						for (int x = 0; x < MapDocument.WIDTH; x++)
						{
							ushort packed = br.ReadUInt16();

							byte tileset = (byte) ((packed >> 14) & 0x3);
							ushort tileId = (ushort) (packed & 0x3FFF);

							doc.Tiles[layer][y][x] = new TileRef
							{
								Tileset = tileset,
								TileId = tileId,
							};
						}
					}
				}

				fs.Seek(MapDocument.LAYERS * MapDocument.HEIGHT * MapDocument.WIDTH, SeekOrigin.Current);

				return doc;
			}
		}
	}
}
