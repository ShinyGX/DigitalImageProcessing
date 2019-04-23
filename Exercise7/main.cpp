
#include <string>
#include <iostream>
#include "../DigitalImageProcessing/ImageUtil.h"
#include "../DigitalImageProcessing/Ftt.h"
#include <vector>
#include <queue>

struct Region
{
	int wBeginIndex = 0, wEndIndex = 0, hBeginIndex = 0, hEndIndex = 0;
	bool Q = false;
};
void regionGrowWithSeed(const ImageUtil::IMGDATA& data, BYTE * unRegion, int *growQueX, int *growQueY, int threshold, int seedX, int seedY, int color);
Region* splitRegion(const Region& r, Region *alloc);
double getAver(const ImageUtil::IMGDATA& data,const Region& r);
ImageUtil::IMGDATA reginGrowWithoutSeed(ImageUtil::IMGDATA data, int threadhold);

int main()
{
	std::string path;
	std::cin >> path;

	auto data = ImageUtil::loadImageToGray(path);
	auto seed = ImageUtil::loadImageToGray("bitmap/6.bmp");

	BYTE *unRegion = new BYTE[data.width * data.height];
	int *growQueX = new int[data.width * data.height];
	int *growQueY = new int[data.width * data.height];
	for (int i = 0; i < data.width * data.height; i++)
		unRegion[i] = 0;

	// for(int i = 0;i < seed.width;i++)
	// {
	// 	for(int j = 0;j < seed.height;j++)
	// 	{
	// 		if(seed.pImg[i * seed.height + j])
	// 		{
	// 			regionGrowWithSeed(data, unRegion, growQueX, growQueY, 5, i, j);
	// 		}
	// 	}
	// }
	
	regionGrowWithSeed(seed, unRegion, growQueX, growQueY, 3, 0, 0, 1);
	regionGrowWithSeed(seed, unRegion, growQueX, growQueY, 2, seed.width / 2, seed.height / 2, 2);
	regionGrowWithSeed(seed, unRegion, growQueX, growQueY, 10, 328, 283 - 45, 3);


	ImageUtil::IMGDATA newImg = seed;
	newImg.rgbquad[1].rgbBlue = 255;
	newImg.rgbquad[1].rgbGreen = 0;
	newImg.rgbquad[1].rgbRed = 0;

	newImg.rgbquad[2].rgbBlue = 0;
	newImg.rgbquad[2].rgbGreen = 255;
	newImg.rgbquad[2].rgbRed = 0;


	newImg.rgbquad[3].rgbBlue = 0;
	newImg.rgbquad[3].rgbGreen = 0;
	newImg.rgbquad[3].rgbRed = 255;

	data.fileHeader.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + sizeof(RGBQUAD) * 4;
	data.fileHeader.bfSize = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + sizeof(RGBQUAD) * 4 + data.infoHeader.biSizeImage;

	data.infoHeader.biClrUsed = 4;


	newImg.pImg = unRegion;
	ImageUtil::outputImage(newImg, "bitmap/region_grow_with_seed.bmp");

	
	ImageUtil::outputBlackWhiteImage(reginGrowWithoutSeed(data, 80), "bitmap/region_grow.bmp");

	delete[] unRegion;
	delete[] growQueX;
	delete[] growQueY;
	delete[] seed.pImg;
	delete[] data.pImg;
	return 0;
}

ImageUtil::IMGDATA reginGrowWithoutSeed(ImageUtil::IMGDATA data,int threadhold)
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
	while(start <= end)
	{
		const Region curRegion = queRegion.front();
		queRegion.pop();
		//if (!curRegion.Q)
		Region * rQList = splitRegion(curRegion, rList);

		for(int i =0;i < 4;i++)
		{
			if(rQList != nullptr)
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
	for(auto& r : qTList)
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

Region* splitRegion(const Region& r,Region *alloc)
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

double getAver(const ImageUtil::IMGDATA& data, const Region& r)
{
	int count = 0, result = 0;
	for(int i = r.hBeginIndex;i < r.hEndIndex;i++)
	{
		for(int j = r.wBeginIndex;j<r.wEndIndex;j++)
		{
			result += data[i][j];
			count++;
		}
	}

	return static_cast<double>(result) / count;
}

void regionGrowWithSeed(const ImageUtil::IMGDATA& data,BYTE * unRegion,int *growQueX,int *growQueY, int threshold, int seedX, int seedY,int color)
{
	int nDx[8] = { 0, 0,1,-1, 1,1,-1,-1 };
	int nDy[8] = { 1,-1,0, 0,-1,1, 1,-1 };



	int start = 0, end = 0;
	growQueX[end] = seedX;
	growQueY[end] = seedY;

	while(start <= end)
	{
		const int currX = growQueX[start];
		const int currY = growQueY[start];

		for (int k = 0; k < 8; k++)
		{
			const int xx = currX + nDx[k];
			const int yy = currY + nDy[k];

			if(xx < data.width && xx >= 0 && 
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