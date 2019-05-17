#include "Bmp.h"
#include <iostream>
#include <algorithm>
#include <queue>


using namespace ImageUtil;

ImageUtil::IMGDATA ImageUtil::balance(const ImageUtil::GRAYHISTOGRAM histogram, const ImageUtil::IMGDATA data)
{
	const ImageUtil::IMGDATA newData = data;
	//newData.pImg = new BYTE[newData.length]{ 0 };
	for (int i = 0; i < data.length; i++)
	{
		newData.pImg[i] = static_cast<int>(ImageUtil::calculate(data.pImg[i], histogram) * 255 + 0.5);
	}
	return newData;
}

ImageUtil::IMGDATA ImageUtil::balance2(const ImageUtil::GRAYHISTOGRAM histogram, const ImageUtil::IMGDATA data)
{
	//histogram.normalize();
	double result[256]{ 0 };
	result[0] = histogram.gray[0];
	for (int i = 1; i < 256; i++)
	{
		result[i] = result[i - 1] + histogram.gray[i];
	}

	const ImageUtil::IMGDATA newData = data;
	//newData.pImg = new BYTE[newData.length]{ 0 };
	for (int i = 0; i < data.length; i++)
	{
		newData.pImg[i] = clamp(result[data.pImg[i]] * 255 + 0.5);
	}

	return newData;
}

double ImageUtil::calculate(int index, const ImageUtil::GRAYHISTOGRAM histogram)
{
	double result = 0;
	for (int i = 0; i < index + 1; i++)
	{
		result += histogram.gray[i];
	}
	if (result > 1)
		result = 1;
	return result;
}


void ImageUtil::colorful(ImageUtil::IMGDATA data, const std::string& path)
{
	IMGDATA newData = data;
	newData.infoHeader.biBitCount = 24;
	newData.infoHeader.biClrUsed = 0;
	newData.infoHeader.biSizeImage = ((data.width * 3 + 3) / 4 * 4) * data.height;
	newData.fileHeader.bfOffBits = 54;
	newData.fileHeader.bfSize = 54 + newData.infoHeader.biSizeImage;

	BYTE * newImg = new BYTE[data.width * data.height * 3];
	int point = -1;
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			//bgr
			newImg[++point] = getBlud(data.pImg[i*data.width + j]);
			newImg[++point] = getGreen(data.pImg[i*data.width + j]);
			newImg[++point] = getRed(data.pImg[i*data.width + j]);
		}
	}

	newData.pImg = newImg;
	outputImage(newData, 0, path);
}

int ImageUtil::getGreen(int g)
{
	if (g <= 130)
		return clamp((255 / 130) * g);
	else
		return clamp((-2.04 * g) + 520.2);
}

int ImageUtil::getBlud(int g)
{
	return clamp((-255 / 130) * g + 255);
}

int ImageUtil::getRed(int g)
{
	return clamp(2.04 * g - 265.2);
}



IMGDATA ImageUtil::laplaceOstu2(IMGDATA data)
{
	BYTE * newData = new BYTE[data.length];

	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			int up, down, left, right;
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
				1 * data.pImg[up * data.width + left] + 1 * data.pImg[up * data.width + j] + 1 * data.pImg[up * data.width + right] +
				1 * data.pImg[i * data.width + left] + -8 * data.pImg[i * data.width + j] + 1 * data.pImg[i * data.width + right] +
				1 * data.pImg[down * data.width + left] + 1 * data.pImg[down * data.width + j] + 1 * data.pImg[down * data.width + right]);
		}

	}

	IMGDATA imgData = data;

	imgData.pImg = newData;
	return imgData;
}

IMGDATA ImageUtil::mid(IMGDATA data)
{
	BYTE *newData = new BYTE[data.length];
	for (int i = 1; i < data.height - 1; i++)
	{
		for (int j = 1; j < data.width - 1; j++)
		{
			int arr[9] = {
				data.pImg[(i - 1) * data.width + j - 1],data.pImg[(i - 1) * data.width + j],data.pImg[(i - 1)*data.width + j + 1],
				data.pImg[(i)* data.width + j - 1],data.pImg[(i)* data.width + j],data.pImg[(i)*data.width + j + 1],
				data.pImg[(i + 1) * data.width + j - 1],data.pImg[(i + 1) * data.width + j],data.pImg[(i + 1)*data.width + j + 1] };
			newData[i * data.width + j] = getMid(arr);
			//delete[] arr;
		}
	}


	IMGDATA newImg = data;
	newImg.pImg = newData;
	return newImg;
}



