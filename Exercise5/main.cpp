

#include "../DigitalImageProcessing/ImageUtil.h"
#include "../DigitalImageProcessing/Ftt.h"

int main()
{
	auto data = ImageUtil::loadImageToGray("bitmap/3.bmp");
	ImageUtil::FFT::fft2d(data);

	return 0;
}
