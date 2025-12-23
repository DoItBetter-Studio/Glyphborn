using System;
using System.Drawing;
using System.Numerics;

namespace Glyphborn.Mapper.Tiles
{
	public sealed class TileDefinition
	{
		// Identity
		public ushort Id;
		public string Name = "";
		public string Category = "Default";

		public CollisionType Collision;

		// Rendering
		public RenderPrimitive Primitive;

		// Editor-only
		public Bitmap? TopTexture;
		public Bitmap? Thumbnail;
	}

	public sealed class Mesh
	{
		public readonly Vertex[] Vertices;
		public readonly ushort[] Indices;

		public readonly Vector3 BoundsMin;
		public readonly Vector3 BoundsMax;

		public Mesh(Vertex[] vertices, ushort[] indices)
		{
			Vertices = vertices;
			Indices = indices;

			ComputeBounds(out BoundsMin, out BoundsMax);
		}

		private void ComputeBounds(out Vector3 min, out Vector3 max)
		{
			min = new Vector3(float.MaxValue);
			max = new Vector3(float.MinValue);

			foreach (var v in Vertices)
			{
				min = Vector3.Min(min, v.Position);
				max = Vector3.Max(max, v.Position);
			}
		}
	}

	public struct Vertex
	{
		public Vector3 Position;
		public Vector2 UV;
	}

	public sealed class Texture
	{
		public readonly int Width;
		public readonly int Height;
		public readonly uint[] Pixels;

		public Texture(int width, int height, uint[] pixels)
		{
			Width = width;
			Height = height;
			Pixels = pixels;
		}

		public uint Sample(float u, float v)
		{
			int x = (int)(u * (Width - 1));
			int y = (int)(v * (Height - 1));

			x = Math.Clamp(x, 0, Width - 1);
			y = Math.Clamp(y, 0, Height - 1);

			return Pixels[y * Width + x];
		}
	}

	public sealed class RenderPrimitive
	{
		public readonly Mesh Mesh;
		public readonly Texture Texture;

		public RenderPrimitive(Mesh mesh, Texture texture)
		{
			Mesh = mesh;
			Texture = texture;
		}
	}
}
