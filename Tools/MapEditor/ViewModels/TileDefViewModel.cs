using System.Windows.Media;
using System.Windows.Media.Imaging;

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
		
		public string? TexturePath => Def.TexturePath;
		public ImageSource? Thumbnail
		{
			get
			{
				if (string.IsNullOrEmpty(Def.TexturePath))
					return null;

				try
				{
					var bi = new BitmapImage();
					bi.BeginInit();
					bi.CacheOption = BitmapCacheOption.OnLoad;
					bi.UriSource = new System.Uri(Def.TexturePath, System.UriKind.RelativeOrAbsolute);
					bi.DecodePixelWidth = 32;
					bi.EndInit();
					bi.Freeze();
					return bi;
				}
				catch
				{
					return null;
				}
			}
		}

		private bool _isSelected;
		public bool IsSelected
		{
			get => _isSelected;
			set => SetProperty(ref _isSelected, value);
		}
	}
}
