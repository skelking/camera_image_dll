//////////////////////////////////////////////////////////////////////////
//ͼ������imageProcess
//////////////////////////////////////////////////////////////////////////
#include <cv.h>
#include <highgui.h>
#include <math.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;

//�Զ������ݽṹ
#define IMAGE_PATH "./test.jpg"
#define  STANDARD_WIDTH  2048
#define POLYGONEDGE  500
#define EDGE 4
//W_NUM,H_NUM  Ϊ�ںϴ��ָ�����
#define W_NUM  8
#define  H_NUM  18

typedef std::map<int, bool> EdgeFlag;

//ֱ��
struct Line2D32f
{
	CvPoint2D32f pt1;
	CvPoint2D32f pt2;
};


//�����
struct Polygone
{
	Line2D32f L[POLYGONEDGE];
	int EdgeCnt;
	float minx;
};


class imageProcess
{
public:
	imageProcess(const char* path);
	virtual ~imageProcess();

	bool  getPolygone();//��ȡ������4������
	void GetSimilarSquare(Polygone &Polygon1, CvPoint2D32f *temp1);
	void transferPoints();

private:
	int FindNextLine(Polygone &srcPoly, CvPoint2D32f &p_out, 
		CvPoint2D32f p2,
		EdgeFlag &eflag);
	int FindFirstLine(Polygone &srcPoly);
	int Polygong2Point(Polygone &srcPoly, cv::Point2f* pointList);//�Ѷ����ת����һ��˳��ĵ���
	
	double angle(CvPoint* pt1, CvPoint* pt2, CvPoint* pt0);
	CvSeq* findSquares(CvMemStorage* storage, IplImage * image, double CannyThresh, double AreaThresh);

private:
	
	CvSeq* sequares;//��ȡ��������
	
public:
	Polygone*  polygone = new Polygone;
	CvPoint2D32f rectange_points[4];//һ��ͶӰ������ε� ������꣨32λ��
	CvPoint* points=new CvPoint[4];//һ��ͶӰ������ε� �������
	IplImage* image;//��Ƭ
	IplImage* resizeImage;

};