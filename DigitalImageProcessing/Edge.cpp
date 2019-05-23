#include "Edge.h"
#include <queue>
#include "ProcessBar.h"


bool ImageUtil::EdgeDetection::WatershedPixel::allNeigbourIsWshed()
{
	for (auto p : neighbour)
	{
		if (p->mask != WSHED)
			return false;
	}

	return true;
}

void ImageUtil::EdgeDetection::WatershedPixel::addNeighbour(WatershedPixel* p)
{
	neighbour.push_back(p);
}

bool ImageUtil::EdgeDetection::WatershedPixel::operator<(WatershedPixel& other) const
{
	return pix < other.pix;
}

ImageUtil::EdgeDetection::WatershedStructure::WatershedStructure(byte* pixels, const int width, const int height)
{
	watershed.reserve(width * height);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			watershed.push_back(new WatershedPixel(x, y, pixels[x + y * width]));
		}
	}

	/* 计算各个像素点的8连通邻域像素 */
	for (int y = 0; y < height; ++y) {
		const int offset = y * width;
		int topOffset = offset + width;
		int bottomOffset = offset - width;

		for (int x = 0; x < width; ++x) {
			const int currentindex = x + y * width;	// 当前像素点索引
			WatershedPixel* currentPixel = watershed.at(currentindex);

			if (x - 1 >= 0) {
				currentPixel->addNeighbour(watershed.at(currentindex - 1)); // 左邻域
				if (y - 1 >= 0)	 // 左下角
					currentPixel->addNeighbour(watershed.at(currentindex - 1 - width));
				if (y + 1 < height)	 // 左上角
					currentPixel->addNeighbour(watershed.at(currentindex - 1 + width));
			}

			if (x + 1 < width) {
				currentPixel->addNeighbour(watershed.at(currentindex + 1)); // 右邻域
				if (y - 1 >= 0)	 // 右下角
					currentPixel->addNeighbour(watershed.at(currentindex + 1 - width));
				if (y + 1 < height) // 右上角
					currentPixel->addNeighbour(watershed.at(currentindex + 1 + width));
			}

			if (y - 1 >= 0) // 下邻域
				currentPixel->addNeighbour(watershed.at(currentindex - width));

			if (y + 1 < height)	 // 上邻域
				currentPixel->addNeighbour(watershed.at(currentindex + width));
		}
	}

	/* 根据灰度值对结构体中的所有像素点从小到大进行排序 */
	std::sort(watershed.begin(), watershed.end());
}

ImageUtil::ImageSize ImageUtil::EdgeDetection::WatershedStructure::size() const
{
	return watershed.size();
}

ImageUtil::EdgeDetection::WatershedPixel* ImageUtil::EdgeDetection::WatershedStructure::at(const ImageSize index)
{
	return watershed.at(index);
}

ImageUtil::EdgeDetection::WatershedStructure::~WatershedStructure()
{
	while(!watershed.empty())
	{
		auto p = watershed.front();
		delete p;
		p = nullptr;

		watershed.pop_back();
	}
}

