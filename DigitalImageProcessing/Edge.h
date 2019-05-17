#pragma once
#include "Ftt.h"


namespace ImageUtil
{
	namespace EdgeDetection
	{

		struct WatershedPixel : Pixel
		{
			std::vector<WatershedPixel *> neighbour;
			int dist = INIT;

			static const int WSHED = 0;
			static const int INIT = -1;
			static const int MASK = -1;
			static const int FICTITIOUS = -2;


			WatershedPixel() { dist = FICTITIOUS; }
			WatershedPixel(const unsigned int x, const unsigned int y, const byte p) :Pixel(x,y,p) {}

			bool allNeigbourIsWshed();

			void addNeighbour(WatershedPixel *p);
			bool operator <(WatershedPixel& other) const;
		};

		class WatershedStructure
		{
			std::vector<WatershedPixel *> watershed;
		public:
			WatershedStructure(byte* pixels, const int width, const int height);
			ImageSize size() const;
			WatershedPixel * at(ImageSize index);

			~WatershedStructure();
		
		};

		ImageUtil::IMGDATA prewitt(ImageUtil::IMGDATA data, int threadhold);
		ImageUtil::IMGDATA sobel(ImageUtil::IMGDATA data, int threadhold);
		ImageUtil::IMGDATA LOG(ImageUtil::IMGDATA data, double sqrSigma, int threadhold);
		ImageUtil::IMGDATA canny(ImageUtil::IMGDATA data, const int minVal, const int maxVal);
		double** getGaussianKernel(const int size, const double sqrSigma);
	}

}