int ImageUtil::getMid(int arr[9])
{
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (arr[j] > arr[j + 1])
			{
				const int temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
		}
	}
	return arr[4];
}

IMGDATA ImageUtil::advenage(IMGDATA data)
{
	BYTE * newData = new BYTE[data.length];

	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			int up, down, left, right;
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

			newData[i * data.width + j] = ImageUtil::clamp(
				(data.pImg[up * data.width + left] + 2 * data.pImg[up * data.width + j] + data.pImg[up * data.width + right] +
					2 * data.pImg[i * data.width + left] + 4 * data.pImg[i * data.width + j] + 2 * data.pImg[i * data.width + right] +
					data.pImg[down * data.width + left] + 2 * data.pImg[down * data.width + j] + data.pImg[down * data.width + right]) / 16);
		}

	}

	delete[] data.pImg;
	data.pImg = newData;
	return data;
}


IMGDATA ImageUtil::rotate(IMGDATA data)
{
	int rotateAngle;
	std::cout << "旋转的角度" << std::endl;
	std::cin >> rotateAngle;

	int k = data.infoHeader.biBitCount / 8;

	BYTE *newData = new BYTE[data.width * data.height * k];
	for (int i = 0; i < data.width * data.height * k; i++)
	{
		newData[i] = 0;
	}



	//弧度制的角度
	double angle = 1.0 * rotateAngle * PI / 180;
	int pixelPoint = -k;
	int midY = static_cast<float>(data.height) / 2, midX = static_cast<float>(data.width) / 2;


	ImageUtil::Math::Matrix3x3d mat({
		std::cos(angle),-std::sin(angle),0,
		std::sin(angle),std::cos(angle), 0,
		0,              0,               1 });

	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			int aftX = j - midX;
			int aftY = i - midY;

			ImageUtil::Math::Matrix3x1d xyMat({ static_cast<double>(aftX),static_cast<double>(aftY),1 });
			auto result = mat * xyMat;
			double originX = result[0][0] + midX; /* (aftX * std::cos(angle) + aftY * std::sin(angle)) + midX; */
			double originY = result[1][0] + midY;  /* (-aftX * std::sin(angle) + aftY * std::cos(angle)) + midY;*/

			pixelPoint += k;
			if (originX < 0 || originX >= data.width || originY < 0 || originY >= data.height)
				continue;

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

			//兼容灰度图，24位图，32位图
			for (int biCount = 0; biCount < k; biCount++)
			{
				newData[pixelPoint + biCount] = ImageUtil::clamp(
					data.pImg[originPixelY * data.width * k + originPixelX * k + biCount] * (1 - distanceOriginPixelX) * (1 - distanceOriginPixelY) +
					data.pImg[originPixelY * data.width * k + originPixelXNext * k + biCount] * (distanceOriginPixelX) * (1 - distanceOriginPixelY) +
					data.pImg[originPixelYNext * data.width * k + originPixelX * k + biCount] * (distanceOriginPixelY) * (1 - distanceOriginPixelX) +
					data.pImg[originPixelYNext * data.width * k + originPixelXNext * k + biCount] * distanceOriginPixelY * distanceOriginPixelX);

			}


			// pixelPoint += k;
			// if (originX < 0 || originX >= data.width || originY < 0 || originY >= data.height)
			// 	continue;
   //
			// for(int biCount = 0;biCount < k;biCount++)
			// {
			// 	newData[pixelPoint + biCount] = data.pImg[static_cast<int>(originY) * data.width * k + static_cast<int>(originX) * k + biCount];
			// }

		}
	}

	IMGDATA img = data;
	img.pImg = newData;

	return img;
}


