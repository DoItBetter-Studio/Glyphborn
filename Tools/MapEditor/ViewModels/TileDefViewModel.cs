using Glyphborn.Common.Models;
using Glyphborn.Common.ViewModels;

namespace MapEditor.ViewModels
{
	public class TileDefViewModel : BaseViewModel
	{
		public TileDef Def { get; }

		public TileDefViewModel(TileDef def)
		{
			Def = def;
		}

		public string Name => Def.Name;
		public ushort Id => Def.Id;

		private bool _isSelected;
		public bool IsSelected
		{
			get => _isSelected;
			set => SetProperty(ref _isSelected, value);
		}
	}
}
