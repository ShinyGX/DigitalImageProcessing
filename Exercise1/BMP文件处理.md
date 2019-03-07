<script type="text/javascript" src="http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=default"></script>
# BMP文件处理

[TOC]



## BITMAP位图

在进行BITMAP图片进行处理的时候，首先要先对BITMAP的数据结构进行分析。

BITMAP位图的结构分为四个部分

### 文件头 BITMAPFILEHEADER

```c++
typedef struct tagBITMAPFILEHEADER{
WORD   	bfType; //文件类型，必须是“BM”
DWORD	bfSize; //文件大小，包含文件头的大小
WORD	bfReserved1; //保留字
WORD	bfReserved2; //保留字
DWORD	bfOffBits; //从文件头到实际位图数据的偏移字节数                                     
} BITMAPFILEHEADER; 

sizeof(BITMAPFILEHEADER)  = 14

```

### 信息头 BITMAPINFOHEADER

```c++
typedef struct tagBITMAPINFOHEADER{
DWORD   biSize; //该结构的长度，为40
LONG	biWidth; //图像宽度
LONG	biHeight; //图像高度
WORD	biPlanes; //位平面数，必须为1
WORD	biBitCount; //颜色位数，
DWORD   biCompression; //是否压缩
DWORD   biSizeImage; //实际位图数据占用的字节数
LONG	biXPelsPerMeter;//目标设备水平分辨率
LONG	biYPelsPerMeter;//目标设备垂直分辨率
DWORD   biClrUsed;//实际使用的颜色数
DWORD   biClrImportant;//图像中重要的颜色数
} BITMAPINFOHEADER; 

sizeof(BITMAPINFOHEADER) = 40

```

在这当中，若biClrUsed为0，则说明用到的颜色为2的biBitCount次方（也就无需用到调色板了）

在这当中，biSizeImage为实际的字节数，其中，要求
$$
biSizeImage = （（biWidth * （biBitCount / 8） + 3） / 4 * 4 ) * biHeight
$$
也就是说，在位图当中的宽的字节数一定要是4的倍数，不是4的倍数将要对其中的字节进行补齐

举个例子，假如当前的图片是一个24位的宽度为13的图片，那么此时一行的存储模型应该是
$$
13 * （BGR) + 1 * BYTE
$$
也就是在末尾补了一个字节，从而达到了一行40字节（4的倍数）的目的。

### 调色板 RGBQUAD（内容可为空)

```c++
typedef struct tagRGBQUAD{
  BYTE  rgbBlue;   	 //该颜色的蓝色分量
  BYTE  rgbGreen;	 //该颜色的绿色分量
  BYTE  rgbRed;		 //该颜色的红色分量
  BYTE  rgbReserved;	 //保留值
} RGBQUAD; 

sizeof(RGBQUAD) = 4
```

调色板是BITMAP当中的可选项，只有需要的时候才会出现，而在有调色板的场合中，一共有biBitCount项 (RGBQUAD[biBitCount])

### 实际位图信息

在这里存放的是实际的位图的数据，其中数据量在BITMAPINFOHEADER有记录，在这里记录的一般是颜色的信息（比如RGB信息）又或者是有调色板的时候，记录调色板的索引值。

当然值得一提的是，这里的行的字节数一定是要4的倍数的

## 24位图像对R，G，B三个分量进行分离，产生3幅新的图像

在基本了解了BITMAP的内存结构之后，就可以根据其中的信息去读取图片，并分理出其中的RGB分量了。值得注意的在于，若图片的宽度不是4的倍数的时候，读取的时候要跳过行末尾的无意义的字节，否则读取出来的rgb值是错误的，而写入成新的图片的时候，同样的，要手动对行末尾进行补齐

### 具体实现

