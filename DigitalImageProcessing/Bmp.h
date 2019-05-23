#pragma once


#include <windows.h>
#include <cstdio>
#include <valarray>
#include <string>
#include "ImageUtil.h"

namespace ImageUtil
{

	typedef struct
	{
		BYTE b;
		BYTE g;
		BYTE r;
	}RGB;
	enum RGBTAG
	{
		R, G, B, ALL
	};

	void writeImg(BITMAPFILEHEADER *header, BITMAPINFOHEADER *info, BYTE *rgb, int size, FILE * pf);
	BYTE* toByte(RGB *rgb, int width, int height, int biSize, RGBTAG tag);
	void bitmapTo3SignalColorBitmap(const std::string& path);
	void bitmap2Gray(const std::string& path);
	void gray2Anticolor();


	IMGDATA balance(GRAYHISTOGRAM histogram, IMGDATA data);
	IMGDATA balance2(GRAYHISTOGRAM histogram, IMGDATA data);
	double calculate(int index, GRAYHISTOGRAM histogram);
	IMGDATA advenage(IMGDATA data);
	IMGDATA mid(IMGDATA data);
	IMGDATA laplaceOstu2(IMGDATA data);

	void colorful(IMGDATA data, const std::string& path);
	int getMid(int arr[9]);

	int getRed(int g);
	int getBlud(int g);
	int getGreen(int g);


	IMGDATA mirror(IMGDATA data);
	IMGDATA scale(IMGDATA data);
	IMGDATA rotate(IMGDATA data);
	IMGDATA translate(IMGDATA data);

	void thresholdByGive(const ImageUtil::ImageData&);
	void thresholdByIterate(const ImageUtil::ImageData& data);
	ImageData otsu(const ImageUtil::ImageData& data);
	double otsuP(int k, const ImageUtil::GrayHistogram& histogram);
	double otsuM(int k, const ImageUtil::GrayHistogram& histogram);
	double otsuVariance(int k, double mG, const ImageUtil::GrayHistogram& histogram);
	void laplaceOstu(const ImageUtil::IMGDATA& data);

	struct Region
	{
		int wBeginIndex = 0, wEndIndex = 0, hBeginIndex = 0, hEndIndex = 0;
		bool Q = false;
	};
	void regionGrowWithSeed(const ImageUtil::IMGDATA& data, BYTE * unRegion, int *growQueX, int *growQueY, int threshold, int seedX, int seedY, int color);
	Region* splitRegion(const Region& r, Region *alloc);
	double getAver(const ImageUtil::IMGDATA& data, const Region& r);
	ImageUtil::IMGDATA reginGrowWithoutSeed(ImageUtil::IMGDATA data, int threadhold);
}
