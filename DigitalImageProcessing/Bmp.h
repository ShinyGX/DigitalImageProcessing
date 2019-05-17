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

	inline void writeImg(BITMAPFILEHEADER *header, BITMAPINFOHEADER *info, BYTE *rgb, int size, FILE * pf)
	{
		fwrite(header, sizeof(BITMAPFILEHEADER), 1, pf);
		fwrite(info, sizeof(BITMAPINFOHEADER), 1, pf);
		fwrite(rgb, sizeof(BYTE), size, pf);
	}

	inline BYTE* toByte(RGB *rgb, int width, int height, int biSize, RGBTAG tag)
	{
		BYTE *byte = new BYTE[biSize];
		int point = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				switch (tag)
				{
				case R:
					byte[point++] = 0;
					byte[point++] = 0;
					byte[point++] = rgb[i * width + j].r;
					break;
				case G:
					byte[point++] = 0;
					byte[point++] = rgb[i * width + j].g;
					byte[point++] = 0;
					break;
				case B:
					byte[point++] = rgb[i * width + j].b;
					byte[point++] = 0;
					byte[point++] = 0;
					break;
				case ALL:
					byte[point++] = rgb[i * width + j].b;
					byte[point++] = rgb[i * width + j].g;
					byte[point++] = rgb[i * width + j].r;
					break;
				}

			}

			while (point % 4 != 0)
				byte[point++] = 0;
		}

		return byte;
	}


	inline void bitmapTo3SignalColorBitmap(std::string path)
	{
		BITMAPFILEHEADER fileHeader;
		BITMAPINFOHEADER infoHeader;

		FILE * pfin = fopen(path.c_str(), "rb");
		FILE * pfoutR = fopen("bitmap/bmp/r.bmp", "wb");
		FILE * pfoutG = fopen("bitmap/bmp/g.bmp", "wb");
		FILE * pfoutB = fopen("bitmap/bmp/b.bmp", "wb");


		fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pfin);
		fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pfin);

		const int height = infoHeader.biHeight;
		int width = infoHeader.biWidth;
		if (infoHeader.biBitCount > 1)
		{
			int byteWidth = (width * infoHeader.biBitCount / 8 + 3) / 4 * 4;
			int size = byteWidth * height;

			BYTE *img = new BYTE[size];
			RGB *imgRGB = new RGB[width * height];

			fseek(pfin, fileHeader.bfOffBits, 0);
			fread(img, sizeof(BYTE), size, pfin);
			int point = 0;
			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < width; j++)
				{
					imgRGB[i * width + j].b = img[point++];
					imgRGB[i * width + j].g = img[point++];
					imgRGB[i * width + j].r = img[point++];
				}

				while (point % 4 != 0)
					point++;
			}

			writeImg(&fileHeader, &infoHeader,
				toByte(imgRGB, width, height, infoHeader.biSizeImage, RGBTAG::R),
				infoHeader.biSizeImage,
				pfoutR);

			writeImg(&fileHeader, &infoHeader,
				toByte(imgRGB, width, height, infoHeader.biSizeImage, RGBTAG::G),
				infoHeader.biSizeImage,
				pfoutG);

			writeImg(&fileHeader, &infoHeader,
				toByte(imgRGB, width, height, infoHeader.biSizeImage, RGBTAG::B),
				infoHeader.biSizeImage,
				pfoutB);

		}

		fclose(pfin);
		fclose(pfoutR);
		fclose(pfoutG);
		fclose(pfoutB);
	}

	inline void bitmap2Gray(std::string path)
	{
		BITMAPFILEHEADER fileHeader;
		BITMAPINFOHEADER infoHeader;

		FILE * pfin = fopen(path.c_str(), "rb");
		FILE * pfout = fopen("bitmap/bmp/gray.bmp", "wb");
		fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pfin);
		fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pfin);

		int height = infoHeader.biHeight, width = infoHeader.biWidth;
		if (infoHeader.biBitCount == 24)
		{
			int byteWidth = (width * infoHeader.biBitCount / 8 + 3) / 4 * 4;
			int size = byteWidth * height;

			BYTE *img = new BYTE[size];
			RGB *imgRGB = new RGB[width * height];

			fseek(pfin, fileHeader.bfOffBits, 0);
			fread(img, sizeof(BYTE), size, pfin);
			int point = 0;
			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < width; j++)
				{
					imgRGB[i * width + j].b = img[point++];
					imgRGB[i * width + j].g = img[point++];
					imgRGB[i * width + j].r = img[point++];
				}

				while (point % 4 != 0)
					point++;
			}

			infoHeader.biBitCount = 8;
			infoHeader.biClrUsed = 256;

			RGBQUAD rgbQuad[256];
			for (int i = 0; i < 256; i++)
			{
				rgbQuad[i].rgbRed = i;
				rgbQuad[i].rgbGreen = i;
				rgbQuad[i].rgbBlue = i;
				rgbQuad[i].rgbReserved = 0;

			}

			fileHeader.bfOffBits = 54 + 4 * 256;

			int byteLine = (width * infoHeader.biBitCount / 8 + 3) / 4 * 4;
			BYTE *newIMG = new BYTE[byteLine * height];
			infoHeader.biSizeImage = byteLine * height;
			fileHeader.bfSize = 54 + byteLine * height + 4 * 256;


			point = 0;
			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < width; j++)
				{
					newIMG[point++] = imgRGB[i * width + j].b * 0.114 +
						imgRGB[i * width + j].g * 0.587 + imgRGB[i * width + j].r * 0.299;

				}

				while (point % 4 != 0)
					newIMG[point++] = 0;
			}

			fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pfout);
			fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pfout);
			fwrite(&rgbQuad, 4 * 256, 1, pfout);
			fwrite(newIMG, sizeof(BYTE), byteLine * height, pfout);

		}

		fclose(pfin);
		fclose(pfout);
	}

	inline void gray2Anticolor()
	{
		BITMAPFILEHEADER fileHeader;
		BITMAPINFOHEADER infoHeader;

		FILE * pfin = fopen("bitmap/bmp/gray.bmp", "rb");
		FILE * pfout = fopen("bitmap/bmp/anti_color.bmp", "wb");
		fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pfin);
		fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pfin);

		int height = infoHeader.biHeight, width = infoHeader.biWidth;
		if (infoHeader.biBitCount == 8)
		{
			int byteWidth = (width * infoHeader.biBitCount / 8 + 3) / 4 * 4;
			int size = byteWidth * height;

			BYTE *img = new BYTE[size];
			BYTE *imgAnticolor = new BYTE[size];

			RGBQUAD rgbquad[256];

			fread(rgbquad, sizeof(RGBQUAD), 256, pfin);
			fread(img, sizeof(BYTE), size, pfin);
			int point1 = 0, point = 0;
			for (int i = 0; i < height; i++)
			{
				for (int j = 0; j < width; j++)
				{
					imgAnticolor[point++] = std::abs(img[point1++] - static_cast<byte>(255));
				}

				while (point % 4 != 0)
				{
					imgAnticolor[point++] = 0;
					point1++;
				}

			}

			fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pfout);
			fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pfout);
			fwrite(&rgbquad, 4 * 256, 1, pfout);
			fwrite(imgAnticolor, sizeof(BYTE), size, pfout);

		}

		fclose(pfin);
		fclose(pfout);
	}




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
	void otsu(const ImageUtil::ImageData& data);
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
