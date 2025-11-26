using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Windows.Media;

namespace MapEditor.Converters
{
	public class BoolToBrushConverter : IValueConverter
	{
		private static readonly SolidColorBrush Selected	= new SolidColorBrush(Color.FromRgb(70, 70, 120));
		private static readonly SolidColorBrush Normal		= new SolidColorBrush(Color.FromRgb(40, 40, 40));

		public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
		{
			if (value is bool b && b)
				return Selected;

			return Normal;
		}

		public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) => Binding.DoNothing;
	}
}
