

#include <string>
#include "../DigitalImageProcessing/ImageUtil.h"
#include <iostream>
#include <valarray>
#include <queue>
#include "../DigitalImageProcessing/Math.h"
#include <map>

struct Pixel
{
	int x = 0, y = 0;
	BYTE pix = 0;

	Pixel(const int x, const int y, const BYTE p)
		:x(x), y(y), pix(p)
	{
	}

	Pixel() = default;
};

struct Hough
{
	double sigma, r;
	Hough() = default;
	Hough(double sigma, double r) :sigma(sigma), r(r) {}
};


ImageUtil::IMGDATA houghLine(ImageUtil::ImageData data);
ImageUtil::IMGDATA canny(ImageUtil::IMGDATA data, const int minVal, const int maxVal);

double** getGaussianKernel(const int size, const double sqrSigma);

int main()
{

	std::string path;
	std::cin >> path;
	auto img = ImageUtil::loadImageToGray(path);
	auto c = canny(img, 80, 100);
	ImageUtil::outputImage(c, "bitmap/canny.bmp");
	ImageUtil::outputBlackWhiteImage(houghLine(c), "bitmap/hough.bmp");
	return 0;
}

double** getGaussianKernel(const int size, const double sqrSigma)
{
	double **gaus = new double*[size];
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

ImageUtil::IMGDATA canny(ImageUtil::IMGDATA data, const int minVal, const int maxVal)
{
	//	BYTE * byte = new BYTE[data.width * data.height];
	double **gaus = getGaussianKernel(5, 0.01);
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
				data[i][j] = sum;
			}

		}
	}

	ImageUtil::outputImage(data, "bitmap/gaus.bmp");
	BYTE *sobelImg = new BYTE[data.width * data.height];
	memset(sobelImg, 0, data.width * data.height);
	for (int i = 1; i < data.height - 1; i++)
	{
		for (int j = 1; j < data.width - 1; j++)
		{
			const int gx = data[i - 1][j - 1] * -1 + data[i][j - 1] * -2 + data[i + 1][j - 1] * -1 +
				data[i - 1][j + 1] * 1 + data[i][j + 1] * 2 + data[i + 1][j + 1] * 1;

			const int gy = data[i + 1][j - 1] * -1 + data[i + 1][j] * -2 + data[i + 1][j + 1] * -1 +
				data[i - 1][j - 1] * 1 + data[i - 1][j] * 2 + data[i - 1][j + 1] * 1;


			const double g = std::sqrt(gx*gx + gy * gy);
			sobelImg[i*data.width + j] = g;
		}
	}

	std::queue<Pixel> pixQue;
	int start = 0, end = 0;


	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			if (sobelImg[i * data.width + j] > maxVal)
			{
				const Pixel p(j, i, sobelImg[i*data.width + j]);
				pixQue.push(p);
			}
			else if (sobelImg[i * data.width + j] < maxVal && sobelImg[i * data.width + j] > minVal)
			{
				for (int h = -1; h <= 1; h++)
				{
					bool isBreak = false;
					for (int w = -1; w <= 1; w++)
					{
						if (i + h < 0 || i + h > data.height || j + w < 0 || j + w > data.width)
							continue;
						if (sobelImg[(i + h) * data.width + j + w] > maxVal)
						{
							const Pixel p(j, i, sobelImg[i*data.width + j]);
							pixQue.push(p);
							isBreak = true;
							break;
						}
					}

					if (isBreak)
						break;
				}
			}
		}
	}

	//BYTE *cannyPix = new BYTE[data.width * data.height];
	memset(sobelImg, 0, data.width*data.height);
	while (!pixQue.empty())
	{
		const auto p = pixQue.front();
		sobelImg[p.y * data.width + p.x] = p.pix;
		pixQue.pop();
	}

	data.pImg = sobelImg;
	//delete[] sobelImg;
	return data;
}

ImageUtil::IMGDATA houghLine(ImageUtil::ImageData data)
{
	std::map<double, double> houghMap;

	for(int i = 0;i < data.height;i++)
	{
		for(int j = 0;j < data.width;j++)
		{
			if(data[i][j] > 0)
			{
				double r = std::sqrt(i * i + j * j);
				double sigma = std::acos(j / r);
				houghMap[sigma] = r;

			}

		}
	}

	int *pointArr = new int[data.width * data.height];
	memset(pointArr, 0, data.width* data.height);
	for (auto& e : houghMap)
	{
		for (int i = 0; i < data.height; i++)
		{
			bool isBeak = false;
			for (int j = 0; j < data.width; j++)
			{
				int y = (e.second - j * std::cos(e.first)) / std::sin(e.first);
				int x = (e.second - i * std::sin(e.first)) / std::cos(e.first);
				if (x >= data.width || x < 0 || y >= data.height || y < 0)
				{
					isBeak = true;
					break;
				}
					

				pointArr[y * data.width + x]++;

			}

			if (isBeak)
				break;
		}
	}

	int maxVal = -1;
	for(int i = 0;i < data.height;i++)
	{
		for(int j = 0;j < data.width;j++)
		{
			if(maxVal < pointArr[i * data.width + j])
			{
				maxVal = pointArr[i * data.width + j];
			}
		}
	}

	BYTE *img = new BYTE[data.width * data.height];
	memset(img, 0, data.width * data.height);
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			if(pointArr[i * data.width + j] > 0)
			{
				img[i * data.width + j] = (static_cast<double>(pointArr[i * data.width + j]) / maxVal) * 255;
			}
		}
	}

	delete[] pointArr;
	data.pImg = img;
	return data;
}
