// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 CAMERA_IMAGE_DLL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// CAMERA_IMAGE_DLL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#ifdef CAMERA_IMAGE_DLL_EXPORTS
#define CAMERA_IMAGE_DLL_API __declspec(dllexport)
#else
#define CAMERA_IMAGE_DLL_API __declspec(dllimport)
#endif

typedef void(_stdcall * CPP_OPENPROJECT)(int Id);
typedef void(_stdcall * CPP_OPENPROJECTFAILD)(int Id);

extern "C"  CAMERA_IMAGE_DLL_API int _stdcall  startTakepicture(int list[], int length, CPP_OPENPROJECT open_project, CPP_OPENPROJECTFAILD open_project_failed);

extern "C" CAMERA_IMAGE_DLL_API int _stdcall  color_recognise(int Large_area_num, int Small_areas_num);
