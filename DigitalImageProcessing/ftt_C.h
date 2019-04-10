#pragma once
#include <crtdbg.h>
#include <stdio.h>
#include "Math.h"
#include <datetimeapi.h>

struct Complex_ {
	double real;
	double imagin;
};
typedef struct Complex_ Complex;
////////////////////////////////////////////////////////////////////
//定义一个复数计算，包括乘法，加法，减法
///////////////////////////////////////////////////////////////////
void Add_Complex(Complex * src1, Complex *src2, Complex *dst) {
	dst->imagin = src1->imagin + src2->imagin;
	dst->real = src1->real + src2->real;
}
void Sub_Complex(Complex * src1, Complex *src2, Complex *dst) {
	dst->imagin = src1->imagin - src2->imagin;
	dst->real = src1->real - src2->real;
}
void Multy_Complex(Complex * src1, Complex *src2, Complex *dst) {
	double r1 = 0.0, r2 = 0.0;
	double i1 = 0.0, i2 = 0.0;
	r1 = src1->real;
	r2 = src2->real;
	i1 = src1->imagin;
	i2 = src2->imagin;
	dst->imagin = r1 * i2 + r2 * i1;
	dst->real = r1 * r2 - i1 * i2;
}

int isBase2(int size_n) {
	int k = size_n;
	int z = 0;
	while (k /= 2) {
		z++;
	}
	k = z;
	if (size_n != (1 << k))
		return -1;
	else
		return k;
}

void Copy_Complex(Complex * src, Complex *dst) {
	dst->real = src->real;
	dst->imagin = src->imagin;
}
void Show_Complex(Complex * src, int size_n) {
	if (size_n == 1) {
		if (src->imagin >= 0.0)
			printf("%lf+%lfj  ", src->real, src->imagin);
		else
			printf("%lf%lfj  ", src->real, src->imagin);

	}
	else if (size_n > 1) {
		for (int i = 0; i < size_n; i++)
			if (src[i].imagin >= 0.0) {
				printf("%lf+%lfj  ", src[i].real, src[i].imagin);
			}
			else
				printf("%lf%lfj  ", src[i].real, src[i].imagin);



	}


}
////////////////////////////////////////////////////////////////////
//计算WN
///////////////////////////////////////////////////////////////////
void getWN(double n, double size_n, Complex * dst) {
	double x = 2.0* ImageUtil::pi*n / size_n;
	dst->imagin = -sin(x);
	dst->real = cos(x);
}

////////////////////////////////////////////////////////////////////
//标准DFT
///////////////////////////////////////////////////////////////////
void DFT(double * src, Complex * dst, int size) {

	for (int m = 0; m < size; m++) {
		double real = 0.0;
		double imagin = 0.0;
		for (int n = 0; n < size; n++) {
			double x = ImageUtil::pi * 2 * m*n;
			real += src[n] * cos(x / size);
			imagin += src[n] * (-sin(x / size));

		}
		dst[m].imagin = imagin;
		dst[m].real = real;

	}
}

