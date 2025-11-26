using System;
using System.IO;
using System.Text;

namespace Glyphborn.Common.IO
{
	public class BinaryWriterLE : IDisposable
	{
		private readonly Stream _s;

		public BinaryWriterLE(Stream s) => _s = s;

		public void Write(byte v) => _s.WriteByte(v);

		public void Write(ushort v)
		{
			_s.WriteByte((byte)(v & 0xFF));
			_s.WriteByte((byte)(v >> 8));
		}

		public void Write(uint v)
		{
			_s.WriteByte(((byte) (v & 0xFF)));
			_s.WriteByte(((byte) ((v >> 8) & 0xFF)));
			_s.WriteByte(((byte) ((v >> 16) & 0xFF)));
			_s.WriteByte(((byte) ((v >> 24) & 0xFF)));
		}

		public void Write(byte[] data) => _s.Write(data, 0, data.Length);

		public void WriteAscii(string s)
		{
			var data = Encoding.UTF8.GetBytes(s);
			_s.Write(data, 0, data.Length);
		}

		public void Dispose() => _s.Dispose();
	}
}
