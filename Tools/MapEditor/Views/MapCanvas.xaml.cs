using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using MapEditor.Render;
using Glyphborn.Common.Models;
using MapEditor.Services;
using System;

namespace MapEditor.Views
{
	public partial class MapCanvas : UserControl
	{
		private WriteableBitmap? _bitmap;
		private SoftwareRenderSurface? _surface;
		private MapRenderer? _renderer;

		private MapModel? _map;
		private int _floor;
		private int _cameraDir;

		private TileDatabase? _tileDb;

		private bool _isPainting;

		public MapCanvas()
		{
			InitializeComponent();
			Loaded += OnLoaded;
			SizeChanged += OnSizeChanged;
		}

		private void OnLoaded(object sender, RoutedEventArgs e)
		{
			InitSurface();
		}

		private void OnSizeChanged(object sender, SizeChangedEventArgs e)
		{
			InitSurface();
			Redraw();
		}

		private void InitSurface()
		{
			// If we don't have a size yet, bail out.
			int viewW = (int) ActualWidth;
			int viewH = (int) ActualHeight;
			if (viewW < 1 || viewH < 1)
				return;

			// Map tile counts (constants, but use MapModel layout semantics)
			int tilesX = MapModel.SizeX;
			int tilesZ = MapModel.SizeZ;

			// Compute tile size so the entire map fits inside the view.
			// If we have a real map, use the view size directly; otherwise fallback to a default scale.
			int tileSize = 32;
			if (_map != null)
			{
				// integer tile size; ensure at least 1
				tileSize = Math.Max(1, Math.Min(viewW / tilesX, viewH / tilesZ));
			}
			else
			{
				// If no map yet, still create a reasonably sized surface based on view
				tileSize = Math.Max(1, Math.Min(viewW / tilesX, viewH / tilesZ));
			}

			int bmpW = tileSize * tilesX;
			int bmpH = tileSize * tilesZ;

			_bitmap = new WriteableBitmap(
				bmpW, bmpH, 96, 96,
				System.Windows.Media.PixelFormats.Bgra32,
				null);

			Surface.Source = _bitmap;
			Surface.Width = bmpW;
			Surface.Height = bmpH;

			_surface = new SoftwareRenderSurface(_bitmap);

			// create or recreate renderer when we have a tile DB
			if (_surface != null && _tileDb != null)
			{
				_renderer = new MapRenderer(_surface, _tileDb)
				{
					TileSize = tileSize,
				};
			}
			else if (_surface != null)
			{
				_renderer = new MapRenderer(_surface, new TileDatabase())
				{
					TileSize = tileSize,
				};
			}
		}

		public void SetContext(MapModel map, int floor, int cameraDir, TileDatabase tileDb)
		{
			_map = map;
			_floor = floor;
			_cameraDir = cameraDir;
			_tileDb = tileDb;

			// Recreate surface/renderer now that we have a map and DB available.
			InitSurface();

			// If InitSurface didn't set renderer (surface existed earlier), ensure tile size sync
			if (_renderer != null && _surface != null)
			{
				_renderer = new MapRenderer(_surface, _tileDb)
				{
					TileSize = _renderer.TileSize // preserve computed tile size if any
				};
			}

			Redraw();
		}

		private void Redraw()
		{
			if (_map == null || _renderer == null)
				return;

			_renderer.Render(_map, _floor, _cameraDir);
		}

		private void OnMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
		{
			if (_map == null)
				return;

			Focus(); // ensure we get move events

			_isPainting = true;
			PaintAt(e.GetPosition(Surface));
		}

		private void OnMouseMove(object sender, MouseEventArgs e)
		{
			if (!_isPainting)
				return;

			if (e.LeftButton == MouseButtonState.Pressed)
			{
				PaintAt(e.GetPosition(Surface));
			}
			else
			{
				_isPainting = false;
			}
		}

		private void PaintAt(Point p)
		{
			if (_map == null || _renderer == null || Surface == null)
				return;

			// p is relative to the Image (Surface). If pointer is outside image, ignore.
			if (p.X < 0 || p.Y < 0 || p.X >= Surface.ActualWidth || p.Y >= Surface.ActualHeight)
				return;

			var tileDef = EditorState.SelectedTile;
			if (tileDef == null)
				return;

			int tileSize = _renderer.TileSize;

			int x = (int) (p.X / tileSize);
			int z = (int) (p.Y / tileSize);

			if (x < 0 || x >= MapModel.SizeX ||
				z < 0 || z >= MapModel.SizeZ)
				return;

			// For now, always paint on current floor (_floor)
			_map.Tiles[x, _floor, z] = TileId.From(tileDef.Id, EditorState.SelectedRotation, EditorState.SelectedVariant);

			Redraw();
		}
	}
}
