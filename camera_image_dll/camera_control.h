//////////////////////////////////////////////////////////////////////////
////一个控制相机的对象，相机的EventHandler并不包括在内


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



