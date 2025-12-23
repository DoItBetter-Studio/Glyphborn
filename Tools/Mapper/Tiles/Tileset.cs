using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO;
using System.Numerics;
using System.Text;

using Glyphborn.Mapper.Editor;

namespace Glyphborn.Mapper.Tiles
{
	public sealed class Tileset
	{
		public string Name { get; set; } = "Unnamed";
		public TilesetType Type { get; set; }
		public List<TileDefinition> Tiles { get; } = new();

		private const uint MAGIC = 0x53544C47;  // "GBTS"
		private const ushort VERSION = 1;

		public void SaveBinary(string path)
		{
			using (var fs = new FileStream(path, FileMode.Create))
			using (var bw = new BinaryWriter(fs))
			{
				bw.Write(MAGIC);
				bw.Write(VERSION);
				bw.Write((ushort)Tiles.Count);

				// Tileset name (64 bytes, null-padded)
				WriteFixedString(bw, Name, 64);
				bw.Write((byte) Type);

				foreach (var tile in Tiles)
				{
					WriteTile(bw, tile);
				}
			}
		}

		private void WriteTile(BinaryWriter bw, TileDefinition tile)
		{
			bw.Write(tile.Id);
			WriteFixedString(bw, tile.Name, 64);
			WriteFixedString(bw, tile.Category, 32);
			bw.Write((byte) tile.Collision);

			if (tile.Primitive != null)
			{
				// Mesh Data
				bw.Write((uint) tile.Primitive.Mesh.Vertices.Length);

				foreach (var v in tile.Primitive.Mesh.Vertices)
				{
					bw.Write(v.Position.X);
					bw.Write(v.Position.Y);
					bw.Write(v.Position.Z);
					bw.Write(v.UV.X);
					bw.Write(v.UV.Y);
				}

				bw.Write((uint)tile.Primitive.Mesh.Indices.Length);

				foreach (var idx in tile.Primitive.Mesh.Indices)
				{
					bw.Write(idx);
				}

				// Texture data
				bw.Write((ushort) tile.Primitive.Texture.Width);
				bw.Write((ushort) tile.Primitive.Texture.Height);

				foreach (var pixel in tile.Primitive.Texture.Pixels)
				{
					bw.Write(pixel);  // ARGB uint32
				}
			}
			else
			{
				// No render data (e.g., Air tile)
				bw.Write((uint) 0);  // vertex_count = 0
				bw.Write((uint) 0);  // index_count = 0
				bw.Write((ushort) 0); // texture width = 0
				bw.Write((ushort) 0); // texture height = 0
			}
		}

		public static Tileset LoadBinary(string path)
		{
			string fullPath = Path.IsPathRooted(path)
				? path
				: Path.Combine(TilesetPaths.Root, path);

			using (var fs = new FileStream(fullPath, FileMode.Open))
			using (var br = new BinaryReader(fs))
			{
				// Verify header
				uint magic = br.ReadUInt32();

				if (magic != MAGIC)
					throw new InvalidDataException("Invalid tileset file");

				ushort version = br.ReadUInt16();
				if (version != VERSION)
					throw new InvalidDataException($"Unsupported version: {version}");

				ushort tileCount = br.ReadUInt16();
				string name = ReadFixedString(br, 64);
				TilesetType type = (TilesetType)br.ReadByte();

				var tileset = new Tileset { Name = name, Type = type };

				for (int i = 0; i < tileCount; i++)
				{
					tileset.Tiles.Add(ReadTile(br));
				}

				return tileset;
			}
		}

		private static TileDefinition ReadTile(BinaryReader br)
		{
			var tile = new TileDefinition
			{
				Id = br.ReadUInt16(),
				Name = ReadFixedString(br, 64),
				Category = ReadFixedString(br, 32),
				Collision = (CollisionType)br.ReadByte(),
			};

			// Read mesh
			uint vertexCount = br.ReadUInt32();

			if (vertexCount > 0)
			{
				var vertices = new Vertex[vertexCount];

				for (int i = 0; i < vertexCount; i++)
				{
					vertices[i] = new Vertex
					{
						Position = new Vector3(br.ReadSingle(), br.ReadSingle(), br.ReadSingle()),
						UV = new Vector2(br.ReadSingle(), br.ReadSingle())
					};
				}

				uint indexCount = br.ReadUInt32();
				var indices = new ushort[indexCount];

				for (int i = 0; i < indexCount; i++)
				{
					indices[i] = br.ReadUInt16();
				}

				var mesh = new Mesh(vertices, indices);

				// Read texture
				ushort texWidth = br.ReadUInt16();
				ushort texHeight = br.ReadUInt16();

				var pixels = new uint[texWidth * texHeight];
				for (int i = 0; i < pixels.Length; i++)
				{
					pixels[i] = br.ReadUInt32();
				}

				var texture = new Texture(texWidth, texHeight, pixels);

				tile.Primitive = new RenderPrimitive(mesh, texture);

				// Generate editor preview from texture
				tile.TopTexture = TextureToBitmap(texture);
				tile.Thumbnail = ResizeBitmap(tile.TopTexture, 32, 32);
			}
			else
			{
				// Air tile or no render data
				br.ReadUInt32();  // Skip index_count
				br.ReadUInt16();  // Skip texture width
				br.ReadUInt16();  // Skip texture height
			}

			return tile;
		}

		private static void WriteFixedString(BinaryWriter bw, string str, int length)
		{
			byte[] bytes = new byte[length];
			if (!string.IsNullOrEmpty(str))
			{
				Encoding.UTF8.GetBytes(str, 0, Math.Min(str.Length, length - 1), bytes, 0);
			}
			bw.Write(bytes);
		}

		private static string ReadFixedString(BinaryReader br, int length)
		{
			byte[] bytes = br.ReadBytes(length);
			return Encoding.UTF8.GetString(bytes).Trim('\0');
		}

		private static Bitmap TextureToBitmap(Texture tex)
		{
			var bmp = new Bitmap(tex.Width, tex.Height);

			for (int y = 0; y < tex.Height; y++)
			{
				for (int x = 0; x < tex.Width; x++)
				{
					uint pixel = tex.Pixels[y * tex.Width + x];

					byte a = (byte) ((pixel >> 24) & 0xFF);
					byte r = (byte) ((pixel >> 16) & 0xFF);
					byte g = (byte) ((pixel >> 8) & 0xFF);
					byte b = (byte) (pixel & 0xFF);

					bmp.SetPixel(x, y, Color.FromArgb(a, r, g, b));
				}
			}

			return bmp;
		}

		private static Bitmap ResizeBitmap(Bitmap source, int width, int height)
		{
			var result = new Bitmap(width, height);
			using (var g = Graphics.FromImage(result))
			{
				g.InterpolationMode = InterpolationMode.NearestNeighbor;
				g.DrawImage(source, 0, 0, width, height);
			}

			return result;
		}
	}
}
