using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

using Glyphborn.Mapper.Editor;
using Glyphborn.Mapper.Tiles;

namespace Glyphborn.Mapper.Controls
{
	sealed class TilesetPane : Panel
	{
		public TilesetPane(string name, byte tilesetIndex, IReadOnlyList<TileDefinition> tiles, EditorState state)
		{
			Dock = DockStyle.Fill;
			BackColor = Color.FromArgb(30, 30, 30);

			var label = new Label
			{
				Text = name,
				Dock = DockStyle.Top,
				Height = 30,
				Padding = new Padding(6),
				BackColor = Color.FromArgb(20, 20, 20),
				ForeColor = Color.White
			};

			var scroll = new Panel
			{
				Dock = DockStyle.Fill,
				AutoScroll = true
			};

			var tileset = new TilesetControl
			{
				Tiles = tiles,
				TilesetIndex = tilesetIndex,
				Width = 266,
				Dock = DockStyle.Top
			};

			tileset.TileSelected += sel =>
			{
				state.SelectedTile = sel;
			};

			tileset.Height = tileset.GetRequiredHeight();
			tileset.Invalidate();

			scroll.Controls.Add(tileset);
			Controls.Add(scroll);
			Controls.Add(label);
		}
	}
}
