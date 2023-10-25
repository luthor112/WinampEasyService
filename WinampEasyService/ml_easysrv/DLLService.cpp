#include "easysrv_internal.h"

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
    _getColumnNames = reinterpret_cast<GetColumnNamesFunc>(GetProcAddress(dllModule, "GetColumnNames"));
    _invokeServiceCustom = reinterpret_cast<InvokeServiceCustomFunc>(GetProcAddress(dllModule, "InvokeServiceCustom"));
    _invokeNextCustom = reinterpret_cast<InvokeNextCustomFunc>(GetProcAddress(dllModule, "InvokeNextCustom"));

    if (_getColumnNames != NULL)
        customColumnsSupported = TRUE;
}

const wchar_t* DLLService::GetNodeName()
{
    if (nodeNameCache == NULL)
    {
        nodeNameCache = _getNodeName();
    }

    return nodeNameCache;
}

const wchar_t* DLLService::GetColumnNames()
{
    if (columnNameCache == NULL)
    {
        if (customColumnsSupported)
        {
            columnNameCache = _getColumnNames();
        }
        else
        {
            columnNameCache = L"Author\tTitle\tInformation";
        }
    }

    return columnNameCache;
}

std::vector<CustomItemInfo> DLLService::InvokeService()
{
    std::vector<CustomItemInfo> retItems;
    
    if (customColumnsSupported)
    {
        CustomItemInfo currItem = _invokeServiceCustom(playerType);
        while (currItem.filename != NULL) {
            retItems.push_back(currItem);
            currItem = _invokeNextCustom(playerType);
        }
    }
    else
    {
        ItemInfo currItem = _invokeService(playerType);
        while (currItem.filename != NULL) {
            wchar_t ciiInfo[1024];
            wsprintf(ciiInfo, L"%s\t%s\t%s", currItem.author, currItem.title, currItem.info);
            wchar_t plTitle[1024];
            wsprintf(plTitle, L"%s - %s", currItem.author, currItem.title);
            CustomItemInfo cii = { _wcsdup(ciiInfo), _wcsdup(plTitle), currItem.filename};
            retItems.push_back(cii);
            currItem = _invokeNext(playerType);
        }
    }

    return retItems;
}

const wchar_t* DLLService::GetFileName(const wchar_t* fileID)
{
    return _getFileName(fileID);
}

HWND DLLService::GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl)
{
    if (_getCustomDialog != NULL)
        return _getCustomDialog(_hwndWinampParent, _hwndLibraryParent, hwndParentControl);
    else
        return NULL;
}

void DLLService::DestroyingCustomDialog() {}
