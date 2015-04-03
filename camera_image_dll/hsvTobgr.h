//////////////////////////////////////////////////////////////////////////
//将opencv计算得到的HSV值转换为BGR颜色空间
//
//////////////////////////////////////////////////////////////////////////
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace std;

class  hsvTobgr
{
public:
	hsvTobgr(CvScalar hsv_scalar) :HSV(hsv_scalar){}
	~hsvTobgr();
	void transfor();

private:
	CvScalar fun_hsv2rgb(CvScalar scalar);
	CvScalar transfor_HSV(CvScalar cv_hsv);
	CvScalar transfor_bgr(CvScalar normal_rgb);

public:
	CvScalar HSV=cvScalar(0,0,0);
	CvScalar BGR=cvScalar(0,0,0);
	bool istransfer = false;
};