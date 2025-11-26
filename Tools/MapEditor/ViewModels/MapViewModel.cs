using Glyphborn.Common.Models;
using Glyphborn.Common.ViewModels;

namespace MapEditor.ViewModels
{
	public class MapViewModel : BaseViewModel
	{
		public MapModel Model { get; }

		public int _currentFloor;
		public int CurrentFloor
		{
			get => _currentFloor;
			set
			{
				if (SetProperty(ref _currentFloor, value))
				{
					
				}
			}
		}

		public int _cameraDirection;
		public int CameraDirection
		{
			get => _cameraDirection;
			set => SetProperty(ref _cameraDirection, value);
		}

		public MapViewModel(MapModel model)
		{
			Model = model;
		}
	}
}