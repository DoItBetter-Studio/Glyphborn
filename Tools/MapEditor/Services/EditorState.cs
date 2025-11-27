using Glyphborn.Common.Models;

namespace MapEditor.Services
{
	public static class EditorState
	{
		public static TileDef? SelectedTile { get; set; }

		// Painting options (rotation 0..7, variant 0..7)
		private static byte _selectedRotation;
		public static byte SelectedRotation
		{
			get => _selectedRotation;
			set => _selectedRotation = (byte) (value & 0x7);
		}

		private static byte _selectedVariant;
		public static byte SelectedVariant
		{
			get => _selectedVariant;
			set => _selectedVariant = (byte) (value & 0x7);
		}
	}
}