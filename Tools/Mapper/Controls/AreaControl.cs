using System;
using System.Windows.Forms;

using Glyphborn.Mapper.Editor;

namespace Glyphborn.Mapper.Controls
{
	public sealed class AreaControl : UserControl
	{
		public AreaDocument? Area { get; private set; }

		public event Action<MapDocument?>? MapSelected;

		private readonly TableLayoutPanel _grid;

		public AreaControl()
		{
			Dock = DockStyle.Left;
			Width = 200;

			_grid = new TableLayoutPanel
			{
				Dock = DockStyle.Fill
			};

			Controls.Add(_grid);
		}

		public void SetArea(AreaDocument area)
		{
			Area = area;
			RebuildGrid();
		}

		private void RebuildGrid()
		{
			_grid.Controls.Clear();
			_grid.ColumnStyles.Clear();
			_grid.RowStyles.Clear();

			if (Area == null)
				return;

			_grid.ColumnCount = Area.Width;
			_grid.RowCount = Area.Height;

			for (int x = 0; x < Area.Width; x++)
				_grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100f / Area.Width));

			for (int y = 0; y < Area.Width; y++)
				_grid.RowStyles.Add(new RowStyle(SizeType.Percent, 100f / Area.Height));

			for (int y = 0; y < Area.Height; y++)
			{
				for (int x = 0; x < Area.Width; x++)
				{
					var map = Area.Maps[x, y];

					var btn = new Button
					{
						Dock = DockStyle.Fill,
						Enabled = map != null,
						Tag = map
					};

					btn.Click += (_, __) =>
					{
						MapSelected?.Invoke((MapDocument?) btn.Tag);
					};

					_grid.Controls.Add(btn, x, y);
				}
			}
		}
	}
}
