using System.Collections.Generic;
using System.IO;
using System.Text;

using Glyphborn.Mapper.Tiles;

namespace Glyphborn.Mapper.Editor
{
	public sealed class MapDocument
	{
		public List<Tileset> Tilesets { get; } = new();

		public const int WIDTH = 32;
		public const int HEIGHT = 32;
		public const int LAYERS = 32;

		public TileRef[][][] Tiles;

		public bool IsDirty { get; private set; }
		public bool IsPreview { get; set; } = false;

		private Stack<ICommand> _undoStack = new();
		private Stack<ICommand> _redoStack = new();
		private List<TileCommand>? _currentBatch;

		public MapDocument()
		{
			Tiles = new TileRef[LAYERS][][];

			for (int l = 0; l < LAYERS; l++)
			{
				Tiles[l] = new TileRef[HEIGHT][];
				for (int y = 0; y < HEIGHT; y++)
					Tiles[l][y] = new TileRef[WIDTH];
			}
		}

		public void BeginBatch()
		{
			_currentBatch = new List<TileCommand>();
		}

		public void EndBatch()
		{
			if (_currentBatch != null && _currentBatch.Count > 0)
			{
				_undoStack.Push(new BatchCommand(_currentBatch.ToArray()));
				_currentBatch = null;
				_redoStack.Clear();
			}
		}

		public void SetTile(int layer, int x, int y, TileRef tile)
		{
			ref var current = ref Tiles[layer][y][x];

			if (current.Tileset == tile.Tileset &&
				current.TileId == tile.TileId)
				return;

			var cmd = new TileCommand
			{
				Layer = layer,
				X = x,
				Y = y,
				OldTile = current,
				NewTile = tile,
			};

			if (_currentBatch != null)
			{
				_currentBatch.Add(cmd);
			}
			else
			{
				_undoStack.Push(cmd);
				_redoStack.Clear();
			}

			current = tile;
			IsDirty = true;
		}

		public bool Undo()
		{
			if (_undoStack.Count == 0) return false;

			var cmd = _undoStack.Pop();
			_redoStack.Push(cmd);

			cmd.Undo(this);
			return true;
		}

		public bool Redo()
		{
			if (_redoStack.Count == 0) return false;

			var cmd = _redoStack.Pop();
			_undoStack.Push(cmd);

			cmd.Redo(this);
			return true;
		}

		public void SaveBinary(string path)
		{
			using (var fs = new FileStream(path, FileMode.Create))
			using (var bw = new BinaryWriter(fs))
			{
				// Write tileset count
				bw.Write((byte) Tilesets.Count);

				// Write tileset paths (relative to Assets/)
				foreach (var tileset in Tilesets)
				{
					string relativePath = $"{tileset.Type.ToString().ToLower()}/{tileset.Name}.gbts";
					byte[] pathBytes = Encoding.UTF8.GetBytes(relativePath);
					bw.Write((ushort) pathBytes.Length);
					bw.Write(pathBytes);
				}

				for (int layers = 0; layers < LAYERS; layers++)
				{
					for (int y = 0; y < HEIGHT; y++)
					{
						for (int x = 0; x < WIDTH; x++)
						{
							var tile = Tiles[layers][y][x];

							// Pack tileset (2 bits) + tileId (14 bits) into ushort
							ushort packed = (ushort) ((tile.Tileset << 14) | tile.TileId);
							bw.Write(packed);
						}
					}
				}

				for (int layer = 0; layer < LAYERS; layer++)
				{
					for (int y = 0; y < HEIGHT; y++)
					{
						for (int x = 0; x < WIDTH; x++)
						{
							bw.Write((byte) 0);
						}
					}
				}
			}
			
			IsDirty = false;
		}

		public static MapDocument LoadBinary(string path)
		{
			using (var fs = new FileStream(path, FileMode.Open))
			using (var br = new BinaryReader(fs))
			{
				var doc = new MapDocument(); 
				
				byte tilesetCount = br.ReadByte();

				for (byte i = 0; i < tilesetCount; i++)
				{
					byte[] bytes = br.ReadBytes(br.ReadUInt16());
					string tilesetName = Encoding.UTF8.GetString(bytes).Trim('\0');

					Tileset tilset = Tileset.LoadBinary(tilesetName);
					doc.Tilesets.Add(tilset);
				}

				for (int layer = 0; layer < LAYERS; layer++)
				{
					for (int y = 0; y < HEIGHT; y++)
					{
						for (int x = 0; x < WIDTH; x++)
						{
							ushort packed = br.ReadUInt16();

							byte tileset = (byte) ((packed >> 14) & 0x3);
							ushort tileId = (ushort) (packed & 0x3FFF);

							doc.Tiles[layer][y][x] = new TileRef
							{
								Tileset = tileset,
								TileId = tileId,
							};
						}
					}
				}

				fs.Seek(LAYERS * HEIGHT * WIDTH, SeekOrigin.Current);

				return doc;
			}
		}

		public void FloodFill(int layer, int startX, int startY, TileRef fillTile)
		{
			if (startX < 0 || startY < 0 || startX >= WIDTH || startY >= HEIGHT)
				return;

			var targetTile = Tiles[layer][startY][startX];

			// Don't fill if already the target tile
			if (targetTile.Tileset == fillTile.Tileset && targetTile.TileId == fillTile.TileId)
				return;

			var stack = new Stack<(int x, int y)>();
			var visited = new HashSet<(int, int)>();

			stack.Push((startX, startY));

			while (stack.Count > 0)
			{
				var (x, y) = stack.Pop();

				if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
					continue;

				if (visited.Contains((x, y)))
					continue;

				var currentTile = Tiles[layer][y][x];

				if (currentTile.Tileset != targetTile.Tileset || currentTile.TileId != targetTile.TileId)
					continue;

				visited.Add((x, y));
				SetTile(layer, x, y, fillTile);

				stack.Push((x - 1, y));
				stack.Push((x + 1, y));
				stack.Push((x, y - 1));
				stack.Push((x, y + 1));
			}
		}
	}

	interface ICommand
	{
		void Undo(MapDocument doc);
		void Redo(MapDocument doc);
	}

	struct TileCommand : ICommand
	{
		public int Layer;
		public int X, Y;
		public TileRef OldTile;
		public TileRef NewTile;

		public void Undo(MapDocument doc)
		{
			doc.Tiles[Layer][Y][X] = OldTile;
		}

		public void Redo(MapDocument doc)
		{
			doc.Tiles[Layer][Y][X] = NewTile;
		}
	}

	class BatchCommand : ICommand
	{
		private TileCommand[] _commands;

		public BatchCommand(TileCommand[] commands)
		{
			_commands = commands;
		}

		public void Undo(MapDocument doc)
		{
			foreach (var cmd in _commands)
				cmd.Undo(doc);
		}

		public void Redo(MapDocument doc)
		{
			foreach (var cmd in _commands)
				cmd.Redo(doc);
		}
	}

	public struct TileRef
	{
		public byte Tileset;
		public ushort TileId;
	}
}
