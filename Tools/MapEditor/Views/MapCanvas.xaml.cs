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
			int width = (int) ActualWidth;
			int height = (int) ActualHeight;

			if (width < 1 || height < 1)
				return;

			_bitmap = new WriteableBitmap(
				width, height, 96, 96,
				System.Windows.Media.PixelFormats.Bgra32,
				null);

			Surface.Source = _bitmap;
			_surface = new SoftwareRenderSurface(_bitmap);
			_renderer = new MapRenderer(_surface);
		}

		public void SetContext(MapModel map, int floor, int cameraDir)
		{
			_map = map;
			_floor = floor;
			_cameraDir = cameraDir;
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
			PaintAt(e.GetPosition(this));
			Console.WriteLine("Painting?");
		}

		private void OnMouseMove(object sender, MouseEventArgs e)
		{
			if (!_isPainting)
				return;

			if (e.LeftButton == MouseButtonState.Pressed)
			{
				PaintAt(e.GetPosition(this));
			}
			else
			{
				_isPainting = false;
			}
		}

		private void PaintAt(Point p)
		{
			if (_map == null || _renderer == null)
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
			_map.Tiles[x, _floor, z] = new TileId(tileDef.Id);

			Redraw();
		}
	}
}