////////////////////////////////////////////////////////////////////
//FFT前，对输入数据重新排序
///////////////////////////////////////////////////////////////////
int FFTReal_remap(double * src, int size_n) {

	if (size_n == 1)
		return 0;
	double * temp = (double *)malloc(sizeof(double)*size_n);
	for (int i = 0; i < size_n; i++)
		if (i % 2 == 0)
			temp[i / 2] = src[i];
		else
			temp[(size_n + i) / 2] = src[i];
	for (int i = 0; i < size_n; i++)
		src[i] = temp[i];
	free(temp);
	FFTReal_remap(src, size_n / 2);
	FFTReal_remap(src + size_n / 2, size_n / 2);
	return 1;


}
int FFTComplex_remap(Complex * src, int size_n) {

	if (size_n == 1)
		return 0;
	Complex * temp = (Complex *)malloc(sizeof(Complex)*size_n);
	for (int i = 0; i < size_n; i++)
		if (i % 2 == 0)
			Copy_Complex(&src[i], &(temp[i / 2]));
		else
			Copy_Complex(&(src[i]), &(temp[(size_n + i) / 2]));
	for (int i = 0; i < size_n; i++)
		Copy_Complex(&(temp[i]), &(src[i]));
	free(temp);
	FFTComplex_remap(src, size_n / 2);
	FFTComplex_remap(src + size_n / 2, size_n / 2);
	return 1;


}
////////////////////////////////////////////////////////////////////
//FFT公式
///////////////////////////////////////////////////////////////////
void FFT(Complex * src, Complex * dst, int size_n) {

	int k = size_n;
	int z = 0;
	while (k /= 2) {
		z++;
	}
	k = z;
	if (size_n != (1 << k))
		exit(0);
	Complex * src_com = (Complex*)malloc(sizeof(Complex)*size_n);
	if (src_com == NULL)
		exit(0);
	for (int i = 0; i < size_n; i++) {
		Copy_Complex(&src[i], &src_com[i]);
	}
	FFTComplex_remap(src_com, size_n);
	for (int i = 0; i < k; i++) {
		z = 0;
		for (int j = 0; j < size_n; j++) {
			if ((j / (1 << i)) % 2 == 1) {
				Complex wn;
				getWN(z, size_n, &wn);
				Multy_Complex(&src_com[j], &wn, &src_com[j]);
				z += 1 << (k - i - 1);
				Complex temp;
				int neighbour = j - (1 << (i));
				temp.real = src_com[neighbour].real;
				temp.imagin = src_com[neighbour].imagin;
				Add_Complex(&temp, &src_com[j], &src_com[neighbour]);
				Sub_Complex(&temp, &src_com[j], &src_com[j]);
			}
			else
				z = 0;
		}

	}


	for (int i = 0; i < size_n; i++) {
		Copy_Complex(&src_com[i], &dst[i]);
	}
	free(src_com);


}

void RealFFT(double * src, Complex * dst, int size_n) {


	int k = size_n;
	int z = 0;
	while (k /= 2) {
		z++;
	}
	k = z;
	if (size_n != (1 << k))
		exit(0);
	Complex * src_com = (Complex*)malloc(sizeof(Complex)*size_n);
	if (src_com == NULL)
		exit(0);
	for (int i = 0; i < size_n; i++) {
		src_com[i].real = src[i];
		src_com[i].imagin = 0;
	}
	FFTComplex_remap(src_com, size_n);
	for (int i = 0; i < k; i++) {
		z = 0;
		for (int j = 0; j < size_n; j++) {
			if ((j / (1 << i)) % 2 == 1) {
				Complex wn;
				getWN(z, size_n, &wn);
				Multy_Complex(&src_com[j], &wn, &src_com[j]);
				z += 1 << (k - i - 1);
				Complex temp;
				int neighbour = j - (1 << (i));
				temp.real = src_com[neighbour].real;
				temp.imagin = src_com[neighbour].imagin;
				Add_Complex(&temp, &src_com[j], &src_com[neighbour]);
				Sub_Complex(&temp, &src_com[j], &src_com[j]);
			}
			else
				z = 0;
		}

	}


	for (int i = 0; i < size_n; i++) {
		Copy_Complex(&src_com[i], &dst[i]);
	}
	free(src_com);

}

////////////////////////////////////////////////////////////////////
//IFFT实现
////////////////////////////////////////////////////////////////////
void IFFT(Complex * src, Complex * dst, int size_n) {
	for (int i = 0; i < size_n; i++)
		src[i].imagin = -src[i].imagin;
	FFTComplex_remap(src, size_n);
	int z, k;
	if ((z = isBase2(size_n)) != -1)
		k = isBase2(size_n);
	else
		exit(0);
	for (int i = 0; i < k; i++) {
		z = 0;
		for (int j = 0; j < size_n; j++) {
			if ((j / (1 << i)) % 2 == 1) {
				Complex wn;
				getWN(z, size_n, &wn);
				Multy_Complex(&src[j], &wn, &src[j]);
				z += 1 << (k - i - 1);
				Complex temp;
				int neighbour = j - (1 << (i));
				Copy_Complex(&src[neighbour], &temp);
				Add_Complex(&temp, &src[j], &src[neighbour]);
				Sub_Complex(&temp, &src[j], &src[j]);
			}
			else
				z = 0;
		}

	}
	for (int i = 0; i < size_n; i++) {
		dst[i].imagin = (1. / size_n)*src[i].imagin;
		dst[i].real = (1. / size_n)*src[i].real;
	}
}