ImageUtil::IMGDATA ImageUtil::EdgeDetection::canny(ImageUtil::IMGDATA data, const int minVal, const int maxVal)
{
	progressBar.reset((data.height - 4) * (data.width - 4), "开始Gaussian滤波");
	double** gaus = getGaussianKernel(5, 0.01);
	for (unsigned int i = 2; i < data.height - 2; i++)
	{
		for (unsigned int j = 2; j < data.width - 2; j++)
		{
			int sum = 0;
			for (int x = -2; x <= 2; x++)
			{
				for (int y = -2; y <= 2; y++)
				{
					sum += data[i + x][j + y] * gaus[4 - (x + 2)][y + 2];
				}
			}
			data[i][j] = sum;
			++progressBar;
		}
	}

	BYTE *sobelImg = new BYTE[data.width * data.height];
	int *gxArr = new int[data.width * data.height];
	int *gyArr = new int[data.width * data.height];
	memset(sobelImg, 0, data.width * data.height);
	memset(gxArr, 0, data.width * data.height);
	memset(gyArr, 0, data.width * data.height);


	progressBar.reset((data.height - 2) * (data.width - 2), "进行Sobel算子计算边缘");

	for (ImageUtil::ImageSize i = 1; i < data.height - 1; i++)
	{
		for (ImageUtil::ImageSize j = 1; j < data.width - 1; j++)
		{
			const int gx = data[i - 1][j - 1] * -1 + data[i][j - 1] * -2 + data[i + 1][j - 1] * -1 +
				data[i - 1][j + 1] * 1 + data[i][j + 1] * 2 + data[i + 1][j + 1] * 1;

			const int gy = data[i + 1][j - 1] * -1 + data[i + 1][j] * -2 + data[i + 1][j + 1] * -1 +
				data[i - 1][j - 1] * 1 + data[i - 1][j] * 2 + data[i - 1][j + 1] * 1;

			gxArr[i*data.width + j] = gx;
			gyArr[i*data.width + j] = gy;

			const double g = std::sqrt(gx*gx + gy * gy);
			sobelImg[i*data.width + j] = g;

			++progressBar;
		}
	}

	BYTE *temp = new BYTE[data.width * data.height];
	for (ImageUtil::ImageSize i = 0; i < data.width * data.height; i++)
	{
		temp[i] = sobelImg[i];
	}

	progressBar.reset((data.height - 2) * (data.width - 2), "非极大值抑制。。。");

	for (ImageUtil::ImageSize i = 1; i < data.height - 1; i++)
	{
		for (ImageUtil::ImageSize j = 1; j < data.width - 1; j++)
		{
			double dir;
			if (gxArr[i*data.width + j] == 0)
				dir = 90;
			else
				dir = (std::atan(gyArr[i*data.width + j] / gxArr[i*data.width + j])) * 180 / ImageUtil::PI;
			//水平
			if ((dir >= 157.5 || dir <= -157.5) || (dir <= 22.5 && dir >= -22.5))
			{
				if (sobelImg[i * data.width + j] < sobelImg[i * data.width + j + 1] ||
					sobelImg[i * data.width + j] < sobelImg[i * data.width + j - 1])
				{
					temp[i * data.width + j] = 0;
				}
			}
			//-45度
			else if ((dir <= -112.5 && dir >= -157.5) || (dir <= 67.5 && dir >= 22.5))
			{
				if (sobelImg[i * data.width + j] < sobelImg[(i + 1) * data.width + j - 1] ||
					sobelImg[i * data.width + j] < sobelImg[(i - 1) * data.width + j + 1])
				{
					temp[i * data.width + j] = 0;
				}
			}
			//垂直
			else if ((dir >= -112.5 && dir <= -67.5) || (dir <= 112.5 && dir >= 67.5))
			{
				if (sobelImg[i * data.width + j] < sobelImg[(i + 1) * data.width + j] ||
					sobelImg[i * data.width + j] < sobelImg[(i - 1) * data.width + j])
				{
					temp[i * data.width + j] = 0;
				}
			}
			//+45度
			else if ((dir >= -67.5 && dir <= -22.5) || (dir <= 157.5 && dir >= 112.5))
			{
				if (sobelImg[i * data.width + j] < sobelImg[(i + 1) * data.width + j + 1] ||
					sobelImg[i * data.width + j] < sobelImg[(i - 1) * data.width + j - 1])
				{
					temp[i * data.width + j] = 0;
				}
			}

			++progressBar;
		}
	}

	for (int i = 0; i < data.width * data.height; i++)
	{
		sobelImg[i] = temp[i];
	}
	delete[] temp;
	delete[] gxArr;
	delete[] gyArr;

	std::queue<Pixel> highPixQue;
	BYTE *lowPix = new BYTE[data.width * data.height];
	for (unsigned int i = 0; i < data.height; i++)
	{
		for (unsigned int j = 0; j < data.width; j++) {
			if (sobelImg[i * data.width + j] > maxVal)
			{
				const Pixel p(j, i, 1);
				highPixQue.push(p);
			}


			if (sobelImg[i * data.width + j] > minVal && sobelImg[i * data.width + j] <= maxVal)
			{
				lowPix[i * data.width + j] = 1;
			}
			else
			{
				lowPix[i * data.width + j] = 0;
			}
		}
	}

	progressBar.reset(highPixQue.size(), "进行边缘8-联通....");

	memset(sobelImg, 0, data.width*data.height);
	while (!highPixQue.empty())
	{
		Pixel p = highPixQue.front();
		highPixQue.pop();

		sobelImg[p.getY() * data.width + p.getX()] = 1;
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				if (p.getY() + i < 0 || p.getY() + i >= data.height || p.getX() + j < 0 || p.getX() + j >= data.width)
					continue;

				if (i == 0 && j == 0)
					continue;

				if (lowPix[(p.getY() + i) * data.width + p.getX() + j] == 1)
				{
					//8联通合并
					for (int x = -1; x <= 1; x++)
					{
						for (int y = -1; y <= 1; y++)
						{
							if (p.getY() + i + y < 0 || p.getY() + i + y >= data.height || p.getX() + j + x < 0 || p.getX() + j + x >= data.width)
								continue;

							if (lowPix[(p.getY() + i + y) * data.width + p.getX() + j + x] == 1)
							{
								highPixQue.push(Pixel(p.getX() + j, p.getY() + i, 1));
								progressBar.addMax(1);
							}


						}
					}
					lowPix[(p.getY() + i) * data.width + p.getX() + j] = 0;

				}
			}
		}
		++progressBar;
	}

	delete[] lowPix;
	for (int i = 0; i < 5; i++)
		delete[] gaus[i];

	delete[] gaus;


	data.pImg = sobelImg;
	return data;
}

