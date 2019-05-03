

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
	int x, y;
	std::vector<Hough> child;
	Hough() = default;
	Hough(double sigma, double r,int x,int y) :sigma(sigma), r(r),x(x),y(y) {}

	bool operator==(Hough& h) const
	{
		return h.sigma == sigma;
	}

	void addChild(Hough& h)
	{
		child.push_back(h);
	}
};


ImageUtil::IMGDATA houghLine(ImageUtil::ImageData data, int max);
ImageUtil::IMGDATA canny(ImageUtil::IMGDATA data, const int minVal, const int maxVal);
void drawLine(BYTE* img, int x0, int y0, int x1, int y1, int width, int clr);
double** getGaussianKernel(const int size, const double sqrSigma);

int main()
{

	std::string path;
	std::cin >> path;
	auto img = ImageUtil::loadImageToGray(path);
	auto c = canny(img, 50,100);
	ImageUtil::outputImage(c, "bitmap/canny.bmp");
	ImageUtil::outputBlackWhiteImage(houghLine(c,3), "bitmap/hough.bmp");
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
	for (int i = 0; i < data.width * data.height; i++)
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
				dir = (std::atan(gyArr[i*data.width + j] / gxArr[i*data.width + j])) * 180 / ImageUtil::pi;
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


ImageUtil::IMGDATA houghLine(ImageUtil::ImageData data,int max)
{
	const int len = std::sqrt(data.width * data.width + data.height * data.height);
	std::vector<Hough> sigma;
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			if (data[i][j] == 0)
				continue;

			if (j == 0)
				continue;

			const double r = std::sqrt(i*i + j * j);
			const double s = std::acos(j / r);
			sigma.emplace_back(s, r, j, i);
		}
	}

	for (std::vector<Hough>::size_type i = 0; i < sigma.size() - 1; i++)
	{
		for (std::vector<Hough>::size_type j = i + 1; j < sigma.size(); j++)
		{
			if (sigma[i] == sigma[j])
			{
				sigma[i].addChild(sigma[j]);
				sigma.erase(sigma.begin() + j);
				j--;
			}
		}
	}

	for (Hough& h : sigma)
	{
		if(h.child.size() >= max - 1)
		{
			drawLine(data.pImg, h.x, h.y, h.child[h.child.size() - 1].x, h.child[h.child.size() - 1].y, data.width, 2);
		}
	}


	data.rgbquad[0].rgbGreen = 0;
	data.rgbquad[0].rgbBlue = 0;
	data.rgbquad[0].rgbRed = 0;

	data.rgbquad[1].rgbGreen = 255;
	data.rgbquad[1].rgbBlue = 255;
	data.rgbquad[1].rgbRed = 255;

	data.rgbquad[2].rgbGreen = 0;
	data.rgbquad[2].rgbBlue = 255;
	data.rgbquad[2].rgbRed = 0;

	data.fileHeader.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + sizeof(RGBQUAD) * 3;
	data.fileHeader.bfSize = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + sizeof(RGBQUAD) * 3 + data.infoHeader.biSizeImage;

	data.infoHeader.biClrUsed = 3;

	ImageUtil::outputImage(data ,"bitmap/l.bmp");
	//ImageUtil::outputImage(houghImg, 90, len, 2, 8, rgb, "bitmap/houghMap.bmp");


	return data;
}

void drawLine(BYTE* img, int x0, int y0, int x1, int y1, const int width, const int clr)
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

	const double k = (y1 - y0) / (x1 - x0);
	const double b = x0 * k - y0;
	for (int i = x0; i < x1; i++)
	{
		const double y = k * i + b;
		img[static_cast<int>(y * width + i)] = clr;
	}
}