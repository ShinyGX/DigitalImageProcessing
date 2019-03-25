#include "ImageUtil.h"
#include <fstream>
#include <iostream>
#include <cassert>


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
	//int byteWidth = (infoHeader.biWidth * (infoHeader.biClrUsed / 8) + 3) / 4 * 4;
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
			for (int i = 0; i < data.height; i++)
			{
				for (int j = 0; j < data.width; j++)
				{
					rgba[i * data.width + j].b = data.pImg[point++];
					rgba[i * data.width + j].g = data.pImg[point++];
					rgba[i * data.width + j].r = data.pImg[point++];
				}
			}
			data.infoHeader.biBitCount = 8;
			data.infoHeader.biClrUsed = 256;

			data.fileHeader.bfOffBits = 54 + 4 * 256;
			int byteLine = (data.width * data.infoHeader.biBitCount / 8 + 3) / 4 * 4;
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
			for (int i = 0; i < data.height; i++)
			{
				for (int j = 0; j < data.width; j++)
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
			for (int i = 0; i < data.height; i++)
			{
				for (int j = 0; j < data.width; j++)
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
			int byteLine = (data.width * data.infoHeader.biBitCount / 8 + 3) / 4 * 4;
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
			for (int i = 0; i < data.height; i++)
			{
				for (int j = 0; j < data.width; j++)
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
	if (!out.is_open())
		return;	

	BYTE *img = new BYTE[data.infoHeader.biSizeImage];
	int byteWidth = (data.infoHeader.biWidth * data.infoHeader.biBitCount / 8);
	int point = -1;
	for(int i = 0;i < data.height;i++)
	{
		for(int j = 0;j < byteWidth;j++)
		{
			img[++point] = data.pImg[i * byteWidth + j];
		}

		while ((point + 1) % 4 != 0)
			img[++point] = 0;
	}

	std::cout << "output " << path << "...." << std::endl;
	out.write(reinterpret_cast<char *>(&data.fileHeader), sizeof(BITMAPFILEHEADER));
	out.write(reinterpret_cast<char *>(&data.infoHeader), sizeof(BITMAPINFOHEADER));
	out.write(reinterpret_cast<char *>(&data.rgbquad), clrUsed * sizeof(RGBQUAD));
	out.write(reinterpret_cast<char *>(img), data.infoHeader.biSizeImage);
	out.close();

	delete[] img;
}

ImageUtil::GRAYHISTOGRAM ImageUtil::getHistogram(const IMGDATA data)
{
	GRAYHISTOGRAM grayhistogram;
	int point = 0;
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			grayhistogram.gray[data.pImg[point++]]++;
		}
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
	for(int i = 0;i < 256 * 256;i++)
	{
		newData.pImg[i] = 0;
	}

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



ImageUtil::ImageData & ImageUtil::ImageData::operator+(ImageData& d0)
{
	for(int i = 0;i < height;i++)
	{
		for(int j = 0;j < width;j++)
		{
			pImg[i * width + j] = ImageUtil::clamp(pImg[i * width + j] + d0.pImg[i * width + j]);
		}
	}

	return *this;

}

ImageUtil::ImageData & ImageUtil::ImageData::operator*(const float k)
{
	for(int i =0;i < height;i++)
	{
		for(int j = 0;j < width;j++)
		{
			pImg[i * width + j] = ImageUtil::clamp(pImg[i * width + j] * k);
		}
	}

	return *this;
}
