
#include "../DigitalImageProcessing/ImageUtil.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>
#include <valarray>
#include "../DigitalImageProcessing/Math.h"
#include "../DigitalImageProcessing/Ftt.h"

const double e = 2.7182818;

struct Pixel
{
	int x = 0, y = 0;
	BYTE pix = 0;

	Pixel(const int x, const int y, const BYTE p)
		:x(x),y(y),pix(p)
	{
	}

	Pixel() = default;
};

ImageUtil::IMGDATA prewitt(ImageUtil::IMGDATA data, int threadhold);
ImageUtil::IMGDATA sobel(ImageUtil::IMGDATA data, int threadhold);
ImageUtil::IMGDATA LOG(ImageUtil::IMGDATA data, double sqrSigma, int threadhold);
ImageUtil::IMGDATA canny(ImageUtil::IMGDATA data, const int minVal, const int maxVal);

double** getGaussianKernel(int size, double sqrSigma);

int main()
{
	std::string path;
	std::cin >> path;
	auto img = ImageUtil::loadImageToGray(path);
	auto p = prewitt(img, 100);
	auto s = sobel(img, 100);
	auto l = LOG(img, 0.05, 100);
	auto c = canny(img, 50, 100);
	ImageUtil::outputBlackWhiteImage(p, "bitmap/prewitt.bmp");
	ImageUtil::outputBlackWhiteImage(s,"bitmap/sobel.bmp");
	ImageUtil::outputBlackWhiteImage(l, "bitmap/LOG.bmp");
	ImageUtil::outputBlackWhiteImage(c, "bitmap/canny.bmp");
	delete[] c.pImg;
	delete[] l.pImg;
	delete[] s.pImg;
	delete[] p.pImg;
	delete[] img.pImg;
	return 0;
}

double** getGaussianKernel(const int size,const double sqrSigma)
{
	double** gaus = new double*[size];
	for (int i = 0; i < size; i++)
	{
		gaus[i] = new double[size];
	}
	const double pi = 4.0 * std::atan(1.0);
	const int center = size / 2;
	double sum = 0;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			gaus[i][j] = (1 / (2 * pi*sqrSigma))*exp(-((1 - center)*(1 - center) + (j - center)*(j - center)) / (2 * sqrSigma));
			sum += gaus[i][j];
		}
	}

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			gaus[i][j] /= sum;
		}
	}

	return gaus;
}

ImageUtil::IMGDATA canny(ImageUtil::IMGDATA data, const int minVal,const int maxVal)
{
//	BYTE * byte = new BYTE[data.width * data.height];
	double** gaus = getGaussianKernel(5,0.01);
	for (int i = 2; i < data.height - 2; i++)
	{
		for (int j = 2; j < data.width - 2; j++)
		{
			int sum = 0;
			for (int x = -2; x <= 2; x++)
			{
				for (int y = -2; y <= 2; y++)
				{
					sum += data[i + x][j + y] * gaus[4 - (x + 2)][y + 2];
				}				
			}
			data[i][j] = sum;
		}
	}

	ImageUtil::outputImage(data, "bitmap/gaus.bmp");
	BYTE *sobelImg = new BYTE[data.width * data.height];
	int *gxArr = new int[data.width * data.height];
	int *gyArr = new int[data.width * data.height];
	memset(sobelImg, 0, data.width * data.height);
	memset(gxArr, 0, data.width * data.height);
	memset(gyArr, 0, data.width * data.height);

	for (int i = 1; i < data.height - 1; i++)
	{
		for (int j = 1; j < data.width - 1; j++)
		{
			const int gx = data[i - 1][j - 1] * -1 + data[i][j - 1] * -2 + data[i + 1][j - 1] * -1 +
				data[i - 1][j + 1] * 1 + data[i][j + 1] * 2 + data[i + 1][j + 1] * 1;

			const int gy = data[i + 1][j - 1] * -1 + data[i + 1][j] * -2 + data[i + 1][j + 1] * -1 +
				data[i - 1][j - 1] * 1 + data[i - 1][j] * 2 + data[i - 1][j + 1] * 1;

			gxArr[i*data.width + j] = gx;
			gyArr[i*data.width + j] = gy;

			const double g = std::sqrt(gx*gx + gy * gy);
			sobelImg[i*data.width + j] = g;
		}
	}

	BYTE *temp = new BYTE[data.width * data.height];
	for(int i = 0;i < data.width * data.height;i++)
	{
		temp[i] = sobelImg[i];
	}

	for (int i = 1; i < data.height - 1; i++)
	{
		for (int j = 1; j < data.width - 1; j++)
		{
			double dir;
			if (gxArr[i*data.width + j] == 0)
				dir = 90;
			else
				dir = (std::atan(gyArr[i*data.width + j] / gxArr[i*data.width + j])) * 180 / ImageUtil::PI;
			//水平
			if ((dir >= 157.5 || dir <= -157.5) || (dir <= 22.5 && dir >= -22.5))
			{
				if (sobelImg[i * data.width + j] < sobelImg[i * data.width + j + 1] ||
					sobelImg[i * data.width + j] < sobelImg[i * data.width + j - 1])
				{
					temp[i * data.width + j] = 0;
				}
			}
			//-45度
			else if ((dir <= -112.5 && dir >= -157.5) || (dir <= 67.5 && dir >= 22.5))
			{
				if (sobelImg[i * data.width + j] < sobelImg[(i + 1) * data.width + j - 1] ||
					sobelImg[i * data.width + j] < sobelImg[(i - 1) * data.width + j + 1])
				{
					temp[i * data.width + j] = 0;
				}
			}
			//垂直
			else if ((dir >= -112.5 && dir <= -67.5) || (dir <= 112.5 && dir >= 67.5))
			{
				if (sobelImg[i * data.width + j] < sobelImg[(i + 1) * data.width + j] ||
					sobelImg[i * data.width + j] < sobelImg[(i - 1) * data.width + j])
				{
					temp[i * data.width + j] = 0;
				}
			}
			//+45度
			else if ((dir >= -67.5 && dir <= -22.5) || (dir <= 157.5 && dir >= 112.5))
			{
				if (sobelImg[i * data.width + j] < sobelImg[(i + 1) * data.width + j + 1] ||
					sobelImg[i * data.width + j] < sobelImg[(i - 1) * data.width + j - 1])
				{
					temp[i * data.width + j] = 0;
				}
			}
		}
	}	
	
	for (int i = 0; i < data.width * data.height; i++)
	{
		sobelImg[i] = temp[i];
	}
	delete[] temp;
	delete[] gxArr;
	delete[] gyArr;

	std::queue<Pixel> highPixQue;
	BYTE *lowPix = new BYTE[data.width * data.height];
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++) {
			if (sobelImg[i * data.width + j] > maxVal)
			{
				const Pixel p(j, i, 1);
				highPixQue.push(p);
			}
		

			if (sobelImg[i * data.width + j] > minVal && sobelImg[i * data.width + j] <= maxVal)
			{
				lowPix[i * data.width + j] = 1;
			}
			else
			{
				lowPix[i * data.width + j] = 0;
			}
		}
	}

	memset(sobelImg, 0, data.width*data.height);
	while (!highPixQue.empty())
	{
		const Pixel p = highPixQue.front();
		highPixQue.pop();

		sobelImg[p.y * data.width + p.x] = 1;
		for(int i = -1;i <= 1;i++)
		{
			for(int j = -1;j <= 1;j++)
			{
				if(p.y + i < 0 || p.y + i >= data.height || p.x + j < 0 || p.x + j >= data.width)
					continue;

				if (i == 0 && j == 0)
					continue;

				if (lowPix[(p.y + i) * data.width + p.x + j] == 1)
				{
					//8联通合并
					for (int x = -1; x <= 1; x++)
					{
						for (int y = -1; y <= 1; y++)
						{
							if (p.y + i + y < 0 || p.y + i + y >= data.height || p.x + j + x < 0 || p.x + j + x >= data.width)
								continue;

							if (lowPix[(p.y + i + y) * data.width + p.x + j + x] == 1)
							{
								highPixQue.push(Pixel(p.x + j, p.y + i, 1));
							}

								
						}
					}
					lowPix[(p.y + i) * data.width + p.x + j] = 0;

				}
			}
		}
	}

	delete[] lowPix;

	data.pImg = sobelImg;
	//delete[] sobelImg;

	for (int i = 0; i < 5; i++)
		delete[] gaus[i];

	delete[] gaus;

	return data;
}



