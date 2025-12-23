using System;
using System.Drawing;
using System.Windows.Forms;

using Glyphborn.Mapper.Editor;

namespace Glyphborn.Mapper.Controls
{
	public sealed class MapCanvasControl : Control
	{
		public MapDocument? Document;
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
			if (Document == null)
				return 1;

			int sizeX = ClientSize.Width / MapDocument.WIDTH;
			int sizeY = ClientSize.Height / MapDocument.HEIGHT;

			return Math.Max(1, Math.Min(sizeX, sizeY));
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			base.OnPaint(e);

			if (Document == null || State == null)
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
					DrawTile(g, x, y, tileSize, ox, oy);
				}
			}

			if (State.ShowGrid)
				DrawGrid(g, tileSize, ox, oy);
		}

		private void DrawTile(Graphics g, int x, int y, int tileSize, int ox, int oy)
		{
			var tileRef = Document!.Tiles[State!.CurrentLayer][y][x];

			if (tileRef.TileId == 0)
				return;

			if (tileRef.Tileset >= Document.Tilesets.Count)
				return;

			var tileset = Document.Tilesets[tileRef.Tileset];

			if (tileRef.TileId >= tileset.Tiles.Count)
				return;

			var def = tileset.Tiles[tileRef.TileId];

			if (def.TopTexture == null)
				return;

			int px = ox + x * tileSize;
			int py = oy + y * tileSize;

			g.DrawImage(def.TopTexture, new Rectangle(px, py, tileSize, tileSize));
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
			Document?.BeginBatch();
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
			Document?.EndBatch();
			base.OnMouseUp(e);
			_isPainting = false;
		}

		private void PaintTileAtMouse(int mouseX, int mouseY, bool erase = false)
		{
			if (Document == null || State == null || State.SelectedTile == null)
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
				Document.SetTile(State.CurrentLayer, x, y, new TileRef
				{
					Tileset = 0,
					TileId = 0
				});
			}
			else
			{
				var sel = State.SelectedTile.Value;

				Document.SetTile(State.CurrentLayer, x, y, new TileRef
				{
					Tileset = sel.TilesetIndex,
					TileId = sel.TileIndex
				});
			}

			Invalidate();
		}
	}
}
