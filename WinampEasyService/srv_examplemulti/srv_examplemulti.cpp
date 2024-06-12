#include "..\ml_easysrv\easysrv.h"
#include <string.h>

AddItemFunc AddItem;
GetOptionFunc GetOption;
SetOptionFunc SetOption;
const wchar_t* myDirectory;

UINT_PTR serviceIDs[3];
int currentMultiID = 0;

void InitService(AddItemFunc addItemFunc, GetOptionFunc getOptionFunc, SetOptionFunc setOptionFunc, const wchar_t* pluginDir, UINT_PTR serviceID)
{
    AddItem = addItemFunc;
    GetOption = getOptionFunc;
    SetOption = setOptionFunc;
    myDirectory = pluginDir;
    serviceIDs[currentMultiID] = serviceID;
}

int GetNodeNum()
{
    return 3;
}

void SelectService(int multiID)
{
    currentMultiID = multiID;
}

NodeDescriptor GetNodeDesc()
{
    NodeDescriptor desc;

    switch (currentMultiID)
    {
    default:
    case 0: desc = { L"Examples", L"Example CPP Multi Plugin 1", L"Title\tType of item", CAP_MULTISERVICE };
          break;
    case 1: desc = { L"Examples", L"Example CPP Multi Plugin 2", L"Title\tType of item", CAP_MULTISERVICE };
          break;
    case 2: desc = { L"Examples", L"Example CPP Multi Plugin 3", L"Title\tType of item", CAP_MULTISERVICE };
          break;
    }

    return desc;
}

void InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl, wchar_t* skinPath)
{
    switch (currentMultiID)
    {
    default:
    case 0: AddItem(L"Example Title\tHello from Subservice 1!", L"Example PLaylist Title", L"e:\\example.mp3", serviceIDs[currentMultiID]);
            AddItem(L"Example Title 2\tHello from Subservice 1!", L"Example PLaylist Title 2", L"e:\\example.mp3", serviceIDs[currentMultiID]);
            break;
    case 1: AddItem(L"Example Title\tHello from Subservice 2!", L"Example PLaylist Title", L"e:\\example.mp3", serviceIDs[currentMultiID]);
            AddItem(L"Example Title 2\tHello from Subservice 2!", L"Example PLaylist Title 2", L"e:\\example.mp3", serviceIDs[currentMultiID]);
            break;
    case 2: AddItem(L"Example Title\tHello from Subservice 3!", L"Example PLaylist Title", L"e:\\example.mp3", serviceIDs[currentMultiID]);
            AddItem(L"Example Title 2\tHello from Subservice 3!", L"Example PLaylist Title 2", L"e:\\example.mp3", serviceIDs[currentMultiID]);
            break;
    }
}
