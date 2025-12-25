using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Numerics;
using System.Runtime.InteropServices.Marshalling;
using System.Windows.Forms;

using Glyphborn.Mapper.Editor;
using Glyphborn.Mapper.Tiles;

namespace Glyphborn.Mapper.Controls
{
	public class Map3DViewportControl : UserControl
	{
		public AreaDocument? Area { get; set; }
		public MapDocument? Map { get; set; }

		private float _yaw = -0.8f;
		private float _pitch = 0.6f;
		private float _distance = 20.0f;

		private float[] _depthBuffer;

		private Point _lastMouse;

		private Matrix4x4 _viewMatrix;
		private Matrix4x4 _projectionMatrix;
		private Bitmap _backbuffer;
		private Timer _timer;

		private SolidBrush _brush;

		public Map3DViewportControl()
		{
			DoubleBuffered = true;
			BackColor = Color.Black;

			_brush = new SolidBrush(Color.FromArgb(100, 150, 200));
			_backbuffer = new Bitmap(Width, Height);
			_depthBuffer = new float[Width * Height];

			_timer = new Timer();
			_timer.Interval = 32;
			_timer.Tick += (_, __) => Invalidate();
			_timer.Start();
		}

		#region Overrides
		protected override void OnMouseDown(MouseEventArgs e)
		{
			_lastMouse = e.Location;
			Capture = true;
		}

		protected override void OnMouseUp(MouseEventArgs e)
		{
			Capture = false;
		}

		protected override void OnMouseMove(MouseEventArgs e)
		{
			if (!Capture)
				return;

			var dx = e.X - _lastMouse.X;
			var dy = e.Y - _lastMouse.Y;
			_lastMouse = e.Location;

			if (e.Button == MouseButtons.Left)
			{
				_yaw += dx * 0.01f;
				_pitch += dy * 0.01f;

				_pitch = Math.Clamp(_pitch, -1.5f, 1.5f);
				Invalidate();
			}
		}

		protected override void OnMouseWheel(MouseEventArgs e)
		{
			_distance *= e.Delta > 0 ? 0.9f : 1.1f;
			_distance = Math.Clamp(_distance, 4.0f, 100.0f);
			Invalidate();
		}

		protected override void OnResize(EventArgs e)
		{
			base.OnResize(e);

			_backbuffer?.Dispose();
			if (Width > 0 && Height > 0)
			{
				_backbuffer = new Bitmap(Width, Height);
				_depthBuffer = new float[Width * Height];
			}
		}

		protected override void OnVisibleChanged(EventArgs e)
		{
			base.OnVisibleChanged(e);
			_timer.Enabled = Visible;
		}
		#endregion

		protected override void OnPaint(PaintEventArgs e)
		{
			base.OnPaint(e);

			var g = e.Graphics;
			g.Clear(Color.Black);

			if (Map == null)
				return;

			var target = new Vector3(
				MapDocument.WIDTH * 0.5f,
				0.0f,
				MapDocument.HEIGHT * 0.5f
			);

			var eye = new Vector3(
				target.X + MathF.Cos(_yaw) * MathF.Cos(_pitch) * _distance,
				target.Y + MathF.Sin(_pitch) * _distance,
				target.Z + MathF.Sin(_yaw) * MathF.Cos(_pitch) * _distance);

			_viewMatrix = Matrix4x4.CreateLookAt(
				eye,
				target,
				Vector3.UnitY
			);

			float aspect = Width / (float) Height;
			_projectionMatrix = Matrix4x4.CreatePerspectiveFieldOfView(
				MathF.PI / 4f,
				aspect,
				0.1f,
				1000f
			);

			// Lock backbuffer ONCE for the whole frame
			var data = _backbuffer.LockBits(
				new Rectangle(0, 0, Width, Height),
				ImageLockMode.WriteOnly,
				PixelFormat.Format32bppArgb
			);

			unsafe
			{
				byte* ptr = (byte*) data.Scan0;
				int stride = data.Stride;

				// Clear raw pixel buffer
				for (int y = 0; y < Height; y++)
				{
					uint* row = (uint*)(ptr + y * stride);
					for (int x = 0; x < Width; x++)
						row[x] = 0xFF000000; // black
				}

				for (int i = 0; i < _depthBuffer.Length; i++)
					_depthBuffer[i] = float.PositiveInfinity;

				DrawMap(ptr, stride);
			}

			_backbuffer.UnlockBits(data);

			// Blit final image once
			e.Graphics.DrawImage(_backbuffer, 0, 0);

			// Draw debug info
			g.DrawString($"Eye: {eye.X:F1}, {eye.Y:F1}, {eye.Z:F1}", Font, Brushes.White, 10, 10);
			g.DrawString($"Distance: {_distance:F1}", Font, Brushes.White, 10, 25);
		}

