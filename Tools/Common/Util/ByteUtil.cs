using System;

namespace Glyphborn.Common.Util
{
	public static class ByteUtil
	{
		// Clamp to 0-255
		public static byte ClampByte(int v)
		{
			if (v < 0) return 0;
			if (v > 255) return 255;
			return (byte) v;
		}

		// Conver bool to 0/1
		public static byte BoolToByte(bool v) => (byte)(v ? 1 : 0);

		// Convert a ushort to little-endian byte[2]
		public static byte[] ToBytesLE(ushort v)
		{
			return new byte[]
			{
				(byte)(v & 0xFF),
				(byte)(v >> 8),
			};
		}

		// Converts a uint to little-endian byte[4]
		public static byte[] ToBytesLE(uint v)
		{
			return new byte[]
			{
				(byte)(v & 0xff),
				(byte)((v >> 8) & 0xff),
				(byte)((v >> 16)  & 0xff),
				(byte)((v >> 24) & 0xff)
			};
		}

		// Read U16 from two bytes LE
		public static ushort ReadU16(byte lo, byte hi)
		{
			return (ushort) (lo | (hi << 8));
		}

		// Read U32 from four bytes LE
		public static uint ReadU32(byte b0, byte b1, byte b2, byte b3)
		{
			return (uint) (
				(b0) |
				(b1 << 8) |
				(b2 << 16) |
				(b3 << 24)
			);
		}
	}
}