IMGDATA ImageUtil::translate(IMGDATA data)
{
	int xTrans, yTrans;
	std::cout << "x轴的位移" << std::endl;
	std::cin >> xTrans;
	std::cout << "y轴的位移" << std::endl;
	std::cin >> yTrans;

	int k = data.infoHeader.biBitCount / 8;

	ImageUtil::Math::Matrix3x3i mat({
		1,0,-xTrans,
		0,1,-yTrans,
		0,0,1 });

	ImageUtil::Math::Matrix3x1i xyMat({ 0,0,0 });

	BYTE *newData = new BYTE[data.width * data.height * k];
	for (int i = 0; i < data.width * data.height * k; i++)
	{
		newData[i] = 0;
	}

	int point = -k;
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			xyMat.reset({ j,i,1 });
			auto result = mat * xyMat;
			int x = result[0][0]; /*j + xTrans;*/
			int y = result[1][0]; /*i + yTrans;*/

			point += k;
			if (x < 0 || x >= data.width || y < 0 || y >= data.height)
				continue;

			for (int biCount = 0; biCount < k; biCount++)
			{
				newData[point + biCount] = data.pImg[y * data.width * k + x * k + biCount];
			}
		}
	}

	IMGDATA img = data;
	img.pImg = newData;
	return img;
}


ImageUtil::IMGDATA ImageUtil::scale(ImageUtil::IMGDATA data)
{
	float xScale, yScale;
	std::cout << "x轴的缩放" << std::endl;
	std::cin >> xScale;
	std::cout << "y轴的缩放" << std::endl;
	std::cin >> yScale;

	const int k = data.infoHeader.biBitCount / 8;

	IMGDATA newImg = data;
	newImg.width = xScale * data.width;
	newImg.height = yScale * data.height;

	newImg.infoHeader.biWidth = newImg.width;
	newImg.infoHeader.biHeight = newImg.height;

	const int byteWidth = (newImg.width * k + 3) / 4 * 4;
	newImg.infoHeader.biSizeImage = byteWidth * newImg.height;
	newImg.fileHeader.bfSize = newImg.infoHeader.biSizeImage + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + newImg.infoHeader.biClrUsed * sizeof(RGBQUAD);

	ImageUtil::Math::Matrix3x3d mat({
		1 / xScale, 0,       0,
		0,        1 / yScale,0,
		0,        0,       1 });
	ImageUtil::Math::Matrix3x1d xyMat({ 0,0,0 });

	//兼容灰度图，24位图，32位图
	BYTE *newData = new BYTE[newImg.width * k * newImg.height];
	int pixelPoint = -k;
	for (int i = 0; i < newImg.height; i++)
	{
		for (int j = 0; j < newImg.width; j++)
		{
			xyMat.reset({ static_cast<double>(j),static_cast<double>(i),1 });
			auto result = mat * xyMat;
			const double originX = result[0][0];  /*static_cast<double>(j) / xScale;*/
			const double originY = result[1][0];  /*static_cast<double>(i) / yScale;*/

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

			//兼容灰度图，24位图，32位图
			pixelPoint += k;
			for (int biCount = 0; biCount < k; biCount++)
			{
				newData[pixelPoint + biCount] = ImageUtil::clamp(
					data.pImg[originPixelY * data.width * k + originPixelX * k + biCount] * (1 - distanceOriginPixelX) * (1 - distanceOriginPixelY) +
					data.pImg[originPixelY * data.width * k + originPixelXNext * k + biCount] * (distanceOriginPixelX) * (1 - distanceOriginPixelY) +
					data.pImg[originPixelYNext * data.width * k + originPixelX * k + biCount] * (distanceOriginPixelY) * (1 - distanceOriginPixelX) +
					data.pImg[originPixelYNext * data.width * k + originPixelXNext * k + biCount] * distanceOriginPixelY * distanceOriginPixelX);

			}

		}
	}

	newImg.pImg = newData;
	return newImg;
}


