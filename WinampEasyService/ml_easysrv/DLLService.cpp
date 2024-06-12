#include "easysrv_internal.h"

#include <Windows.h>
#include "Winamp/wa_ipc.h"

DLLService::DLLService(const wchar_t* dllFullPath, const wchar_t* _shortName, int _multiID)
{
    HMODULE dllModule = LoadLibraryW(dllFullPath);
    _initService = reinterpret_cast<InitServiceFunc>(GetProcAddress(dllModule, "InitService"));
    _getNodeDesc = reinterpret_cast<GetNodeDescFunc>(GetProcAddress(dllModule, "GetNodeDesc"));
    _invokeService = reinterpret_cast<InvokeServiceFunc>(GetProcAddress(dllModule, "InvokeService"));
    _getFileName = reinterpret_cast<GetFileNameFunc>(GetProcAddress(dllModule, "GetFileName"));
    _getCustomDialog = reinterpret_cast<GetCustomDialogFunc>(GetProcAddress(dllModule, "GetCustomDialog"));
    _getNodeNum = reinterpret_cast<GetNodeNumFunc>(GetProcAddress(dllModule, "GetNodeNum"));
    _selectService = reinterpret_cast<SelectServiceFunc>(GetProcAddress(dllModule, "SelectService"));

    if (_selectService)
    {
        _selectService(_multiID);
    }

    if (_initService && _getNodeDesc)
    {
        isValid = true;

        nodeDescCache = _getNodeDesc();
        if (nodeDescCache.ColumnNames == NULL)
            nodeDescCache.ColumnNames = L"Author\tTitle\tInformation";
    }
    else
    {
        isValid = false;
    }

    multiID = _multiID;
    shortName = _wcsdup(_shortName);
}

void DLLService::InitService(UINT_PTR serviceID)
{
    if (_selectService)
    {
        _selectService(multiID);
    }

    _serviceID = serviceID;
    _initService(
        [](const wchar_t* displayInfo, const wchar_t* playlistInfo, const wchar_t* filename, UINT_PTR serviceID) -> void {
            ItemInfo currentItem = { displayInfo, playlistInfo, filename };
            serviceListItemMap[serviceID].push_back(currentItem);
        },
        [](UINT_PTR serviceID, const wchar_t* optionName, const wchar_t* defaultValue, wchar_t* output, DWORD outputSize) -> void {
            GetPrivateProfileString(serviceMap[serviceID]->GetShortName(), optionName, defaultValue, output, outputSize, configFileName);
        },
        [](UINT_PTR serviceID, const wchar_t* optionName, const wchar_t* optionValue) -> void {
            WritePrivateProfileString(serviceMap[serviceID]->GetShortName(), optionName, optionValue, configFileName);
        },
        pluginDir,
        serviceID
        );
}

NodeDescriptor& DLLService::GetNodeDesc()
{
    return nodeDescCache;
}

int DLLService::GetNodeNum()
{
    return _getNodeNum();
}

void DLLService::InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl)
{
    if (_selectService)
    {
        _selectService(multiID);
    }

    std::lock_guard<std::mutex> guard(serviceListItemMapMutex);
    serviceListItemMap[_serviceID].clear();

    wchar_t skinPath[MAX_PATH];
    SendMessage(hwndWinampParent, WM_WA_IPC, (WPARAM)skinPath, IPC_GETSKINW);

    _invokeService(hwndWinampParent, hwndLibraryParent, hwndParentControl, skinPath);
}

const wchar_t* DLLService::GetFileName(const wchar_t* fileID)
{
    if (_selectService)
    {
        _selectService(multiID);
    }

    return _getFileName(fileID);
}

HWND DLLService::GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl)
{
    if (_selectService)
    {
        _selectService(multiID);
    }

    wchar_t skinPath[MAX_PATH];
    SendMessage(_hwndWinampParent, WM_WA_IPC, (WPARAM)skinPath, IPC_GETSKINW);

    return _getCustomDialog(_hwndWinampParent, _hwndLibraryParent, hwndParentControl, skinPath);
}

void DLLService::DestroyingCustomDialog() {}

bool DLLService::IsValid()
{
    return isValid;
}

const wchar_t* DLLService::GetShortName()
{
    return shortName;
}
