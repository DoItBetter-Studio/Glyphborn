using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

using Microsoft.Win32;

using TileDefEditor.ViewModels;

namespace TileDefEditor.Views
{
    /// <summary>
    /// Interaction logic for TileDefDetails.xaml
    /// </summary>
    public partial class TileDefDetails : UserControl
    {
        public TileDefDetails()
        {
            InitializeComponent();
        }

		private void Save_Click(object sender, RoutedEventArgs e)
		{
			var mainVm = Application.Current?.MainWindow?.DataContext as MainViewModel;
			if (mainVm == null) return;

			// Authoritative selection from the list Vm
			var selectedFromList = mainVm.TileList.SelectedTile;

			// If user is editing a new, unattached item, it may be the Details DataContext.
			var editingVm = this.DataContext as TileDefViewModel;

			// Prefer the selected tile in the list; if none, fall back to the details DataContext.
			var vm = selectedFromList ?? editingVm;
			if (vm == null) return;

			// If the underlying model is not yet in the database, add it.
			var existing = mainVm.Database.GetById(vm.Id);
			if (existing == null)
			{
				// If id is zero or collides, assign a new unique id.
				if (vm.Id == 0 || mainVm.Database.GetById(vm.Id) != null)
				{
					ushort newId = 1;
					if (mainVm.Database.Tiles.Count > 0)
						newId = (ushort) (mainVm.Database.Tiles.Max(t => (int) t.Id) + 1);

					vm.Id = newId; // updates vm.Def.Id via TileDefViewModel
				}

				mainVm.Database.Tiles.Add(vm.Def);

				// Ensure the ObservableCollection contains this VM instance.
				if (!mainVm.TileList.Tiles.Contains(vm))
					mainVm.TileList.Tiles.Add(vm);

				mainVm.TileList.SelectedTile = vm;
				return;
			}

			// Existing tile: properties are bound to the same model so no copy is necessary.
			// Ensure the ObservableCollection selection points to the corresponding VM.
			var listVm = mainVm.TileList.Tiles.FirstOrDefault(t => t.Def == vm.Def || t.Id == vm.Id);
			if (listVm != null)
				mainVm.TileList.SelectedTile = listVm;
		}

		private TileDefViewModel? GetEditingVm()
		{
			// Prefer the selected item in TileList, fall back to DataContext
			var mainVm = Application.Current?.MainWindow?.DataContext as MainViewModel;
			var selected = mainVm?.TileList.SelectedTile;
			if (selected != null) return selected;

			return this.DataContext as TileDefViewModel;
		}

		private void BrowseModel_Click(object sender, RoutedEventArgs e)
		{
			var vm = GetEditingVm();
			if (vm == null) return;

			var dlg = new OpenFileDialog
			{
				Filter = "Model Files (*.gmdl;*.obj;*.fbx)|*.gmdl;*.obj;*.fbx|All Files|*.*"
			};

			if (dlg.ShowDialog() == true)
			{
				vm.ModelPath = dlg.FileName;
			}
		}

		private void BrowseTexture_Click(object sender, RoutedEventArgs e)
		{
			var vm = GetEditingVm();
			if (vm == null) return;

			var dlg = new OpenFileDialog
			{
				Filter = "Image Files (*.png;*.jpg;*.jpeg;*.bmp)|*.png;*.jpg;*.jpeg;*.bmp|All Files|*.*"
			};

			if (dlg.ShowDialog() == true)
			{
				vm.TexturePath = dlg.FileName;
			}
		}

		private void IncVariants_Click(object sender, RoutedEventArgs e)
		{
			var vm = GetEditingVm();
			if (vm == null) return;

			vm.MaxVariants = Math.Max(1, vm.MaxVariants + 1);
		}

		private void DecVariants_Click(object sender, RoutedEventArgs e)
		{
			var vm = GetEditingVm();
			if (vm == null) return;

			vm.MaxVariants = Math.Max(1, vm.MaxVariants - 1);
		}
	}
}