IMGDATA ImageUtil::mirror(IMGDATA data)
{
	ImageUtil::Math::Matrix3x3i mat({
								-1, 0, static_cast<int>(data.width),
								 0, 1, 0,
								 0, 0, 1 });
	std::cout << mat;

	const int k = data.infoHeader.biBitCount / 8;
	BYTE *newData = new BYTE[data.length];
	int point = -k;
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			ImageUtil::Math::Matrix3x1i xyMat({ j,i,1 });
			ImageUtil::Math::Matrix3x1i result = mat * xyMat;
			point += k;
			for (int b = 0; b < k; b++)
			{
				//newData[point + b] = data.pImg[i * data.width * k + (data.width * k - 1 - j - (k - 1 - b))];
				newData[point + b] = data.pImg[result[1][0] * data.width * k + result[0][0] * k + b];
			}

		}
	}

	IMGDATA newImg = data;
	newImg.pImg = newData;
	return newImg;
}

void ImageUtil::laplaceOstu(const ImageUtil::IMGDATA& data)
{
	BYTE * newData = new BYTE[data.length];

	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			int up, down, left, right;
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

			newData[i * data.width + j] = ImageUtil::clamp(
				1 * data.pImg[up * data.width + left] + 1 * data.pImg[up * data.width + j] + 1 * data.pImg[up * data.width + right] +
				1 * data.pImg[i * data.width + left] + -8 * data.pImg[i * data.width + j] + 1 * data.pImg[i * data.width + right] +
				1 * data.pImg[down * data.width + left] + 1 * data.pImg[down * data.width + j] + 1 * data.pImg[down * data.width + right]);
		}

	}


	std::vector<int> his(data.width * data.height);
	for (int i = 0; i < data.width * data.height; i++)
	{
		his[i] = data.pImg[i];
	}

	std::sort(his.begin(), his.end());
	double t = his[his.size() / 2];

	for (int i = 0; i < data.width * data.height; i++)
	{
		newData[i] = newData[i] > t ? 1 : 0;
	}


	ImageUtil::GRAYHISTOGRAM grayhistogram;
	grayhistogram.pixelCount = 0;
	int point = -1;
	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{

			if (newData[++point] == 1)
			{
				grayhistogram.gray[data.pImg[point]]++;
				grayhistogram.pixelCount++;
			}

		}
	}
	grayhistogram.normalize();
	ImageUtil::outputHistogram(grayhistogram, "bitmap/laplace_histogram.bmp");
	const int len = 255;

	const double mG = otsuM(len, grayhistogram);
	double delta[len];
	for (int i = 0; i < len; i++)
	{
		delta[i] = otsuVariance(i, mG, grayhistogram);
	}

	double max = -1;
	for (double i : delta)
	{
		if (i > max)
		{
			max = i;
		}
	}

	std::vector<int> maxList;
	for (int i = 0; i < len; i++)
	{
		if (delta[i] == max)
			maxList.push_back(i);
	}

	int k = 0;
	for (int i : maxList)
	{
		k += i;
	}

	k /= maxList.size();

	ImageUtil::ImageData img = data;
	BYTE *mg = new BYTE[data.width * data.height];
	for (int i = 0; i < data.width * data.height; i++)
	{
		mg[i] = data.pImg[i] > k ? 1 : 0;
	}

	std::string name("bitmap/threshold_by_laplace_otsu");
	name.append("_")
		.append(std::to_string(k))
		.append(".bmp");

	img.pImg = mg;
	ImageUtil::outputBlackWhiteImage(img, name);
	ImageUtil::outputHistogram(ImageUtil::getHistogram(data), "bitmap/histogram/threshold_by_laplace_otsu_h.bmp", k);
	delete[] newData;
	delete[] mg;

}

void ImageUtil::otsu(const ImageUtil::ImageData& data)
{
	ImageUtil::GrayHistogram histogram = ImageUtil::getHistogram(data);
	histogram.normalize();

	const int len = 255;

	const double mG = otsuM(len, histogram);
	double delta[len];
	for (int i = 0; i < len; i++)
	{

		delta[i] = otsuVariance(i, mG, histogram);
	}

	double max = -1;
	for (double i : delta)
	{
		if (i > max)
		{
			max = i;
		}
	}

	std::vector<int> maxList;
	for (int i = 0; i < len; i++)
	{
		if (delta[i] == max)
			maxList.push_back(i);
	}

	int k = 0;
	for (int i : maxList)
	{
		k += i;
	}

	k /= maxList.size();

	if (k == -1)
		return;

	ImageUtil::ImageData img = data;
	BYTE *imgData = new BYTE[data.width * data.height];
	for (int i = 0; i < data.width * data.height; i++)
	{
		imgData[i] = data.pImg[i] > k ? 1 : 0;
	}

	std::string name("bitmap/threshold_by_otsu");
	name.append("_")
		.append(std::to_string(k))
		.append(".bmp");

	img.pImg = imgData;
	ImageUtil::outputBlackWhiteImage(img, name);
	ImageUtil::outputHistogram(histogram, "bitmap/histogram/threshold_by_otsu_h.bmp", k);
}