		private Vector3 Transform(Vector3 worldPos)
		{
			Vector4 pos = new Vector4(worldPos, 1.0f);
			pos = Vector4.Transform(pos, _viewMatrix);
			pos = Vector4.Transform(pos, _projectionMatrix);

			// Perspective divide
			if (MathF.Abs(pos.W) > 0.0001f)
			{
				pos.X /= pos.W;
				pos.Y /= pos.W;
				pos.Z /= pos.W;
			}

			return new Vector3(pos.X, pos.Y, pos.Z);
		}

		private PointF Project(Vector3 clipSpacePos)
		{
			// Clip space [-1, 1] -> Screen Space [0, Width], [0, Height]
			float x = (clipSpacePos.X + 1.0f) * 0.5f * Width;
			float y = (1.0f - clipSpacePos.Y) * 0.5f * Height;

			return new PointF(x, y);
		}

		private unsafe void DrawMap(byte* ptr, int stride)
		{
			// Draw each tile as a wireframe cube
			for (int layer = 0; layer < MapDocument.LAYERS; layer++)
			{
				for (int y = 0; y < MapDocument.HEIGHT; y++)
				{
					for (int x = 0; x < MapDocument.WIDTH; x++)
					{
						var tileRef = Map.Tiles[layer][y][x];

						if (tileRef.TileId == 0)
							continue;

						TileDefinition def = Area.Tilesets[tileRef.Tileset].Tiles[tileRef.TileId];


						DrawMesh(def.Primitive, new Vector3(x, layer, y), ptr, stride);
					}
				}
			}
		}

		private void DrawCube(Graphics g, Vector3 center, float size)
		{
			float h = size * 0.5f;

			Vector3[] corners =
			{
				new(-h, -h, -h), new(h, -h, -h),  // Bottom front-left, front-right
				new(h,  h, -h), new(-h,  h, -h),  // Top front-right, front-left
				new(-h, -h,  h), new(h, -h,  h),  // Bottom back-left, back-right
				new(h,  h,  h), new(-h,  h,  h),  // Top back-right, back-left
			};

			// Transform to world position
			for (int i = 0; i < corners.Length; i++)
			{
				corners[i] += center;
			}

			// Transform to clip space
			Vector3[] clipSpace = new Vector3[8];
			for (int i = 0; i < 8; i++)
			{
				clipSpace[i] = Transform(corners[i]);
			}

			// Check if behind camera (clip space Z < 0)
			bool allBehind = true;
			for (int i = 0; i < 8; i++)
			{
				if (clipSpace[i].Z >= 0)
				{
					allBehind = false;
					break;
				}
			}

			if (allBehind)
				return;  // Don't draw cubes behind camera

			// Project to screen space
			PointF[] screen = new PointF[8];
			for (int i = 0; i < 8; i++)
			{
				screen[i] = Project(clipSpace[i]);
			}

			// Draw edges
			using (var pen = new Pen(Color.FromArgb(100, 150, 200), 1))
			{
				void DrawEdge(int a, int b)
				{
					// Only draw if both points are in front of camera
					if (clipSpace[a].Z >= 0 && clipSpace[b].Z >= 0)
					{
						g.DrawLine(pen, screen[a], screen[b]);
					}
				}

				// Bottom face
				DrawEdge(0, 1); DrawEdge(1, 2); DrawEdge(2, 3); DrawEdge(3, 0);
				// Top face
				DrawEdge(4, 5); DrawEdge(5, 6); DrawEdge(6, 7); DrawEdge(7, 4);
				// Vertical edges
				DrawEdge(0, 4); DrawEdge(1, 5); DrawEdge(2, 6); DrawEdge(3, 7);
			}
		}

