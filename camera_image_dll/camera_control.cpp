//////////////////////////////////////////////////////////////////////////
//定义camera_control类的静态成员firstCamera，及其他成员函数
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "camera_control.h"

camera_control::camera_control()
{
	EdsInitializeSDK();
	//获取相机
	EdsError err_construct = EDS_ERR_OK;
	EdsUInt32	 count = 0;
	EdsCameraListRef cameraList = NULL;
	err_construct = EdsGetCameraList(&cameraList);
	if (err_construct == EDS_ERR_OK)
	{
		err_construct = EdsGetChildCount(cameraList, &count);
		if (count == 0)
		{
			err_construct = EDS_ERR_DEVICE_NOT_FOUND;
		}
	}
	if (err_construct == EDS_ERR_OK)
	{
		err_construct = EdsGetChildAtIndex(cameraList, 0, &firstCamera);//初始化firstCamera

	}

	if (err_construct == EDS_ERR_OK)
	{
		EdsRelease(cameraList);
	}

	err = err_construct;//初始化err
}

//析构函数
camera_control::~camera_control()
{
	err = EdsCloseSession(firstCamera);
	if (firstCamera!=NULL)
	{
		EdsRelease(firstCamera);
		firstCamera = NULL;
	}
	err = EdsTerminateSDK();
	if (err==EDS_ERR_OK)
	{
		err = NULL;
	}
}

//open session 
EdsError camera_control::openSession()
{
	EdsError err = EDS_ERR_OK;
	bool locked = false;
	err = EdsOpenSession(firstCamera);
	if (err == EDS_ERR_OK)
	{
		EdsUInt32 saveTo = kEdsSaveTo_Host;
		err = EdsSetPropertyData(firstCamera	, kEdsPropID_SaveTo, 0, sizeof(saveTo), &saveTo);
	}
	if (err == EDS_ERR_OK)
	{
		err = EdsSendStatusCommand(firstCamera, kEdsCameraStatusCommand_UILock, 0);
	}
	if (err == EDS_ERR_OK)
	{
		locked = true;
	}

	if (err == EDS_ERR_OK)
	{
		EdsCapacity capacity = { 0x7FFFFFFF, 0x1000, 1 };
		err = EdsSetCapacity(firstCamera, capacity);
	}
	if (locked)
	{
		EdsSendStatusCommand(firstCamera, kEdsCameraStatusCommand_UIUnLock, 0);
	}
	return err;
}

//get property
EdsError camera_control::getProperty(EdsPropertyID propertyID)
{
	EdsError err = EDS_ERR_OK;
	EdsDataType	dataType = kEdsDataType_Unknown;
	EdsUInt32   dataSize = 0;


	if (propertyID == kEdsPropID_Unknown)
	{
		//If unknown is returned for the property ID , the required property must be retrieved again
		if (err == EDS_ERR_OK) err = getProperty(kEdsPropID_AEMode);
		if (err == EDS_ERR_OK) err = getProperty(kEdsPropID_Tv);
		if (err == EDS_ERR_OK) err = getProperty(kEdsPropID_Av);
		if (err == EDS_ERR_OK) err = getProperty(kEdsPropID_ISOSpeed);
		if (err == EDS_ERR_OK) err = getProperty(kEdsPropID_MeteringMode);
		if (err == EDS_ERR_OK) err = getProperty(kEdsPropID_ExposureCompensation);
		if (err == EDS_ERR_OK) err = getProperty(kEdsPropID_ImageQuality);

		return err;
	}

	//Acquisition of the property size
	if (err == EDS_ERR_OK)
	{
		err = EdsGetPropertySize(firstCamera,
			propertyID,
			0,
			&dataType,
			&dataSize);
	}

	if (err == EDS_ERR_OK)
	{

		if (dataType == kEdsDataType_UInt32)
		{
			EdsUInt32 data;

			//Acquisition of the property
			err = EdsGetPropertyData(firstCamera,
				propertyID,
				0,
				dataSize,
				&data);

			//Acquired property value is set
			if (err == EDS_ERR_OK)
			{
				EdsSetPropertyData(firstCamera, propertyID, 0, dataSize, &data);

			}
		}

		if (dataType == kEdsDataType_String)
		{

			EdsChar str[EDS_MAX_NAME];
			//Acquisition of the property
			err = EdsGetPropertyData(firstCamera,
				propertyID,
				0,
				dataSize,
				str);

			//Acquired property value is set
			if (err == EDS_ERR_OK)
			{
				EdsSetPropertyData(firstCamera, propertyID, 0, dataSize, &str);
			}
		}
		if (dataType == kEdsDataType_FocusInfo)
		{
			EdsFocusInfo focusInfo;
			//Acquisition of the property
			err = EdsGetPropertyData(firstCamera,
				propertyID,
				0,
				dataSize,
				&focusInfo);

			//Acquired property value is set
			if (err == EDS_ERR_OK)
			{
				EdsSetPropertyData(firstCamera, propertyID, 0, dataSize, &focusInfo);
			}
		}
	}

	return err;
}




