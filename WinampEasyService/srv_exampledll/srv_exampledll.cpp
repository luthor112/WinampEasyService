#include "..\ml_easysrv\easysrv.h"
#include <string.h>

AddItemFunc AddItem;
GetOptionFunc GetOption;
SetOptionFunc SetOption;
const wchar_t* myDirectory;
UINT_PTR myServiceID;

void InitService(AddItemFunc addItemFunc, GetOptionFunc getOptionFunc, SetOptionFunc setOptionFunc, const wchar_t* pluginDir, UINT_PTR serviceID)
{
    AddItem = addItemFunc;
    GetOption = getOptionFunc;
    SetOption = setOptionFunc;
    myDirectory = pluginDir;
    myServiceID = serviceID;
}

NodeDescriptor GetNodeDesc()
{
    NodeDescriptor desc = { L"Examples", L"Example CPP Plugin", L"Title\tType of item", CAP_DEFAULT };
    return desc;
}

void InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl, wchar_t* skinPath)
{
    AddItem(L"Example Title\tThis is a direct filename", L"Example PLaylist Title", L"e:\\example.mp3", myServiceID);
    AddItem(L"Example Title 2\tThis will use a reference", L"Example PLaylist Title 2", L"ref_examplefile", myServiceID);
}

const wchar_t* GetFileName(const wchar_t* fileID)
{
    if (!wcscmp(fileID, L"ref_examplefile")) {
        return L"e:\\example.mp3";
    }
    else {
        return L"This really shouldn't happen...";
    }
}
