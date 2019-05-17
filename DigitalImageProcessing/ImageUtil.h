#pragma once
#include <windows.h>
#include <string>
#include "Math.h"
#include <vector>

namespace ImageUtil
{
	typedef unsigned int ImageSize;

	typedef struct Pixel {
		Math::Vector<unsigned int, 2> vec2{};
		int mask = 0;
		byte pix = 0;

		Pixel() = default;
		Pixel(const unsigned int x, const unsigned int y, const byte p) :vec2({ y,x }), pix(p) {}

		unsigned int getX();
		unsigned int getY();
		void setX(unsigned int x);
		void setY(unsigned int y);
		bool operator<(Pixel& other) const;
		bool operator==(Pixel& other);

	} PVec2;

	typedef struct ImageColor {
		BYTE r, g, b, a;
	}RGBA;
	typedef struct ImageData
	{
		BITMAPFILEHEADER fileHeader;
		BITMAPINFOHEADER infoHeader;
		RGBQUAD rgbquad[256];
		BYTE * pImg;
		ImageSize length;
		ImageSize width, height;
		//std::shared_ptr<BYTE*> pImg;
		ImageData& operator+(ImageData& d0);
		ImageData& operator*(float k);

		BYTE* operator[](int i) const;

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

	ImageData loadImage(const std::string& path);
	ImageData loadImageToGray(const std::string& path);
	void outputImage(ImageData data, int clrUsed, const std::string&path);
	void outputImage(const ImageData& data, const std::string&path);
	void outputImage(BYTE *data, int width, int height, int clrUse, int bitCount, RGBQUAD *rgbquad,const std::string& path);
	void outputBlackWhiteImage(ImageData data, const std::string&path);
	GRAYHISTOGRAM getHistogram(const IMGDATA& data);
	void outputHistogram(const IMGDATA& data, const std::string& path);
	void outputHistogram(const GrayHistogram& histogram, const std::string&path);
	void outputHistogram(const GrayHistogram& histogram, const std::string&path, int mark);

	ImageData toTwoValueImage(ImageData &data,byte t = 0);
	
	double toRadian(double angle);

	int clamp(int c);

	template<typename T,int size = sizeof T>
	void initWithZero(T* dst, const unsigned int len)
	{
		memset(dst, 0, len * size);
	}
}

