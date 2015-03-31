#include "stdafx.h"
#include "image_process.h"


//构造函数
imageProcess::imageProcess(const char* path)
{
	image = cvLoadImage(path);
	if (image==NULL)
	{
		throw runtime_error("Initialise image failed!!");
	}
}

//析构函数
imageProcess::~imageProcess()
{
	if (image!=NULL)
	{
		cvReleaseImage(&image);
	}

	delete polygone;
	
}

bool imageProcess::getPolygone()
{
	bool isGetPolygone = false;
	if (image==NULL)
	{
		throw runtime_error("invalid image!!");
		return isGetPolygone;
	}

	//调整图片大小
	float scaleImage = image->width / STANDARD_WIDTH;
	resizeImage = cvCreateImage(cvSize(STANDARD_WIDTH, (int)(image->height / scaleImage)), image->depth, image->nChannels);
	cvResize(image, resizeImage, CV_INTER_LINEAR);
	cvSaveImage("resizeImage.jpg", resizeImage, NULL);

	//获取所有轮廓
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* sequares = findSquares(storage, resizeImage, 50000, 1000);//52500

	if (sequares == 0)
	{
		throw runtime_error("find no contous in the image");
		return isGetPolygone;
	}

	//////////////////////////////////////////////////////////////////////////
	//读取轮廓序列
	CvSeqReader reader;
	CvPoint pt[4];
	cvStartReadSeq(sequares, &reader, 0);

	static bool GetRightRect = false;

	//read 4 sequeue elements at a time(all vertices of a square)
	for (int i = 0; i < sequares->total; i += 4)
	{
		//read 4 vertices
		memcpy(pt, reader.ptr, sequares->elem_size);
		CV_NEXT_SEQ_ELEM(sequares->elem_size, reader);
		memcpy(pt + 1, reader.ptr, sequares->elem_size);
		CV_NEXT_SEQ_ELEM(sequares->elem_size, reader);
		memcpy(pt + 2, reader.ptr, sequares->elem_size);
		CV_NEXT_SEQ_ELEM(sequares->elem_size, reader);
		memcpy(pt + 3, reader.ptr, sequares->elem_size);
		CV_NEXT_SEQ_ELEM(sequares->elem_size, reader);

		if ((pt[2].x >= (resizeImage->width - 10)) || (pt[2].y >= (resizeImage->height - 10)) || (pt[0].x <= 10) || (pt[0].y <= 10))
		{
			GetRightRect = false;
			continue;
		}
		else
		{
			//SqCnt++;
			GetRightRect = true;
			break;
		}
	}

	if (GetRightRect)
	{
		//////////////////////////////////////////////////////////////////////////
		//将轮廓数据转换为多边形数据
		CvPoint2D32f Square[EDGE];
		Line2D32f SquareLine[EDGE];

		for (int j = 0; j < EDGE; j++)
		{
			Square[j].x = pt[j].x;
			Square[j].y = pt[j].y;
		}

		for (int j = 0; j < EDGE; j++)
		{
			SquareLine[j].pt1 = Square[j];
			SquareLine[j].pt2 = Square[(j + 1) % EDGE];
		}

		for (int j = 0; j < EDGE; j++)
		{
			polygone->L[j].pt1 = SquareLine[j].pt1;
			polygone->L[j].pt2 = SquareLine[j].pt2;
		}
		polygone->EdgeCnt = EDGE;
		isGetPolygone = true;
	}
	cvReleaseMemStorage(&storage);
	return isGetPolygone;
}

