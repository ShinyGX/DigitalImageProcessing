#include "ImageUtil.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include "Math.h"
#include "ProcessBar.h"


int ImageUtil::clamp(const int c)
{
	if (c > 255)
		return 255;
	if (c < 0)
		return 0;
	return c;
}


ImageUtil::ImageData ImageUtil::loadImage(const std::string& path)
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

	BYTE *imgWithoutError = new BYTE[(infoHeader.biWidth * infoHeader.biBitCount / 8) * infoHeader.biHeight];


	int point = -1;
	for(int i = 0;i < infoHeader.biHeight;i++)
	{
		for(int j = 0;j < (infoHeader.biWidth * infoHeader.biBitCount / 8);j++)
		{
			imgWithoutError[i * (infoHeader.biWidth * infoHeader.biBitCount / 8) + j] = img[++point];
		}

		while ((point + 1) % 4 != 0)
			point++;
	}

	delete[] img;
	
	imgdate.pImg = imgWithoutError;
	imgdate.length = infoHeader.biWidth * infoHeader.biHeight * infoHeader.biBitCount / 8;
	imgdate.width = infoHeader.biWidth;
	imgdate.height = infoHeader.biHeight;

	
	ifstream.close();
	return imgdate;
}

ImageUtil::ImageData ImageUtil::loadImageToGray(const std::string & path)
{
	ImageData data = loadImage(path);
	if (data.infoHeader.biBitCount != 8)
	{
		RGBA *rgba = new RGBA[data.width * data.height];
		switch (static_cast<int>(data.infoHeader.biBitCount))
		{
		case 16:
			std::cout << "无法转换16位图(太麻烦，懒癌发作)" << std::endl;
			break;
		case 24:
		{
			int point = 0;
			for (ImageSize i = 0; i < data.height; i++)
			{
				for (ImageSize j = 0; j < data.width; j++)
				{
					rgba[i * data.width + j].b = data.pImg[point++];
					rgba[i * data.width + j].g = data.pImg[point++];
					rgba[i * data.width + j].r = data.pImg[point++];
				}
			}
			data.infoHeader.biBitCount = 8;
			data.infoHeader.biClrUsed = 256;

			data.fileHeader.bfOffBits = 54 + 4 * 256;
			const int byteLine = (data.width * data.infoHeader.biBitCount / 8 + 3) / 4 * 4;
			data.infoHeader.biSizeImage = byteLine * data.height;
			data.fileHeader.bfSize = 54 + byteLine * data.height + 4 * 256;

			for (int i = 0; i < 256; i++)
			{
				data.rgbquad[i].rgbRed = i;
				data.rgbquad[i].rgbGreen = i;
				data.rgbquad[i].rgbBlue = i;
				data.rgbquad[i].rgbReserved = 0;

			}

			BYTE * newData = new BYTE[data.width * data.height];
			point = 0;
			for (ImageSize i = 0; i < data.height; i++)
			{
				for (ImageSize j = 0; j < data.width; j++)
				{
					newData[point++] = rgba[i * data.width + j].r * 0.299 + rgba[i * data.width + j].g * 0.587 + rgba[i * data.width + j].b * 0.114;
				}
			}

			delete[] data.pImg;
			data.pImg = newData;
			break;
		}
		case 32:
		{
			int point = 0;
			for (ImageSize i = 0; i < data.height; i++)
			{
				for (ImageSize j = 0; j < data.width; j++)
				{
					rgba[i * data.width + j].b = data.pImg[point++];
					rgba[i * data.width + j].g = data.pImg[point++];
					rgba[i * data.width + j].r = data.pImg[point++];
					rgba[i * data.width + j].a = data.pImg[point++];
				}
			}
			data.infoHeader.biBitCount = 8;
			data.infoHeader.biClrUsed = 256;

			data.fileHeader.bfOffBits = 54 + 4 * 256;
			const int byteLine = (data.width * data.infoHeader.biBitCount / 8 + 3) / 4 * 4;
			data.infoHeader.biSizeImage = byteLine * data.height;
			data.fileHeader.bfSize = 54 + byteLine * data.height + 4 * 256;

			for (int i = 0; i < 256; i++)
			{
				data.rgbquad[i].rgbRed = i;
				data.rgbquad[i].rgbGreen = i;
				data.rgbquad[i].rgbBlue = i;
				data.rgbquad[i].rgbReserved = 0;

			}

			BYTE * newData = new BYTE[data.width * data.height];
			point = 0;
			for (ImageSize i = 0; i < data.height; i++)
			{
				for (ImageSize j = 0; j < data.width; j++)
				{
					newData[point++] = rgba[i * data.width + j].r * 0.299 + rgba[i * data.width + j].g * 0.587 + rgba[i * data.width + j].b * 0.114;
				}
			}

			delete[] data.pImg;
			data.pImg = newData;
			break;
		}
		default:
			break;
		}

		delete[] rgba;
	}
	

	return data;
}

