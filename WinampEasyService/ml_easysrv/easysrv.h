#pragma once

#include <Windows.h>

#ifdef EASYSRV_IMPORTING
#define EASY_API __declspec(dllimport)
#else
#define EASY_API __declspec(dllexport)
#endif

// Functions passed to InitService()
typedef void (*AddItemFunc)(wchar_t* displayInfo, wchar_t* playlistInfo, wchar_t* filename);
typedef wchar_t* (*GetOptionFunc)(wchar_t* optionName, wchar_t* defaultValue);
typedef void (*SetOptionFunc)(wchar_t* optionName, wchar_t* optionValue);

// Values for NodeDescriptor.Capabilities
#define CAP_DEFAULT 0;
#define CAP_CUSTOMDIALOG 1;

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

#ifdef __cplusplus
}
#endif