		unsafe void DrawMesh(RenderPrimitive prim, Vector3 worldPos, byte* ptr, int stride)
		{
			var mesh = prim.Mesh;

			// Transform vertices
			Vector3[] world = new Vector3[mesh.Vertices.Length];
			Vector3[] clip = new Vector3[mesh.Vertices.Length];
			PointF[] screen = new PointF[mesh.Vertices.Length];

			for (int i = 0; i < mesh.Vertices.Length; i++)
			{
				world[i] = mesh.Vertices[i].Position + worldPos;
				clip[i] = Transform(world[i]);
				screen[i] = Project(clip[i]);
			}

			// Rasterize triangles
			for (int i = 0; i < mesh.Indices.Length; i += 3)
			{
				int a = mesh.Indices[i];
				int b = mesh.Indices[i + 1];
				int c = mesh.Indices[i + 2];

				if (clip[a].Z <= -1 && clip[b].Z <= -1 && clip[c].Z <= -1)
					continue;

				DrawTriangle(
					new Vector3(screen[a].X, screen[a].Y, clip[a].Z),
					new Vector3(screen[b].X, screen[b].Y, clip[b].Z),
					new Vector3(screen[c].X, screen[c].Y, clip[c].Z),
					mesh.Vertices[a].UV,
					mesh.Vertices[b].UV,
					mesh.Vertices[c].UV,
					prim.Texture,
					ptr, stride
				);
			}
		}

		unsafe void DrawTriangle(
			Vector3 p0, Vector3 p1, Vector3 p2,
			Vector2 uv0, Vector2 uv1, Vector2 uv2,
			Texture texture,
			byte* ptr, int stride)
		{
			Vector2 a = new(p1.X - p0.X, p1.Y - p0.Y);
			Vector2 b = new(p2.X - p0.X, p2.Y - p0.Y);

			float cross = a.X * b.Y - a.Y * b.X;
			if (cross >= 0)
				return;

			// Compute bounding box
			int minX = (int) MathF.Floor(MathF.Min(p0.X, MathF.Min(p1.X, p2.X)));
			int maxX = (int) MathF.Ceiling(MathF.Max(p0.X, MathF.Max(p1.X, p2.X)));
			int minY = (int) MathF.Floor(MathF.Min(p0.Y, MathF.Min(p1.Y, p2.Y)));
			int maxY = (int) MathF.Ceiling(MathF.Max(p0.Y, MathF.Max(p1.Y, p2.Y)));

			// Clamp to screen
			minX = Math.Clamp(minX, 0, Width - 1);
			maxX = Math.Clamp(maxX, 0, Width - 1);
			minY = Math.Clamp(minY, 0, Height - 1);
			maxY = Math.Clamp(maxY, 0, Height - 1);

			// Precompute edge function denominators
			float denom = Edge(p0, p1, p2);

			if (p0.Z <= -1 && p1.Z <= -1 && p2.Z <= -1)
				return;

			float z0 = MathF.Max(p0.Z, 0.0001f);
			float z1 = MathF.Max(p1.Z, 0.0001f);
			float z2 = MathF.Max(p2.Z, 0.0001f);

			float iz0 = 1.0f / z0;
			float iz1 = 1.0f / z1;
			float iz2 = 1.0f / z2;

			Vector2 uv0z = uv0 * iz0;
			Vector2 uv1z = uv1 * iz1;
			Vector2 uv2z = uv2 * iz2;

			for (int y = minY; y <= maxY; y++)
			{
				for (int x = minX; x <= maxX; x++)
				{
					Vector3 p = new Vector3(x + 0.5f, y + 0.5f, 0);

					float w0 = Edge(p1, p2, p);
					float w1 = Edge(p2, p0, p);
					float w2 = Edge(p0, p1, p);

					if (w0 >= 0 && w1 >= 0 && w2 >= 0)
					{
						w0 /= denom;
						w1 /= denom;
						w2 /= denom;

						// Interpolate UV
						float iz = iz0 * w0 + iz1 * w1 + iz2 * w2;

						float u = (uv0z.X * w0 + uv1z.X * w1 + uv2z.X * w2) / iz;
						float v = (uv0z.Y * w0 + uv1z.Y * w1 + uv2z.Y * w2) / iz;

						uint color = texture.Sample(u, v);

						float z = 1.0f / iz;

						int idx = y * Width + x;
						if (z >= _depthBuffer[idx])
							continue;

						_depthBuffer[idx] = z;

						PutPixel(x, y, color, ptr, stride);
					}
				}
			}
		}

		float Edge(Vector3 a, Vector3 b, Vector3 c)
		{
			return (c.X - a.X) * (b.Y - a.Y) -
				   (c.Y - a.Y) * (b.X - a.X);
		}

		unsafe void PutPixel(int x, int y, uint color, byte* ptr, int stride)
		{
			if (x < 0 || y < 0 || x >= Width || y >= Height)
				return;

			uint* pixel = (uint*) (ptr + y * stride + x * 4);
			*pixel = color;
		}
	}
}
