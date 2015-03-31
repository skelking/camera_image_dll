#include "stdafx.h"
#include "image_process.h"


//���캯��
imageProcess::imageProcess(const char* path)
{
	image = cvLoadImage(path);
	if (image==NULL)
	{
		throw runtime_error("Initialise image failed!!");
	}
}

//��������
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

	//����ͼƬ��С
	float scaleImage = image->width / STANDARD_WIDTH;
	resizeImage = cvCreateImage(cvSize(STANDARD_WIDTH, (int)(image->height / scaleImage)), image->depth, image->nChannels);
	cvResize(image, resizeImage, CV_INTER_LINEAR);
	cvSaveImage("resizeImage.jpg", resizeImage, NULL);

	//��ȡ��������
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* sequares = findSquares(storage, resizeImage, 50000, 1000);//52500

	if (sequares == 0)
	{
		throw runtime_error("find no contous in the image");
		return isGetPolygone;
	}

	//////////////////////////////////////////////////////////////////////////
	//��ȡ��������
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
		//����������ת��Ϊ���������
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
	//ʹ��gaussian�������ֽ������ͼ�����²��������ȶ��������ͼ����ָ���˲���   
	//���о����Ȼ��ͨ���ܾ�ż�������������²���   
	cvPyrDown(timg, pyr, CV_GAUSSIAN_5x5);

	//���� cvPyrUp ʹ��Gaussian �������ֽ������ͼ�����ϲ���������ͨ����ͼ���в��� 0 ż���к�ż���У�Ȼ��Եõ���ͼ����ָ�����˲������и�˹����������˲�������4����ֵ���������ͼ��������ͼ��� 4 ����С��   
	cvPyrUp(pyr, timg, CV_GAUSSIAN_5x5);

	tgray = cvCreateImage(sz, 8, 1);

	// find squares in every color plane of the image   
	for (c = 0; c < 3; c++)
	{
		// extract the c-th color plane   
		//���� cvSetImageCOI ���ڸ�����ֵ���ø���Ȥ��ͨ����ֵ 0 ��ζ�����е�ͨ������ѡ��, 1 ��ζ�ŵ�һ��ͨ����ѡ���ȵȡ�   
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
				//ʹ������ṹԪ������ͼ��   
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
				//��ָ�����ȱƽ����������  
				result = cvApproxPoly(contours, sizeof(CvContour), storage,
					CV_POLY_APPROX_DP, cvContourPerimeter(contours) * 0.02, 0);
				// square contours should have 4 vertices after approximation   
				// relatively large area (to filter out noisy contours)   
				// and be convex.   
				// Note: absolute value of an area is used because   
				// area may be positive or negative - in accordance with the   
				// contour orientation   
				//cvContourArea �������������򲿷����������   
				//cvCheckContourConvexity����������͹��   

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
					if (s < 0.3)//ѡ���ı��ε���С�Ƕ�   
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
	//1e-10���ǡ�aeb������ʽ����ʾa����10��b�η���   
	//����b������������a������С����   
	//?���Ҷ���CosB=(a^2+c^2-b^2)/2ac������������ļ����ƺ�������   
	return ((dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10));
}

void imageProcess::GetSimilarSquare(Polygone &Polygon1, CvPoint2D32f *temp1)
{
	cv::Point2f temp[4];
	Polygong2Point(Polygon1, temp);//�ѱ���ɵ��ı���ת��һ��˳��ĵ���
	float* x_arr = new float[4];
	float* y_arr = new float[4];
	for (int i = 0; i < 4; i++)
	{
		x_arr[i] = temp[i].x;
		y_arr[i] = temp[i].y;
	}


	//��xlist��ylist������������
	for (int i = 1; i < 4; i++){
		if (x_arr[i] < x_arr[i - 1]){               //����i��Ԫ�ش���i-1Ԫ�أ�ֱ�Ӳ��롣С�ڵĻ����ƶ����������  
			int j = i - 1;
			float x = x_arr[i];        //����Ϊ�ڱ������洢������Ԫ��  
			x_arr[i] = x_arr[i - 1];           //�Ⱥ���һ��Ԫ��  
			while (x < x_arr[j]){  //�����������Ĳ���λ��  
				x_arr[j + 1] = x_arr[j];
				j--;         //Ԫ�غ���  
			}
			x_arr[j + 1] = x;      //���뵽��ȷλ��  
		}
	}

	for (int i = 1; i < 4; i++){
		if (y_arr[i] < y_arr[i - 1]){               //����i��Ԫ�ش���i-1Ԫ�أ�ֱ�Ӳ��롣С�ڵĻ����ƶ����������  
			int j = i - 1;
			int x = y_arr[i];        //����Ϊ�ڱ������洢������Ԫ��  
			y_arr[i] = y_arr[i - 1];           //�Ⱥ���һ��Ԫ��  
			while (x < y_arr[j]){  //�����������Ĳ���λ��  
				y_arr[j + 1] = y_arr[j];
				j--;         //Ԫ�غ���  
			}
			y_arr[j + 1] = x;      //���뵽��ȷλ��  
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

//�Ѷ���νṹת���ɶ�����б�
int imageProcess::Polygong2Point(Polygone &srcPoly, cv::Point2f* pointList)
{
	EdgeFlag eflag;
	for (int i = 0; i<srcPoly.EdgeCnt; i++)
	{
		eflag.insert(std::map<int, bool>::value_type(i, false));
	}

	int indexFirstLine = FindFirstLine(srcPoly);//�ҵ�һ������Ϊ��ʼ�ߣ������ҵ�������ߵ�
	eflag[indexFirstLine] = true;
	Line2D32f line = srcPoly.L[indexFirstLine];
	CvPoint2D32f p1 = line.pt1.y > line.pt2.y ? line.pt2 : line.pt1;
	CvPoint2D32f p2 = line.pt1.y < line.pt2.y ? line.pt2 : line.pt1;//p1��Ϊ��һ���㣬p2��Ϊ�ڶ�����

	pointList[0] = p1;
	pointList[1] = p2;
	int index = 2;

	int nextline;

	while ((nextline = FindNextLine(srcPoly, p1, p2, eflag)))//��p2Ϊ��һ���ߵ��յ㣬�ҵ���һ���ߵ��յ�p1�������رߵ����
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

//�ҵ���һ����
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

//�ҵ��ڶ�����
int imageProcess::FindNextLine(Polygone &srcPoly, CvPoint2D32f &p_out, //���ߵ��յ�
	CvPoint2D32f p2,//��ߵ��յ�
	EdgeFlag &eflag)//ѡ���ߵı��
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

//��32λ�ĵ�ת��Ϊ����
void imageProcess::transferPoints()
{
	for (int i = 0; i < 4; i++)
	{
		points[i] = cvPointFrom32f(rectange_points[i]);
	}
}