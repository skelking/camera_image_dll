//////////////////////////////////////////////////////////////////////////
////һ����������Ķ��������EventHandler������������


#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"
#include <string>
using namespace std;


class camera_control
{
	/*friend EdsError getProperty(EdsPropertyID propertyID);
	*/
public:
	EdsCameraRef  firstCamera;
	EdsError err;
	
public:
	camera_control();
	EdsError openSession();
	EdsError getProperty(EdsPropertyID propertyID);
	~camera_control();
	
};



