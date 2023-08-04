#include "..\ml_easysrv\easysrv.h"
#include <string.h>

int currentItem = 0;

const char* GetNodeName() {
    return "Example CPP Service Plugin";
}

ItemInfo InvokeService(int PlayerType) {
    currentItem++;
    ItemInfo currentItem = { L"Example Author", L"Example Title", L"This is a direct filename", L"e:\\example.mp3" };
    return currentItem;
}

ItemInfo InvokeNext(int PleyerType) {
    if (currentItem == 1) {
        currentItem++;
        ItemInfo currentItem = { L"Example Author", L"Example Title 2", L"This will use a reference", L"ref_examplefile" };
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