double ImageUtil::otsuVariance(const int k, const double mG, const ImageUtil::GrayHistogram& histogram)
{
	const double p = otsuP(k, histogram);
	if (p == 0)
		return -1;
	return std::pow(mG*otsuP(k, histogram) - otsuM(k, histogram), 2) / (p* (1 - p));
}

double ImageUtil::otsuM(const int k, const ImageUtil::GrayHistogram& histogram)
{
	double result = 0;
	for (int i = 0; i < k; i++)
	{
		result += i * histogram.gray[i];
	}

	return result;
}

double ImageUtil::otsuP(const int k, const ImageUtil::GrayHistogram& histogram)
{
	double result = 0;
	for (int i = 0; i < k; i++)
	{
		result += histogram.gray[i];
	}

	return result;
}

void ImageUtil::thresholdByIterate(const ImageUtil::ImageData& data)
{
	ImageUtil::GrayHistogram histogram = ImageUtil::getHistogram(data);
	histogram.normalize();


	std::vector<int> his(data.width * data.height);
	for (int i = 0; i < data.width * data.height; i++)
	{
		his[i] = data.pImg[i];
	}

	std::sort(his.begin(), his.end());
	double t0 = 0, t1 = his[his.size() / 2];

	while (std::abs(t0 - t1) > 1)
	{
		double a0 = 0, n0 = 0, a1 = 0, n1 = 0;
		for (int i = 0; i < t1; i++)
		{
			a0 += histogram.gray[i] * i;
		}
		for (int i = 0; i < t1; i++)
		{
			n0 += histogram.gray[i];
		}

		for (int i = t1; i < 255; i++)
		{
			a1 += histogram.gray[i] * i;
		}
		for (int i = t1; i < 255; i++)
		{
			n1 += histogram.gray[i];
		}

		t0 = t1;
		t1 = (a0 / n0 + a1 / n1) * 0.5;
	}


	ImageUtil::ImageData img = data;
	BYTE *imgData = new BYTE[data.width * data.height];
	for (int i = 0; i < data.width * data.height; i++)
	{
		imgData[i] = data.pImg[i] > t1 ? 1 : 0;
	}

	std::string name("bitmap/threshold_by_iterate");
	name.append("_")
		.append(std::to_string(t1))
		.append(".bmp");


	img.pImg = imgData;
	ImageUtil::outputBlackWhiteImage(img, name);
	ImageUtil::outputHistogram(histogram, "bitmap/histogram/threshold_by_iter_h.bmp", t1);

	delete[] imgData;
}

void ImageUtil::thresholdByGive(const ImageUtil::ImageData& data)
{
	int t = 0;
	std::cout << "阈值 : ";
	std::cin >> t;

	ImageUtil::ImageData img = data;
	BYTE *imgData = new BYTE[data.width * data.height];
	for (int i = 0; i < data.width * data.height; i++)
	{
		imgData[i] = data.pImg[i] > t ? 1 : 0;
	}

	img.pImg = imgData;
	ImageUtil::outputBlackWhiteImage(img, "bitmap/threshold_by_give.bmp");

	delete[] imgData;
}