void FFT_Shift(double * src, int size_w, int size_h) {
	for (int j = 0; j < size_h; j++)
		for (int i = 0; i < size_w; i++) {
			if ((i + j) % 2)
				src[j*size_w + i] = -src[j*size_w + i];

		}

}

/*
 将幅度值伸缩到0到255，用于频谱显示
 */
void Nomalsize(double *src, double *dst, int size_w, int size_h) {
	double  max = 0.0, min = DBL_MAX;
	for (int i = 0; i < size_w*size_h; i++) {
		max = src[i] > max ? src[i] : max;
		min = src[i] < min ? src[i] : min;
	}
	double step = 255.0 / (max - min);
	//printf("%d",test);
	//printf("max:%lf   min:%lf\n",max,min);
	for (int i = 0; i < size_w*size_h; i++) {
		dst[i] = (src[i] - min)*step;
		dst[i] *= 45.9*log((double)(1 + dst[i]));
	}

}
/*
 获得频谱
 */
double* getAmplitudespectrum(Complex * src, int size_w, int size_h, BYTE *dst) {
	double *despe = (double *)malloc(sizeof(double)*size_w*size_h);
	if (despe == NULL)
		exit(0);
	double real = 0.0;
	double imagin = 0.0;

	for (int j = 0; j < size_h; j++)
		for (int i = 0; i < size_w; i++) {
			real = src[j*size_w + i].real;
			imagin = src[j*size_w + i].imagin;
			despe[j*size_w + i] = sqrt(real*real + imagin * imagin);

		}
	Nomalsize(despe, despe, size_w, size_h);
	
	return despe;

}
void ColumnVector(Complex * src, Complex * dst, int size_w, int size_h) {
	for (int i = 0; i < size_h; i++)
		Copy_Complex(&src[size_w*i], &dst[i]);

}
/*

 */
void IColumnVector(Complex * src, Complex * dst, int size_w, int size_h) {
	for (int i = 0; i < size_h; i++)
		Copy_Complex(&src[i], &dst[size_w*i]);

}
/*
 */
int FFT2D(double *src, Complex *dst, int size_w, int size_h) {
	if (isBase2(size_w) == -1 || isBase2(size_h) == -1)
		exit(0);
	Complex *temp = (Complex *)malloc(sizeof(Complex)*size_h*size_w);
	if (temp == NULL)
		return -1;
	for (int i = 0; i < size_h; i++) {
		RealFFT(&src[size_w*i], &temp[size_w*i], size_w);
	}

	Complex *Column = (Complex *)malloc(sizeof(Complex)*size_h);
	if (Column == NULL)
		return -1;
	for (int i = 0; i < size_w; i++) {
		ColumnVector(&temp[i], Column, size_w, size_h);
		FFT(Column, Column, size_h);
		IColumnVector(Column, &temp[i], size_w, size_h);

	}

	for (int i = 0; i < size_h*size_w; i++)
		Copy_Complex(&temp[i], &dst[i]);
	free(temp);
	free(Column);
	return 0;
}

/*
 图像快速傅里叶变换，图像大小为2的N次幂
 */
void ImageFFT(BYTE * src, Complex * dst, int width, int height) {

	double *image_data = (double*)malloc(sizeof(double)*width*height);
	for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++) {
			image_data[j*width + i] = src[j*width + i];

		}
	FFT_Shift(image_data, width, height);//图像中心化
	FFT2D(image_data, dst, width, height);
	free(image_data);

}