void ImageUtil::outputImage(ImageData data, const int clrUsed, const std::string& path)
{
	std::ofstream out;
	out.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
	
	if (!out || !out.is_open())
		return;	

	BYTE *img = new BYTE[data.infoHeader.biSizeImage];
	const ImageSize byteWidth = (data.infoHeader.biWidth * data.infoHeader.biBitCount / 8);
	int point = -1;

	for(ImageSize i = 0;i < data.height;i++)
	{
		for(ImageSize j = 0;j < byteWidth;j++)
		{
			img[++point] = data.pImg[i * byteWidth + j];
		}

		while ((point + 1) % 4 != 0)
			img[++point] = 0;
	}

	std::cout << std::endl << "output " << path << "...." << std::endl;
	out.write(reinterpret_cast<char *>(&data.fileHeader), sizeof(BITMAPFILEHEADER));
	out.write(reinterpret_cast<char *>(&data.infoHeader), sizeof(BITMAPINFOHEADER));
	out.write(reinterpret_cast<char *>(&data.rgbquad), clrUsed * sizeof(RGBQUAD));
	out.write(reinterpret_cast<char *>(img), data.infoHeader.biSizeImage);
	out.close();

	delete[] img;
}

void ImageUtil::outputImage(const ImageData& data, const std::string& path)
{
	outputImage(data, data.infoHeader.biClrUsed, path);
}

void ImageUtil::outputImage(BYTE* data, const int width, const int height,int clrUse, const int bitCount,RGBQUAD *rgbquad, const std::string& path)
{
	if (clrUse > 256)
		clrUse = 256;
	if (clrUse < 0)
		clrUse = 0;

	const int byteWidth = (width * bitCount / 8 + 3) / 4 * 4;

	BITMAPFILEHEADER header;
	header.bfType = 0x4d42;
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	header.bfSize = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * clrUse + byteWidth * height * bitCount / 8;
	header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * clrUse;

	BITMAPINFOHEADER info;
	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;
	info.biBitCount = bitCount;
	info.biClrUsed = clrUse;
	info.biClrImportant = 0;
	info.biCompression = 0;
	info.biSizeImage = byteWidth * height * bitCount / 8;
	info.biXPelsPerMeter = 0;
	info.biYPelsPerMeter = 0;

	ImageData img{};
	img.fileHeader = header;
	img.infoHeader = info;

	img.width = width;
	img.height = height;

	for(int i = 0;i < clrUse;i++)
	{
		img.rgbquad[i] = rgbquad[i];
	}

	img.pImg = data;
	outputImage(img, path);
}

void ImageUtil::outputBlackWhiteImage(ImageData data, const std::string& path)
{
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

	data.fileHeader.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + sizeof(RGBQUAD) * 2;
	data.fileHeader.bfSize = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + sizeof(RGBQUAD) * 2 + data.infoHeader.biSizeImage;

	data.infoHeader.biClrUsed = 2;

	data.rgbquad[0] = black;
	data.rgbquad[1] = white;

	for(ImageSize i = 0;i < data.width *data.height;i++)
	{
		data.pImg[i] = data.pImg[i] > 0 ? 1 : 0;
	}

	outputImage(data, 2, path);
}

ImageUtil::GRAYHISTOGRAM ImageUtil::getHistogram(const IMGDATA& data)
{
	GRAYHISTOGRAM grayhistogram;
	int point = 0;
	for (ImageSize i = 0; i < data.height; i++)
	{
		for (ImageSize j = 0; j < data.width; j++)
		{
			grayhistogram.gray[data.pImg[point++]]++;
		}
	}

	grayhistogram.pixelCount = data.width * data.height;
	return grayhistogram;
}

