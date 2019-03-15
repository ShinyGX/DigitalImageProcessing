#pragma once
#include <windows.h>
#include <string>

typedef struct ImageData
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	RGBQUAD rgbquad[256];
	BYTE * pImg;
	int length;
	int width, height;
}IMGDATA;

typedef struct GrayHistogram
{
	double gray[256] = { 0 };
	int pixelCount = 0;
	void normalize();

private:
	bool isNormalize = false;
}GRAYHISTOGRAM;




inline void GrayHistogram::normalize()
{
	if (isNormalize)
		return;

	for (auto& i : gray)
	{
		i = i / pixelCount;
	}

	isNormalize = true;
}

class ImageUtil
{
public:
	ImageUtil();
	~ImageUtil();

	static ImageData loadImage(const std::string& path);
	static void outputImage(ImageData data,int clrUsed, const std::string&path);
	static GRAYHISTOGRAM getHistogram(IMGDATA data);
	static void outputHistogram(IMGDATA data, const std::string& path);
};

