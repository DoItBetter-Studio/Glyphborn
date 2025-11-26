using Glyphborn.Common.Models;

namespace Glyphborn.Common.Util
{
	public static class MapLoader
	{
		public static MapModel FromJson(MapJsonModel json)
		{
			var map = new MapModel()
			{
				Name = json.Name
			};

			// Tiles
			foreach (var t in json.Tiles)
			{
				if (ValidateXYZ(t.X, t.Y, t.Z))
					map.Tiles[t.X, t.Y, t.Z] = new TileId(t.Id);
			}

			// Collision
			foreach (var c in json.Collisions)
			{
				if (ValidateXYZ(c.X, c.Y, c.Z))
					map.Collision[c.X, c.Y, c.Z] = c.Value;
			}

			// Events
			map.Events.AddRange(json.Events);

			// Text
			map.LocalTexts.AddRange(json.Text);

			// Scripts
			map.Scripts.AddRange(json.Scripts);

			return map;
		}

		private static bool ValidateXYZ(int x, int y, int z)
		{
			return x >= 0 && x < MapModel.SizeX &&
				   y >= 0 && y < MapModel.SizeY &&
				   z >= 0 && z < MapModel.SizeZ;
		}
	}
}
