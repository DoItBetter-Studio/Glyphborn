using System;

using Glyphborn.Common.Models;
using Glyphborn.Common.ViewModels;

namespace TileDefEditor.ViewModels
{
	public class TileDefViewModel : BaseViewModel
	{
		public TileDef Def { get; }

		public TileDefViewModel(TileDef def)
		{
			Def = def;
		}

		public string Name
		{
			get => Def.Name;
			set { Def.Name = value; OnPropertyChanged(); }
		}

		public ushort Id
		{
			get => Def.Id;
			set { Def.Id = value; OnPropertyChanged(); }
		}

		public bool IsSolid
		{
			get => Def.IsSolid;
			set { Def.IsSolid = value; OnPropertyChanged(); }
		}

		public bool IsWall
		{
			get => Def.IsWall;
			set { Def.IsWall = value; OnPropertyChanged(); }
		}

		public bool IsFloor
		{
			get => Def.IsFloor;
			set { Def.IsFloor = value; OnPropertyChanged(); }
		}

		public bool IsRoof
		{
			get => Def.IsRoof;
			set { Def.IsRoof = value; OnPropertyChanged(); }
		}

		public TileCategory Category
		{
			get => Def.Category;
			set
			{
				Def.Category = value;
				OnPropertyChanged();
			}
		}

		public Array CategoryValues => Enum.GetValues(typeof(TileCategory));

		public int RenderHeight
		{
			get => Def.RenderHeight;
			set { Def.RenderHeight = value; OnPropertyChanged(); }
		}

		public int MaxVariants
		{
			get => Def.MaxVariants;
			set { Def.MaxVariants = value; OnPropertyChanged(); }
		}

		public string? ModelPath
		{
			get => Def.ModelPath;
			set { Def.ModelPath = value; OnPropertyChanged(); }
		}

		public string? TexturePath
		{
			get => Def.TexturePath;
			set { Def.TexturePath = value; OnPropertyChanged(); }
		}
	}
}
