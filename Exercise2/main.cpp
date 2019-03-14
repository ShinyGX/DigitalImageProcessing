#include <windows.h>
#include <string>
#include <fstream>
#include <iostream>

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
	long double gray[256] = {0};
	int pixelCount = 0;
	void normalize();
	void draw();

private :
	bool isNormalize = false;
}GRAYHISTOGRAM;

IMGDATA loadImage(const std::string& path);
GRAYHISTOGRAM getHistogram(IMGDATA data);
IMGDATA balance(GRAYHISTOGRAM histogram, IMGDATA data);
IMGDATA balance2(GRAYHISTOGRAM histogram, IMGDATA data);
float calculate(int index, GRAYHISTOGRAM histogram);
void output(IMGDATA data, int clrUsed, const std::string& path);
void outputHistogram(const IMGDATA data, const std::string& path);

int main()
{
	std::string path;
	std::cin >> path;
	const IMGDATA data = loadImage(path);
	GRAYHISTOGRAM grayhistogram = getHistogram(data);
	grayhistogram.normalize();
	//grayhistogram.draw();
	outputHistogram(data, "bitmap/_grayHistogram.bmp");
	const IMGDATA newData = balance(grayhistogram, data);
	//histogram.draw();
	outputHistogram(newData, "bitmap/_afterBalance.bmp");
	output(newData,256, "bitmap/balance.bmp");
	//system("pause");
	return 0;
}

void outputHistogram(const IMGDATA data,const std::string& path)
{
	IMGDATA newData = data;
	GRAYHISTOGRAM histogram = getHistogram(data);

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

	newData.pImg = new BYTE[256 * 256]{ 0 };

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

	for(int i = 0 ;i < 256;i++)
	{		
		int length = histogram.gray[i] * 255 * 25;
		if (length > 255)
			length = 255;
		for(int j = 0;j < length;j++)
		{
			newData.pImg[j * 256 + i] = 1;
		}
	}

	newData.length = 256 * 256;
	newData.width = 256;
	newData.height = 256;

	output(newData, 2, path);
}

void output(IMGDATA data,int clrUsed,const std::string& path)
{
	std::ofstream out;
	out.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
	if (!out.is_open())
		return;

	std::cout << "output " << path << "...." << std::endl;
	out.write(reinterpret_cast<char *>(&data.fileHeader), sizeof(BITMAPFILEHEADER));
	out.write(reinterpret_cast<char *>(&data.infoHeader), sizeof(BITMAPINFOHEADER));
	out.write(reinterpret_cast<char *>(&data.rgbquad), clrUsed * sizeof(RGBQUAD));
	out.write(reinterpret_cast<char *>(data.pImg), data.length);

	out.close();
}

IMGDATA balance(const GRAYHISTOGRAM histogram,const IMGDATA data)
{
	const IMGDATA newData = data;
	//newData.pImg = new BYTE[newData.length]{ 0 };
	for(int i = 0;i < data.length;i++)
	{
		newData.pImg[i] = static_cast<int>(calculate(data.pImg[i], histogram) * 255 + 0.5);
	}
	return newData;
}

IMGDATA balance2(const GRAYHISTOGRAM histogram, const IMGDATA data)
{
	//histogram.normalize();
	float result[256]{0};
	result[0] = histogram.gray[0];
	for(int i = 1;i < 256;i++)
	{
		result[i] = result[i - 1] + histogram.gray[i];
	}

	const IMGDATA newData = data;
	//newData.pImg = new BYTE[newData.length]{ 0 };
	for(int i = 0;i < data.length;i++)
	{
		newData.pImg[i] = result[data.pImg[i]] * 255 + 0.5;
	}

	return newData;
}

float calculate(int index, const GRAYHISTOGRAM histogram)
{
	float result = 0;
	for(int i = 0;i < index + 1;i++)
	{
		result += histogram.gray[i];
	}
	if (result > 1)
		result = 1;
	return result;
}

GRAYHISTOGRAM getHistogram(const IMGDATA data)
{
	GRAYHISTOGRAM grayhistogram;
	int point = 0;
	for (int i = 0; i < data.height; i++)
	{
		for(int j = 0;j < data.width;j++)
		{
			grayhistogram.gray[data.pImg[point++]]++;
		}

		while (point % 4 != 0)
			point++;
	}

	grayhistogram.pixelCount = data.width * data.width;
	return grayhistogram;
}


void GrayHistogram::normalize()
{
	if (isNormalize)
		return;

	for (auto& i : gray)
	{
		i = i / pixelCount;
	}

	isNormalize = true;
}

void GrayHistogram::draw()
{
	for(int i = 0;i < 256;i++)
	{
		std::cout << i << " : ";
		if(!isNormalize)
			for (int j = 0; j < gray[i] / 100; j++)
			{
				std::cout << '|';
			}
		else
			for (int j = 0; j < gray[i] * (static_cast<float>(pixelCount) / 100); j++)
			{
				std::cout << '|';
			}

		std::cout << std::endl;
	}
}



IMGDATA loadImage(const std::string& path)
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
	for(int i = 0;i < 256;i++)
	{
		imgdate.rgbquad[i] = rgbquad[i];
	}
	imgdate.pImg = img;
	imgdate.length = infoHeader.biSizeImage;
	imgdate.width = infoHeader.biWidth;
	imgdate.height = infoHeader.biHeight;

	ifstream.close();
	return imgdate;
}
