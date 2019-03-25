

#include "../DigitalImageProcessing/ImageUtil.h"
#include <iostream>

using ImageUtil::IMGDATA;

IMGDATA mirror(IMGDATA data);

int main()
{
	std::string path;
	std::cin >> path;
	IMGDATA data = ImageUtil::loadImage(path);
	ImageUtil::outputImage(mirror(data), 0, "bitmap/mirror.bmp");

	return 0;
}

IMGDATA mirror(IMGDATA data)
{
	BYTE *newData = new BYTE[data.length];
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width * 3; j += 3)
		{
			newData[i * data.width * 3 + j] = data.pImg[i*data.width * 3 + (data.width * 3 - 1 - j - 2)];
			newData[i * data.width * 3 + j + 1] = data.pImg[i*data.width * 3 + (data.width * 3 - 1 - j - 1)];
			newData[i * data.width * 3 + j + 2] = data.pImg[i*data.width * 3 + (data.width * 3 - 1 - j)];
		}
	}

	IMGDATA newImg = data;
	newImg.pImg = newData;
	return newImg;
}



