#pragma once

#ifdef EASYSRV_IMPORTING
#define EASY_API __declspec(dllimport)
#else
#define EASY_API __declspec(dllexport)
#endif

#define PLAYERTYPE_WINAMP 0
#define PLAYERTYPE_WACUP 1

typedef void (*AddItemFunction)(const char* author, const char* title, const char* info, const char* filename);

#ifdef __cplusplus
extern "C" {
#endif

    EASY_API const char* GetNodeName();
    EASY_API void InvokeService(AddItemFunction addItem, int PlayerType);
    EASY_API const char* GetFileName(const char* fileID);

#ifdef __cplusplus
}
#endif
