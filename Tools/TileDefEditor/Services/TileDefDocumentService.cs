using Microsoft.Win32;

namespace TileDefEditor.Services
{
	public class TileDefDocumentService
	{
		public string? CurrentPath { get; set; }

		public string? OpenDialog()
		{
			var dlg = new OpenFileDialog
			{
				Filter = "Tile Definitions|*.json"
			};

			if (dlg.ShowDialog() == true)
				return dlg.FileName;

			return null;
		}

		public string? SaveDialog()
		{
			var dlg = new SaveFileDialog
			{
				Filter = "Tile Definitions|*.json"
			};

			if (dlg.ShowDialog() == true)
				return dlg.FileName;

			return null;
		}
	}
}
