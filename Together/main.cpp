
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include "../DigitalImageProcessing/Math.h"
#include <iostream>
#include "../DigitalImageProcessing/Bmp.h"
#include "../DigitalImageProcessing/Edge.h"
#include "../DigitalImageProcessing/Extend.h"


using namespace std;
using namespace ImageUtil;

void outputMsg(const string& msg)
{
	cout << msg << endl;
}

void one(const string& path){
	outputMsg("进行RGB分离.....");
	ImageUtil::bitmapTo3SignalColorBitmap(path);
	ImageUtil::bitmap2Gray(path);
	ImageUtil::gray2Anticolor();

	outputMsg("分离完成");
}
void two(const string& path)
{
	outputMsg("开始直方图转换");
	const ImageUtil::IMGDATA data = ImageUtil::loadImage(path);
	ImageUtil::GRAYHISTOGRAM grayhistogram = ImageUtil::getHistogram(data);
	grayhistogram.normalize();
	ImageUtil::outputHistogram(data, "bitmap/histogram/_grayHistogram.bmp");
	const ImageUtil::IMGDATA newData = ImageUtil::balance2(grayhistogram, data);
	ImageUtil::outputHistogram(newData, "bitmap/histogram/_afterBalance.bmp");
	//ImageUtil::outputImage(newData, 256, "bitmap/histogram/balance.bmp");
	outputMsg("直方图转换完成");

}
void three(const string& path)
{
	outputMsg("开始空间域滤波");
	IMGDATA data1 = ImageUtil::loadImageToGray(path);
	IMGDATA laplace1 = data1;
	ImageUtil::outputHistogram(data1, "bitmap/3/before_advence_histogram.bmp");


	//----------------拉普拉斯滤波----------
	IMGDATA laplaceIMG = laplaceOstu2(data1);
	ImageUtil::outputImage(laplaceIMG, 256, "bitmap/3/laplace_step_1.bmp");
	laplaceIMG = data1 + (laplaceIMG * -1);
	ImageUtil::outputImage(laplaceIMG, 256, "bitmap/3/laplace.bmp");
	ImageUtil::outputHistogram(laplaceIMG, "bitmap/3/laplace_step_2.bmp");

	//---------------伪彩色化--------------
	colorful(data1, "bitmap/3/colorful.bmp");

	//---------------中值滤波---------------
	IMGDATA midData = mid(data1);
	int j = 0;
	while (++j < 4)
		midData = mid(midData);
	ImageUtil::outputHistogram(midData, "bitmap/3/after_mid_histogram.bmp");
	ImageUtil::outputImage(midData, 256, "bitmap/3/mid.bmp");


	//-------------平均滤波-----------------
	int i = 0;
	while (++i < 4)
		data1 = advenage(data1);
	ImageUtil::outputHistogram(data1, "bitmap/3/after_advence_histogram.bmp");
	ImageUtil::outputImage(data1, 256, "bitmap/3/advenage.bmp");
	outputMsg("空间域滤波完成");
}
void four(const string& path)
{
	outputMsg("图像变换开始。。。。");
	IMGDATA data = ImageUtil::loadImage(path);
	IMGDATA mirrorIMG = mirror(data),
		scaleIMG = scale(data),
		rotateImg = rotate(data),
		transImg = translate(data);
	//	perspectiveIMG = perspective(data, 0, 0, 1, 0, 1, 1, 0, 1);//(0,0)(1,0)(1,1)(0,1）


	ImageUtil::outputImage(mirrorIMG, "bitmap/4/mirror.bmp");
	ImageUtil::outputImage(scaleIMG, "bitmap/4/scale.bmp");
	ImageUtil::outputImage(rotateImg, "bitmap/4/rotate.bmp");
	ImageUtil::outputImage(transImg, "bitmap/4/translate.bmp");
	//	ImageUtil::outputImage(perspectiveIMG, "bitmap/perspective.bmp");

	//	delete[] perspectiveIMG.pImg;
	delete[] data.pImg;
	delete[] mirrorIMG.pImg;
	delete[] scaleIMG.pImg;
	delete[] rotateImg.pImg;
	delete[] transImg.pImg;

	outputMsg("图像变换结束。。。。");
}
void five(const string& path)
{
	outputMsg("开始图像平滑....");
	const ImageUtil::IMGDATA img = ImageUtil::loadImageToGray(path);
	ImageUtil::outputHistogram(img, "bitmap/5/histogram.bmp");
	//thresholdByGive(img);
	thresholdByIterate(img);
	laplaceOstu(img);
	otsu(img);
	outputMsg("开始图像平滑结束....");
}
void six(const string& path)
{
	outputMsg("联通域连接开始....");
	auto data = ImageUtil::loadImageToGray(path);
	auto seed = ImageUtil::loadImageToGray("bitmap/6.bmp");

	BYTE *unRegion = new BYTE[data.width * data.height];
	int *growQueX = new int[data.width * data.height];
	int *growQueY = new int[data.width * data.height];
	for (int i = 0; i < data.width * data.height; i++)
		unRegion[i] = 0;


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
	ImageUtil::outputImage(newImg, "bitmap/6/region_grow_with_seed.bmp");


	ImageUtil::outputBlackWhiteImage(reginGrowWithoutSeed(data, 80), "bitmap/6/region_grow.bmp");

	delete[] unRegion;
	delete[] growQueX;
	delete[] growQueY;
	delete[] seed.pImg;
	delete[] data.pImg;

	outputMsg("联通域连接结束....");
}
void seven(const string&path)
{
	outputMsg("开始边缘检测...");
	auto img = ImageUtil::loadImageToGray(path);
	auto p = EdgeDetection::prewitt(img, 100);
	auto s = EdgeDetection::sobel(img, 100);
	auto l = EdgeDetection::LOG(img, 0.05, 100);
	auto c = EdgeDetection::canny(img, 50, 100);
	ImageUtil::outputBlackWhiteImage(p, "bitmap/7/prewitt.bmp");
	ImageUtil::outputBlackWhiteImage(s, "bitmap/7/sobel.bmp");
	ImageUtil::outputBlackWhiteImage(l, "bitmap/7/LOG.bmp");
	ImageUtil::outputBlackWhiteImage(c, "bitmap/7/canny.bmp");
	delete[] c.pImg;
	delete[] l.pImg;
	delete[] s.pImg;
	delete[] p.pImg;
	delete[] img.pImg;

	outputMsg("开始边缘结束....");
}
void nine(const string& path)
{
	outputMsg("开始图像分割。。");
	auto g = ImageUtil::loadImageToGray(path);


	//ImageUtil::outputImage(watershed(ImageUtil::EdgeDetection::canny(g, 50, 100)), "bitmap/watershed.bmp");

	WatershedAlgorithm watershedAlgorithm;
	watershedAlgorithm.run(&g, "bitmap/9/watershed.bmp");

	outputMsg("图像分割结束。。");
}
int main(int argc, char* argv[])
{
	const char* fileName = "0.bmp";

	string path;
	cin >> path;

	one(path);
	two(path);
	three(path);
	four(path);

	outputMsg("开始傅里叶变换");
	auto data = ImageUtil::loadImageToGray(path);
	ImageUtil::FFT::fft2d(data);
	//outputImage(data, "bitmap/fft/f.bmp");
	outputMsg("傅里叶变换完成");

	five(path);
	six("bitmap/6.bmp");
	seven(path);

	outputMsg("开始霍夫直线检测");

	int bmpHeight;
	int bmpWidth;
	int LineByte;

	unsigned char *pBmpBuf;
	RGBQUAD *pColorTable;
	BITMAPINFOHEADER head;
	BITMAPFILEHEADER filehead;


	FILE *inputFile = fopen(fileName, "rb");
	if (inputFile == 0)
	{
		printf("Load file failed!");
		return 0;
	}
	fread(&filehead, sizeof(BITMAPFILEHEADER), 1, inputFile);


	fread(&head, 40, 1, inputFile);
	bmpHeight = head.biHeight;
	bmpWidth = head.biWidth;
	pColorTable = new RGBQUAD[256];

	fread(pColorTable, sizeof(RGBQUAD), 256, inputFile);

	LineByte = (bmpWidth * 8 / 8 + 3) / 4 * 4;
	pBmpBuf = new unsigned char[LineByte*bmpHeight];


	fread(pBmpBuf, LineByte*bmpHeight, 1, inputFile);
	fclose(inputFile);


	int **buff = new int*[bmpHeight];
	for (int i = 0; i < bmpHeight; i++)
		buff[i] = new int[bmpWidth];

	unsigned char* pb1;

	for (int i = 0; i < bmpHeight; i++)
	{
		for (int j = 0; j < bmpWidth; j++)
		{
			pb1 = pBmpBuf + i * LineByte + j;
			int color = *pb1;
			buff[i][j] = color;
		}
	}

	int h = int(sqrt(bmpHeight*bmpHeight + bmpWidth * bmpWidth));
	int **buff2 = new int*[h];
	for (int i = 0; i < h; i++)
		buff2[i] = new int[181];

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < 181; j++)
		{
			buff2[i][j] = 0;
		}
	}

	for (int i = 0; i < bmpHeight; i++)
	{
		for (int j = 0; j < bmpWidth; j++)
		{
			if (buff[i][j] > 10)
			{
				for (int k = 0; k < 181; k++)
				{
					int p = int(j * cos(((double)k* ImageUtil::PI / 2.0) / 180) + i * sin(((double)k* ImageUtil::PI / 2.0) / 180));
					buff2[p][k] += 1;
				}
			}
		}
	}



	int **buff3 = new int*[bmpHeight];
	for (int i = 0; i < h; i++)
		buff3[i] = new int[bmpWidth];

	for (int i = 0; i < bmpHeight; i++)
	{
		for (int j = 0; j < bmpWidth; j++)
		{
			buff3[i][j] = 0;

		}
	}



	for (int i = 0; i < bmpHeight; i++)
	{
		for (int j = 0; j < bmpWidth; j++)
		{
			for (int k = 0; k < 181; k++)
			{
				int p = int(j * cos(((double)k* ImageUtil::PI / 2.0) / 180) + i * sin(((double)k* ImageUtil::PI / 2.0) / 180));
				if (buff2[p][k] > 85)
				{
					buff3[i][j] = 255;
				}
			}
		}
	}


	FILE *outFile = fopen("1.bmp", "wb");
	if (outFile == 0)
	{
		return 0;
	}
	fwrite(&filehead, sizeof(BITMAPFILEHEADER), 1, outFile);
	fwrite(&head, 40, 1, outFile);
	fwrite(pColorTable, sizeof(RGBQUAD), 256, outFile);
	int LineByte2 = (bmpWidth * 8 / 8 + 3) / 4 * 4;
	for (int i = 0; i < bmpHeight; i++) // ÿ��
	{
		for (int j = 0; j < LineByte2; j++) // ÿ��
		{
			fwrite(&buff3[i][j], sizeof(BYTE), 1, outFile);
		}
	}
	fclose(outFile);

	outputMsg("霍夫直线检测结束");

	nine("7.bmp");

	return 0;

}
