using System.Drawing;
using System.Windows.Forms;

using Glyphborn.Mapper.Controls;
using Glyphborn.Mapper.Editor;

namespace Glyphborn.Mapper
{
	public partial class _3DViewDialog : Form
	{
		private readonly Map3DViewportControl _view;

		public _3DViewDialog(MapDocument map, AreaDocument area)
		{
			Text = "3D Map Preview";
			Width = 900;
			Height = 700;
			StartPosition = FormStartPosition.CenterParent;

			BackColor = Color.Black;

			_view = new Map3DViewportControl
			{
				Dock = DockStyle.Fill,
				Map = map,
				Area = area
			};

			Controls.Add( _view );
		}
	}
}
