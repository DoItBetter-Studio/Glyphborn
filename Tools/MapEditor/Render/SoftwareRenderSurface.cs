using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Imaging;

namespace MapEditor.Render
{
	public class SoftwareRenderSurface
	{
		private readonly WriteableBitmap _bmp;
		private readonly int _stride;
		private readonly int _width;
		private readonly int _height;

		public SoftwareRenderSurface(WriteableBitmap bmp)
		{
			_bmp = bmp;
			_width = bmp.PixelWidth;
			_height = bmp.PixelHeight;
			_stride = _width * 4;
		}

		public void Clear(uint color)
		{
			_bmp.Lock();

			unsafe
			{
				byte* ptr = (byte*) _bmp.BackBuffer.ToPointer();
				uint* pixel = (uint*) ptr;

				int total = _width * _height;
				for (int i = 0; i < total; i++)
					pixel[i] = color;
			}

			_bmp.AddDirtyRect(new System.Windows.Int32Rect(0, 0, _width, _height));
			_bmp.Unlock();
		}

		public void DrawPixel(int x, int y, uint color)
		{
			if (x < 0 || y < 0 || x >= _width || y >= _height) return;

			_bmp.Lock();

			unsafe
			{
				byte* ptr = (byte*) _bmp.BackBuffer.ToPointer();
				uint* pixel = (uint*) ptr + (y * _width + x);
				*pixel = color;
			}

			_bmp.AddDirtyRect(new Int32Rect(x, y, 1, 1));
			_bmp.Unlock();
		}
	}
}
