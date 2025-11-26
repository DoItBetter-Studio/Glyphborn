
using Glyphborn.Common.Models;

namespace Glyphborn.Common.Util
{
	public static class MapSaver
	{
		public static MapJsonModel ToJson(MapModel model)
		{
			var json = new MapJsonModel()
			{
				Name = model.Name,
			};

			// Tiles
			for (int y = 0; y < MapModel.SizeY; y++)
			for (int z = 0; z < MapModel.SizeZ; z++)
			for (int x = 0; x < MapModel.SizeX; x++)
			{
				var id = model.Tiles[x, y, z].Value;
				if (id != 0)
				{
					json.Tiles.Add(new MapJsonTile
					{
						X = x, Y = y, Z = z,
						Id = id
					});
				}
			}

			// Collision
			for (int y = 0; y < MapModel.SizeY; y++)
			for (int z = 0; z < MapModel.SizeZ; z++)
			for (int x = 0; x < MapModel.SizeX; x++)
			{
				var c = model.Collision[x, y, z];
				if (c != 0)
				{
					json.Collisions.Add(new MapJsonCollision
					{
						X = x,
						Y = y,
						Z = z,
						Value = c
					});
				}
			}

			json.Events.AddRange(model.Events);
			json.Text.AddRange(model.LocalTexts);
			json.Scripts.AddRange(model.Scripts);

			return json;
		}
	}
}