double** ImageUtil::EdgeDetection::getGaussianKernel(const int size, const double sqrSigma)
{
	double **gaus = new double*[size];
	for (int i = 0; i < size; i++)
	{
		gaus[i] = new double[size];
	}
	const double pi = 4.0 * std::atan(1.0);
	const int center = size / 2;
	double sum = 0;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			gaus[i][j] = (1 / (2 * pi*sqrSigma))*exp(-((1 - center)*(1 - center) + (j - center)*(j - center)) / (2 * sqrSigma));
			sum += gaus[i][j];
		}
	}

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			gaus[i][j] /= sum;
		}
	}

	return gaus;
}

ImageUtil::IMGDATA ImageUtil::EdgeDetection::LOG(ImageUtil::IMGDATA data, double sqrSigma, const int threadhold)
{
	BYTE *img = new BYTE[data.width * data.height];
	memset(img, 0, data.width * data.height);
	double** gaus = getGaussianKernel(5, sqrSigma);
	progressBar.reset((data.width - 4) * (data.height - 4), "高斯滤波....");
	for (int i = 2; i < data.height - 2; i++)
	{
		for (int j = 2; j < data.width - 2; j++)
		{
			int sum = 0;
			for (int x = -2; x <= 2; x++)
			{
				for (int y = -2; y <= 2; y++)
				{
					sum += data[i + x][j + y] * gaus[4 - (x + 2)][y + 2];
				}

			}
			img[i * data.width + j] = sum;
			++progressBar;
		}
	}
	// for (int i = 2; i < data.height - 2; i++)
	// {
	// 	for (int j = 2; j < data.width - 2; j++)
	// 	{
	// 		const int result = data[i + 2][j] +
	// 			data[i + 1][j - 1] + data[i + 1][j] * 2 + data[i + 1][j + 1] +
	// 			data[i][j - 2] + data[i][j - 1] * 2 + data[i][j] * -16 + data[i][j + 1] * 2 + data[i][j + 2] +
	// 			data[i - 1][j - 1] + data[i - 1][j] * 2 + data[i - 1][j + 1] +
	// 			data[i - 2][j];
 //
	// 		img[i*data.width + j] = result;
	// 	}
 //
	// }
	progressBar.reset(data.width * data.height, "LOG边缘检测....");
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

			img[i * data.width + j] = ImageUtil::clamp(
				1 * data.pImg[up * data.width + left] + 1 * data.pImg[up * data.width + j] + 1 * data.pImg[up * data.width + right] +
				1 * data.pImg[i * data.width + left] + -8 * data.pImg[i * data.width + j] + 1 * data.pImg[i * data.width + right] +
				1 * data.pImg[down * data.width + left] + 1 * data.pImg[down * data.width + j] + 1 * data.pImg[down * data.width + right]);

			++progressBar;
		}

	}

	for (int i = 0; i < data.width * data.height; i++)
	{
		if (img[i] > threadhold)
			img[i] = 1;
		else
			img[i] = 0;
	}

	data.pImg = img;
	return data;
}
ImageUtil::IMGDATA ImageUtil::EdgeDetection::sobel(ImageUtil::IMGDATA data, const int threadhold)
{
	BYTE *img = new BYTE[data.width * data.height];
	memset(img, 0, data.width * data.height);

	progressBar.reset((data.width - 1) * (data.height - 1), "sobel边缘检测....");
	for (int i = 1; i < data.height - 1; i++)
	{
		for (int j = 1; j < data.width - 1; j++)
		{
			const int gx = data[i - 1][j - 1] * -1 + data[i][j - 1] * -2 + data[i + 1][j - 1] * -1 +
				data[i - 1][j + 1] * 1 + data[i][j + 1] * 2 + data[i + 1][j + 1] * 1;

			const int gy = data[i + 1][j - 1] * -1 + data[i + 1][j] * -2 + data[i + 1][j + 1] * -1 +
				data[i - 1][j - 1] * 1 + data[i - 1][j] * 2 + data[i - 1][j + 1] * 1;


			const double g = std::sqrt(gx*gx + gy * gy);
			if (g > threadhold)
				img[i*data.width + j] = 1;

			++progressBar;
		}
	}
	data.pImg = img;
	return data;
}
ImageUtil::IMGDATA ImageUtil::EdgeDetection::prewitt(ImageUtil::IMGDATA data, const int threadhold)
{
	BYTE *img = new BYTE[data.width * data.height];
	memset(img, 0, data.width * data.height);

	progressBar.reset((data.width - 1) * (data.height - 1), "prewitt边缘检测....");
	for (int i = 1; i < data.height - 1; i++)
	{
		for (int j = 1; j < data.width - 1; j++)
		{
			const int dx = data.pImg[(i + 1)*data.width + j - 1] + data.pImg[(i + 1)*data.width + j] + data.pImg[(i + 1)*data.width + j + 1]
				- (data.pImg[(i - 1)*data.width + j - 1] + data.pImg[(i - 1)*data.width + j] + data.pImg[(i - 1)*data.width + j + 1]);

			const int dy = data.pImg[(i - 1) *data.width + j + 1] + data.pImg[i*data.width + j + 1] + data.pImg[(i - 1)*data.width + j + 1]
				- (data.pImg[(i + 1) *data.width + j - 1] + data.pImg[i*data.width + j - 1] + data.pImg[(i - 1)*data.width + j - 1]);

			if (std::_Max_value(dx, dy) > threadhold)
			{
				img[i * data.width + j] = 1;
			}

			++progressBar;
		}
	}
	data.pImg = img;
	return data;
}

ImageUtil::IMGDATA ImageUtil::Hough::line(ImageUtil::ImageData data, double deltaSigma)
{

	typedef ImageUtil::ImageSize uint;

	const int r = std::sqrt(data.width * data.width + data.height * data.height);
	const int d = 2 * r;
	const int sigma = 181 / deltaSigma;

	uint *houghSpace = new uint[d  * sigma];
	memset(houghSpace, 0, d * sigma * sizeof(uint));


	ImageUtil::ImageData cannyImg = ImageUtil::EdgeDetection::canny(data, 40, 80);

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
	for (int i = 0; i < r * sigma; i++)
	{
		if (houghSpace[i] > max)
			max = houghSpace[i];
	}


	ImageUtil::progressBar.reset(data.height * data.width, "检测直线....");
	for (uint i = 0; i < data.height; i++)
	{
		for (uint j = 0; j < data.width; j++)
		{
			double s = 0;
			while (true)
			{
				const int p = j * std::cos(ImageUtil::toRadian(static_cast<double>(s)))
					+ i * std::sin(ImageUtil::toRadian(static_cast<double>(s))) + r;
				if (houghSpace[p * sigma + static_cast<int>(s / deltaSigma)] > max * 0.9)
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
