

#include <string>
#include <iostream>
#include "../DigitalImageProcessing/ImageUtil.h"
#include <valarray>
#include <vector>

void thresholdByGive(const ImageUtil::ImageData&);
void thresholdByIterate(const ImageUtil::ImageData& data);
void otsu(const ImageUtil::ImageData& data);
double otsuP(int k, const ImageUtil::GrayHistogram& histogram);
double otsuM(int k, const ImageUtil::GrayHistogram& histogram);
double otsuVariance(int k, double mG, const ImageUtil::GrayHistogram& histogram);

int main()
{
	std::string path;
	std::cin >> path;

	const ImageUtil::IMGDATA img = ImageUtil::loadImageToGray(path);
	ImageUtil::outputHistogram(img, "bitmap/histogram.bmp");
	//thresholdByGive(img);
	thresholdByIterate(img);
	otsu(img);
	return 0;
}

void otsu(const ImageUtil::ImageData& data)
{
	ImageUtil::GrayHistogram histogram = ImageUtil::getHistogram(data);
	histogram.normalize();

	const int len = 255;

	const double mG = otsuM(len, histogram);
	double delta[len];
	for (int i = 0; i < len; i++)
	{
		delta[i] = otsuVariance(i, mG, histogram);
	}

	double max = -1;
	for (double i : delta)
	{
		if(i > max)
		{
			max = i;
		}
	}

	std::vector<int> maxList;
	for(int i = 0;i < len;i++)
	{
		if (delta[i] == max)
			maxList.push_back(i);
	}

	int k = 0;
	for (int i : maxList)
	{
		k += i;
	}

	k /= maxList.size();

	ImageUtil::ImageData img = data;
	BYTE *imgData = new BYTE[data.width * data.height];
	for (int i = 0; i < data.width * data.height; i++)
	{
		imgData[i] = data.pImg[i] > k ? 1 : 0;
	}

	std::string name("bitmap/threshold_by_otsu");
	name.append("_")
		.append(std::to_string(k))
		.append(".bmp");

	img.pImg = imgData;
	ImageUtil::outputBlackWhiteImage(img, name);
}

double otsuVariance(const int k, const double mG,const ImageUtil::GrayHistogram& histogram)
{
	return std::pow(mG*otsuP(k, histogram) - otsuM(k, histogram), 2) / (otsuP(k, histogram) * (1 - otsuP(k, histogram)));
}

double otsuM(const int k,const ImageUtil::GrayHistogram& histogram)
{
	double result = 0;
	for(int i = 0;i < k;i++)
	{
		result += i * histogram.gray[i];
	}

	return result;
}

double otsuP(const int k,const ImageUtil::GrayHistogram& histogram)
{
	double result = 0;
	for(int i = 0;i < k;i++)
	{
		result += histogram.gray[i];
	}

	return result;
}

void thresholdByIterate(const ImageUtil::ImageData& data)
{
	ImageUtil::GrayHistogram histogram = ImageUtil::getHistogram(data);
	histogram.normalize();
	double t0 = 0,t1 = 128;

	while (std::abs(t0 - t1) > 10)
	{
		double a0 = 0,n0 = 0, a1 = 0,n1 = 0;
		for (int i = 0; i < t1; i++)
		{
			a0 += histogram.gray[i] * i;
		}
		for (int i = 0; i < t1; i++)
		{
			n0 += histogram.gray[i];
		}

		for (int i = t1; i < 255; i++)
		{
			a1 += histogram.gray[i] * i;
		}
		for (int i = t1; i < 255; i++)
		{
			n1 += histogram.gray[i];
		}

		t0 = t1;
		t1 = (a0 / n0 + a1 / n1) * 0.5;
	}


	ImageUtil::ImageData img = data;
	BYTE *imgData = new BYTE[data.width * data.height];
	for (int i = 0; i < data.width * data.height; i++)
	{
		imgData[i] = data.pImg[i] > t1 ? 1 : 0;
	}

	std::string name("bitmap/threshold_by_iterate");
	name.append("_")
		.append(std::to_string(t1))
		.append(".bmp");


	img.pImg = imgData;
	ImageUtil::outputBlackWhiteImage(img, name);

	delete[] imgData;
}


void thresholdByGive(const ImageUtil::ImageData& data)
{
	int t = 0;
	std::cout << "ãÐÖµ : ";
	std::cin >> t;

	ImageUtil::ImageData img = data;
	BYTE *imgData = new BYTE[data.width * data.height];
	for (int i = 0; i < data.width * data.height; i++)
	{
		imgData[i] = data.pImg[i] > t ? 1 : 0;
	}

	img.pImg = imgData;
	ImageUtil::outputBlackWhiteImage(img, "bitmap/threshold_by_give.bmp");

	delete[] imgData;
}
