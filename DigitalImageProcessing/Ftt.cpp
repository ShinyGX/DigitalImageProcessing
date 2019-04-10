#include "Ftt.h"
#include "Math.h"
#include <iostream>


std::complex<double>* ImageUtil::FFT::dft2d(ImageData& data)
{
	std::complex<double> *dest = new std::complex<double>[data.width * data.height];

	for (int u = 0; u < data.width; u++)
	{
		for (int v = 0; v < data.height; v++)
		{
			double real = 0;
			double imagin = 0;
			for (int i = 0; i < data.width; i++)
			{
				for (int j = 0; j < data.height; j++)
				{
					double I = data.pImg[i * data.width + j];
					double X = pi * 2 * (static_cast<double>(i)*u / static_cast<double>(data.width) +
						static_cast<double>(j)*v / static_cast<double>(data.height));

					real += std::cos(X)*I;
					imagin += -std::sin(X)*I;
				}
			}


			dest[u * data.width + v].real(real);
			dest[u * data.width + v].imag(imagin);
		}
	}
	return dest;
}

void ImageUtil::FFT::nomalisize(double* dest, ImageData& data)
{
	double max = 0, min = DBL_MAX;
	for (int i = 0; i < data.width * data.height; i++)
	{
		max = dest[i] > max ? dest[i] : max;
		min = dest[i] < min ? dest[i] : min;
	}

	const double step = static_cast<double>(255) / (max - min);
	for (int i = 0; i < data.width * data.height; i++)
	{
		dest[i] = clamp(dest[i] * 0.01);
		//dest[i] = (dest[i] - min) * step;
		//dest[i] *=45.9 * std::log(static_cast<double>(1 + dest[i]));
		//std::cout << dest[i] << std::endl;
	}

	//shift(dest, data.width, data.height);

	byte* newImg = new byte[data.width * data.height];
	for (int i = 0; i < data.width * data.height; i++)
	{
		newImg[i] = dest[i];
	}

	data.pImg = newImg;
	outputImage(data, "bitmap/ftt.bmp");

}

void ImageUtil::FFT::getAmpgetAmplitudespectrumlitu(std::complex<double>* dest, ImageData data)
{
	double *d = new double[data.width * data.height];

	for (int i = 0; i < data.height; i++)
	{
		for (int j = 0; j < data.width; j++)
		{
			const double real = dest[i * data.width + j].real();
			const double imagin = dest[i * data.width + j].imag();
			d[i * data.width + j] = std::sqrt(real * real + imagin * imagin);
		}
	}

	nomalisize(d, data);

	delete[] d;
}

void ImageUtil::FFT::fttRealRemap(BYTE* src, int size)
{
	if (size == 1)
		return;

	double *temp = new double[size];
	for (int i = 0; i < size; i++)
	{
		if (i % 2 == 0)
			temp[i / 2] = src[i];
		else
			temp[(size + i) / 2] = src[i];
	}

	for (int i = 0; i < size; i++)
	{
		src[i] = temp[i];
	}

	delete[] temp;

	fttRealRemap(src, size / 2);
	fttRealRemap(src + size / 2, size / 2);
}

void ImageUtil::FFT::fttComplexRemap(std::complex<double>* src, int length)
{
	if (length == 1)
		return;
	std::complex<double> *temp = new std::complex<double>[length];
	for(int i = 0;i < length;i++)
	{
		if (i % 2 == 0)
			temp[i / 2] = src[i];
		else
			temp[(length + i) / 2] = src[i];
	}

	for(int i =0;i < length;i++)
	{
		src[i] = temp[i];
	}

	delete[] temp;

	fttComplexRemap(src, length / 2);
	fttComplexRemap(src + (length / 2), length / 2);
}

bool ImageUtil::FFT::isBase2(const int size)
{
	int k = size;
	int z = 0;
	while (k /= 2)
		z++;
	k = z;

	return size == (1 << k);
}

void ImageUtil::FFT::shift(double* dest, const int width, const int height)
{
	for(int i = 0;i<height;i++)
	{
		for(int j =0;j < width;j++)
		{
			if ((i + j) % 2 == 0)
			{
				dest[i * width + j] = -dest[i * width + j];
			}
		}
	}
}

