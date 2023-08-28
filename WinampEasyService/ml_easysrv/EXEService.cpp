#include "easyservice.h"

#include <Windows.h>
#include <string>
#include <sstream>
#include <codecvt>

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

std::wstring ReadProcessOutputW(const wchar_t* cmdLine)
{
    DWORD dwRead;
    wchar_t chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;

    HANDLE g_hChildStd_OUT_Rd = GetProcessOutputPipe(cmdLine);

    std::wostringstream outSS;
    for (;;)
    {
        bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE * sizeof(wchar_t), &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;

        outSS.write(chBuf, dwRead);
    }

    return outSS.str();
}

EXEService::EXEService(const wchar_t* exeName, int _playerType)
{
    playerType = _playerType;
    _exeName = _wcsdup(exeName);
}

const wchar_t* EXEService::GetNodeName()
{
    wchar_t cmdLine[1024];
    wsprintf(cmdLine, L"%s GetNodeName", _exeName);
    std::string s1 = ReadProcessOutput(cmdLine);
    std::wstring ws1 = std::wstring(s1.begin(), s1.end());
    std::wistringstream inSS(ws1);

    std::wstring line;
    std::getline(inSS, line);
    return _wcsdup(line.c_str());
}

const wchar_t* EXEService::GetColumnNames()
{
    // TODO
    return NULL;
}

std::vector<CustomItemInfo> EXEService::InvokeService()
{
    // TODO
    std::vector<CustomItemInfo> retItems;
    return retItems;
}

/*std::vector<ItemInfo> EXEService::InvokeService()
{
    wchar_t cmdLine[1024];
    wsprintf(cmdLine, L"%s InvokeService %s", _exeName, playerType == PLAYERTYPE_WACUP ? L"PLAYERTYPE_WACUP" : L"PLAYERTYPE_WINAMP");
    // Bugfixing
    std::string s1 = ReadProcessOutput(cmdLine);
    std::wstring ws1 = std::wstring(s1.begin(), s1.end());
    std::wistringstream inSS(ws1);

    std::wstring authorLine;
    std::wstring titleLine;
    std::wstring infoLine;
    std::wstring fileNameLine;
    
    std::vector<ItemInfo> retItems;
    std::getline(inSS, authorLine);
    while (!authorLine.empty()) {
        std::getline(inSS, titleLine);
        std::getline(inSS, infoLine);
        std::getline(inSS, fileNameLine);

        if (fileNameLine[fileNameLine.length() - 1] == '\r' || fileNameLine[fileNameLine.length() - 1] == '\n')
            fileNameLine[fileNameLine.length() - 1] = '\0';

        ItemInfo currentItem = {
            _wcsdup(authorLine.c_str()),
            _wcsdup(titleLine.c_str()),
            _wcsdup(infoLine.c_str()),
            _wcsdup(fileNameLine.c_str())
        };
        retItems.push_back(currentItem);

        std::getline(inSS, authorLine);
    }

    return retItems;
}*/

const wchar_t* EXEService::GetFileName(const wchar_t* fileID)
{
    wchar_t cmdLine[1024];
    wsprintf(cmdLine, L"%s GetFileName %s", _exeName, fileID);
    // Bugfixing
    std::string s1 = ReadProcessOutput(cmdLine);
    std::wstring ws1 = std::wstring(s1.begin(), s1.end());
    std::wistringstream inSS(ws1);

    std::wstring line;
    std::getline(inSS, line);
    return _wcsdup(line.c_str());
}

HWND EXEService::GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl)
{
    return NULL;
}
