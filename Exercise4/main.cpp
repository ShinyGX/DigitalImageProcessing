

#include "../DigitalImageProcessing/ImageUtil.h"
#include <iostream>

using ImageUtil::IMGDATA;

IMGDATA mirror(IMGDATA data);
IMGDATA scale(IMGDATA data);

int main()
{
	std::string path;
	std::cin >> path;
	IMGDATA data = ImageUtil::loadImage(path);
	ImageUtil::outputImage(mirror(data), 0, "bitmap/mirror.bmp");
	ImageUtil::outputImage(scale(data), 0, "bitmap/scale.bmp");
	return 0;
}

ImageUtil::IMGDATA scale(ImageUtil::IMGDATA data)
{
	float xScale, yScale;
	std::cout << "x÷·µƒÀı∑≈" << std::endl;
	std::cin >> xScale;
	std::cout << "y÷·µƒÀı∑≈" << std::endl;
	std::cin >> yScale;

	const int k = data.infoHeader.biBitCount / 8;

	//ºÊ»›ª“∂»Õº£¨24ŒªÕº£¨32ŒªÕº
	BYTE *newData = new BYTE[xScale * data.width * k * yScale * data.height];
	for (int i = 0; i < yScale * data.height; i++)
	{
		for (int j = 0; j < xScale * data.width ; j++)
		{
			const double originX = static_cast<double>(j) / xScale;
			const double originY = static_cast<double>(i) / yScale;

			const int originPixelX = originX;
			const int originPixelY = originY;

			const double distanceOriginPixelX = originX - originPixelX;
			const double distanceOriginPixelY = originY - originPixelY;

			int originPixelXNext = originPixelX + 1;
			int originPixelYNext = originPixelY + 1;

			if (originPixelXNext >= data.width)
				originPixelXNext = data.width - 1;
			if (originPixelYNext >= data.height)
				originPixelYNext = data.height - 1;

			//ºÊ»›ª“∂»Õº£¨24ŒªÕº£¨32ŒªÕº
			const int pixelPoint = i * xScale * data.width * k + j * k;
			for (int biCount = 0; biCount < k; biCount++)
			{
				newData[pixelPoint + biCount] = ImageUtil::clamp(
					data.pImg[originPixelY * data.width * k + originPixelX * k + biCount] * (1 - distanceOriginPixelX) * (1 - distanceOriginPixelY) +
					data.pImg[originPixelY * data.width * k + originPixelXNext * k + biCount] * (distanceOriginPixelX) * (1 - distanceOriginPixelY) +
					data.pImg[originPixelYNext * data.width * k + originPixelX * k + biCount] * (distanceOriginPixelY) * (1 - distanceOriginPixelX) +
					data.pImg[originPixelYNext * data.width * k + originPixelXNext * k + biCount] * distanceOriginPixelY * distanceOriginPixelX);

				// newData[pixelPoint + 1] = data.pImg[originPixelY * data.width * k + originPixelX * k + 1] * (1 - distanceOriginPixelX) * (1 - distanceOriginPixelY) +
				// 	data.pImg[originPixelY * data.width * k + originPixelXNext * k + 1] * (distanceOriginPixelX) * (1 - distanceOriginPixelY) +
				// 	data.pImg[originPixelYNext * data.width * k + originPixelX * k + 1] * (distanceOriginPixelY * (1 - distanceOriginPixelX)) +
				// 	data.pImg[originPixelYNext * data.width * k + originPixelXNext * k + 1] * distanceOriginPixelY * distanceOriginPixelX;
    //
				// newData[pixelPoint + 2] = data.pImg[originPixelY * data.width * k + originPixelX * k + 2] * (1 - distanceOriginPixelX) * (1 - distanceOriginPixelY) +
				// 	data.pImg[originPixelY * data.width * k + originPixelXNext * k + 2] * (distanceOriginPixelX) * (1 - distanceOriginPixelY) +
				// 	data.pImg[originPixelYNext * data.width * k + originPixelX * k + 2] * (distanceOriginPixelY * (1 - distanceOriginPixelX)) +
				// 	data.pImg[originPixelYNext * data.width * k + originPixelXNext * k + 2] * distanceOriginPixelY * distanceOriginPixelX;
			}

		}
	}

	IMGDATA newImg = data;
	newImg.pImg = newData;
	newImg.width = xScale * data.width;
	newImg.height = yScale * data.height;

	newImg.infoHeader.biWidth = newImg.width;
	newImg.infoHeader.biHeight = newImg.height;

	const int byteWidth = (newImg.width * k + 3) / 4 * 4;
	newImg.infoHeader.biSizeImage = byteWidth * newImg.height;

	newImg.fileHeader.bfSize = newImg.infoHeader.biSizeImage + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
	return newImg;
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



