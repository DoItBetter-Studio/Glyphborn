using System;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

using Glyphborn.Mapper.Editor;
using Glyphborn.Mapper.Tiles;

namespace Glyphborn.Mapper
{
	public partial class NewMapDialog : Form
	{
		public Tileset? Regional { get; private set; }
		public Tileset? Local { get; private set; }
		public Tileset? Interior { get; private set; }

		private ListView _regionalList;
		private ListView _localList;
		private ListView _interiorList;
		private CheckBox _enableInterior;

		public NewMapDialog()
		{
			Text = "New Map";
			Width = 700;
			Height = 500;
			FormBorderStyle = FormBorderStyle.FixedDialog;
			StartPosition = FormStartPosition.CenterParent;
			MaximizeBox = false;
			MinimizeBox = false;
			BackColor = Color.FromArgb(45, 45, 48);
			ForeColor = Color.White;

			BuildUI();
			LoadTilesets();
		}

		private ListView CreateTilesetList()
		{
			var lv = new ListView
			{
				View = View.Details,
				FullRowSelect = true,
				MultiSelect = false,
				Dock = DockStyle.Fill,
				HeaderStyle = ColumnHeaderStyle.None,
				BackColor = Color.FromArgb(30, 30, 30),
				ForeColor = Color.White,
				BorderStyle = BorderStyle.None
			};

			lv.Columns.Add("Tilesets", -2);
			return lv;
		}

		private void BuildUI()
		{
			var root = new TableLayoutPanel
			{
				Dock = DockStyle.Fill,
				ColumnCount = 3,
				RowCount = 2
			};

			root.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
			root.RowStyles.Add(new RowStyle(SizeType.Absolute, 50));

			root.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 33));
			root.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 33));
			root.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 34));

			_regionalList = CreateTilesetList();
			_localList = CreateTilesetList();
			_interiorList = CreateTilesetList();

			root.Controls.Add(Wrap("Regional", _regionalList), 0, 0);
			root.Controls.Add(Wrap("Local", _localList), 1, 0);
			root.Controls.Add(Wrap("Interior", _interiorList), 2, 0);

			_enableInterior = new CheckBox
			{
				Text = "Enable Interior Tileset",
				Dock = DockStyle.Fill,
				BackColor = Color.FromArgb(45, 45, 48),
				ForeColor = Color.White
			};

			_enableInterior.CheckedChanged += (_, __) =>
			{
				_interiorList.Enabled = _enableInterior.Checked;
				_interiorList.BackColor = _enableInterior.Checked
					? Color.FromArgb(30, 30, 30)
					: Color.FromArgb(20, 20, 20);
			};

			var createTilesetBtn = new Button
			{
				Width = 90,
				Text = "Create Tileset",
				Dock = DockStyle.Right,
				BackColor = Color.FromArgb(45, 45, 45),
				ForeColor = Color.White,
				FlatStyle = FlatStyle.Flat,
				Margin = new Padding(6)
			};

			createTilesetBtn.FlatAppearance.BorderColor = Color.FromArgb(70, 70, 70);
			createTilesetBtn.FlatAppearance.BorderSize = 1;
			createTilesetBtn.Click += OnCreateTileset;

			var createBtn = new Button
			{
				Width = 90,
				Text = "Create Map",
				Dock = DockStyle.Right,
				BackColor = Color.FromArgb(45, 45, 45),
				ForeColor = Color.White,
				FlatStyle = FlatStyle.Flat,
				Margin = new Padding(6)
			};

			createBtn.FlatAppearance.BorderColor = Color.FromArgb(70, 70, 70);
			createBtn.FlatAppearance.BorderSize = 1;
			createBtn.Click += OnCreateMap;

			var bottom = new Panel { Dock = DockStyle.Fill };
			bottom.Controls.Add(_enableInterior);
			bottom.Controls.Add(createTilesetBtn);
			bottom.Controls.Add(createBtn);

			root.Controls.Add(bottom, 0, 1);
			root.SetColumnSpan(bottom, 3);

			Controls.Add(root);
		}

		private Control Wrap(string title, Control content)
		{
			var panel = new Panel { Dock= DockStyle.Fill };

			panel.Controls.Add(content);
			panel.Controls.Add(new Label
			{
				Text = title,
				Dock = DockStyle.Top,
				Height = 28,
				Padding = new Padding(6, 6, 6, 0),
				BackColor = Color.FromArgb(20, 20, 20),
				ForeColor = Color.White,
				Font = new Font("Segoe UI Semibold", 9f)
			});

			return panel;
		}

		private void LoadTilesets()
		{
			Populate(_regionalList, TilesetPaths.Regional);
			Populate(_localList, TilesetPaths.Local);
			Populate(_interiorList, TilesetPaths.Interior);
		}

		private void Populate(ListView lv, string path)
		{
			lv.Items.Clear();

			if (!Directory.Exists(path))
				Directory.CreateDirectory(path);

			foreach (var file in Directory.EnumerateFiles(path, "*.gbts"))
			{
				lv.Items.Add(new ListViewItem(Path.GetFileNameWithoutExtension(file))
				{
					Tag = file
				});
			}
		}

		private void OnCreateMap(object? sender, EventArgs e)
		{
			if (_regionalList.SelectedItems.Count == 0 ||
				_localList.SelectedItems.Count == 0)
			{
				MessageBox.Show("Regional and Local tilesets are required.");
				return;
			}

			Regional = Tileset.LoadBinary((string) _regionalList.SelectedItems[0].Tag);
			Local = Tileset.LoadBinary((string) _localList.SelectedItems[0].Tag);

			if (_enableInterior.Checked && _interiorList.SelectedItems.Count == 0)
			{
				Interior = Tileset.LoadBinary((string)_interiorList.SelectedItems[0].Tag);
			}

			DialogResult = DialogResult.OK;
			Close();
		}

		private void OnCreateTileset(object? sender, EventArgs e)
		{
			var dlg = new CreateTilesetDialog();

			if (dlg.ShowDialog(this) != DialogResult.OK)
				return;

			string basePath = dlg.TilesetType == TilesetType.Regional ? TilesetPaths.Regional :
							  dlg.TilesetType == TilesetType.Local ? TilesetPaths.Local :
																	TilesetPaths.Interior;

			string path = Path.Combine(basePath, $"{dlg.TilesetName}.gbts");

			var tileset = new Tileset
			{
				Name = dlg.TilesetName,
				Type = dlg.TilesetType
			};

			// Always add Air tile first
			tileset.Tiles.Add(new TileDefinition
			{
				Id = 0,
				Name = "Air",
				Category = "System"
			});

			tileset.SaveBinary(path);
			LoadTilesets();
		}
	}
}
