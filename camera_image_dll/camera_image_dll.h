// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� CAMERA_IMAGE_DLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// CAMERA_IMAGE_DLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

#ifdef CAMERA_IMAGE_DLL_EXPORTS
#define CAMERA_IMAGE_DLL_API __declspec(dllexport)
#else
#define CAMERA_IMAGE_DLL_API __declspec(dllimport)
#endif

typedef void(_stdcall * CPP_OPENPROJECT)(int Id);
typedef void(_stdcall * CPP_OPENPROJECTFAILD)(int Id);

extern "C"  CAMERA_IMAGE_DLL_API int _stdcall  startTakepicture(int list[], int length, CPP_OPENPROJECT open_project, CPP_OPENPROJECTFAILD open_project_failed);

extern "C" CAMERA_IMAGE_DLL_API int _stdcall  color_recognise(int Large_area_num, int Small_areas_num);
