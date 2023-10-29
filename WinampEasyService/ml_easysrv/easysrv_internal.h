#pragma once

#include "easysrv.h"

#include <Windows.h>
#include <vector>
#include <map>
#include <mutex>

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
    virtual void InitService(UINT_PTR serviceID) = 0;
    virtual NodeDescriptor& GetNodeDesc() = 0;
    virtual void InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl) = 0;
    virtual const wchar_t* GetFileName(const wchar_t* fileID) = 0;
    virtual HWND GetCustomDialog(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl) = 0;
    virtual void DestroyingCustomDialog() = 0;
    virtual const wchar_t* GetShortName() = 0;

    virtual ~EasyService() {}
};

class DLLService : public EasyService
{
public:
    DLLService(const wchar_t* dllFullPath, const wchar_t* _shortName);

    virtual void InitService(UINT_PTR serviceID);
    virtual NodeDescriptor& GetNodeDesc();
    virtual void InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl);
    virtual const wchar_t* GetFileName(const wchar_t* fileID);
    virtual HWND GetCustomDialog(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl);
    virtual void DestroyingCustomDialog();
    virtual const wchar_t* GetShortName();

private:
    InitServiceFunc _initService;
    GetNodeDescFunc _getNodeDesc;
    InvokeServiceFunc _invokeService;
    GetFileNameFunc _getFileName;
    GetCustomDialogFunc _getCustomDialog;
    
    const wchar_t* shortName;
    NodeDescriptor nodeDescCache;
    UINT_PTR _serviceID;
};

class EXEService : public EasyService
{
public:
    EXEService(const wchar_t* exeName, const wchar_t* _shortName);

    virtual void InitService(UINT_PTR serviceID);
    virtual NodeDescriptor& GetNodeDesc();
    virtual void InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl);
    virtual const wchar_t* GetFileName(const wchar_t* fileID);
    virtual HWND GetCustomDialog(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl);
    virtual void DestroyingCustomDialog();
    virtual const wchar_t* GetShortName();

private:
    wchar_t* _exeName;
    const wchar_t* shortName;
    
    NodeDescriptor nodeDescCache;
    UINT_PTR _serviceID;
    
    DWORD customDialogPID = -1;
};

extern std::map<UINT_PTR, EasyService*> serviceMap;
extern std::map<HWND, UINT_PTR> serviceHwndMap;
extern std::map<UINT_PTR, std::vector<ItemInfo>> serviceListItemMap;
extern std::mutex serviceListItemMapMutex;

extern wchar_t configFileName[MAX_PATH];
extern wchar_t pluginDir[MAX_PATH];
