//////////////////////////////////////////////////////////////////////////
///CAMERA_IMAGE_DLL_API   startTakepicture  
///C#���ýӿ�
///list[]ͶӰ��Id���飬length������Ч���ȣ�
///open_project��ͶӰ���ص�������open_project_failed��ͶӰ��ʧ��֪ͨ�ص�����
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "io.h"
#include "stdio.h"
#include "stdlib.h"
#include "camera_image_dll.h"
#include "camera_control.h"
#include "hsvTobgr.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "image_process.h"
#include <vector>

using namespace std;

//ȫ�ֱ���
camera_control controler;

//�ṹ��rect
struct rect
{
	CvPoint point0;
	CvPoint point1;
	CvPoint point2;
	CvPoint point3;
};
vector<rect>  vec_rect;    //���ͶӰ��������λ�õ�����

//struct fusionBrightness
struct fusionBrightness
{
	int r;
	int g;
	int b;
	//fusionBrightness(int mr, int mg, int mb) :r(mr), g(mg), b(mb){}
	
};



//download image
EdsError downLoadImage(EdsDirectoryItemRef dirItemRef)
{
	EdsError				err = EDS_ERR_OK;
	EdsStreamRef			stream = NULL;

	//Acquisition of the downloaded image information
	EdsDirectoryItemInfo	dirItemInfo;
	err = EdsGetDirectoryItemInfo(dirItemRef, &dirItemInfo);
	//Make the file stream at the forwarding destination
	if (err == EDS_ERR_OK)
	{
		err = EdsCreateFileStream(dirItemInfo.szFileName, kEdsFileCreateDisposition_CreateAlways, kEdsAccess_ReadWrite, &stream);
		err = EdsCreateFileStream("./target.jpg", kEdsFileCreateDisposition_CreateAlways, kEdsAccess_ReadWrite, &stream);
	}

	//Download image
	if (err == EDS_ERR_OK)
	{
		err = EdsDownload(dirItemRef, dirItemInfo.size, stream);
	}

	//Forwarding completion
	if (err == EDS_ERR_OK)
	{
		err = EdsDownloadComplete(dirItemRef);
	}

	//Release Item
	if (dirItemRef != NULL)
	{
		err = EdsRelease(dirItemRef);
		dirItemRef = NULL;
	}

	//Release stream
	if (stream != NULL)
	{
		err = EdsRelease(stream);
		stream = NULL;
	}

	return err;
}

//Event Handler
// objecthandler
EdsError EDSCALLBACK HandlerObjectEvent(EdsObjectEvent inEvent, EdsDirectoryItemRef inRef, EdsVoid* inContext)
{
	EdsError err = EDS_ERR_OK;
	switch (inEvent)
	{
	case kEdsObjectEvent_DirItemRequestTransfer:
		err = downLoadImage(inRef);
		break;

	default:
		if (inRef != NULL)
		{
			EdsRelease(inRef);
		}
		break;
	}
	return err;
}


//propertyhandler
EdsError EDSCALLBACK  HandlerPropertyEvent(EdsPropertyEvent inEvent, EdsPropertyID inPropertyID, EdsUInt32 parameter, EdsVoid* inContext)
{

	{
		EdsError err = EDS_ERR_OK;
		switch (inEvent)
		{
		case kEdsPropertyEvent_PropertyChanged:
		{
												  err = controler.getProperty(inPropertyID);//controlerȫ�ֱ���


		}
			break;

		case kEdsPropertyEvent_PropertyDescChanged:

			break;
		}

		return err;
	}
}

//statehandler
EdsError EDSCALLBACK HandlerStateEvent(EdsStateEvent inEvent, EdsUInt32 paramter, EdsVoid* inContext)
{
	switch (inEvent)
	{
	case kEdsStateEvent_Shutdown:
		return 111;////��ʱ���...................................................................................................................................

	default:
		break;
	}
	return EDS_ERR_OK;
}

EdsError setEventHandler(EdsCameraRef camera)
{
	EdsError err = EDS_ERR_OK;
	if (err == EDS_ERR_OK)
	{
		err = EdsSetPropertyEventHandler(camera, kEdsPropertyEvent_All, HandlerPropertyEvent, NULL);
	}
	if (err == EDS_ERR_OK)
	{
		err = EdsSetObjectEventHandler(camera, kEdsObjectEvent_All, HandlerObjectEvent, NULL);
	}
	if (err == EDS_ERR_OK)
	{
		err = EdsSetCameraStateEventHandler(camera, kEdsStateEvent_All, HandlerStateEvent, NULL);
	}

	return err;
}


