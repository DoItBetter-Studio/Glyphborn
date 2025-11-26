using System;
using System.Windows;

using MapEditor.ViewModels;

namespace MapEditor
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainWindowViewModel VM { get; private set; }

		public MainWindow()
		{
			InitializeComponent();
			Loaded += OnLoaded;
		}

		private void OnLoaded(object sender, RoutedEventArgs e)
		{
			VM = (MainWindowViewModel)DataContext;

			if (VM.CurrentMap != null)
			{
				MapCanvas.SetContext(
					VM.CurrentMap.Model,
					VM.CurrentMap.CurrentFloor,
					VM.CurrentMap.CameraDirection);
			}
		}
	}
}