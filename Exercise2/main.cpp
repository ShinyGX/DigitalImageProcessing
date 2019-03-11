#include <windows.h>
#include <string>
#include <fstream>
#include <iostream>

typedef struct ImageData
{
	BYTE * img;
	int length;
	int width, height;
}IMGDATA;

typedef struct GrayHistogram
{
	float gray[256] = {0};
	int pixelCount = 0;
	void normalize();
	void draw();

private :
	bool isNormalize = false;
}GRAYHISTOGRAM;

IMGDATA loadImage(const std::string& path);
GRAYHISTOGRAM getHistogram(IMGDATA data);

int main()
{	
	IMGDATA data = loadImage("bitmap/gray.bmp");
	GRAYHISTOGRAM grayhistogram = getHistogram(data);
	grayhistogram.normalize();
	grayhistogram.draw();
	system("pause");
	return 0;
}



GRAYHISTOGRAM getHistogram(const IMGDATA data)
{
	GRAYHISTOGRAM grayhistogram;
	int point = 0;
	for (int i = 0; i < data.height; i++)
	{
		for(int j = 0;j < data.width;j++)
		{
			grayhistogram.gray[data.img[point++]]++;
		}

		while (point % 4 != 0)
			point++;
	}

	grayhistogram.pixelCount = data.width * data.width;
	return grayhistogram;
}


void GrayHistogram::normalize()
{
	for (float& i : gray)
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

	ifstream.read(reinterpret_cast<char*>(&fileHeader), sizeof(BITMAPFILEHEADER));
	ifstream.read(reinterpret_cast<char *>(&infoHeader), sizeof(BITMAPINFOHEADER));
	
	ifstream.read(reinterpret_cast<char *>(&rgbquad), sizeof(RGBQUAD) * infoHeader.biClrUsed);

	BYTE *img = new BYTE[infoHeader.biSizeImage];
	ifstream.read(reinterpret_cast<char*>(img), infoHeader.biSizeImage);

	IMGDATA imgdate;
	imgdate.img = img;
	imgdate.length = infoHeader.biSizeImage;
	imgdate.width = infoHeader.biWidth;
	imgdate.height = infoHeader.biHeight;

	ifstream.close();
	return imgdate;
}