CvSeq*imageProcess:: findSquares(CvMemStorage* storage, IplImage * image, double CannyThresh, double AreaThresh)
{
	CvSeq* contours;
	int i, c, l, N = 11;
	CvSize sz = cvSize(image->width & -2, image->height & -2);
	IplImage* timg = cvCloneImage(image); // make a copy of input image   
	IplImage* gray = cvCreateImage(sz, 8, 1);
	IplImage* pyr = cvCreateImage(cvSize(sz.width / 2, sz.height / 2), 8, 3);
	IplImage* tgray;
	CvSeq* result;
	double s, t;
	// create empty sequence that will contain points -   
	// 4 points per square (the square's vertices)   
	CvSeq* squares = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvPoint), storage);

	// select the maximum ROI in the image   
	// with the width and height divisible by 
	cvSetImageROI(timg, cvRect(0, 0, sz.width, sz.height));

	// down-scale and upscale the image to filter out the noise   
	//使用gaussian金字塔分解对输入图像向下采样，首先对它输入的图像用指定滤波器   
	//进行卷积，然后通过拒绝偶数的行与列来下采样   
	cvPyrDown(timg, pyr, CV_GAUSSIAN_5x5);

	//函数 cvPyrUp 使用Gaussian 金字塔分解对输入图像向上采样。首先通过在图像中插入 0 偶数行和偶数列，然后对得到的图像用指定的滤波器进行高斯卷积，其中滤波器乘以4做插值。所以输出图像是输入图像的 4 倍大小。   
	cvPyrUp(pyr, timg, CV_GAUSSIAN_5x5);

	tgray = cvCreateImage(sz, 8, 1);

	// find squares in every color plane of the image   
	for (c = 0; c < 3; c++)
	{
		// extract the c-th color plane   
		//函数 cvSetImageCOI 基于给定的值设置感兴趣的通道。值 0 意味着所有的通道都被选定, 1 意味着第一个通道被选定等等。   
		cvSetImageCOI(timg, c + 1);
		cvCopy(timg, tgray, 0);

		// try several threshold levels   
		for (l = 0; l < N; l++)
		{
			// hack: use Canny instead of zero threshold level.   
			// Canny helps to catch squares with gradient shading      
			if (l == 0)
			{
				// apply Canny. Take the upper threshold from slider   
				// and set the lower to 0 (which forces edges merging)    
				cvCanny(tgray, gray, CannyThresh, CannyThresh * 3, 3);

				// dilate canny output to remove potential   
				// holes between edge segments    
				//使用任意结构元素膨胀图像   
				cvDilate(gray, gray, 0, 1);
			}
			else
			{
				// apply threshold if l!=0:   
				//        tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0   
				cvThreshold(tgray, gray, (l + 1) * 255 / N, 255, CV_THRESH_BINARY);
				//cvThreshold( tgray, gray, 50, 255, CV_THRESH_BINARY );
				//cvThreshold( tgray, gray, thresh, 255, CV_THRESH_BINARY );
			}

			// find contours and store them all as a list   
			cvFindContours(gray, storage, &contours, sizeof(CvContour),
				CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
		
			// test each contour   
			while (contours)
			{
				// approximate contour with accuracy proportional   
				// to the contour perimeter   
				//用指定精度逼近多边形曲线  
				result = cvApproxPoly(contours, sizeof(CvContour), storage,
					CV_POLY_APPROX_DP, cvContourPerimeter(contours) * 0.02, 0);
				// square contours should have 4 vertices after approximation   
				// relatively large area (to filter out noisy contours)   
				// and be convex.   
				// Note: absolute value of an area is used because   
				// area may be positive or negative - in accordance with the   
				// contour orientation   
				//cvContourArea 计算整个轮廓或部分轮廓的面积   
				//cvCheckContourConvexity测试轮廓的凸性   

				if (result->total == 4 &&
					fabs(cvContourArea(result, CV_WHOLE_SEQ)) > AreaThresh &&
					cvCheckContourConvexity(result))
				{

					s = 0;

					for (i = 0; i < 5; i++)
					{
						// find minimum angle between joint   
						// edges (maximum of cosine)   
						if (i >= 2)
						{
							t = fabs(angle(
								(CvPoint*)cvGetSeqElem(result, i),
								(CvPoint*)cvGetSeqElem(result, i - 2),
								(CvPoint*)cvGetSeqElem(result, i - 1)));

							s = s > t ? s : t;
						}
					}

					// if cosines of all angles are small   
					// (all angles are ~90 degree) then write quandrange   
					// vertices to resultant sequence    
					if (s < 0.3)//选择四边形的最小角度   
					{
						for (i = 0; i < 4; i++)
						{
							cvSeqPush(squares, (CvPoint*)cvGetSeqElem(result, i));
						}
					}
				}

				// take the next contour   
				contours = contours->h_next;
			}
		}
	}

	// release all the temporary images   
	cvReleaseImage(&gray);
	cvReleaseImage(&pyr);
	cvReleaseImage(&tgray);
	cvReleaseImage(&timg);

	return squares;
}

double imageProcess::angle(CvPoint* pt1, CvPoint* pt2, CvPoint* pt0)
{
	double dx1 = pt1->x - pt0->x;
	double dy1 = pt1->y - pt0->y;
	double dx2 = pt2->x - pt0->x;
	double dy2 = pt2->y - pt0->y;
	//1e-10就是“aeb”的形式，表示a乘以10的b次方。   
	//其中b必须是整数，a可以是小数。   
	//?余弦定理CosB=(a^2+c^2-b^2)/2ac？？所以这里的计算似乎有问题   
	return ((dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10));
}

