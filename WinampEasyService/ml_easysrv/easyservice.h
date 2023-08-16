#pragma once

#include <Windows.h>
#include <vector>

#define PLAYERTYPE_WINAMP 0
#define PLAYERTYPE_WACUP 1

struct ItemInfo
{
    const wchar_t* author;
    const wchar_t* title;
    const wchar_t* info;
    const wchar_t* filename;
};

typedef const wchar_t* (*GetNodeNameFunc)();
typedef ItemInfo (*InvokeServiceFunc)(int PlayerType);
typedef ItemInfo (*InvokeNextFunc)(int PleyerType);
typedef const wchar_t* (*GetFileNameFunc)(const wchar_t* fileID);
typedef HWND (*GetCustomDialogFunc)(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl);

class EasyService
{
public:
    virtual const wchar_t* GetNodeName() = 0;
    virtual std::vector<ItemInfo> InvokeService() = 0;
    virtual const wchar_t* GetFileName(const wchar_t* fileID) = 0;
    virtual HWND GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl) = 0;

    virtual ~EasyService() {}
};

class DLLService : public EasyService
{
public:
    DLLService(const wchar_t* dllName, int _playerType);

    virtual const wchar_t* GetNodeName();
    virtual std::vector<ItemInfo> InvokeService();
    virtual const wchar_t* GetFileName(const wchar_t* fileID);
    virtual HWND GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl);

private:
    int playerType;
    GetNodeNameFunc _getNodeName;
    InvokeServiceFunc _invokeService;
    InvokeNextFunc _invokeNext;
    GetFileNameFunc _getFileName;
    GetCustomDialogFunc _getCustomDialog;
};

class EXEService : public EasyService
{
public:
    EXEService(const wchar_t* exeName, int _playerType);

    virtual const wchar_t* GetNodeName();
    virtual std::vector<ItemInfo> InvokeService();
    virtual const wchar_t* GetFileName(const wchar_t* fileID);
    virtual HWND GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl);

private:
    int playerType;
    wchar_t* _exeName;
};
