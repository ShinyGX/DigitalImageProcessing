#include <windows.h>
#include <string>
#include <iostream>
#include "../DigitalImageProcessing/ImageUtil.h"

void advenage(IMGDATA data);

int main()
{
	std::string path;
	std::cin >> path;
	const IMGDATA data = ImageUtil::loadImage(path);
	ImageUtil::outputHistogram(data, "bitmap/_before.bmp");
	advenage(data);
	return 0;
}


void advenage(IMGDATA data)
{
	BYTE * newData = new BYTE[data.length];
	for(int i = 1;i < data.height - 1;i++)
	{
		for(int j =1;j < data.width - 1;j++)
		{
			newData[i * data.width + j] =
				(data.pImg[(i - 1) * data.width + (j - 1)] +2 * data.pImg[(i - 1) * data.width + j] + data.pImg[(i - 1) * data.width + (j + 1)] +
					2 *data.pImg[(i)* data.width + (j - 1)] + 4 *data.pImg[(i)* data.width + j] + 2 *data.pImg[(i)* data.width + (j + 1)] +
					data.pImg[(i + 1) * data.width + (j + 1)] + 2 *data.pImg[(i - 1) * data.width + j] + data.pImg[(i + 1) * data.width + (j + 1)]) / 16;
		}
	}

	delete[] data.pImg;
	data.pImg = newData;
	ImageUtil::outputImage(data, 256, "bitmap/a.bmp");
	ImageUtil::outputHistogram(data, "bitmap/_after.bmp");
}

