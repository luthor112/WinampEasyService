#include "easysrv_internal.h"
#include "img_util.h"

#include "Winamp/wa_ipc.h"

#include <Windows.h>
#include <string>
#include <sstream>
#include <codecvt>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define BUFSIZE 4096

HANDLE GetProcessOutputPipe(const wchar_t* cmdLine)
{
    HANDLE g_hChildStd_IN_Rd = NULL;
    HANDLE g_hChildStd_IN_Wr = NULL;
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    SECURITY_ATTRIBUTES saAttr;

    // Set the bInheritHandle flag so pipe handles are inherited
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT
    CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0);
    // Ensure the read handle to the pipe for STDOUT is not inherited
    SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    // Set up members of the STARTUPINFO structure
    // This structure specifies the STDIN and STDOUT handles for redirection
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    siStartInfo.wShowWindow = SW_HIDE;
    siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;

    // Create the child process
    bSuccess = CreateProcess(NULL,
        const_cast<wchar_t*>(cmdLine),  // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

    if (bSuccess)
    {
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_IN_Rd);
    }

    return g_hChildStd_OUT_Rd;
}

std::string ReadProcessOutput(const wchar_t* cmdLine)
{
    DWORD dwRead;
    char chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;

    HANDLE g_hChildStd_OUT_Rd = GetProcessOutputPipe(cmdLine);

    std::ostringstream outSS;
    for (;;)
    {
        bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;

        outSS.write(chBuf, dwRead);
    }

    return outSS.str();
}

EXEService::EXEService(const wchar_t* exeName, const wchar_t* _shortName, int _multiID)
{
    _exeName = _wcsdup(exeName);
    shortName = _wcsdup(_shortName);
    multiID = _multiID;

    // Get and cache Node Description
    wchar_t cmdLine[1024];
    if (multiID == 0)
        wsprintf(cmdLine, L"%s GetNodeDesc", _exeName);
    else
        wsprintf(cmdLine, L"%s select %d GetNodeDesc", _exeName, multiID);
    std::string s1 = ReadProcessOutput(cmdLine);
    std::wstring ws1 = std::wstring(s1.begin(), s1.end());
    std::wistringstream inSS(ws1);

    try
    {
        std::wstring line;
        std::getline(inSS, line);
        if (line[line.length() - 1] == '\r' || line[line.length() - 1] == '\n')
            line[line.length() - 1] = '\0';
        if (line == L"INVALID_SERVICE")
            throw std::runtime_error("INVALID_SERVICE");
        if (!line.empty())
            nodeDescCache.Category = _wcsdup(line.c_str());
        else
            nodeDescCache.Category = NULL;

        std::getline(inSS, line);
        nodeDescCache.NodeName = _wcsdup(line.c_str());

        std::getline(inSS, line);
        if (!line.empty())
            nodeDescCache.ColumnNames = _wcsdup(line.c_str());
        else
            nodeDescCache.ColumnNames = L"Author\tTitle\tInformation";

        std::getline(inSS, line);
        nodeDescCache.Capabilities = std::stoul(line);

        isValid = true;
    }
    catch (const std::exception& e)
    {
        isValid = false;
    }
}

void EXEService::InitService(UINT_PTR serviceID)
{
    _serviceID = serviceID;
}

NodeDescriptor& EXEService::GetNodeDesc()
{
    return nodeDescCache;
}

int EXEService::GetNodeNum()
{
    wchar_t cmdLine[1024];
    wsprintf(cmdLine, L"%s GetNodeNum", _exeName);
    std::string s1 = ReadProcessOutput(cmdLine);
    std::wstring ws1 = std::wstring(s1.begin(), s1.end());
    std::wistringstream inSS(ws1);

    std::wstring line;
    std::getline(inSS, line);
    return std::stoul(line);
}

