#pragma once
#include <vector>
#include <algorithm>
#include <queue>
#include "ImageUtil.h"


class WatershedPixel {
	static const int INIT = -1;	// 用来初始化图像
	static const int MASK = -2;	// 指示新像素点将被处理（每个层级的初始值）
	static const int WSHED = 0;	// 表明像素点属于某个分水岭
	static const int FICTITIOUS = -3;	// 虚拟像素点

	int x;	// 像素点x坐标
	int y;	// 像素点y坐标
	char height;	// 像素点的灰度值
	int label;	// 用于分水岭浸没算法的标签
	int dist;	// 操作像素时用到的距离

	std::vector<WatershedPixel*> neighbours;	// 储存邻域像素

public:
	WatershedPixel(int x, int y, char height) { // 像素点构造函数
		this->x = x;
		this->y = y;
		this->height = height;
		label = INIT;
		dist = 0;
		neighbours.reserve(8);	// 用于存储8连通的邻域像素
	}
	WatershedPixel() { label = FICTITIOUS; } // 虚拟像素点构造函数

	void addNeighbour(WatershedPixel* neighbour) { // 添加邻域像素
		neighbours.push_back(neighbour);
	}
	std::vector<WatershedPixel*>& getNeighbours() { // 获取邻域像素
		return neighbours;
	}

	/* 获取像素灰度值和坐标*/
	char getHeight() const { return height; }
	int getIntHeight() const { return (int)height & 0xff; }
	int getX() const { return x; }
	int getY() const { return y; }

	/* 设置和获取标签 */
	void setLabel(int label) { this->label = label; }
	void setLabelToINIT() { label = INIT; }
	void setLabelToMASK() { label = MASK; }
	void setLabelToWSHED() { label = WSHED; }
	int getLabel() { return label; }

	/* 判断当前标签状态 */
	bool isLabelINIT() { return label == INIT; }
	bool isLabelMASK() { return label == MASK; }
	bool isLabelWSHED() { return label == WSHED; }

	/* 设置和获取距离 */
	void setDistance(int distance) { dist = distance; }
	int getDistance() { return dist; }

	/* 判断是否为虚拟像素 */
	bool isFICTITIOUS() { return label == FICTITIOUS; }

	/* 判断是否所有邻域像素为分水岭（用于绘制最后的分水岭） */
	bool allNeighboursAreWSHED() {
		for (unsigned i = 0; i < neighbours.size(); i++) {
			WatershedPixel* r = neighbours.at(i);
			if (!r->isLabelWSHED()) return false;
		}
		return true;
	}
};

class WatershedStructure {
	std::vector<WatershedPixel*> watershedStructure;

public:
	WatershedStructure(byte* pixels, int width, int height) {	// 结构体构造函数

		watershedStructure.reserve(width * height);	 // 根据像素点总数预分配空间
		/* 将每个像素点信息存入结构体 */
		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
				watershedStructure.push_back(new WatershedPixel(x, y, pixels[x + y * width]));

		/* 计算各个像素点的8连通邻域像素 */
		for (int y = 0; y < height; ++y) {
			int offset = y * width;
			int topOffset = offset + width;
			int bottomOffset = offset - width;

			for (int x = 0; x < width; ++x) {
				int currentindex = x + y * width;	// 当前像素点索引
				WatershedPixel* currentPixel = watershedStructure.at(currentindex);

				if (x - 1 >= 0) {
					currentPixel->addNeighbour(watershedStructure.at(currentindex - 1)); // 左邻域
					if (y - 1 >= 0)	 // 左下角
						currentPixel->addNeighbour(watershedStructure.at(currentindex - 1 - width));
					if (y + 1 < height)	 // 左上角
						currentPixel->addNeighbour(watershedStructure.at(currentindex - 1 + width));
				}

				if (x + 1 < width) {
					currentPixel->addNeighbour(watershedStructure.at(currentindex + 1)); // 右邻域
					if (y - 1 >= 0)	 // 右下角
						currentPixel->addNeighbour(watershedStructure.at(currentindex + 1 - width));
					if (y + 1 < height) // 右上角
						currentPixel->addNeighbour(watershedStructure.at(currentindex + 1 + width));
				}

				if (y - 1 >= 0) // 下邻域
					currentPixel->addNeighbour(watershedStructure.at(currentindex - width));

				if (y + 1 < height)	 // 上邻域
					currentPixel->addNeighbour(watershedStructure.at(currentindex + width));
			}
		}

		/* 根据灰度值对结构体中的所有像素点从小到大进行排序 */
		std::sort(watershedStructure.begin(), watershedStructure.end(),
			[](WatershedPixel * pl, WatershedPixel * pr) { return pl->getIntHeight() < pr->getIntHeight(); });
	} // 构造结束

	~WatershedStructure() {	// 释放像素类所占空间
		while (!watershedStructure.empty()) {
			WatershedPixel* p = watershedStructure.back();
			delete p; p = NULL;
			watershedStructure.pop_back();
		}
	}

	int size() { return watershedStructure.size(); }	// 返回像素点总数

	WatershedPixel* at(int i) { return watershedStructure.at(i); }	// 返回某个像素点
};

