using System;
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
		private Panel _clientHost;
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

			this.BackColor = Color.Black;
			this.ForeColor = Color.White;
		}

		private void MapperForm_Load(object sender, EventArgs e)
		{
			_clientHost = new Panel
			{
				Dock = DockStyle.Fill,
				AutoScroll = true,
				BackColor = Color.FromArgb(30, 30, 30),
				BorderStyle = BorderStyle.None,
				Padding = new Padding(0, 2, 0, 0)
			};

			var root = new TableLayoutPanel
			{
				Dock = DockStyle.Fill,
				ColumnCount = 4,
				RowCount = 1,
				BackColor = Color.FromArgb(45, 45, 48)
			};

			root.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 276));
			root.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 134));
			root.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
			root.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 400));

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

			for (int i = 0; i < MapDocument.LAYERS; i++)
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

			layersFlow.Controls[0].BackColor = Color.DodgerBlue;

			_tilesetColumn = new TableLayoutPanel
			{
				Dock = DockStyle.Fill,
				ColumnCount = 1
			};

			root.Controls.Add(_tilesetColumn, 0, 0);

			_clientHost.Controls.Add(root);
			Controls.Add(_clientHost);
			_clientHost.BringToFront();

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
				map.Name = dlg.MapName!;

				if (dlg.Interior != null)
					map.Tilesets.Add(dlg.Interior);

				SetMap(map);
				_mapDocument!.IsPreview = false;

				saveMapToolStripMenuItem.Enabled = true;
				saveMapAsToolStripMenuItem.Enabled = true;
				exportMapToolStripMenuItem.Enabled = true;
			}
		}

		private void LoadMap_Click(object sender, EventArgs e)
		{
			var ofd = new OpenFileDialog { Filter = "Map Binary|*.gbm" };

			if (ofd.ShowDialog() == DialogResult.OK)
			{
				_mapDocument = MapDocument.LoadBinary(ofd.FileName);
				SetMap(_mapDocument);

				saveMapToolStripMenuItem.Enabled = true;
				saveMapAsToolStripMenuItem.Enabled = true;
				exportMapToolStripMenuItem.Enabled = true;
			}
		}

		private void SaveMap_Click(object sender, EventArgs e)
		{
			if (_mapDocument == null || _mapDocument.IsPreview)
				return;

			_mapDocument.SaveBinary();
			MessageBox.Show("Map Saved!", "Success");
		}

		private void SaveMapAs_Click(object sender, EventArgs e)
		{
			var sad = new SaveAsDialog();

			if (sad.ShowDialog() == DialogResult.OK)
			{
				_mapDocument!.Name = sad.MapName!;
				MapSerializer.SaveBinary(_mapDocument);
			}
		}

		private void Export_Click(object sender, EventArgs e)
		{
			MessageBox.Show("Export is not currently setup.\nThis will be apart of a future update.", "Export", MessageBoxButtons.OK, MessageBoxIcon.Question);
		}

		private void Exit_Click(object sender, EventArgs e)
		{
			Application.Exit();
		}

		private void ShowGrid_Click(object sender, EventArgs e)
		{
			_editorState.ShowGrid = !_editorState.ShowGrid;
			_mapCanvasControl.Refresh();
		}

		private void _3DView_Click(object sender, EventArgs e)
		{
			if (_mapDocument == null)
				return;

			var dlg = new _3DViewDialog(_mapDocument);
			dlg.Show();
		}

		// Prompt to save when closing. Cancelable.
		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			base.OnFormClosing(e);

			// If no document or it's preview, nothing to do
			if (_mapDocument == null || _mapDocument.IsPreview)
				return;

			if (!_mapDocument.IsDirty)
				return;

			var result = MessageBox.Show(
				"You have unsaved changes. Save before exit?",
				"Unsaved Changes",
				MessageBoxButtons.YesNoCancel,
				MessageBoxIcon.Warning,
				MessageBoxDefaultButton.Button1
			);

			if (result == DialogResult.Cancel)
			{
				// Cancel the close
				e.Cancel = true;
				return;
			}

			if (result == DialogResult.Yes)
			{
				try
				{
					_mapDocument.SaveBinary();
				}
				catch (Exception ex)
				{
					MessageBox.Show($"Failed to save map: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
			}

			// if DialogResult.No we proceed and close
		}

		// Final cleanup after form closes
		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			// Clear editor-only caches and dispose resources
			try
			{
				TilePreviewer.ClearCache();
			}
			catch { }

			base.OnFormClosed(e);
		}
	}
}
