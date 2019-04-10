#pragma once
#include <complex>
#include "ImageUtil.h"

namespace ImageUtil
{
	namespace FFT
	{

		std::complex<double>* dft2d(ImageData& data);
		void nomalisize(double *dest, ImageData& data);

		void getAmpgetAmplitudespectrumlitu(std::complex<double>* dest, ImageData data);
		void getWN(double n, double size, std::complex<double>& dst);

		void fttRealRemap(BYTE *src, int size);
		void fttComplexRemap(std::complex<double>* src, int length);

		bool isBase2(int size);
		void shift(double *dest, int width, int height);

	
		void fft2d(ImageData& data);
		std::complex<double> * realFtt(double * src,int size);
		std::complex<double> * complexFtt(std::complex<double> *dst,int size);

	}
}

