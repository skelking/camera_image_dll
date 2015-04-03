#include "hsvTobgr.h"

void hsvTobgr::transfor()
{
	CvScalar temp_hsv = cvScalar(0, 0, 0);
	CvScalar temp_rgb = cvScalar(0, 0, 0);
	temp_hsv = transfor_HSV(HSV);
	temp_rgb = fun_hsv2rgb(temp_hsv);
	BGR = transfor_bgr(temp_rgb);
	istransfer = true;
}

//现实中hsv到rgb转换的计算
CvScalar hsvTobgr::fun_hsv2rgb(CvScalar scalar)
{
	double R, G, B;
	CvScalar RGB = cvScalar(0, 0, 0);
	if (abs(scalar.val[1]) <= 1e-8)
	{
		R = G = B = scalar.val[2];
	}
	else
	{
		int Hi = (int)abs(scalar.val[0] / 60.0);
		double f = scalar.val[0] / 60.0 - Hi;
		double a = scalar.val[2] * (1 - scalar.val[1]);
		double b = scalar.val[2] * (1 - f*scalar.val[1]);
		double c = scalar.val[2] * (1 - (1 - f)*scalar.val[1]);


		switch (Hi)
		{
		case 0:
			R = scalar.val[2];
			G = c;
			B = a;
			break;
		case 1:
			R = b;
			G = scalar.val[2];
			B = a;
			break;
		case 2:
			R = a;
			G = scalar.val[2];
			B = c;
			break;
		case 3:
			R = a;
			G = b;
			B = scalar.val[2];
			break;
		case 4:
			R = c;
			G = a;
			B = scalar.val[2];
			break;
		case 5:
			R = scalar.val[2];
			G = a;
			B = b;
			break;
		default:

			break;
		}
	}


	RGB = cvScalar(R, G, B);
	return RGB;
}

//将计算得到的rgb格式转换为bgr格式
CvScalar hsvTobgr::transfor_bgr(CvScalar normal_rgb)
{
	CvScalar cv_bgr;
	int blue = int(normal_rgb.val[2] * 255 + 0.5);
	int B = (blue > 255) ? 255 : blue;
	B = (blue < 0) ? 0 : blue;

	int green = int(normal_rgb.val[1] * 255 + 0.5);
	int G = (green > 255) ? 255 : green;
	G = (green < 0) ? 0 : green;

	int red = int(normal_rgb.val[0] * 255 + 0.5);
	int R = (red > 255) ? 255 : red;
	R = (red < 0) ? 0 : red;

	cv_bgr.val[0] = B;
	cv_bgr.val[1] = G;
	cv_bgr.val[2] = R;
	return cv_bgr;
}

//将opencv中的hsv格式转换为现实中的hsv格式
CvScalar hsvTobgr::transfor_HSV(CvScalar cv_hsv)
{
	CvScalar normal_hsv;
	normal_hsv.val[0] = cv_hsv.val[0] * 2;
	normal_hsv.val[1] = (double)(cv_hsv.val[1]) / 255;
	normal_hsv.val[2] = (double)(cv_hsv.val[2]) / 255;
	return normal_hsv;
}