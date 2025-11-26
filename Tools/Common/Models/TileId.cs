namespace Glyphborn.Common.Models
{
	public readonly struct TileId
	{
		public ushort Value { get; }

		public TileId(ushort value) => Value = value;

		public static TileId From(ushort type, byte rot, byte variant)
		{
			ushort v = (ushort)((type << 6) | ((rot & 0x7) << 3) | (variant & 0x7));
			return new TileId(v);
		}

		public ushort Type => (ushort) (Value >> 6);
		public byte Rotation => (byte)((Value >> 3) & 0x7);
		public byte Variant => (byte) (Value & 0x7);

		public override string ToString() => $"TileId(Type={Type}, Rot={Rotation}, Var={Variant}";
	}
}
