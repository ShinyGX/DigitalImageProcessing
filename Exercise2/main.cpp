// #include <windows.h>
#include <string>
#include <iostream>
#include "../DigitalImageProcessing/ImageUtil.h"

// typedef struct ImageData
// {
// 	BITMAPFILEHEADER fileHeader;
// 	BITMAPINFOHEADER infoHeader;
// 	RGBQUAD rgbquad[256];
// 	BYTE * pImg;
// 	int length;
// 	int width, height;
// }IMGDATA;

using namespace ImageUtil;


IMGDATA balance(GRAYHISTOGRAM histogram, IMGDATA data);
IMGDATA balance2(GRAYHISTOGRAM histogram, IMGDATA data);
double calculate(int index, GRAYHISTOGRAM histogram);

int main()
{
	std::string path;
	std::cin >> path;
	const IMGDATA data = ImageUtil::loadImage(path);
	GRAYHISTOGRAM grayhistogram = ImageUtil::getHistogram(data);
	grayhistogram.normalize();
	//grayhistogram.draw();
	ImageUtil::outputHistogram(data, "bitmap/_grayHistogram.bmp");
	const IMGDATA newData = balance2(grayhistogram, data);
	//histogram.draw();
	ImageUtil::outputHistogram(newData, "bitmap/_afterBalance.bmp");
	ImageUtil::outputImage(newData, 256, "bitmap/balance.bmp");
	//output(newData,256, "bitmap/balance.bmp");
	//system("pause");
	return 0;
}




IMGDATA balance(const GRAYHISTOGRAM histogram,const IMGDATA data)
{
	const IMGDATA newData = data;
	//newData.pImg = new BYTE[newData.length]{ 0 };
	for(int i = 0;i < data.length;i++)
	{
		newData.pImg[i] = static_cast<int>(calculate(data.pImg[i], histogram) * 255 + 0.5);
	}
	return newData;
}

IMGDATA balance2(const GRAYHISTOGRAM histogram, const IMGDATA data)
{
	//histogram.normalize();
	double result[256]{0};
	result[0] = histogram.gray[0];
	for(int i = 1;i < 256;i++)
	{
		result[i] = result[i - 1] + histogram.gray[i];
	}

	const IMGDATA newData = data;
	//newData.pImg = new BYTE[newData.length]{ 0 };
	for(int i = 0;i < data.length;i++)
	{
		newData.pImg[i] = result[data.pImg[i]] * 255 + 0.5;
	}

	return newData;
}

double calculate(int index, const GRAYHISTOGRAM histogram)
{
	double result = 0;
	for(int i = 0;i < index + 1;i++)
	{
		result += histogram.gray[i];
	}
	if (result > 1)
		result = 1;
	return result;
}



