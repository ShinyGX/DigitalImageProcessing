

#include <string>
#include "../DigitalImageProcessing/ImageUtil.h"
#include <iostream>
#include "../DigitalImageProcessing/Math.h"
#include <queue>
#include <valarray>

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


ImageUtil::IMGDATA hough(ImageUtil::ImageData data, double deltaSigma, double deltaRadius, int threadhold);
ImageUtil::IMGDATA canny(ImageUtil::IMGDATA data, const int minVal, const int maxVal);
void drawLine(ImageUtil::ImageData& img, int x0, int y0, int x1, int y1, const int clr);
void drawLine(ImageUtil::ImageData& img, const double k, const double b, const int clr);
double** getGaussianKernel(const int size, const double sqrSigma);

int main()
{

	std::string path;
	std::cin >> path;
	auto img = ImageUtil::loadImageToGray(path);
	auto c = canny(img, 40,80);
	ImageUtil::outputBlackWhiteImage(c, "bitmap/canny.bmp");
	auto h = hough(c, 0.5, 2, 30);
	
	ImageUtil::outputImage(h.pImg, h.width, h.height, 3, 8, h.rgbquad, "bitmap/hough_result.bmp");

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
	double** gaus = getGaussianKernel(5, 0.01);
	for (unsigned int i = 2; i < data.height - 2; i++)
	{
		for (unsigned int j = 2; j < data.width - 2; j++)
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

	for (ImageUtil::ImageSize i = 1; i < data.height - 1; i++)
	{
		for (ImageUtil::ImageSize j = 1; j < data.width - 1; j++)
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
	for (ImageUtil::ImageSize i = 0; i < data.width * data.height; i++)
	{
		temp[i] = sobelImg[i];
	}

	for (ImageUtil::ImageSize i = 1; i < data.height - 1; i++)
	{
		for (ImageUtil::ImageSize j = 1; j < data.width - 1; j++)
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
	for (unsigned int i = 0; i < data.height; i++)
	{
		for (unsigned int j = 0; j < data.width; j++) {
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
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				if (p.y + i < 0 || p.y + i >= data.height || p.x + j < 0 || p.x + j >= data.width)
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

ImageUtil::IMGDATA hough(ImageUtil::ImageData data, const double deltaSigma, const double deltaRadius, const int threadhold)
{
	typedef ImageUtil::ImageSize size;

	const size maxRadius = std::sqrt(data.width * data.width + data.height * data.height);


	const size radius = std::ceil(maxRadius / deltaRadius) * 2;
	const size sigma = std::ceil(360 / deltaSigma);

	size *houghArr = new size[radius * sigma];
	memset(houghArr, 0,radius*sigma * sizeof size);

	for (size i = 0; i < data.height; i++)
	{
		for (size j = 0; j < data.width; j++)
		{
			if (data[i][j] == 0)
				continue;

			for (double theta = 0; theta < 360;)
			{

				const double radian = ImageUtil::toRadian(theta);
				const double r = j * std::cos(radian) + i * std::sin(radian) + maxRadius;
				theta += deltaSigma;
				// if(r > maxRadius)
				// 	continue;
				houghArr[static_cast<unsigned int>((r / deltaRadius) * sigma + theta / deltaSigma)]++;

			
			}

		}
	}

	size max = 0;
	for(size i = 0;i < radius * sigma;i++)
	{
		if (houghArr[i] > max)
			max = houghArr[i];
	}

	BYTE * houghImg = new BYTE[sigma*radius];
	for(size i = 0;i < radius*sigma;i++)
	{
		houghImg[i] = static_cast<double>(houghArr[i]) / max * 255;
	}
	std::cout << max;

	ImageUtil::outputImage(houghImg,radius, sigma, 256, 8, data.rgbquad, "bitmap/hough.bmp");


	for (double i = 0; i < radius;)
	{
		for (double j = 0; j < sigma;)
		{
			if(houghArr[static_cast<int>(i * sigma + j)] < max * 0.6f)
			{
				j += deltaSigma;
				continue;
			}
				
			const double radian = ImageUtil::toRadian(j * deltaSigma);
			if (std::sin(radian) == 0)
			{
				drawLine(data, 0, i - maxRadius, 2);
			}
			else
			{
				drawLine(data, -(std::cos(radian) / std::sin(radian)), (i * deltaRadius - maxRadius) / std::sin(radian), 2);
			}

			j += deltaSigma;
		}

		i += deltaRadius;
	}

	RGBQUAD rgb[3];
	rgb[0].rgbBlue = 0;
	rgb[0].rgbGreen = 0;
	rgb[0].rgbRed = 0;
	rgb[0].rgbReserved = 0;

	rgb[1].rgbBlue = 255;
	rgb[1].rgbGreen = 255;
	rgb[1].rgbRed = 255;
	rgb[1].rgbReserved = 0;

	rgb[2].rgbBlue = 255;
	rgb[2].rgbGreen = 0;
	rgb[2].rgbRed = 0;
	rgb[2].rgbReserved = 0;

	for (int i = 0; i < 3; i++)
	{
		data.rgbquad[i] = rgb[i];
	}

	delete[] houghImg;
	delete[] houghArr;
	return data;
}

void drawLine(ImageUtil::ImageData& img, const double k, const double b, const int clr)
{
	for (ImageUtil::ImageSize i = 0; i < img.width; i++)
	{
		const double y = k * i + b;

		if (y < 0 || y >= img.height)
			continue;
		const double y1 = k * (i - 1) + b;
		if (i > 0)
		{
			for (int j = y1; j < static_cast<int>(y); j++)
			{
				if(j < 0 || j >= static_cast<int>(img.height))
					continue;
				img[j][i] = clr;
			}
		}

		img[y][i] = clr;

	}
}

void drawLine(ImageUtil::ImageData& img, int x0, int y0, int x1, int y1, const int clr)
{
	if (x0 > x1)
	{
		const int temp = x0;
		x0 = x1;
		x1 = temp;
	}

	if(y0 > y1)
	{
		const int temp = y0;
		y0 = y1;
		y1 = temp;
	}

	const double k = static_cast<double>(y1 - y0) / (x1 - x0);
	const double b = x0 * k - y0;
	for (int i = x0; i < x1; i++)
	{
		const double y = k * i + b;
		if (y > img.height || y < 0 || i > img.width || i < 0)
			break;
		img[y][i] = clr;
	}
}