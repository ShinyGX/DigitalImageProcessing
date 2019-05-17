

#include "../DigitalImageProcessing/ImageUtil.h"
#include <iostream>
#include <valarray>
#include <queue>
#include "../DigitalImageProcessing/Math.h"
#include "../DigitalImageProcessing/Ftt.h"
#include <set>
#include "../DigitalImageProcessing/Edge.h"
#include "../DigitalImageProcessing/Extend.h"

ImageUtil::ImageData watershed(ImageUtil::ImageData origin, ImageUtil::ImageData seed);
ImageUtil::ImageData watershed(ImageUtil::ImageData origin);
int main()
{
	std::string path;
	std::cin >> path;
	auto g = ImageUtil::loadImageToGray(path);


	//ImageUtil::outputImage(watershed(ImageUtil::EdgeDetection::canny(g, 50, 100)), "bitmap/watershed.bmp");

	WatershedAlgorithm watershedAlgorithm;
	watershedAlgorithm.run(&g, "bitmap/watershed.bmp");
	return 0;
}

ImageUtil::ImageData watershed(ImageUtil::ImageData origin)
{
	typedef ImageUtil::EdgeDetection::WatershedStructure Image;
	typedef ImageUtil::EdgeDetection::WatershedPixel PVec2;
	typedef ImageUtil::ImageSize uint;


	using namespace std;

	const int width = origin.width, height = origin.height;

	Image img(origin.pImg, width, height);

	queue<PVec2 *>que;
	int pixelIndex = 0, hightIndex = 0,curLab = 0;
	for(int water = 0;water < 256;water++)
	{
		for (uint p = pixelIndex; p < img.size(); p++)
		{
			PVec2 * pix = img.at(p);
			if(pix->pix != water)
			{
				pixelIndex = p;
				break;
			}

			pix->mask = PVec2::MASK;

			vector<PVec2 *> neigbhour = pix->neighbour;
			for (auto q : neigbhour)
			{
				if(q->mask > 0)
				{
					q->dist = 1;
					que.push(q);
					break;
				}
			}
		}

		int dist = 0;
		que.push(new PVec2());

		//扩大聚水盆地
		while(!que.empty())
		{
			PVec2 *q = que.front();
			que.pop();

			if(q->mask == PVec2::FICTITIOUS)
			{
				if(que.empty())
				{
					delete q;
					q = nullptr;
					break;
				}
				que.push(new PVec2());
				dist++;
				delete q;
				q = que.front();
				que.pop();
			}

			vector<PVec2 *> neighbour = q->neighbour;
			for (auto p : neighbour)
			{
				if(p->dist <= dist && p->mask >= 0)
				{
					if(p->mask > 0)
					{
						if(q->mask == PVec2::MASK)
						{
							q->mask = p->mask;
						}
						else if(p->mask != q->mask)
						{
							q->mask = PVec2::WSHED;
						}
					}
					else if(q->mask == PVec2::MASK)
					{
						p->mask = PVec2::WSHED;
					}
				}
				else if (p->mask == PVec2::MASK && p->dist == 0)
				{
					p->dist = dist + 1;
					que.push(p);
				}

			}

		}

		for(uint hIndex = hightIndex;hIndex < img.size();hIndex++)
		{
			PVec2 *p = img.at(hIndex);

			if(p->pix != water)
			{
				hightIndex = hIndex;
				break;
			}

			p->dist = 0;

			if(p->mask == PVec2::MASK)
			{
				curLab++;
				p->mask = curLab;

				que.push(p);

				while(!que.empty())
				{
					PVec2 * q = que.front();
					que.pop();

					vector<PVec2 *> neighbour = q->neighbour;

					for (auto pv : neighbour)
					{
						if(pv->mask == PVec2::MASK)
						{
							pv->mask = curLab;
							que.push(pv);
						}
					}
				}
			}
		}

	}


	for(uint pixIndex = 0;pixelIndex < img.size();pixelIndex++)
	{
		PVec2 * p = img.at(pixelIndex);
		if(p->mask == PVec2::WSHED && !p->allNeigbourIsWshed())
		{
			origin[p->getY()][p->getX()] = 255;
		}
	}

	return origin;

}