```c++
//用于存储RGB
typedef struct
{
	BYTE b;
	BYTE g;
	BYTE r;
}RGB;

void bitmapTo3SignalColorBitmap()
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	FILE * pfin = fopen("bitmap/n4x.bmp", "rb");
	FILE * pfoutR = fopen("bitmap/r.bmp", "wb");
	FILE * pfoutG = fopen("bitmap/g.bmp", "wb");
	FILE * pfoutB = fopen("bitmap/b.bmp", "wb");


	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pfin);
	fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pfin);

	int height = infoHeader.biHeight, width = infoHeader.biWidth;
    //方便，偷懒的做法（只处理24位）
	if (infoHeader.biBitCount == 24)
	{
        
        //获取补齐后的宽度
		int byteWidth = (width * infoHeader.biBitCount / 8 + 3) / 4 * 4;
		int size = byteWidth * height;

		BYTE *img = new BYTE[size];
		RGB *imgRGB = new RGB[width * height];

		fseek(pfin, fileHeader.bfOffBits, 0);
		fread(img, sizeof(BYTE), size, pfin);
        
        //跳过无用的字节
		int point = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				imgRGB[i * width + j].b = img[point++];
				imgRGB[i * width + j].g = img[point++];
				imgRGB[i * width + j].r = img[point++];
			}

			while (point % 4 != 0)
				point++;
		}

		writeImg(&fileHeader, &infoHeader,
			toByte(imgRGB, width, height, infoHeader.biSizeImage, RGBTAG::R),
			infoHeader.biSizeImage,
			pfoutR);

		writeImg(&fileHeader, &infoHeader,
			toByte(imgRGB, width, height, infoHeader.biSizeImage, RGBTAG::G),
			infoHeader.biSizeImage,
			pfoutG);

		writeImg(&fileHeader, &infoHeader,
			toByte(imgRGB, width, height, infoHeader.biSizeImage, RGBTAG::B),
			infoHeader.biSizeImage,
			pfoutB);

	}

	fclose(pfin);
	fclose(pfoutR);
	fclose(pfoutG);
	fclose(pfoutB);
}

//写入到文件当中
void writeImg(BITMAPFILEHEADER *header,BITMAPINFOHEADER *info,BYTE *rgb,int size,FILE * pf)
{
	fwrite(header, sizeof (BITMAPFILEHEADER), 1, pf);
	fwrite(info, sizeof (BITMAPINFOHEADER), 1, pf);
	fwrite(rgb, sizeof(BYTE), size, pf);
}

//将RGB格式转化为byte（注意RGB的排列方式），以及补齐无用字节
BYTE* toByte(RGB *rgb,int width,int height,int biSize,RGBTAG tag)
{
	BYTE *byte = new BYTE[biSize];
	int point = 0;
	for(int i = 0;i < height;i++)
	{
		for(int j = 0;j < width;j++)
		{
			switch (tag)
			{
			case R:
				byte[point++] = 0;
				byte[point++] = 0;
				byte[point++] = rgb[i * width + j].r;
				break;
			case G:
				byte[point++] = 0;
				byte[point++] = rgb[i * width + j].g;
				byte[point++] = 0;
				break;
			case B:
				byte[point++] = rgb[i * width + j].b;
				byte[point++] = 0;
				byte[point++] = 0;
				break;
			case ALL:
				byte[point++] = rgb[i * width + j].r;
				byte[point++] = rgb[i * width + j].g;
				byte[point++] = rgb[i * width + j].b;
				break;
			}		
			
		}

		while (point % 4 != 0)
			byte[point++] = 0;
	}

	return byte;
}
```



## 对24彩色图像灰度化

灰度图实际上是一个利用心理学灰度公式来得出的一个适合人眼的灰度图片的图
$$
GRAY = B * 0.114 + R * 0.299 + G * 0.587
$$
在这个图片当中，由于RGB三个分量的值都是一样的（灰色的特性）因此，可以将这个图片从24位图变成一个8位的图

当我们将一个图片从24位转化为8位的时候，有几点是值得注意的

### 改变BITMAPFILEHEADER与BITMAPINFOHEADER

这是当然的，首先要改的就是BITMAPINFOHEADER当中的biBitCount了，毕竟这是标志了这是几位图的信息，其次，还要添加biClrUsed的信息，因为，当图片是8位图的时候，自然而然的就要用到了调色板了。

既然添加了调色板，那么就需要改变BITMAPFILEHEADER当中的biOffBits了，毕竟当中添加了一个调色板，那么图片数据的位置自然会产生变化了

最后，还要分别对BITMAPFILEHEADER当中的bfSize与BITMAPINFOHEADER当中的biSizeImage进行修改，从而使得这些数据指向的内容是符合这张图片的

### 添加调色板

8位图用到的颜色只有256位，因此，添加256个调色板就足够了

