using System.Windows;

using TileDefEditor.ViewModels;

namespace TileDefEditor
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainViewModel VM { get; }

		public MainWindow()
		{
			InitializeComponent();
			VM = new MainViewModel();
			DataContext = VM;
		}
	}
}