void ImageUtil::outputHistogram(const IMGDATA& data, const std::string& path)
{
	outputHistogram(ImageUtil::getHistogram(data), path);
}

void ImageUtil::outputHistogram(const GrayHistogram& histogram, const std::string& path)
{
	outputHistogram(histogram, path, -1);
}

void ImageUtil::outputHistogram(const GrayHistogram& histogram, const std::string& path, const int mark)
{
	int clrUse = 2;
	if (mark >= 0 && mark <= 255)
		clrUse = 3;

	BITMAPFILEHEADER header;
	header.bfType = 0x4d42;
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	header.bfSize = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2 + 256 * 256;
	header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2;

	BITMAPINFOHEADER info;
	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = 256;
	info.biHeight = 256;
	info.biPlanes = 1;
	info.biBitCount = 8;
	info.biClrUsed = clrUse;
	info.biClrImportant = 0;
	info.biCompression = 0;
	info.biSizeImage = 256 * 256;
	info.biXPelsPerMeter = 0;
	info.biYPelsPerMeter = 0;

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

	RGBQUAD red;
	red.rgbReserved = 0;
	red.rgbRed = 255;
	red.rgbBlue = 0;
	red.rgbGreen = 0;

	

	BYTE *imgData = new BYTE[256 * 256];

	GrayHistogram grayHistogram = histogram;
	grayHistogram.normalize();

	double max = -1;
	for (double i : grayHistogram.gray)
	{
		if (i > max)
			max = i;
	}

	for (int i = 0; i < 256; i++)
	{
		int length = grayHistogram.gray[i] * 255 * (100 / (100 * max));
		if (length > 255)
			length = 255;
		for (int j = 0; j < length; j++)
		{
			imgData[j * 256 + i] = 1;
		}

		if (i == mark)
		{
			for (int j = 0; j < 256; j++)
			{
				imgData[j * 256 + i] = 2;
			}
		}

	}

	IMGDATA img;
	img.fileHeader = header;
	img.infoHeader = info;
	img.rgbquad[0] = black;
	img.rgbquad[1] = white;
	img.rgbquad[2] = red;	
	img.width = 256;
	img.height = 256;
	img.length = 256 * 256;
	img.pImg = imgData;
	outputImage(img, path);

	delete[] imgData;
}

ImageUtil::ImageData ImageUtil::toTwoValueImage(ImageData& data, const byte t)
{
	for (ImageSize i = 0; i < data.height; i++)
	{
		for (ImageSize j = 0; j < data.width; j++) {
			if (data[i][j] > t)
			{
				data[i][j] = 1;
			}
			else
			{
				data[i][j] = 0;
			}
		}
	}

	return data;
}

double ImageUtil::toRadian(const double angle)
{
	return angle * PI / 180;
}


unsigned ImageUtil::Pixel::getX()
{
	return vec2[1];
}

unsigned ImageUtil::Pixel::getY()
{
	return vec2[0];
}

void ImageUtil::Pixel::setX(const unsigned int x)
{
	vec2[1] = x;
}

void ImageUtil::Pixel::setY(const unsigned int y)
{
	vec2[0] = y;
}


bool ImageUtil::Pixel::operator<(Pixel& other) const
{
	return pix < other.pix;
}

bool ImageUtil::Pixel::operator==(Pixel& other)
{
	return vec2[0] == other.vec2[0] && vec2[1] == other.vec2[1];
}


ImageUtil::ImageData & ImageUtil::ImageData::operator+(ImageData& d0)
{
	for(ImageSize i = 0;i < height;i++)
	{
		for(ImageSize j = 0;j < width;j++)
		{
			pImg[i * width + j] = ImageUtil::clamp(pImg[i * width + j] + d0.pImg[i * width + j]);
		}
	}

	return *this;

}

ImageUtil::ImageData & ImageUtil::ImageData::operator*(const float k)
{
	for(ImageSize i =0;i < height;i++)
	{
		for(ImageSize j = 0;j < width;j++)
		{
			pImg[i * width + j] = ImageUtil::clamp(pImg[i * width + j] * k);
		}
	}

	return *this;
}

BYTE* ImageUtil::ImageData::operator[](const int i) const
{
	return &pImg[i * width];
}