```c++
	RGBQUAD rgbQuad[256];
	for(int i = 0;i < 256;i++)
	{
		rgbQuad[i].rgbRed = i;
		rgbQuad[i].rgbGreen = i;
		rgbQuad[i].rgbBlue = i;
		rgbQuad[i].rgbReserved = 0;

	}
```

### 具体实现

```c++
void bitmap2Gray()
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	FILE * pfin = fopen("bitmap/n4x.bmp", "rb");
	FILE * pfout = fopen("bitmap/gray.bmp", "wb");
	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pfin);
	fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pfin);

	int height = infoHeader.biHeight, width = infoHeader.biWidth;
	if (infoHeader.biBitCount == 24)
	{
		int byteWidth = (width * infoHeader.biBitCount / 8 + 3) / 4 * 4;
		int size = byteWidth * height;

		BYTE *img = new BYTE[size];
		RGB *imgRGB = new RGB[width * height];

		fseek(pfin, fileHeader.bfOffBits, 0);
		fread(img, sizeof(BYTE), size, pfin);
        
        //读取信息
		int point = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				imgRGB[i * width + j].b = img[point++];
				imgRGB[i * width + j].g = img[point++];
				imgRGB[i * width + j].r = img[point++];
			}

			while (point % 4 != 0)
				point++;
		}

		infoHeader.biBitCount = 8;
		infoHeader.biClrUsed = 256;
		
        //建立调色板
		RGBQUAD rgbQuad[256];
		for(int i = 0;i < 256;i++)
		{
			rgbQuad[i].rgbRed = i;
			rgbQuad[i].rgbGreen = i;
			rgbQuad[i].rgbBlue = i;
			rgbQuad[i].rgbReserved = 0;

		}

		fileHeader.bfOffBits = 54 + 4 * 256;

		int byteLine = (width * infoHeader.biBitCount / 8  + 3) / 4 * 4;
		BYTE *newIMG = new BYTE[byteLine * height];
		infoHeader.biSizeImage = byteLine * height;
		fileHeader.bfSize = 54 + byteLine * height + 4 * 256;
		
		//重新写回BMP当中
		point = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				newIMG[point++] = imgRGB[i * width + j].b * 0.114 + 
					imgRGB[i * width + j].g * 0.587 + imgRGB[i * width + j].r * 0.299;

			}

			while (point % 4 != 0)
				newIMG[point++] = 0;
		}

		fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pfout);
		fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pfout);
		fwrite(&rgbQuad, 4 * 256, 1, pfout);
		fwrite(newIMG, sizeof(BYTE), byteLine * height, pfout);

	}

	fclose(pfin);
	fclose(pfout);
}
```



## 对8位灰度图进行反色

对图片进行反色是十分简单的一件事了，公式也十分的简单
$$
antiColor = abs(color - 255)
$$
具体实现和之前的是基本一致的。

当然，由于有灰度图（8位图）有调色板的存在，因此，需要在读取之前跳过调色板的内存块

```c++
fseek(pfin, fileHeader.bfOffBits, 0);
```



### 具体实现

```c++
void gray2Anticolor()
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	FILE * pfin = fopen("bitmap/gray.bmp", "rb");
	FILE * pfout = fopen("bitmap/anti_color.bmp", "wb");
	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pfin);
	fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pfin);

	int height = infoHeader.biHeight, width = infoHeader.biWidth;
	if (infoHeader.biBitCount == 8)
	{
		int byteWidth = (width * infoHeader.biBitCount / 8 + 3) / 4 * 4;
		int size = byteWidth * height;

		BYTE *img = new BYTE[size];
		BYTE *imgAnticolor = new BYTE[size];

		RGBQUAD rgbquad[256];

		fread(rgbquad, sizeof(RGBQUAD), 256, pfin);
		fread(img, sizeof(BYTE), size, pfin);
		int point1 =0, point = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				imgAnticolor[point++] = std::abs(img[point1++] - static_cast<byte>(255));
			}

			while (point % 4 != 0)
			{
				imgAnticolor[point++] = 0;
				point1++;
			}
				
		}

		fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, pfout);
		fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pfout);
		fwrite(&rgbquad, 4 * 256, 1, pfout);
		fwrite(imgAnticolor, sizeof(BYTE), size, pfout);
		
	}

	fclose(pfin);
	fclose(pfout);
}

```

