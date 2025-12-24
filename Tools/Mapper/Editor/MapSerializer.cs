using System.IO;
using System.Text;

using Glyphborn.Mapper.Tiles;

namespace Glyphborn.Mapper.Editor
{
	internal static class MapSerializer
	{
		private const uint MAGIC = 0x204D4247;  // "GBM "
		private const ushort VERSION = 1;

		public static void SaveBinary(MapDocument doc)
		{
			string path = Path.Combine(EditorPaths.Maps, $"{doc.Name}.gbm");

			// Ensure maps folder exists
			var dir = Path.GetDirectoryName(path) ?? EditorPaths.Maps;
			if (!Directory.Exists(dir))
				Directory.CreateDirectory(dir);

			using (var fs = new FileStream(path, FileMode.Create))
			using (var bw = new BinaryWriter(fs))
			{
				bw.Write(MAGIC);
				bw.Write(VERSION);
				//bw.Write((byte) doc.Tilesets.Count);

				byte[] nameBytes = Encoding.UTF8.GetBytes(doc.Name);
				bw.Write((ushort)  nameBytes.Length);
				bw.Write(nameBytes);

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

				// Verify header
				uint magic = br.ReadUInt32();

				if (magic != MAGIC)
					throw new InvalidDataException("Invalid tileset file");

				ushort version = br.ReadUInt16();
				if (version != VERSION)
					throw new InvalidDataException($"Unsupported version: {version}");

				byte tilesetCount = br.ReadByte();

				byte[] nameBytes = br.ReadBytes(br.ReadUInt16());
				string mapName = Encoding.UTF8.GetString(nameBytes).Trim('\0');

				doc.Name = mapName;

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
