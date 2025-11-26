using System.Collections.Generic;
using System.IO;

using Glyphborn.Common.IO;
using Glyphborn.Common.Models;

namespace Glyphborn.Common.Compilers
{
	public static class ScriptTableCompiler
	{
		public static void WriteScriptsBin(List<MapScriptStub> scriptStubs, string path)
		{
			using var fs = File.Create(path);
			using var bw = new BinaryWriterLE(fs);

			bw.Write((uint) scriptStubs.Count);

			foreach (var s in scriptStubs)
			{
				bw.Write((uint) s.Id);
				bw.Write((uint) s.Offset);
				bw.Write((uint) s.Length);
			}
		}
	}
}
