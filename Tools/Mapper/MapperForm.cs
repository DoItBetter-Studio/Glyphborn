using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

using Glyphborn.Mapper.Colors;
using Glyphborn.Mapper.Controls;
using Glyphborn.Mapper.Editor;
using Glyphborn.Mapper.Tiles;

namespace Glyphborn.Mapper
{
	public partial class MapperForm : Form
	{
		private MapDocument? _mapDocument;
		private EditorState _editorState = new();
		private Panel _canvasHost;
		private MapCanvasControl _mapCanvasControl;
		private TableLayoutPanel? _tilesetColumn;

		public MapperForm()
		{
			InitializeComponent();

			// 1. Apply our dark renderer
			menuStrip.Renderer = new ToolStripProfessionalRenderer(new MenuStripColor());

			// 2. Style the top bar itself
			menuStrip.BackColor = Color.FromArgb(45, 45, 48);

			ApplyDarkThemeToMenu(menuStrip.Items);

			this.BackColor = Color.FromArgb(45, 45, 48);
			this.ForeColor = Color.White;
		}

		private void MapperForm_Load(object sender, EventArgs e)
		{
			_canvasHost = new Panel
			{
				Dock = DockStyle.Fill,
				AutoScroll = true,
				BackColor = Color.FromArgb(30, 30, 30)
			};

			var root = new TableLayoutPanel
			{
				Dock = DockStyle.Fill,
				ColumnCount = 3,
				RowCount = 1,
				BackColor = Color.FromArgb(45, 45, 48)
			};

			root.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 266));
			root.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 120));
			root.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));

			_mapCanvasControl = new MapCanvasControl
			{
				Dock = DockStyle.Fill,
				State = _editorState
			};

			root.Controls.Add(_mapCanvasControl, 2, 0);

			var layersPanel = new Panel
			{
				Dock = DockStyle.Fill,
				AutoScroll = true,
			};

			root.Controls.Add(layersPanel, 1, 0);	

			var layersFlow = new FlowLayoutPanel
			{
				Dock = DockStyle.Top,
				FlowDirection = FlowDirection.TopDown,
				WrapContents = false,
				AutoSize = true,
				BackColor = Color.FromArgb(30, 30, 30)
			};

			layersPanel.Controls.Add(layersFlow);

			layersFlow.Controls.Clear();

			for (int i = 0; i < 32; i++)
			{
				int layerIndex = i;

				var btn = new Button
				{
					Text = $"Layer {i}",
					Width = 90,
					Height = 28,
					Margin = new Padding(4),
					BackColor = Color.FromArgb(45, 45, 45),
					ForeColor = Color.White,
					FlatStyle = FlatStyle.Flat,
				};

				btn.FlatAppearance.BorderColor = Color.FromArgb(45, 45, 45);
				btn.FlatAppearance.BorderSize = 1;

				btn.Click += (_, __) =>
				{
					_editorState.CurrentLayer = layerIndex;
					_mapCanvasControl.Invalidate();
					UpdateLayerButtons(layersFlow, layerIndex);
				};

				layersFlow.Controls.Add(btn);
			}

			_tilesetColumn = new TableLayoutPanel
			{
				Dock = DockStyle.Fill,
				ColumnCount = 1
			};

			root.Controls.Add(_tilesetColumn, 0, 0);

			_canvasHost.Controls.Add(root);
			Controls.Add(_canvasHost);
			_canvasHost.BringToFront();


			_mapDocument = CreateStartupMap();
			SetMap(_mapDocument);

			undoToolStripMenuItem.Click += (s, e) => { _mapDocument.Undo(); _mapCanvasControl.Invalidate(); };
			redoToolStripMenuItem.Click += (s, e) => { _mapDocument.Redo(); _mapCanvasControl.Invalidate(); };
		}

		private MapDocument CreateStartupMap()
		{
			var map = new MapDocument();

			map.Tilesets.Add(new Tileset { Name = "Regional" });
			map.Tilesets.Add(new Tileset { Name = "Local" });
			map.Tilesets.Add(new Tileset { Name = "Interior" });

			map.IsPreview = true;

			return map;
		}

		private void UpdateLayerButtons(FlowLayoutPanel layersFlow, int layerIndex)
		{
			for (int i = 0; i < layersFlow.Controls.Count; i++)
			{
				var btn = (Button)layersFlow.Controls[i];
				btn.BackColor = (i == layerIndex) ? Color.DodgerBlue : Color.FromArgb(45, 45, 45);
			}
		}

		private void ApplyDarkThemeToMenu(ToolStripItemCollection items)
		{
			foreach (ToolStripItem item in items)
			{
				item.ForeColor = Color.White;

				if (item is ToolStripMenuItem menuItem)
				{
					ApplyDarkThemeToMenu(menuItem.DropDownItems);
				}
			}
		}

		void SetMap(MapDocument map)
		{
			_mapDocument = map;
			_mapCanvasControl.Document = map;
			BindMap(map);

			// Canvas fills available space
			_mapCanvasControl.Dock = DockStyle.Fill;

			_mapCanvasControl.Invalidate();
		}

		void BindMap(MapDocument map)
		{
			_tilesetColumn!.SuspendLayout();
			_tilesetColumn.Controls.Clear();
			_tilesetColumn.RowStyles.Clear();

			_tilesetColumn.RowCount = map.Tilesets.Count;

			for (int i = 0; i < map.Tilesets.Count; i++)
			{
				_tilesetColumn.RowStyles.Add(new RowStyle(SizeType.Percent, 100f / map.Tilesets.Count));
			}

			for (byte i = 0; i < map.Tilesets.Count; i++)
			{
				var ts = map.Tilesets[i];

				var pane = new TilesetPane(ts.Name, i, ts, _editorState);

				_tilesetColumn.Controls.Add(pane, 0, i);
			}

			_tilesetColumn.ResumeLayout();
		}

		private void NewMap_Click(object sender, EventArgs e)
		{
			using var dlg = new NewMapDialog();

			if (dlg.ShowDialog(this) == DialogResult.OK)
			{
				var map = new MapDocument();
				map.Tilesets.Add(dlg.Regional!);
				map.Tilesets.Add(dlg.Local!);

				if (dlg.Interior != null)
					map.Tilesets.Add(dlg.Interior);

				SetMap(map);

				saveMapToolStripMenuItem.Enabled = true;
				saveMapAsToolStripMenuItem.Enabled = true;
				exportMapToolStripMenuItem.Enabled = true;
			}
		}


		private void SaveMap_Click(Object sender, EventArgs e)
		{
			if (_mapDocument!.IsPreview || _mapDocument == null)
				return;

			var sfd = new SaveFileDialog
			{
				Filter = "Map Binary|*.gbm",
				FileName = "chunk_0000_0000.gbm"
			};

			if (sfd.ShowDialog() == DialogResult.OK)
			{
				_mapDocument.SaveBinary(sfd.FileName);
				MessageBox.Show("Map Saved!", "Success");
			}
		}

		private void LoadMap_Click(object sender, EventArgs e)
		{
			var ofd = new OpenFileDialog { Filter = "Map Binary|*.gbm" };

			if (ofd.ShowDialog() == DialogResult.OK)
			{
				_mapDocument = MapDocument.LoadBinary(ofd.FileName);
				SetMap(_mapDocument);
			}
		}

		private void ShowGrid_Click(object sender, EventArgs e)
		{
			_editorState.ShowGrid = !_editorState.ShowGrid;
			_mapCanvasControl.Refresh();
		}
	}
}
