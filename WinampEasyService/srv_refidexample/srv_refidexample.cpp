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
    NodeDescriptor desc = { L"Examples", L"Custom RefId Example", L"Title\tType of item", CAP_CUSTOMREFID };
    return desc;
}

const wchar_t* GetCustomRefId()
{
    return L"refidexample";
}

void InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl, wchar_t* skinPath)
{
    AddItem(L"Example Title\tThis is a direct filename", L"Example PLaylist Title", L"e:\\example.mp3", myServiceID);
    AddItem(L"Example Title 2\tThis will use a default reference", L"Example PLaylist Title 2", L"ref_examplefile", myServiceID);
    AddItem(L"Example Title 3\tThis will use a custom reference", L"Example PLaylist Title 3", L"refidexample_ref_examplefile.ref", myServiceID);
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