ImageUtil::ImageData watershed(ImageUtil::ImageData origin, ImageUtil::ImageData seed)
{
	typedef ImageUtil::PVec2 PVec2;
	typedef ImageUtil::ImageSize uint;

	using namespace std;
	int regionNum = 0;

	const uint width = origin.width, height = origin.height;

	vector<int *> seedCounts;
	queue<PVec2> seedQue;
	vector<queue<PVec2> *> allSeedQue;

	byte* label = new byte[width * height];
	ImageUtil::initWithZero(label, width * height);

	toTwoValueImage(seed);
	ImageUtil::outputBlackWhiteImage(seed, "bitmap/seed.bmp");

	for (uint i = 0; i < height; i++)
	{
		for (uint j = 0; j < width; j++)
		{
			if (seed[i][j] == 1)
			{
				regionNum++;
				int *arr = new int[256];
				ImageUtil::initWithZero(arr, 256);

				seedCounts.push_back(arr);
				queue<PVec2>* que = new queue<PVec2>[256];
				allSeedQue.push_back(que);

				PVec2 temp;
				temp.setX(j);
				temp.setY(i);

				seedQue.push(temp);

				//当前点标记为已处理
				label[i * width + j] = regionNum;
				seed[i][j] = 127;

				while (!seedQue.empty())
				{

					temp = seedQue.front();
					seedQue.pop();

					bool dirGrow = false;

					const uint m = temp.getX(), n = temp.getY();
					for (int x = -1; x <= 1; x++)
					{
						for (int y = -1; y <= 1; y++)
						{
							if (m + x < 0 || m + x >= width ||
								n + y < 0 || n + y >= height ||
								(x == 0 && y == 0))
								continue;

							if (seed[n + y][m + x] == 1)
							{
								temp.setX(m + x);
								temp.setY(n + y);

								label[(n + y)*width + m + x] = regionNum;
								seed[n + y][m + x] = 127;
							}
							else
							{
								dirGrow = true;
							}
						}
					}

					if (dirGrow)
					{
						temp.setX(m);
						temp.setY(n);

						allSeedQue[regionNum - 1][origin[n][m]].push(temp);
						seedCounts[regionNum - 1][origin[n][m]]++;
					}

				}

			}
		}
	}


	for (int waterLevel = 0; waterLevel < 256; waterLevel++)
	{
		cout << waterLevel << endl;
		bool actives = true;
		while (actives)
		{
			actives = false;
			for (int i = 0; i < regionNum; i++)
			{
				if (!allSeedQue[i]->empty())
				{
					actives = true;
					while (seedCounts[i][waterLevel] > 0)
					{				
						while (!allSeedQue[i][waterLevel].empty()) {
							seedCounts[i][waterLevel]--;
							PVec2 temp = allSeedQue[i][waterLevel].front();
							allSeedQue[i][waterLevel].pop();

							const uint m = temp.getX(), n = temp.getY();

							for (int x = -1; x <= 1; x++)
							{
								for (int y = -1; y <= 1; y++)
								{
									if (m + x < 0 || m + x >= width ||
										n + y < 0 || n + y >= height ||
										(x == 0 && y == 0))
										continue;


									if (label[(n + y)*width + m + x] == 0)
									{
										temp.setX(m + x);
										temp.setY(n + y);

										//标记为已淹没
										label[(n + y) * width + m + x] = waterLevel;

										//若可以淹没
										if (origin[n + y][m + x] <= waterLevel)
										{
											allSeedQue[i][waterLevel].push(temp);
										}
										else
										{
											allSeedQue[i][origin[n + y][m + x]].push(temp);
											seedCounts[i][origin[n + y][m + x]]++;
										}
									}

								}
							}
						}

					}
				}
			}
		}

	}
	  while (!allSeedQue.empty())
	  {
	  	queue<PVec2> * p = allSeedQue.front();
	  	allSeedQue.pop_back();
	  //	delete[] p;
	  	
	  }

	  while(!seedCounts.empty())
	  {
	  	int *arr = seedCounts.front();
	  	seedCounts.pop_back();
	  	//delete[] arr;
	  	
	 }

	memcpy(origin.pImg, label, width * height);

	delete[] label;
	return origin;
}