ImageUtil::IMGDATA LOG(ImageUtil::IMGDATA data,double sqrSigma, const int threadhold)
{
	BYTE *img = new BYTE[data.width * data.height];
	memset(img, 0, data.width * data.height);
	double** gaus = getGaussianKernel(5, sqrSigma);
	for (int i = 2; i < data.height - 2; i++)
	{
		for (int j = 2; j < data.width - 2; j++)
		{
			int sum = 0;
			for (int x = -2; x <= 2; x++)
			{
				for (int y = -2; y <= 2; y++)
				{
					sum += data[i + x][j + y] * gaus[4 - (x + 2)][y + 2];
				}
				
			}
			img[i * data.width + j] = sum;
		}
	}
	// for (int i = 2; i < data.height - 2; i++)
	// {
	// 	for (int j = 2; j < data.width - 2; j++)
	// 	{
	// 		const int result = data[i + 2][j] +
	// 			data[i + 1][j - 1] + data[i + 1][j] * 2 + data[i + 1][j + 1] +
	// 			data[i][j - 2] + data[i][j - 1] * 2 + data[i][j] * -16 + data[i][j + 1] * 2 + data[i][j + 2] +
	// 			data[i - 1][j - 1] + data[i - 1][j] * 2 + data[i - 1][j + 1] +
	// 			data[i - 2][j];
 //
	// 		img[i*data.width + j] = result;
	// 	}
 //
	// }

	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			int up, down, left, right;
			if (i == 0)
				up = 0;
			else
				up = i - 1;

			if (i == data.height - 1)
				down = data.height - 1;
			else
				down = i + 1;

			if (j == 0)
				left = 0;
			else
				left = j - 1;

			if (j == data.width - 1)
				right = data.width - 1;
			else
				right = j + 1;

			img[i * data.width + j] = ImageUtil::clamp(
				1 * data.pImg[up * data.width + left] + 1 * data.pImg[up * data.width + j] + 1 * data.pImg[up * data.width + right] +
				1 * data.pImg[i * data.width + left] + -8 * data.pImg[i * data.width + j] + 1 * data.pImg[i * data.width + right] +
				1 * data.pImg[down * data.width + left] + 1 * data.pImg[down * data.width + j] + 1 * data.pImg[down * data.width + right]);
		}

	}

	for(int i = 0;i < data.width * data.height;i++)
	{
		if (img[i] > threadhold)
			img[i] = 1;
		else
			img[i] = 0;
	}

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