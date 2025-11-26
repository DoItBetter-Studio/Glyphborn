using System.IO;
using System.Runtime.ExceptionServices;

namespace Glyphborn.Common.Util
{
	public static class PathUtil
	{
		// Ensures a directory exists (creates if missing)
		public static void EnsureDirectory(string path)
		{
			if (!Directory.Exists(path))
				Directory.CreateDirectory(path);
		}

		// Returns a path with OS-correct separators
		public static string Normalize(string path)
		{
			return path.Replace('/', Path.DirectorySeparatorChar).Replace('\\', Path.DirectorySeparatorChar);
		}


		// Combines multiple paths safely
		public static string Combine(params string[] paths)
		{
			return Normalize(Path.Combine(paths));
		}

		// Returns directory of file
		public static string DirectoryOf(string filePath)
		{
			return Path.GetDirectoryName(Normalize(filePath))!;
		}

		// Returns filename without extension
		public static string FileNameWithoutExt(string filePath)
		{
			return Path.GetFileNameWithoutExtension(filePath);
		}

		// Ensures the parent folder of a file exists
		public static void EnsureParentDirectory(string filePath)
		{
			string? dir = Path.GetDirectoryName(filePath);
			if (dir != null)
				EnsureDirectory(dir);
		}
	}
}
