#define __DEBUG

#ifdef __DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif



#include "../DigitalImageProcessing/ImageUtil.h"
#include <iostream>

using ImageUtil::IMGDATA;

IMGDATA mirror(IMGDATA data);
IMGDATA scale(IMGDATA data);

int main()
{
	std::string path;
	std::cin >> path;
	IMGDATA data = ImageUtil::loadImage(path);
	IMGDATA mirrorIMG = mirror(data), scaleIMG = scale(data);

	ImageUtil::outputImage(mirrorIMG, 0, "bitmap/mirror.bmp");
	ImageUtil::outputImage(scaleIMG, 0, "bitmap/scale.bmp");

	delete[] data.pImg;
	delete[] mirrorIMG.pImg;
	delete[] scaleIMG.pImg;

#ifdef __DEBUG
	 _CrtDumpMemoryLeaks();
	 system("pause");
#endif
	
	return 0;
}

ImageUtil::IMGDATA scale(ImageUtil::IMGDATA data)
{
	float xScale, yScale;
	std::cout << "x÷·µƒÀı∑≈" << std::endl;
	std::cin >> xScale;
	std::cout << "y÷·µƒÀı∑≈" << std::endl;
	std::cin >> yScale;

	const int k = data.infoHeader.biBitCount / 8;

	IMGDATA newImg = data;
	newImg.width = xScale * data.width;
	newImg.height = yScale * data.height;

	newImg.infoHeader.biWidth = newImg.width;
	newImg.infoHeader.biHeight = newImg.height;

	const int byteWidth = (newImg.width * k + 3) / 4 * 4;
	newImg.infoHeader.biSizeImage = byteWidth * newImg.height;
	newImg.fileHeader.bfSize = newImg.infoHeader.biSizeImage + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);



	//ºÊ»›ª“∂»Õº£¨24ŒªÕº£¨32ŒªÕº
	BYTE *newData = new BYTE[newImg.width * k * newImg.height];
	int pixelPoint = -k;
	for (int i = 0; i < newImg.height; i++)
	{
		for (int j = 0; j < newImg.width ; j++)
		{
			const double originX = static_cast<double>(j) / xScale;
			const double originY = static_cast<double>(i) / yScale;

			const int originPixelX = originX;
			const int originPixelY = originY;

			const double distanceOriginPixelX = originX - originPixelX;
			const double distanceOriginPixelY = originY - originPixelY;

			int originPixelXNext = originPixelX + 1;
			int originPixelYNext = originPixelY + 1;

			if (originPixelXNext >= data.width)
				originPixelXNext = data.width - 1;
			if (originPixelYNext >= data.height)
				originPixelYNext = data.height - 1;

			//ºÊ»›ª“∂»Õº£¨24ŒªÕº£¨32ŒªÕº
			pixelPoint += k;
			for (int biCount = 0; biCount < k; biCount++)
			{
				newData[pixelPoint + biCount] = ImageUtil::clamp(
					data.pImg[originPixelY * data.width * k + originPixelX * k + biCount] * (1 - distanceOriginPixelX) * (1 - distanceOriginPixelY) +
					data.pImg[originPixelY * data.width * k + originPixelXNext * k + biCount] * (distanceOriginPixelX) * (1 - distanceOriginPixelY) +
					data.pImg[originPixelYNext * data.width * k + originPixelX * k + biCount] * (distanceOriginPixelY) * (1 - distanceOriginPixelX) +
					data.pImg[originPixelYNext * data.width * k + originPixelXNext * k + biCount] * distanceOriginPixelY * distanceOriginPixelX);

			}

		}
	}

	newImg.pImg = newData;
	return newImg;
}


IMGDATA mirror(IMGDATA data)
{
	BYTE *newData = new BYTE[data.length];
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width * 3; j += 3)
		{
			newData[i * data.width * 3 + j] = data.pImg[i*data.width * 3 + (data.width * 3 - 1 - j - 2)];
			newData[i * data.width * 3 + j + 1] = data.pImg[i*data.width * 3 + (data.width * 3 - 1 - j - 1)];
			newData[i * data.width * 3 + j + 2] = data.pImg[i*data.width * 3 + (data.width * 3 - 1 - j)];
		}
	}

	IMGDATA newImg = data;
	newImg.pImg = newData;
	return newImg;
}



