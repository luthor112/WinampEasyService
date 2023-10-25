#pragma once

#include "easysrv.h"

#include <Windows.h>
#include <vector>

struct ItemInfo
{
    const wchar_t* info;
    const wchar_t* plTitle;
    const wchar_t* filename;
};

typedef void (*InitServiceFunc)(AddItemFunc addItemFunc, GetOptionFunc getOptionFunc, SetOptionFunc setOptionFunc, const wchar_t* pluginDir, UINT_PTR serviceID);
typedef NodeDescriptor (*GetNodeDescFunc)();
typedef void (*InvokeServiceFunc)(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl, wchar_t* skinPath);
typedef const wchar_t* (*GetFileNameFunc)(const wchar_t* fileID);
typedef HWND (*GetCustomDialogFunc)(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl, wchar_t* skinPath);

class EasyService
{
public:
    virtual void InitService(const wchar_t* pluginDir, UINT_PTR serviceID) = 0;
    virtual NodeDescriptor GetNodeDesc() = 0;
    virtual std::vector<ItemInfo> InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl) = 0;
    virtual const wchar_t* GetFileName(const wchar_t* fileID) = 0;
    virtual HWND GetCustomDialog(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl) = 0;
    virtual void DestroyingCustomDialog() = 0;

    virtual ~EasyService() {}
};

class DLLService : public EasyService
{
public:
    DLLService(const wchar_t* dllName);

    virtual void InitService(const wchar_t* pluginDir, UINT_PTR serviceID);
    virtual NodeDescriptor GetNodeDesc();
    virtual std::vector<ItemInfo> InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl);
    virtual const wchar_t* GetFileName(const wchar_t* fileID);
    virtual HWND GetCustomDialog(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl);
    virtual void DestroyingCustomDialog();

private:
    InitServiceFunc _initService;
    GetNodeDescFunc _getNodeDesc;
    InvokeServiceFunc _invokeService;
    GetFileNameFunc _getFileName;
    GetCustomDialogFunc _getCustomDialog;
    NodeDescriptor nodeDescCache;
};

class EXEService : public EasyService
{
public:
    EXEService(const wchar_t* exeName);

    virtual void InitService(const wchar_t* pluginDir, UINT_PTR serviceID);
    virtual NodeDescriptor GetNodeDesc();
    virtual std::vector<ItemInfo> InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl);
    virtual const wchar_t* GetFileName(const wchar_t* fileID);
    virtual HWND GetCustomDialog(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl);
    virtual void DestroyingCustomDialog();

private:
    wchar_t* _exeName;
    NodeDescriptor nodeDescCache;
    DWORD customDialogPID = -1;
};