class WatershedAlgorithm {
	static const int HMIN = 0;	// 最小层
	static const int HMAX = 256;	// 最大层

public:
	void run(ImageUtil::IMGDATA* pSrc, const std::string& imgName) { // 分水岭核心算法
		std::string inTmp;


		byte* img = new byte[pSrc->width * pSrc->height];
		for (int i = 1; i < pSrc->height; i++)
		{
			for (int j = 1; j < pSrc->width; j++)
			{
				img[i * pSrc->width + j] = (pow(pSrc->pImg[i * pSrc->width + j] - pSrc->pImg[i * pSrc->width + j - 1], 2)
					+ pow(pSrc->pImg[i * pSrc->width + j] - pSrc->pImg[(i - 1) * pSrc->width + j], 2))*0.5;
			}
		}


		/* 获取图像信息 */
		byte* pixels = img;
		int width = pSrc->width;
		int height = pSrc->height;


		/* Vincent and Soille 分水岭算法（1991）第一步: 将像素点存入结构体并排序 */
		WatershedStructure  watershedStructure(pixels, width, height);

		/* Vincent and Soille 分水岭算法（1991）第二步: 泛洪（模拟浸没） */
		/************************ 泛洪（浸没）开始 ****************************/
		std::queue<WatershedPixel*> pque;	// 存储像素的临时队列
		int curlab = 0;
		int heightIndex1 = 0;
		int heightIndex2 = 0;

		for (int h = HMIN; h < HMAX; ++h) { // h-1 层的 Geodesic SKIZ

			for (int pixelIndex = heightIndex1; pixelIndex < watershedStructure.size(); ++pixelIndex) {
				WatershedPixel* p = watershedStructure.at(pixelIndex);

				/* 此像素点位于 h+1 层，暂不处理，跳出循环 */
				if (p->getIntHeight() != h) { heightIndex1 = pixelIndex; break; }

				p->setLabelToMASK(); // 标记此像素将被处理

				std::vector<WatershedPixel*> neighbours = p->getNeighbours();
				for (unsigned i = 0; i < neighbours.size(); ++i) {
					WatershedPixel* q = neighbours.at(i);

					/* 将处于盆地或分水岭的h层的邻接像素点入队 */
					if (q->getLabel() >= 0) { p->setDistance(1); pque.push(p); break; }
				}
			}

			int curdist = 1;
			pque.push(new WatershedPixel());

			while (true) { // 扩展聚水盆地
				WatershedPixel* p = pque.front(); pque.pop();

				if (p->isFICTITIOUS())
					if (pque.empty()) { delete p; p = NULL; break; }
					else {
						pque.push(new WatershedPixel());
						curdist++;
						delete p; p = pque.front(); pque.pop();
					}

				std::vector<WatershedPixel*> neighbours = p->getNeighbours();
				for (unsigned i = 0; i < neighbours.size(); ++i) { // 通过检查邻接像素来标记 p
					WatershedPixel* q = neighbours.at(i);

					/* q属于一个存在的盆地或分水线 */
					if ((q->getDistance() <= curdist) && (q->getLabel() >= 0)) {

						if (q->getLabel() > 0) {
							if (p->isLabelMASK())
								p->setLabel(q->getLabel());
							else if (p->getLabel() != q->getLabel())
								p->setLabelToWSHED();
						}
						else if (p->isLabelMASK())
							p->setLabelToWSHED();
					}
					else if (q->isLabelMASK() && (q->getDistance() == 0)) {
						q->setDistance(curdist + 1);
						pque.push(q);
					}
				} // 处理邻接像素的for循环
			} // 扩展盆地的while循环

			/* 搜寻并处理h层中新的最小值 */
			for (int pixelIndex = heightIndex2; pixelIndex < watershedStructure.size(); pixelIndex++) {
				WatershedPixel* p = watershedStructure.at(pixelIndex);

				/* 此像素点位于 h+1 层，暂不处理，跳出循环 */
				if (p->getIntHeight() != h) { heightIndex2 = pixelIndex; break; }

				p->setDistance(0); // 重置距离为0

				if (p->isLabelMASK()) { // 该像素位于新最小值区域
					curlab++;
					p->setLabel(curlab);
					pque.push(p);

					while (!pque.empty()) {
						WatershedPixel* q = pque.front();
						pque.pop();

						std::vector<WatershedPixel*> neighbours = q->getNeighbours();

						for (unsigned i = 0; i < neighbours.size(); i++) { // 检查p2的邻域像素
							WatershedPixel* r = neighbours.at(i);

							if (r->isLabelMASK()) { r->setLabel(curlab); pque.push(r); }
						}
					} // end while
				} // end if
			} // end for
		}
		/************************ 泛洪（浸没）结束 ****************************/

		//cvCopyImage(pBW, pWS);

		/*
		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
				wsPixels[x + y * width] = (char)0;
		*/
		ImageUtil::outputImage(*pSrc, "bitmap/source_1.bmp");
		for (int pixelIndex = 0; pixelIndex < watershedStructure.size(); pixelIndex++) {
			WatershedPixel* p = watershedStructure.at(pixelIndex);

			if (p->isLabelWSHED() && !p->allNeighboursAreWSHED()) {
				img[p->getX() + p->getY()*width] = (char)255; // 在黑色背景中绘制白色分水线
				pSrc->pImg[p->getX() + p->getY() * width] = 255;
				//grayPixels[p->getX() + p->getY()*width] = (char)255;	// 在灰度图中绘制白色分水线
			}
		}
		ImageUtil::outputImage(*pSrc, "bitmap/source.bmp");

		memcpy(pSrc->pImg, img, width * height);
		ImageUtil::outputImage(*pSrc, imgName);

	}
};