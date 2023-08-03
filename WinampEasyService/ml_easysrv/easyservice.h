#pragma once

#include <vector>

#define PLAYERTYPE_WINAMP 0
#define PLAYERTYPE_WACUP 1

struct ItemInfo
{
    const wchar_t* author;
    const wchar_t* title;
    const wchar_t* info;
    const wchar_t* filename;
};

typedef const char* (*GetNodeNameFunc)();
typedef ItemInfo (*InvokeServiceFunc)(int PlayerType);
typedef ItemInfo (*InvokeNextFunc)(int PleyerType);
typedef const wchar_t* (*GetFileNameFunc)(const wchar_t* fileID);

class EasyService
{
public:
    virtual const char* GetNodeName() = 0;
    virtual std::vector<ItemInfo> InvokeService() = 0;
    virtual const wchar_t* GetFileName(const wchar_t* fileID) = 0;

    virtual ~EasyService() {}
};

class DLLService : public EasyService
{
public:
    DLLService(const wchar_t* dllName, int _playerType);

    virtual const char* GetNodeName();
    virtual std::vector<ItemInfo> InvokeService();
    virtual const wchar_t* GetFileName(const wchar_t* fileID);

private:
    int playerType;
    GetNodeNameFunc _getNodeName;
    InvokeServiceFunc _invokeService;
    InvokeNextFunc _invokeNext;
    GetFileNameFunc _getFileName;
};

// TODO
/*class EXEService : public EasyService
{
public:
    EXEService(const wchar_t* exeName);

    virtual const char* GetNodeName();
    virtual std::vector<ItemInfo> InvokeService();
    virtual const wchar_t* GetFileName(const wchar_t* fileID);
};*/
