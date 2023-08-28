#pragma once

#include <Windows.h>

#ifdef EASYSRV_IMPORTING
#define EASY_API __declspec(dllimport)
#else
#define EASY_API __declspec(dllexport)
#endif

#define PLAYERTYPE_WINAMP 0
#define PLAYERTYPE_WACUP 1

struct ItemInfo
{
    const wchar_t* author;
    const wchar_t* title;
    const wchar_t* info;
    const wchar_t* filename;
};

struct CustomItemInfo
{
    const wchar_t* info;
    const wchar_t* plTitle;
    const wchar_t* filename;
};

#ifdef __cplusplus
extern "C" {
#endif

    EASY_API const wchar_t* GetNodeName();
    EASY_API ItemInfo InvokeService(int PlayerType);
    EASY_API ItemInfo InvokeNext(int PleyerType);
    EASY_API const wchar_t* GetFileName(const wchar_t* fileID);
    EASY_API HWND GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl);
    EASY_API const wchar_t* GetColumnNames();
    EASY_API CustomItemInfo InvokeServiceCustom(int PlayerType);
    EASY_API CustomItemInfo InvokeNextCustom(int PlayerType);

#ifdef __cplusplus
}
#endif
