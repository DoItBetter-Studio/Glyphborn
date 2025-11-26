using System;
using System.IO;

namespace Glyphborn.Common.IO
{
	public class BinaryReaderLE : IDisposable
	{
		private readonly Stream _s;

		public BinaryReaderLE(Stream s) => _s = s;

		public byte ReadByte() => (byte)_s.ReadByte();

		public ushort ReadU16()
		{
			int lo = _s.ReadByte();
			int hi = _s.ReadByte();
			return (ushort)(lo | (hi << 8));
		}

		public uint ReadU32()
		{
			uint b0 = (uint) _s.ReadByte();
			uint b1 = (uint) _s.ReadByte();
			uint b2 = (uint) _s.ReadByte();
			uint b3 = (uint) _s.ReadByte();
			return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
		}

		public void Dispose() => _s.Dispose();
	}
}