void ImageUtil::FFT::fft2d(ImageData& data)
{
	if (!isBase2(data.width) || !isBase2(data.height))
		return;

	const int size = data.width * data.height;

	double *imgData = new double[size];
	for(int i =0;i < size;i++)
	{
		imgData[i] = data.pImg[i];
	}

	shift(imgData, data.width, data.height);

	std::complex<double>** temp = new std::complex<double>*[data.height];
	std::complex<double> *real = new std::complex<double>[data.width];
	for(int i = 0;i < data.height;i++)
	{
		for(int j = 0;j < data.width;j++)
		{
			real[j].real(imgData[data.width * i + j]);
			real[j].imag(0);
		}
		temp[i] = complexFtt(real, data.width);
	}

	std::complex<double>* colunm = new std::complex<double>[data.height];
	for (int i = 0; i < data.width; i++)
	{
		for (int j = 0; j < data.height; j++)
			colunm[j] = temp[j][i];
		colunm = complexFtt(colunm, data.height);
		for (int j = 0; j < data.height; j++)
			temp[j][i] = colunm[j];
	}


	std::complex<double> *result = new std::complex<double>[data.width * data.height];
	for(int i = 0;i < data.height;i++)
	{
		for(int j = 0;j < data.width;j++)
		{
			result[i * data.width + j] = temp[i][j];
		}
	}

	getAmpgetAmplitudespectrumlitu(result, data);

	for(int i = 0; i<data.height;i++)
	{
		delete[] temp[i];
	}

}

std::complex<double>* ImageUtil::FFT::realFtt(double* src, int size)
{

	int k = size;
	int z = 0;
	while (k /= 2)
		z++;

	k = z;
	if (size != (1<<k))
		return nullptr;

	std::complex<double> *srcComplex = new std::complex<double>[size];
	for (int i = 0; i < size; i++)
	{
		srcComplex[i].real(static_cast<double>(src[i]));
		srcComplex[i].imag(0);
		//std::cout << src[i] << std::endl;
	}

	fttComplexRemap(srcComplex, size);
	for (int i = 0; i < k; i++)
	{
		z = 0;
		for (int j = 0; j < size; j++)
		{
			if ((j / (1 << i)) % 2 == 1)
			{
				std::complex<double> wn;
				getWN(z, size, wn);
				srcComplex[j] = wn * srcComplex[j];
				z += 1 << (k - i - 1);
				std::complex<double> temp;
				const int neighbour = j - (1 << i);
				temp.real(srcComplex[neighbour].real());
				temp.imag(srcComplex[neighbour].imag());
				srcComplex[neighbour] = temp + srcComplex[j];
				srcComplex[j] = temp - srcComplex[j];
				//std::cout << srcComplex[neighbour].real() << " " << srcComplex[neighbour].imag() << " " <<  j + i * size << std::endl;
			}
			else
				z = 0;
		}
	}


	return srcComplex;
}

void ImageUtil::FFT::getWN(double n, double size, std::complex<double>& dst)
{
	const double x = 2.0 * pi * n / size;
	dst.real(std::cos(x));
	dst.imag(-std::sin(x));
}

std::complex<double> * ImageUtil::FFT::complexFtt(std::complex<double>* dst,int size)
{
	int k = size;
	int z = 0;
	while (k /= 2)
		z++;

	k = z;
	if (size != (1 << k))
		return nullptr;

	std::complex<double> *srcComplex = new std::complex<double>[size];
	for(int i= 0;i < size;i++)
	{
		srcComplex[i] = dst[i];
	}

	//fttComplexRemap(dst, size);

	fttComplexRemap(srcComplex, size);
	for (int i = 0; i < k; i++)
	{
		z = 0;
		for (int j = 0; j < size; j++)
		{
			if ((j / (1 << i)) % 2 == 1)
			{
				std::complex<double> wn;
				getWN(z, size, wn);
				srcComplex[j] = wn * srcComplex[j];
				z += 1 << (k - i - 1);
				std::complex<double> temp;
				const int neighbour = j - (1 << i);
				temp.real(srcComplex[neighbour].real());
				temp.imag(srcComplex[neighbour].imag());
				srcComplex[neighbour] = temp + srcComplex[j];
				srcComplex[j] = temp - srcComplex[j];
				//std::cout << srcComplex[neighbour].real() << " " << srcComplex[neighbour].imag() << " " <<  j + i * size << std::endl;
			}
			else
				z = 0;
		}
	}

	return srcComplex;
}
