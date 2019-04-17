
#include "../DigitalImageProcessing/ImageUtil.h"
#include <iostream>
#include "../DigitalImageProcessing/Ftt.h"
#include <algorithm>

const double e = 2.7182818;

ImageUtil::IMGDATA prewitt(ImageUtil::IMGDATA data, int threadhold);
ImageUtil::IMGDATA sobel(ImageUtil::IMGDATA data, int threadhold);
ImageUtil::IMGDATA LOG(ImageUtil::IMGDATA data, double sqrSigma, int threadhold);

int main()
{
	std::string path;
	std::cin >> path;
	auto img = ImageUtil::loadImageToGray(path);
	auto p = prewitt(img, 100);
	auto s = sobel(img, 100);
	auto l = LOG(img, 2, 100);
	ImageUtil::outputBlackWhiteImage(p, "bitmap/prewitt.bmp");
	ImageUtil::outputBlackWhiteImage(s,"bitmap/sobel.bmp");
	ImageUtil::outputBlackWhiteImage(l, "bitmap/LOG.bmp");

	delete[] l.pImg;
	delete[] s.pImg;
	delete[] p.pImg;
	delete[] img.pImg;
	return 0;
}

ImageUtil::IMGDATA LOG(ImageUtil::IMGDATA data,double sqrSigma, const int threadhold)
{
	BYTE *img = new BYTE[data.width * data.height];
//	BYTE *gaussian = new BYTE[data.width * data.height];
	memset(img, 0, data.width * data.height);
//	memset(gaussian, 0, data.width * data.height);
	for (int i = 2; i < data.height - 2; i++)
	{
		for (int j = 2; j < data.width - 2; j++)
		{
			const int result = data[i + 2][j] +
				data[i + 1][j - 1] + data[i + 1][j] * 2 + data[i + 1][j + 1] +
				data[i][j - 2] + data[i][j - 1] * 2 + data[i][j] * -16 + data[i][j + 1] * 2 + data[i][j + 2] +
				data[i - 1][j - 1] + data[i - 1][j] * 2 + data[i - 1][j + 1] +
				data[i - 2][j];

			if (result > threadhold)
				img[i*data.width + j] = 1;
		}

	}

	// for (int i = 0; i < data.height; i++)
	// {
	// 	for (int j = 0; j < data.width; j++)
	// 	{
	// 		int up, down, left, right;
	// 		if (i == 0)
	// 			up = 0;
	// 		else
	// 			up = i - 1;
 //
	// 		if (i == data.height - 1)
	// 			down = data.height - 1;
	// 		else
	// 			down = i + 1;
 //
	// 		if (j == 0)
	// 			left = 0;
	// 		else
	// 			left = j - 1;
 //
	// 		if (j == data.width - 1)
	// 			right = data.width - 1;
	// 		else
	// 			right = j + 1;
 //
	// 	   const int result = ImageUtil::clamp(
	// 			1 * gaussian[up * data.width + left] + 1 * gaussian[up * data.width + j] + 1 * gaussian[up * data.width + right] +
	// 			1 * gaussian[i * data.width + left] + -8 * gaussian[i * data.width + j] + 1 * gaussian[i * data.width + right] +
	// 			1 * gaussian[down * data.width + left] + 1 * gaussian[down * data.width + j] + 1 * gaussian[down * data.width + right]);
 //
	// 		if(result > threadhold)
	// 		{
	// 			img[i * data.width + j] = 1;
	// 		}
	// 	}
 //
	// }
 //
	// delete[] gaussian;
	data.pImg = img;
	return data;
}

ImageUtil::IMGDATA sobel(ImageUtil::IMGDATA data, const int threadhold)
{
	BYTE *img = new BYTE[data.width * data.height];
	memset(img, 0, data.width * data.height);
	for (int i = 1; i < data.height - 1; i++)
	{
		for (int j = 1; j < data.width - 1; j++)
		{
			const int gx = data[i - 1][j - 1] * -1 + data[i][j - 1] * -2 + data[i + 1][j - 1] * -1 +
				data[i - 1][j + 1] * 1 + data[i][j + 1] * 2 + data[i + 1][j + 1] * 1;

			const int gy = data[i + 1][j - 1] * -1 + data[i + 1][j] * -2 + data[i + 1][j + 1] * -1 +
				data[i - 1][j - 1] * 1 + data[i - 1][j] * 2 + data[i - 1][j + 1] * 1;


			const double g = std::sqrt(gx*gx + gy * gy);
			if (g > threadhold)
				img[i*data.width + j] = 1;
		}
	}
	data.pImg = img;
	return data;
}

ImageUtil::IMGDATA prewitt(ImageUtil::IMGDATA data, const int threadhold)
{
	BYTE *img = new BYTE[data.width * data.height];
	memset(img, 0, data.width * data.height);
	for (int i = 1; i < data.height - 1; i++)
	{
		for (int j = 1; j < data.width - 1; j++)
		{
			const int dx = data.pImg[(i + 1)*data.width + j - 1] + data.pImg[(i + 1)*data.width + j] + data.pImg[(i + 1)*data.width + j + 1]
				- (data.pImg[(i - 1)*data.width + j - 1] + data.pImg[(i - 1)*data.width + j] + data.pImg[(i - 1)*data.width + j + 1]);

			const int dy = data.pImg[(i - 1) *data.width + j + 1] + data.pImg[i*data.width + j + 1] + data.pImg[(i - 1)*data.width + j + 1]
				- (data.pImg[(i + 1) *data.width + j - 1] + data.pImg[i*data.width + j - 1] + data.pImg[(i - 1)*data.width + j - 1]);

			if (std::_Max_value(dx, dy) > threadhold)
			{
				img[i * data.width + j] = 1;
			}

		}
	}
	data.pImg = img;
	return data;
}