void imageProcess::GetSimilarSquare(Polygone &Polygon1, CvPoint2D32f *temp1)
{
	cv::Point2f temp[4];
	Polygong2Point(Polygon1, temp);//把边组成的四边形转成一定顺序的点列
	float* x_arr = new float[4];
	float* y_arr = new float[4];
	for (int i = 0; i < 4; i++)
	{
		x_arr[i] = temp[i].x;
		y_arr[i] = temp[i].y;
	}


	//对xlist，ylist进行升序排序
	for (int i = 1; i < 4; i++){
		if (x_arr[i] < x_arr[i - 1]){               //若第i个元素大于i-1元素，直接插入。小于的话，移动有序表后插入  
			int j = i - 1;
			float x = x_arr[i];        //复制为哨兵，即存储待排序元素  
			x_arr[i] = x_arr[i - 1];           //先后移一个元素  
			while (x < x_arr[j]){  //查找在有序表的插入位置  
				x_arr[j + 1] = x_arr[j];
				j--;         //元素后移  
			}
			x_arr[j + 1] = x;      //插入到正确位置  
		}
	}

	for (int i = 1; i < 4; i++){
		if (y_arr[i] < y_arr[i - 1]){               //若第i个元素大于i-1元素，直接插入。小于的话，移动有序表后插入  
			int j = i - 1;
			int x = y_arr[i];        //复制为哨兵，即存储待排序元素  
			y_arr[i] = y_arr[i - 1];           //先后移一个元素  
			while (x < y_arr[j]){  //查找在有序表的插入位置  
				y_arr[j + 1] = y_arr[j];
				j--;         //元素后移  
			}
			y_arr[j + 1] = x;      //插入到正确位置  
		}
	}


	temp1[0].x = x_arr[1];
	temp1[0].y = y_arr[1];
	temp1[1].x = x_arr[1];
	temp1[1].y = y_arr[2];
	temp1[3].x = x_arr[2];
	temp1[3].y = y_arr[1];
	temp1[2].x = x_arr[2];
	temp1[2].y = y_arr[2];
}

//把多边形结构转换成顶点的列表
int imageProcess::Polygong2Point(Polygone &srcPoly, cv::Point2f* pointList)
{
	EdgeFlag eflag;
	for (int i = 0; i<srcPoly.EdgeCnt; i++)
	{
		eflag.insert(std::map<int, bool>::value_type(i, false));
	}

	int indexFirstLine = FindFirstLine(srcPoly);//找到一条边作为起始边，这里找的是最左边的
	eflag[indexFirstLine] = true;
	Line2D32f line = srcPoly.L[indexFirstLine];
	CvPoint2D32f p1 = line.pt1.y > line.pt2.y ? line.pt2 : line.pt1;
	CvPoint2D32f p2 = line.pt1.y < line.pt2.y ? line.pt2 : line.pt1;//p1作为第一个点，p2作为第二个点

	pointList[0] = p1;
	pointList[1] = p2;
	int index = 2;

	int nextline;

	while ((nextline = FindNextLine(srcPoly, p1, p2, eflag)))//以p2为第一条边的终点，找到下一条边的终点p1，并返回边的序号
	{
		pointList[index] = p1;
		p2 = p1;
		if (index >= srcPoly.EdgeCnt - 1)
		{
			break;
		}
		index++;
	}
	return 0;
}

//找到第一条边
int imageProcess::FindFirstLine(Polygone &srcPoly)
{
	int minx = srcPoly.L[0].pt1.x + srcPoly.L[0].pt2.x;
	int intdex = 0;
	for (int i = 1; i < srcPoly.EdgeCnt; i++)
	{
		float sumx = srcPoly.L[i].pt1.x + srcPoly.L[i].pt2.x;
		if (sumx < minx)
		{
			minx = sumx;
			intdex = i;
		}
	}
	return intdex;
}

//找到第二条边
int imageProcess::FindNextLine(Polygone &srcPoly, CvPoint2D32f &p_out, //出边的终点
	CvPoint2D32f p2,//入边的终点
	EdgeFlag &eflag)//选过边的标记
{
	for (int i = 0; i < srcPoly.EdgeCnt; i++)
	{
		if (eflag[i] == true)
		{
			continue;
		}
		if (srcPoly.L[i].pt1.x == p2.x && srcPoly.L[i].pt1.y == p2.y)
		{
			p_out = srcPoly.L[i].pt2;
			eflag[i] = true;
			return i;
		}
		else if (srcPoly.L[i].pt2.x == p2.x && srcPoly.L[i].pt2.y == p2.y)
		{
			p_out = srcPoly.L[i].pt1;
			eflag[i] = true;
			return i;
		}
	}
}

//将32位的点转换为整型
void imageProcess::transferPoints()
{
	for (int i = 0; i < 4; i++)
	{
		points[i] = cvPointFrom32f(rectange_points[i]);
	}
}