EdsError take_picture()
{
	controler.err = controler.openSession();
	if (controler.err == EDS_ERR_OK)
	{
		controler.err = setEventHandler(controler.firstCamera);
	}
	bool	 locked = false;
	controler.err = EdsSendStatusCommand(controler.firstCamera, kEdsCameraStatusCommand_UILock, 0);
	if (controler.err == EDS_ERR_OK)
	{
		locked = true;
	}
	//Taking a picture
	controler.err = EdsSendCommand(controler.firstCamera, kEdsCameraCommand_TakePicture, 0);
	//It releases it when locked
	if (locked)
	{
		controler.err = EdsSendStatusCommand(controler.firstCamera, kEdsCameraStatusCommand_UIUnLock, 0);
	}

	//close session
	return controler.err;

}

CAMERA_IMAGE_DLL_API int _stdcall  startTakepicture(int list[], int length, CPP_OPENPROJECT open_project, CPP_OPENPROJECTFAILD open_project_failed)
{
	//vector ��Ϊ��ʱ�����vector
	if (vec_rect.size() != 0)
	{
		vec_rect.clear();
	}

	for (int i = 0; i < length; i++)
	{
		open_project(list[i]);
		//�ж�target.jpg�Ƿ���ڣ�������ɾ����
		if (_access("target.jpg", 0) == 0)
		{
			int i = DeleteFile("target.jpg");
			if (i == 1)
				cout << "delete file sucess" << endl;
			else
				cout << "delete file failed" << endl;
		}

		//��ʾͼƬ����ʵ�����պ󱣴�ͼƬ�����صĻص�
		IplImage* picture = cvLoadImage("lena.jpg");
		cvNamedWindow("My picture", 1);
		cvShowImage("My picture", picture);
		cvWaitKey(2000);//�ȴ�2S��ͶӰ��
		take_picture();
		cvWaitKey(1500);//�ȴ�1s����ͼƬ

		imageProcess  my_process("target.jpg");
		//��γɹ���ȡͶӰ����
		if (my_process.getPolygone())
		{
			cout << "true";
			my_process.GetSimilarSquare(*my_process.polygone, my_process.rectange_points);
			my_process.transferPoints();

			int out = 4;
			cvPolyLine(my_process.resizeImage, &my_process.points, &out, 1, 1, cvScalar(0, 0, 255), 5, CV_AA, 0);

			//����ȡ��������λ�ô���vector
			rect  temp_rect;
			temp_rect.point0 = my_process.points[0];
			temp_rect.point1 = my_process.points[1];
			temp_rect.point2 = my_process.points[2];
			temp_rect.point3 = my_process.points[3];
			vec_rect.push_back(temp_rect);

		}
		else
		{
			cout << "false";
			open_project_failed(list[i]);//notify open project failed
			break;
			return -1;
		}
		cvNamedWindow("image", 0);
		cvShowImage("image", my_process.resizeImage);
		cvWaitKey(2000);
		cvReleaseImage(&picture);
		cvDestroyAllWindows();
	}
	return 1;
	
}

