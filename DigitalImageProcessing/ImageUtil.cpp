#include "ImageUtil.h"
#include <fstream>
#include <iostream>


int clamp(const int c)
{
	if (c > 255)
		return 255;
	if (c < 0)
		return 0;
	return c;
}

ImageUtil::ImageUtil()
= default;


ImageUtil::~ImageUtil()
= default;

ImageData ImageUtil::loadImage(const std::string& path)
{
	std::ifstream ifstream;
	ifstream.open(path, std::ios::binary);
	if (!ifstream.is_open())
		return {};

	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	RGBQUAD rgbquad[256];

	ifstream.read(reinterpret_cast<char *>(&fileHeader), sizeof(BITMAPFILEHEADER));
	ifstream.read(reinterpret_cast<char *>(&infoHeader), sizeof(BITMAPINFOHEADER));
	ifstream.read(reinterpret_cast<char *>(&rgbquad), sizeof(RGBQUAD) * infoHeader.biClrUsed);

	BYTE *img = new BYTE[infoHeader.biSizeImage];
	ifstream.read(reinterpret_cast<char*>(img), infoHeader.biSizeImage);

	IMGDATA imgdate;
	imgdate.infoHeader = infoHeader;
	imgdate.fileHeader = fileHeader;
	for (int i = 0; i < 256; i++)
	{
		imgdate.rgbquad[i] = rgbquad[i];
	}

	BYTE *imgWithoutError = new BYTE[infoHeader.biWidth * infoHeader.biHeight];
	//int byteWidth = (infoHeader.biWidth * (infoHeader.biClrUsed / 8) + 3) / 4 * 4;
	int point = -1;
	for(int i = 0;i < infoHeader.biHeight;i++)
	{
		for(int j = 0;j < infoHeader.biWidth;j++)
		{
			imgWithoutError[i * infoHeader.biWidth + j] = img[++point];
		}

		while (point % 4 != 0)
			point++;
	}

	delete[] img;

	imgdate.pImg = imgWithoutError;
	imgdate.length = infoHeader.biSizeImage;
	imgdate.width = infoHeader.biWidth;
	imgdate.height = infoHeader.biHeight;

	
	ifstream.close();
	return imgdate;
}

void ImageUtil::outputImage(ImageData data, const int clrUsed, const std::string& path)
{
	std::ofstream out;
	out.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!out.is_open())
		return;	

	BYTE *img = new BYTE[data.length];
	int point = -1;
	for(int i = 0;i < data.height;i++)
	{
		for(int j = 0;j < data.width;j++)
		{
			img[++point] = data.pImg[i * data.width + j];
		}

		while (point % 4 != 0)
			img[++point] = 0;
	}

	std::cout << "output " << path << "...." << std::endl;
	out.write(reinterpret_cast<char *>(&data.fileHeader), sizeof(BITMAPFILEHEADER));
	out.write(reinterpret_cast<char *>(&data.infoHeader), sizeof(BITMAPINFOHEADER));
	out.write(reinterpret_cast<char *>(&data.rgbquad), clrUsed * sizeof(RGBQUAD));
	out.write(reinterpret_cast<char *>(img), data.length);
	out.close();

	
}

GRAYHISTOGRAM ImageUtil::getHistogram(const IMGDATA data)
{
	GRAYHISTOGRAM grayhistogram;
	int point = 0;
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			grayhistogram.gray[data.pImg[point++]]++;
		}

		while (point % 4 != 0)
			point++;
	}

	grayhistogram.pixelCount = data.width * data.height;
	return grayhistogram;
}

void ImageUtil::outputHistogram(const IMGDATA data, const std::string& path)
{
	IMGDATA newData = data;
	GRAYHISTOGRAM histogram = ImageUtil::getHistogram(data);

	// newData.fileHeader.bfType = 0x4d42;
	// newData.fileHeader.bfReserved1 = 0;
	// newData.fileHeader.bfReserved2 = 0;
	newData.fileHeader.bfSize = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2 + 256 * 256;
	newData.fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2;
	//
	   // newData.infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	   // newData.infoHeader.biPlanes = 1;
	newData.infoHeader.biBitCount = 8;
	newData.infoHeader.biClrUsed = 2;
	//newData.infoHeader.biCompression = BI_RGB;
	newData.infoHeader.biSizeImage = 256 * 256;
	newData.infoHeader.biHeight = 256;
	newData.infoHeader.biWidth = 256;
	// newData.infoHeader.biClrImportant = data.infoHeader.biClrImportant;
	// newData.infoHeader.biXPelsPerMeter = data.infoHeader.biXPelsPerMeter;
	// newData.infoHeader.biYPelsPerMeter = data.infoHeader.biYPelsPerMeter;

	newData.pImg = new BYTE[256 * 256];

	histogram.normalize();

	RGBQUAD white;
	white.rgbReserved = 0;
	white.rgbRed = 255;
	white.rgbBlue = 255;
	white.rgbGreen = 255;

	RGBQUAD black;
	black.rgbReserved = 0;
	black.rgbRed = 0;
	black.rgbBlue = 0;
	black.rgbGreen = 0;

	newData.rgbquad[0] = black;
	newData.rgbquad[1] = white;

	for (int i = 0; i < 256; i++)
	{
		int length = histogram.gray[i] * 255 * 20;
		if (length > 255)
			length = 255;
		for (int j = 0; j < length; j++)
		{
			newData.pImg[j * 256 + i] = 1;
		}
	}

	newData.length = 256 * 256;
	newData.width = 256;
	newData.height = 256;

	outputImage(newData, 2, path);
}