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
vector<rect>  vec_rect;   //���ͶӰ��������λ�õ�����



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
		//err = EdsCreateFileStream(dirItemInfo.szFileName, kEdsFileCreateDisposition_CreateAlways, kEdsAccess_ReadWrite, &stream);
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

	}
	return 1;
}