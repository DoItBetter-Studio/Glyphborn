using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;

using Glyphborn.Mapper.Editor;

namespace Glyphborn.Mapper.Controls
{
	public sealed class MapCanvasControl : Control
	{
		public AreaDocument? AreaDocument;
		public MapDocument? MapDocument;
		public EditorState? State;

		private bool _isPainting;
		private bool _isErasing;

		public MapCanvasControl()
		{
			DoubleBuffered = true;
			SetStyle(ControlStyles.ResizeRedraw, true);
		}

		private int ComputeTileSize()
		{
			if (MapDocument == null)
				return 1;

			int sizeX = ClientSize.Width / MapDocument.WIDTH;
			int sizeY = ClientSize.Height / MapDocument.HEIGHT;

			return Math.Max(1, Math.Min(sizeX, sizeY));
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			base.OnPaint(e);

			if (MapDocument == null || State == null)
				return;

			var g = e.Graphics;

			int tileSize = ComputeTileSize();

			int mapW = MapDocument.WIDTH * tileSize;
			int mapH = MapDocument.HEIGHT * tileSize;

			int ox = (ClientSize.Width - mapW) / 2;
			int oy = (ClientSize.Height - mapH) / 2;

			for (int y = 0; y < MapDocument.HEIGHT; y++)
			{
				for (int x = 0; x < MapDocument.WIDTH; x++)
				{
					// Draw underlying layer faintly
					for (byte layer = 0; layer < State.CurrentLayer; layer++)
					{
						float distance = State.CurrentLayer - layer;
						float fadeStart = 0.0f;   // layers within 2 stay fully visible
						float fadeRange = 8.0f;   // fade over the next 6 layers

						float alpha;

						if (distance <= fadeStart)
						{
							alpha = 1.0f; // fully visible
						}
						else
						{
							float t = (distance - fadeStart) / fadeRange;
							alpha = 1.0f - Math.Clamp(t, 0.25f, 1f);
						}

						DrawTile(g, x, y, tileSize, ox, oy, layer, alpha);
					}

					DrawTile(g, x, y, tileSize, ox, oy, State.CurrentLayer);
				}
			}

			if (State.ShowGrid)
				DrawGrid(g, tileSize, ox, oy);
		}

		private void DrawTile(Graphics g, int x, int y, int tileSize, int ox, int oy, int layer, float alpha = 1.0f)
		{
			var tileRef = MapDocument!.Tiles[layer][y][x];

			if (tileRef.TileId == 0)
				return;

			if (tileRef.Tileset >= AreaDocument.Tilesets.Count)
				return;

			var tileset = AreaDocument.Tilesets[tileRef.Tileset];

			if (tileRef.TileId >= tileset.Tiles.Count)
				return;

			var def = tileset.Tiles[tileRef.TileId];

			if (def.Primitive == null)
				return;

			int px = ox + x * tileSize;
			int py = oy + y * tileSize;

			var preview = TilePreviewer.GetPreview(def.Primitive.Texture);

			if (alpha < 1.0f)
			{
				ColorMatrix cm = new ColorMatrix
				{
					Matrix33 = alpha
				};

				using var ia = new ImageAttributes();
				ia.SetColorMatrix(cm);

				g.DrawImage(
					preview,
					new Rectangle(px, py, tileSize, tileSize),
					0, 0, preview.Width, preview.Height,
					GraphicsUnit.Pixel,
					ia
				);
			}
			else
			{
				g.DrawImage(preview, new Rectangle(px, py, tileSize, tileSize));
			}
		}

		private void DrawGrid(Graphics g, int tileSize, int ox, int oy)
		{
			using var pen = new Pen(Color.FromArgb(80, Color.White));

			int mapW = MapDocument.WIDTH * tileSize;
			int mapH = MapDocument.HEIGHT * tileSize;

			for (int x = 0; x <= MapDocument.WIDTH; x++)
			{
				int px = ox + x * tileSize;
				g.DrawLine(pen, px, oy, px, oy + mapH);
			}

			for (int y = 0; y <= MapDocument.HEIGHT; y++)
			{
				int py = oy + y * tileSize;
				g.DrawLine(pen, ox, py, ox + mapW, py);
			};
		}

		protected override void OnMouseDown(MouseEventArgs e)
		{
			MapDocument?.BeginBatch();
			base.OnMouseDown(e);

			if (e.Button == MouseButtons.Left)
			{
				_isPainting = true;
				_isErasing = false;
				PaintTileAtMouse(e.X, e.Y);
			}
			else if (e.Button == MouseButtons.Right)
			{
				_isPainting = true;
				_isErasing = true;
				PaintTileAtMouse(e.X, e.Y, erase: true);
			}
		}

		protected override void OnMouseMove(MouseEventArgs e)
		{
			base.OnMouseMove(e);

			if (_isPainting)
			{
				PaintTileAtMouse(e.X, e.Y, erase: _isErasing);
			}
		}

		protected override void OnMouseUp(MouseEventArgs e)
		{
			MapDocument?.EndBatch();
			base.OnMouseUp(e);
			_isPainting = false;
		}

		private void PaintTileAtMouse(int mouseX, int mouseY, bool erase = false)
		{
			if (MapDocument == null || State == null || State.SelectedTile == null)
				return;

			int tileSize = ComputeTileSize();
			int mapW = MapDocument.WIDTH * tileSize;
			int mapH = MapDocument.HEIGHT * tileSize;
			int ox = (ClientSize.Width - mapW) / 2;
			int oy = (ClientSize.Height - mapH) / 2;

			int x = (mouseX - ox) / tileSize;
			int y = (mouseY - oy) / tileSize;

			if (x < 0 || y < 0 || x >= MapDocument.WIDTH || y >= MapDocument.HEIGHT)
				return;

			if (erase)
			{
				MapDocument.SetTile(State.CurrentLayer, x, y, new TileRef
				{
					Tileset = 0,
					TileId = 0
				});
			}
			else
			{
				var sel = State.SelectedTile.Value;

				MapDocument.SetTile(State.CurrentLayer, x, y, new TileRef
				{
					Tileset = sel.TilesetIndex,
					TileId = sel.TileIndex
				});
			}

			Invalidate();
		}
	}
}
