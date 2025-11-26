using Glyphborn.Common.Models;

namespace Glyphborn.Common.Util
{
	public static class MapFile
	{
		public static MapModel Load(string path)
		{
			var json = JsonUtil.Load<MapJsonModel>(path);
			return MapLoader.FromJson(json);
		}

		public static void Save(string path, MapModel model)
		{
			var json = MapSaver.ToJson(model);
			JsonUtil.Save(path, json);
		}
	}
}
