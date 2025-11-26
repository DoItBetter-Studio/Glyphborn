using System.IO;

using Newtonsoft.Json;

namespace Glyphborn.Common.Util
{
	public static class JsonUtil
	{
		private static readonly JsonSerializerSettings Settings = new JsonSerializerSettings
			{
				Formatting = Formatting.Indented,
				NullValueHandling = NullValueHandling.Ignore,
			};

		public static T Load<T>(string path)
		{
			string json = File.ReadAllText(path);
			return JsonConvert.DeserializeObject<T>(json, Settings)!;
		}

		public static void Save<T>(string path, T obj)
		{
			var json = JsonConvert.SerializeObject(obj, Settings);
			PathUtil.EnsureParentDirectory(path);
			File.WriteAllText(path, json);
		}

		// Try-load version (no exception)
		public static bool TryLoad<T>(string path, out T? value)
		{
			try
			{
				if (!File.Exists(path))
				{
					value = default;
					return false;
				}

				value = Load<T>(path);
				return true;
			}
			catch
			{
				value = default;
				return false;
			}
		}
	}
}
