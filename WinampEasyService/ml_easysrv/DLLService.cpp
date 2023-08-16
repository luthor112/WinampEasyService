#include "easyservice.h"

#include <Windows.h>

DLLService::DLLService(const wchar_t* dllName, int _playerType)
{
    playerType = _playerType;

    HMODULE dllModule = LoadLibraryW(dllName);
    _getNodeName = reinterpret_cast<GetNodeNameFunc>(GetProcAddress(dllModule, "GetNodeName"));
    _invokeService = reinterpret_cast<InvokeServiceFunc>(GetProcAddress(dllModule, "InvokeService"));
    _invokeNext = reinterpret_cast<InvokeNextFunc>(GetProcAddress(dllModule, "InvokeNext"));
    _getFileName = reinterpret_cast<GetFileNameFunc>(GetProcAddress(dllModule, "GetFileName"));
    _getCustomDialog = reinterpret_cast<GetCustomDialogFunc>(GetProcAddress(dllModule, "GetCustomDialog"));
}

const wchar_t* DLLService::GetNodeName()
{
    return _getNodeName();
}

std::vector<ItemInfo> DLLService::InvokeService()
{
    std::vector<ItemInfo> retItems;
    
    ItemInfo currItem = _invokeService(playerType);
    while (currItem.filename != NULL) {
        retItems.push_back(currItem);
        currItem = _invokeNext(playerType);
    }

    return retItems;
}

const wchar_t* DLLService::GetFileName(const wchar_t* fileID)
{
    return _getFileName(fileID);
}

HWND DLLService::GetCustomDialog()
{
    if (_getCustomDialog != NULL)
        return _getCustomDialog();
    else
        return NULL;
}