void EXEService::InvokeService(HWND hwndWinampParent, HWND hwndLibraryParent, HWND hwndParentControl)
{
    std::lock_guard<std::mutex> guard(serviceListItemMapMutex);
    serviceListItemMap[_serviceID].clear();

    wchar_t skinPath[MAX_PATH];
    SendMessage(hwndWinampParent, WM_WA_IPC, (WPARAM)skinPath, IPC_GETSKINW);

    wchar_t genexPath[MAX_PATH];
    wsprintf(genexPath, L"%s\\genex.bmp", skinPath);
    if (!PathFileExists(genexPath))
    {
        HBITMAP skinBitmap = (HBITMAP)SendMessage(hwndWinampParent, WM_WA_IPC, 0, IPC_GET_GENSKINBITMAP);
        if (skinBitmap)
        {
            WriteBitmap(skinBitmap, genexPath);
        }
    }

    wchar_t cmdLine[1024];
    if (multiID == 0)
        wsprintf(cmdLine, L"%s InvokeService %d %d %d \"%s\" \"%s\" \"%s\" \"%s\" %d", _exeName, hwndWinampParent, hwndLibraryParent, hwndParentControl, pluginDir, skinPath, configFileName, shortName, _serviceID);
    else
        wsprintf(cmdLine, L"%s select %d InvokeService %d %d %d \"%s\" \"%s\" \"%s\" \"%s\" %d", _exeName, multiID, hwndWinampParent, hwndLibraryParent, hwndParentControl, pluginDir, skinPath, configFileName, shortName, _serviceID);
    std::string s1 = ReadProcessOutput(cmdLine);
    std::wstring ws1 = std::wstring(s1.begin(), s1.end());
    std::wistringstream inSS(ws1);

    std::wstring infoLine;
    std::wstring plTitleLine;
    std::wstring fileNameLine;

    std::getline(inSS, infoLine);
    while (!infoLine.empty()) {
        std::getline(inSS, plTitleLine);
        std::getline(inSS, fileNameLine);

        if (fileNameLine[fileNameLine.length() - 1] == '\r' || fileNameLine[fileNameLine.length() - 1] == '\n')
            fileNameLine[fileNameLine.length() - 1] = '\0';

        ItemInfo currentItem = {
            _wcsdup(infoLine.c_str()),
            _wcsdup(plTitleLine.c_str()),
            _wcsdup(fileNameLine.c_str())
        };
        serviceListItemMap[_serviceID].push_back(currentItem);

        std::getline(inSS, infoLine);
    }
}

const wchar_t* EXEService::GetFileName(const wchar_t* fileID)
{
    wchar_t cmdLine[1024];
    if (multiID == 0)
        wsprintf(cmdLine, L"%s GetFileName %s", _exeName, fileID);
    else
        wsprintf(cmdLine, L"%s select %d GetFileName %s", _exeName, multiID, fileID);
    std::string s1 = ReadProcessOutput(cmdLine);
    std::wstring ws1 = std::wstring(s1.begin(), s1.end());
    std::wistringstream inSS(ws1);

    std::wstring line;
    std::getline(inSS, line);
    return _wcsdup(line.c_str());
}

HWND EXEService::GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl)
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    wchar_t skinPath[MAX_PATH];
    wchar_t* skinName = (wchar_t*)SendMessage(_hwndWinampParent, WM_WA_IPC, (WPARAM)skinPath, IPC_GETSKINW);

    wchar_t genexPath[MAX_PATH];
    wsprintf(genexPath, L"%s\\genex.bmp", skinPath);
    if (!PathFileExists(genexPath))
    {
        HBITMAP skinBitmap = (HBITMAP)SendMessage(_hwndWinampParent, WM_WA_IPC, 0, IPC_GET_GENSKINBITMAP);
        if (skinBitmap)
        {
            WriteBitmap(skinBitmap, genexPath);
        }
    }

    wchar_t cmdLine[1024];
    if (multiID == 0)
        wsprintf(cmdLine, L"%s GetCustomDialog %d %d %d \"%s\" \"%s\" \"%s\" \"%s\" %d", _exeName, _hwndWinampParent, _hwndLibraryParent, hwndParentControl, pluginDir, skinPath, configFileName, shortName, _serviceID);
    else
        wsprintf(cmdLine, L"%s select %d GetCustomDialog %d %d %d \"%s\" \"%s\" \"%s\" \"%s\" %d", _exeName, multiID, _hwndWinampParent, _hwndLibraryParent, hwndParentControl, pluginDir, skinPath, configFileName, shortName, _serviceID);
    if (CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        customDialogPID = pi.dwProcessId;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    // Return NULL, as the foreign process will handle everything
    return NULL;
}

void EXEService::DestroyingCustomDialog()
{
    if (customDialogPID != -1)
    {
        HANDLE procHandle = OpenProcess(PROCESS_TERMINATE, false, customDialogPID);
        TerminateProcess(procHandle, 0);
        CloseHandle(procHandle);

        customDialogPID = -1;
    }
}

bool EXEService::IsValid()
{
    return isValid;
}

const wchar_t* EXEService::GetShortName()
{
    return shortName;
}