CAMERA_IMAGE_DLL_API int _stdcall  color_recognise(int Large_area_num, int Small_areas_num,fusionBrightness *boardlist,fusionBrightness (*fusionboardlist)[10])
{
	vector<rect>  vec_single; //���ں���С���ο�����
	vector<rect>  vec_blend;  //�ں���С���ο�����
	vector<rect>  singles;//���ں�������ο�
	vector<rect>  blends;//�ں�������ο�

	//����ں����ȷ��ۺ���С��������HSV��ֵ��vector
	vector<CvScalar>  vec_blend_rgb;
	vector<CvScalar>  vec_single_rgb;


	if (vec_rect.size() <= 1)
	{
		//////////////////////////////////////////////////////////////////////////
		//��ʱ�����쳣
	}
	/*
	struct A
	{
	int r;
	int g;
	,int b;
	}
	A a[32];
	int aLength;
	A a1[�ںϴ�����,�ںϴ��ֳ��ĸ�������]��
	*/

	//////////////////////////////////////////////////////////////////////////
	//�ָ����

	for (vector<rect>::size_type i = 0; i < vec_rect.size(); i++)
	{
		if (i == 0)
		{
			//��һ�����ں���������
			rect tem_rect;
			tem_rect.point0 = vec_rect[i].point0;
			tem_rect.point1 = vec_rect[i].point1;
			tem_rect.point2 = vec_rect[i + 1].point1;
			tem_rect.point3 = vec_rect[i + 1].point0;
			singles.push_back(tem_rect);
			//��һ���ں���������
			rect tem_rect_blend;
			tem_rect_blend.point0 = vec_rect[i + 1].point0;
			tem_rect_blend.point1 = vec_rect[i + 1].point1;
			tem_rect_blend.point2 = vec_rect[i].point2;
			tem_rect_blend.point3 = vec_rect[i].point3;
			blends.push_back(tem_rect_blend);
		}
		else if (i == vec_rect.size() - 1)
		{
			//���һ�����ں���
			rect tem_rect;
			tem_rect.point0 = vec_rect[i - 1].point3;
			tem_rect.point1 = vec_rect[i - 1].point2;
			tem_rect.point2 = vec_rect[i].point2;
			tem_rect.point3 = vec_rect[i].point3;
			singles.push_back(tem_rect);
		}
		else
		{
			//���ں���
			rect tem_rect;
			tem_rect.point0 = vec_rect[i - 1].point3;
			tem_rect.point1 = vec_rect[i - 1].point2;
			tem_rect.point2 = vec_rect[i + 1].point1;
			tem_rect.point3 = vec_rect[i + 1].point0;
			singles.push_back(tem_rect);
			//�ں���
			rect tem_rect_blend;
			tem_rect_blend.point0 = vec_rect[i + 1].point0;
			tem_rect_blend.point1 = vec_rect[i + 1].point1;
			tem_rect_blend.point2 = vec_rect[i].point2;
			tem_rect_blend.point3 = vec_rect[i].point3;
			blends.push_back(tem_rect_blend);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//�ָ�С����
	//���ں���
	for (vector<rect>::size_type i = 0; i < singles.size(); i++)
	{
		int height = (singles[i].point1.y - singles[i].point0.y) / Large_area_num;
		vector<CvPoint>  vec_center;
		for (int j = 0; j < Large_area_num; j++)
		{
			CvPoint center;
			if (j == 0)
			{
				center.x = (singles[i].point0.x + singles[i].point3.x) / 2;
				center.y = height / 2 + singles[j].point0.y;
				vec_center.push_back(center);
			}
			else
			{
				center.x = vec_center[j - 1].x;
				center.y = vec_center[j - 1].y + height;
				vec_center.push_back(center);
			}
		}

		for (vector<CvPoint>::size_type k = 0; k < vec_center.size(); k++)
		{
			rect tem_rect;
			tem_rect.point0 = cvPoint(vec_center[k].x - 16, vec_center[k].y - 8);
			tem_rect.point1 = cvPoint(vec_center[k].x - 16, vec_center[k].y + 8);
			tem_rect.point2 = cvPoint(vec_center[k].x + 16, vec_center[k].y + 8);
			tem_rect.point3 = cvPoint(vec_center[k].x + 16, vec_center[k].y - 8);

			vec_single.push_back(tem_rect);

		}
	}
	//�ں���
	for (vector<rect>::size_type i = 0; i < blends.size(); i++)
	{
		int height = (blends[i].point1.y - blends[i].point0.y) / Large_area_num;
		vector<CvPoint>  vec_center;
		for (int j = 0; j < Large_area_num; j++)
		{
			CvPoint center;
			if (j == 0)
			{
				center.x = (blends[j].point0.x + blends[j].point3.x) / 2;
				center.y = height / 2 + blends[j].point0.y;
				vec_center.push_back(center);
			}
			else
			{
				center.x = vec_center[j - 1].x;
				center.y = vec_center[j - 1].y + height;
				vec_center.push_back(center);
			}
		}

		for (vector<CvPoint>::size_type k = 0; k < vec_center.size(); k++)
		{
			CvPoint start;
			rect tem_rect;
			start.x = vec_center[k].x - (blends[i].point3.x - blends[i].point0.x - 20) / 2;
			start.y = vec_center[k].y;
			int distance = (blends[i].point3.x - blends[i].point0.x - 20) / Small_areas_num;
			int len_of_side = distance - 4;//С��ı߳�
			for (int m = 0; m < Small_areas_num; m++)
			{
				tem_rect.point0 = cvPoint(start.x + m*distance, start.y - H_NUM / 2);
				tem_rect.point1 = cvPoint(start.x + m*distance, start.y + H_NUM / 2);
				tem_rect.point2 = cvPoint(start.x + len_of_side + m*distance, start.y + H_NUM / 2);
				tem_rect.point3 = cvPoint(start.x + len_of_side + m*distance, start.y - H_NUM / 2);
				vec_blend.push_back(tem_rect);
			}

		}
	}

	//�ж�target.jpg�Ƿ���ڣ�������ɾ����
	if (_access("target.jpg", 0) == 0)
	{
		int i = DeleteFile("target.jpg");
		if (i == 1)
			cout << "delete file sucess" << endl;
		else
			cout << "delete file failed" << endl;
	}

	//��ʾͼƬ����ʵ�����պ󱣴�ͼƬ�����صĻص�
	IplImage* picture = cvLoadImage("lena.jpg");
	cvNamedWindow("My picture", 1);
	cvShowImage("My picture", picture);
	cvWaitKey(2000);//�ȴ�2S��ͶӰ��
	take_picture();
	cvWaitKey(1500);//�ȴ�1s����ͼƬ

	//����Ŀ��ͼƬ��resize��
	IplImage* color_image = cvLoadImage("resizeImage.jpg");
	double resize_value = (double)color_image->width / (double)STANDARD_WIDTH;
	int standard_height = (double)color_image->height / resize_value;
	IplImage* resizeImage = cvCreateImage(cvSize(STANDARD_WIDTH, standard_height), color_image->depth, color_image->nChannels);
	cvResize(color_image, resizeImage);

	//��BGR��ɫ�ռ�ת����HSV��ɫ�ռ�
	IplImage* hsv_image = cvCreateImage(cvGetSize(resizeImage), resizeImage->depth, resizeImage->nChannels);
	cvCvtColor(resizeImage, hsv_image, CV_BGR2HSV);

	//���ζ�ȡ�ںϴ������ںϴ�С��������hsv��ֵ
	for (vector<rect>::size_type i = 0; i < vec_blend.size();i++)
	{
		
		double H_all = 0, S_all = 0, V_all = 0;
		int num = 0;
		for (int w = vec_blend[i].point0.x; w <= vec_blend[i].point2.x;w++)
		{
			for (int h = vec_blend[i].point0.y; h <= vec_blend[i].point2.y;h++)
			{
				H_all += cvGet2D(hsv_image, w, h).val[0];
				S_all += cvGet2D(hsv_image, w, h).val[1];
				V_all += cvGet2D(hsv_image, w, h).val[2];
				num++;
			}
		}
		int H_value = H_all / (double)num + 0.5;
		int S_value = S_all / (double)num + 0.5;
		int V_value = V_all / (double)num + 0.5;
		CvScalar  tem_scalar = cvScalar(H_value, S_value, V_value);
		vec_blend_rgb.push_back(tem_scalar);
	}

	for (vector<rect>::size_type i = 0; i < vec_single.size(); i++)
	{

		double H_all = 0, S_all = 0, V_all = 0;
		int num = 0;
		for (int w = vec_single[i].point0.x; w <= vec_single[i].point2.x; w++)
		{
			for (int h = vec_single[i].point0.y; h <= vec_single[i].point2.y; h++)
			{
				H_all += cvGet2D(hsv_image, w, h).val[0];
				S_all += cvGet2D(hsv_image, w, h).val[1];
				V_all += cvGet2D(hsv_image, w, h).val[2];
				num++;
			}
		}
		int H_value = H_all / (double)num + 0.5;
		int S_value = S_all / (double)num + 0.5;
		int V_value = V_all / (double)num + 0.5;
		CvScalar  tem_scalar = cvScalar(H_value, S_value, V_value);
		hsvTobgr hsvtobgr(tem_scalar);
		hsvtobgr.transfor();
		CvScalar BGR = cvScalar(0, 0, 0);
		if (hsvtobgr.istransfer)
		{
			BGR = hsvtobgr.BGR;
		}
		vec_single_rgb.push_back(BGR);
	}

	//////////////////////////////////////////////////////////////////////////
	//��vector����ת��Ϊ����Ͷ�ά����
	fusionBrightness  singlelist[100];
	for (vector<Scalar>::size_type i = 0; i < vec_single_rgb.size();i++)
	{
		Scalar  temp_rgb;
		if (i%Large_area_num == 0)
		{
			Scalar  tem_rgb = vec_single_rgb[i / Large_area_num];
		}
		else
		{
			Scalar temp_rgb = vec_single_rgb[(i / Large_area_num)*Large_area_num + i%Large_area_num];
		}
		singlelist[i].r = temp_rgb[0];
		singlelist[i].g = temp_rgb[1];
		singlelist[i].b = temp_rgb[2];

		*boardlist++ = singlelist[i];
	}
	
	for (vector<Scalar>::size_type i = 0; i < vec_blend_rgb.size();i++)
	{
		int col, raw;
		raw = i%Small_areas_num;
		int j = i / Small_areas_num;
		Scalar temp_rgb = vec_blend_rgb[i];
		if (j%Large_area_num==0)
		{
			col = j/Large_area_num;
		}
		else
		{
			col = (j % 3) * 3 + j / 3;
		}
		fusionboardlist[col][raw].r = temp_rgb[0];
		fusionboardlist[col][raw].g = temp_rgb[1];
		fusionboardlist[col][raw].b = temp_rgb[2];
	}
	

	cvReleaseImage(&color_image);
	cvReleaseImage(&resizeImage);
	cvReleaseImage(&hsv_image);
	cvDestroyAllWindows();

	return 1;
}