ImageUtil::IMGDATA ImageUtil::reginGrowWithoutSeed(ImageUtil::IMGDATA data, int threadhold)
{
	Region origin;
	origin.wBeginIndex = 0;
	origin.wEndIndex = data.width;
	origin.hBeginIndex = 0;
	origin.hEndIndex = data.height;

	std::vector<Region> qTList;
	std::queue<Region> queRegion;

	Region * rList = new Region[4];

	queRegion.push(origin);
	int start = 0, end = 0;
	while (start <= end)
	{
		const Region curRegion = queRegion.front();
		queRegion.pop();
		//if (!curRegion.Q)
		Region * rQList = splitRegion(curRegion, rList);

		for (int i = 0; i < 4; i++)
		{
			if (rQList != nullptr)
			{
				if (getAver(data, rList[i]) > threadhold)
				{
					queRegion.push(rList[i]);
					end++;
				}
				else
					qTList.push_back(rList[i]);

			}


		}

		start++;
	}

	delete[] rList;

	BYTE *byte = new BYTE[data.width * data.height];
	memset(byte, 0, data.width * data.height);
	for (auto& r : qTList)
	{
		for (int i = r.hBeginIndex; i < r.hEndIndex; i++)
		{
			for (int j = r.wBeginIndex; j < r.wEndIndex; j++)
			{
				byte[i * data.width + j] = 1;
			}
		}
	}

	data.pImg = byte;
	return data;
}

Region* ImageUtil::splitRegion(const Region& r, Region *alloc)
{

	if (r.wEndIndex - r.wBeginIndex <= 1 || r.hEndIndex - r.hBeginIndex <= 1)
		return nullptr;

	alloc[0].wBeginIndex = r.wBeginIndex;
	alloc[0].wEndIndex = r.wBeginIndex + (r.wEndIndex - r.wBeginIndex) / 2;
	alloc[0].hBeginIndex = r.hBeginIndex;
	alloc[0].hEndIndex = r.hBeginIndex + (r.hEndIndex - r.hBeginIndex) / 2;

	alloc[1].wBeginIndex = r.wBeginIndex + (r.wEndIndex - r.wBeginIndex) / 2;
	alloc[1].wEndIndex = r.wEndIndex;
	alloc[1].hBeginIndex = r.hBeginIndex;
	alloc[1].hEndIndex = r.hBeginIndex + (r.hEndIndex - r.hBeginIndex) / 2;

	alloc[2].wBeginIndex = r.wBeginIndex;
	alloc[2].wEndIndex = r.wBeginIndex + (r.wEndIndex - r.wBeginIndex) / 2;
	alloc[2].hBeginIndex = r.hBeginIndex + (r.hEndIndex - r.hBeginIndex) / 2;
	alloc[2].hEndIndex = r.hEndIndex;

	alloc[3].wBeginIndex = r.wBeginIndex + (r.wEndIndex - r.wBeginIndex) / 2;
	alloc[3].wEndIndex = r.wEndIndex;
	alloc[3].hBeginIndex = r.hBeginIndex + (r.hEndIndex - r.hBeginIndex) / 2;
	alloc[3].hEndIndex = r.hEndIndex;

	return alloc;
}

double ImageUtil::getAver(const ImageUtil::IMGDATA& data, const Region& r)
{
	int count = 0, result = 0;
	for (int i = r.hBeginIndex; i < r.hEndIndex; i++)
	{
		for (int j = r.wBeginIndex; j < r.wEndIndex; j++)
		{
			result += data[i][j];
			count++;
		}
	}

	return static_cast<double>(result) / count;
}

void ImageUtil::regionGrowWithSeed(const ImageUtil::IMGDATA& data, BYTE * unRegion, int *growQueX, int *growQueY, int threshold, int seedX, int seedY, int color)
{
	int nDx[8] = { 0, 0,1,-1, 1,1,-1,-1 };
	int nDy[8] = { 1,-1,0, 0,-1,1, 1,-1 };



	int start = 0, end = 0;
	growQueX[end] = seedX;
	growQueY[end] = seedY;

	while (start <= end)
	{
		const int currX = growQueX[start];
		const int currY = growQueY[start];

		for (int k = 0; k < 8; k++)
		{
			const int xx = currX + nDx[k];
			const int yy = currY + nDy[k];

			if (xx < data.width && xx >= 0 &&
				yy < data.height && yy >= 0 &&
				unRegion[yy * data.width + xx] == 0 &&
				std::abs(data.pImg[yy * data.width + xx] - data.pImg[currY * data.width + currX]) < threshold)
			{
				end++;
				growQueX[end] = xx;
				growQueY[end] = yy;

				unRegion[yy * data.width + xx] = color;

			}
		}

		start++;
	}

}