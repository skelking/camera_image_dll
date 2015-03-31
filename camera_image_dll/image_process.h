//////////////////////////////////////////////////////////////////////////
//图像处理类imageProcess
//////////////////////////////////////////////////////////////////////////
#include <cv.h>
#include <highgui.h>
#include <math.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;

//自定义数据结构
#define IMAGE_PATH "./test.jpg"
#define  STANDARD_WIDTH  2048
#define POLYGONEDGE  500
#define EDGE 4
//W_NUM,H_NUM  为融合带分割数量
#define W_NUM  8
#define  H_NUM  18

typedef std::map<int, bool> EdgeFlag;

//直线
struct Line2D32f
{
	CvPoint2D32f pt1;
	CvPoint2D32f pt2;
};


//多边行
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

	bool  getPolygone();//获取轮廓的4个顶点
	void GetSimilarSquare(Polygone &Polygon1, CvPoint2D32f *temp1);
	void transferPoints();

private:
	int FindNextLine(Polygone &srcPoly, CvPoint2D32f &p_out, 
		CvPoint2D32f p2,
		EdgeFlag &eflag);
	int FindFirstLine(Polygone &srcPoly);
	int Polygong2Point(Polygone &srcPoly, cv::Point2f* pointList);//把多边形转换成一定顺序的点列
	
	double angle(CvPoint* pt1, CvPoint* pt2, CvPoint* pt0);
	CvSeq* findSquares(CvMemStorage* storage, IplImage * image, double CannyThresh, double AreaThresh);

private:
	
	CvSeq* sequares;//提取出的轮廓
	
public:
	Polygone*  polygone = new Polygone;
	CvPoint2D32f rectange_points[4];//一个投影区域矩形的 点的坐标（32位）
	CvPoint* points=new CvPoint[4];//一个投影区域矩形的 点的坐标
	IplImage* image;//照片
	IplImage* resizeImage;

};