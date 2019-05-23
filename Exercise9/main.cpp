

#include <string>
#include "../DigitalImageProcessing/ImageUtil.h"
#include <iostream>
#include "../DigitalImageProcessing/Math.h"
#include <queue>
#include <valarray>
#include "../DigitalImageProcessing/Edge.h"
#include "../DigitalImageProcessing/Bmp.h"
#include "../DigitalImageProcessing/ProcessBar.h"


ImageUtil::IMGDATA hough(ImageUtil::ImageData data, double deltaSigma);

int main()
{

	std::string path;
	std::cin >> path;
	auto img = ImageUtil::loadImageToGray(path);
	//auto c = ImageUtil::EdgeDetection::canny(img, 40,80);
	ImageUtil::outputImage(hough(img, 0.25),"bitmap/hough.bmp");
	return 0;
}



ImageUtil::IMGDATA hough(ImageUtil::ImageData data, const double deltaSigma)
{
	typedef ImageUtil::ImageSize uint;

	const int r = std::sqrt(data.width * data.width + data.height * data.height);
	const int d = 2 * r;
	const int sigma = 181 / deltaSigma;

	uint *houghSpace = new uint[d  * sigma];
	memset(houghSpace, 0, d * sigma * sizeof(uint));
	

	ImageUtil::ImageData cannyImg = ImageUtil::EdgeDetection::canny(data,40,80);
	
	ImageUtil::outputBlackWhiteImage(cannyImg, "bitmap/canny.bmp");
	//ImageUtil::toTwoValueImage(cannyImg);

	ImageUtil::progressBar.reset(data.height * data.width, "生成HoughSpace");

	for (uint i = 0; i < data.height; i++)
	{
		for (uint j = 0; j < data.width; j++)
		{
			if (cannyImg[i][j] > 0)
			{
				double s = 0;
				while (true) {
					const int p = j * std::cos(ImageUtil::toRadian(static_cast<double>(s))) +
						i * std::sin(ImageUtil::toRadian(static_cast<double>(s))) + r;
					houghSpace[p * sigma + static_cast<int>(s / deltaSigma)]++;


					
					s += deltaSigma;
					if (s > 180)
						break;
				}
			}
			++ImageUtil::progressBar;
		}
	}

	uint max = 0;
	for(int i= 0;i < r * sigma ;i++)
	{
		if (houghSpace[i] > max)
			max = houghSpace[i];
	}


	ImageUtil::progressBar.reset(data.height * data.width, "检测直线....");
	for(uint i = 0;i < data.height;i++)
	{
		for(uint j = 0;j < data.width;j++)
		{
			double s = 0;
			while(true)
			{
				const int p = j * std::cos(ImageUtil::toRadian(static_cast<double>(s))) 
				+ i * std::sin(ImageUtil::toRadian(static_cast<double>(s))) + r;
				if (houghSpace[p * sigma + static_cast<int>(s / deltaSigma)] > max * 0.85)
				{
					data[i][j] = static_cast<byte>(255);
				}
				
				s += deltaSigma;
				if (s > 180)
					break;
			}

			++ImageUtil::progressBar;
		}
	}

	BYTE* houghSpaceImg = new BYTE[d * sigma];
	
	for (int i = 0; i < d*sigma; i++)
	{
		houghSpaceImg[i] = ImageUtil::clamp(static_cast<double>(houghSpace[i]) / max * 255);
	}

	ImageUtil::outputImage(houghSpaceImg, sigma, d, 256, 8, data.rgbquad, "bitmap/houghSpace.bmp");

	data.rgbquad[255].rgbBlue = 0;
	data.rgbquad[255].rgbGreen = 0;

	delete[] houghSpace;
	delete[] cannyImg.pImg;
	delete[] houghSpaceImg;
	return data;
}

