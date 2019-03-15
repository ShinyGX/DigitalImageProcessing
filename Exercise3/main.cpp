#include <windows.h>
#include <string>
#include <iostream>
#include "../DigitalImageProcessing/ImageUtil.h"

IMGDATA advenage(IMGDATA data);

int main()
{
	std::string path;
	std::cin >> path;
	IMGDATA data = ImageUtil::loadImage(path);

	ImageUtil::outputHistogram(data, "bitmap/before_advence_histogram.bmp");
	int i = 0;
	while (++i < 4)
		data = advenage(data);
	ImageUtil::outputHistogram(data, "bitmap/after_advence_histogram.bmp");
	ImageUtil::outputImage(data, 256, "bitmap/advenage.bmp");
	return 0;
}


IMGDATA advenage(IMGDATA data)
{
	BYTE * newData = new BYTE[data.length];

	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			int up, down,left,right;
			if (i == 0)
				up = 0;
			else
				up = i - 1;

			if (i == data.height - 1)
				down = data.height - 1;
			else
				down = i + 1;

			if (j == 0)
				left = 0;
			else
				left = j - 1;

			if (j == data.width - 1)
				right = data.width - 1;
			else
				right = j + 1;

			newData[i * data.width + j] = clamp(
				(data.pImg[up * data.width + left] + 2 * data.pImg[up * data.width + j] + data.pImg[up * data.width + right] +
					2 * data.pImg[i * data.width + left] + 4 * data.pImg[i * data.width + j] + 2 * data.pImg[i * data.width + right] +
					data.pImg[down * data.width + left] + 2 * data.pImg[down * data.width + j] + data.pImg[down * data.width + right]) / 16);
 		}

	}

	delete[] data.pImg;
	data.pImg = newData;
	return data;
}

