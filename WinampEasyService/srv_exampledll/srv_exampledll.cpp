#include "..\ml_easysrv\easysrv.h"
#include <string.h>

const char* GetNodeName() {
    return "Example Node 1";
}

void InvokeService(AddItemFunction addItem, int PlayerType) {
    addItem("Example Author", "Example Title", "This is a direct filename", "e:\\example.mp3");
    addItem("Example Author", "Example Title 2", "This will use a reference", "ref:examplefile");
}

const char* GetFileName(const char* fileID)
{
    if (!strcmp(fileID, "ref:examplefile")) {
        return "e:\\example.mp3";
    } else {
        return "This really shouldn't happen...";
    }
}