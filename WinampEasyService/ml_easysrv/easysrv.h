#pragma once

#include <Windows.h>

#ifdef EASYSRV_IMPORTING
#define EASY_API __declspec(dllimport)
#else
#define EASY_API __declspec(dllexport)
#endif

// Functions passed to InitService()
typedef void (*AddItemFunc)(const wchar_t* displayInfo, const wchar_t* playlistInfo, const wchar_t* filename, UINT_PTR serviceID);
typedef void (*GetOptionFunc)(UINT_PTR serviceID, const wchar_t* optionName, const wchar_t* defaultValue, wchar_t* output, DWORD outputSize);
typedef void (*SetOptionFunc)(UINT_PTR serviceID, const wchar_t* optionName, const wchar_t* optionValue);

// Values for NodeDescriptor.Capabilities
#define CAP_DEFAULT 0u
#define CAP_CUSTOMDIALOG 1u
#define CAP_MULTISERVICE 2u
#define CAP_CUSTOMREFID 4u

struct NodeDescriptor
{
    const wchar_t* Category;
    const wchar_t* NodeName;
    const wchar_t* ColumnNames;
    unsigned int Capabilities;
};

#ifdef __cplusplus
extern "C" {
#endif

    EASY_API void InitService(AddItemFunc addItemFunc, GetOptionFunc getOptionFunc, SetOptionFunc setOptionFunc, const wchar_t* pluginDir, UINT_PTR serviceID);
    EASY_API NodeDescriptor GetNodeDesc();
    EASY_API void InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl, wchar_t* skinPath);
    EASY_API const wchar_t* GetFileName(const wchar_t* fileID);
    EASY_API HWND GetCustomDialog(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl, wchar_t* skinPath);
    EASY_API int GetNodeNum();
    EASY_API void SelectService(int multiID);
    EASY_API const wchar_t* GetCustomRefId();

#ifdef __cplusplus
}
#endif
