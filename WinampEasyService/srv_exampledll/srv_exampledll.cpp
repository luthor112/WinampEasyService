#include "..\ml_easysrv\easysrv.h"
#include <string.h>

int currentItemIndex = 0;

const wchar_t* GetNodeName() {
    return L"Example CPP Service Plugin";
}

ItemInfo InvokeService(int PlayerType) {
    currentItemIndex = 0;
    ItemInfo currentItem = { L"Example Author", L"Example Title", L"This is a direct filename", L"e:\\example.mp3" };

    currentItemIndex++;
    return currentItem;
}

ItemInfo InvokeNext(int PleyerType) {
    if (currentItemIndex == 1) {
        ItemInfo currentItem = { L"Example Author", L"Example Title 2", L"This will use a reference", L"ref_examplefile" };

        currentItemIndex++;
        return currentItem;
    } else {
        return ItemInfo();
    }
}

const wchar_t* GetFileName(const wchar_t* fileID)
{
    if (!wcscmp(fileID, L"ref_examplefile")) {
        return L"e:\\example.mp3";
    } else {
        return L"This really shouldn't happen...";
    }
}