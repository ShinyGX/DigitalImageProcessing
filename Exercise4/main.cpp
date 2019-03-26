#define __DEBUG

#define PI 3.1415926

#ifdef __DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif



#include "../DigitalImageProcessing/ImageUtil.h"
#include <iostream>
#include <valarray>

using ImageUtil::IMGDATA;

IMGDATA mirror(IMGDATA data);
IMGDATA scale(IMGDATA data);
IMGDATA rotate(IMGDATA data);
IMGDATA translate(IMGDATA data);

int main()
{
	std::string path;
	std::cin >> path;
	IMGDATA data = ImageUtil::loadImage(path);
	IMGDATA mirrorIMG = mirror(data), scaleIMG = scale(data),rotateImg = rotate(data),transImg = translate(data);

	ImageUtil::outputImage(mirrorIMG,"bitmap/mirror.bmp");
	ImageUtil::outputImage(scaleIMG, "bitmap/scale.bmp");
	ImageUtil::outputImage(rotateImg, "bitmap/rotate.bmp");
	ImageUtil::outputImage(transImg, "bitmap/translate.bmp");

	delete[] data.pImg;
	delete[] mirrorIMG.pImg;
	delete[] scaleIMG.pImg;
	delete[] rotateImg.pImg;
	delete[] transImg.pImg;

#ifdef __DEBUG
	 _CrtDumpMemoryLeaks();
	 system("pause");
#endif
	
	return 0;
}

IMGDATA rotate(IMGDATA data)
{
	int rotateAngle;
	std::cout << "旋转的角度" << std::endl;
	std::cin >> rotateAngle;

	int k = data.infoHeader.biBitCount / 8;

	BYTE *newData = new BYTE[data.width * data.height * k];
	for(int i =0;i < data.width * data.height * k;i++)
	{
		newData[i] = 0;
	}

	//弧度制的角度
	double angle = 1.0 * rotateAngle * PI / 180;
	int point = -k;
	int midY = static_cast<float>(data.height) / 2, midX = static_cast<float>(data.width) / 2;
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			int aftX = j - midX;
			int aftY = i - midY;
			int x = (aftX * std::cos(angle) + aftY * std::sin(angle)) + midX;
			int y = (-aftX * std::sin(angle) + aftY * std::cos(angle)) + midY;


			point += k;
			if (x < 0 || x >= data.width || y < 0 || y >= data.height)
				continue;

			for(int biCount = 0;biCount < k;biCount++)
			{
				newData[point + biCount] = data.pImg[y * data.width * k + x * k + biCount];
			}

		}
	}

	IMGDATA img = data;
	img.pImg = newData;

	return img;
}


IMGDATA translate(IMGDATA data)
{
	int xTrans, yTrans;
	std::cout << "x轴的位移" << std::endl;
	std::cin >> xTrans;
	std::cout << "y轴的位移" << std::endl;
	std::cin >> yTrans;

	int k = data.infoHeader.biBitCount / 8;

	BYTE *newData = new BYTE[data.width * data.height * k];
	for (int i = 0; i < data.width * data.height * k; i++)
	{
		newData[i] = 0;
	}

	int point = -k;
	for(int i = 0;i < data.height;i++)
	{
		for(int j = 0;j < data.width;j++)
		{

			int x = j + xTrans;
			int y = i + yTrans;

			point += k;
			if (x < 0 || x >= data.width || y < 0 || y >= data.height)
				continue;

			for(int biCount = 0;biCount < k;biCount++)
			{
				newData[point + biCount] = data.pImg[y * data.width * k + x * k + biCount];
			}
		}
	}

	IMGDATA img = data;
	img.pImg = newData;
	return img;
}


ImageUtil::IMGDATA scale(ImageUtil::IMGDATA data)
{
	float xScale, yScale;
	std::cout << "x轴的缩放" << std::endl;
	std::cin >> xScale;
	std::cout << "y轴的缩放" << std::endl;
	std::cin >> yScale;

	const int k = data.infoHeader.biBitCount / 8;

	IMGDATA newImg = data;
	newImg.width = xScale * data.width;
	newImg.height = yScale * data.height;

	newImg.infoHeader.biWidth = newImg.width;
	newImg.infoHeader.biHeight = newImg.height;

	const int byteWidth = (newImg.width * k + 3) / 4 * 4;
	newImg.infoHeader.biSizeImage = byteWidth * newImg.height;
	newImg.fileHeader.bfSize = newImg.infoHeader.biSizeImage + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + newImg.infoHeader.biClrUsed * sizeof(RGBQUAD);



	//兼容灰度图，24位图，32位图
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

			//兼容灰度图，24位图，32位图
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



