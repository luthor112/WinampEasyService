/*#include "easysrv_internal.h"

#include "Winamp/wa_ipc.h"

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
    if (nodeNameCache == NULL)
    {
        wchar_t cmdLine[1024];
        wsprintf(cmdLine, L"%s GetNodeName", _exeName);
        std::string s1 = ReadProcessOutput(cmdLine);
        std::wstring ws1 = std::wstring(s1.begin(), s1.end());
        std::wistringstream inSS(ws1);

        std::wstring line;
        std::getline(inSS, line);
        nodeNameCache = _wcsdup(line.c_str());
    }

    return nodeNameCache;
}

const wchar_t* EXEService::GetColumnNames()
{
    if (columnNameCache == NULL)
    {
        wchar_t cmdLine[1024];
        wsprintf(cmdLine, L"%s GetColumnNames", _exeName);
        std::string s1 = ReadProcessOutput(cmdLine);
        std::wstring ws1 = std::wstring(s1.begin(), s1.end());
        std::wistringstream inSS(ws1);

        std::wstring line;
        std::getline(inSS, line);
        if (!line.empty())
        {
            columnNameCache = _wcsdup(line.c_str());
            customColumnsSupported = TRUE;
        }
        else
        {
            columnNameCache = L"Author\tTitle\tInformation";
            customColumnsSupported = FALSE;
        }
    }

    return columnNameCache;
}

std::vector<CustomItemInfo> EXEService::InvokeService()
{
    wchar_t cmdLine[1024];
    if (customColumnsSupported)
        wsprintf(cmdLine, L"%s InvokeServiceCustom %s", _exeName, playerType == PLAYERTYPE_WACUP ? L"PLAYERTYPE_WACUP" : L"PLAYERTYPE_WINAMP");
    else
        wsprintf(cmdLine, L"%s InvokeService %s", _exeName, playerType == PLAYERTYPE_WACUP ? L"PLAYERTYPE_WACUP" : L"PLAYERTYPE_WINAMP");
    // Bugfixing
    std::string s1 = ReadProcessOutput(cmdLine);
    std::wstring ws1 = std::wstring(s1.begin(), s1.end());
    std::wistringstream inSS(ws1);

    std::vector<CustomItemInfo> retItems;
    if (customColumnsSupported)
    {
        std::wstring infoLine;
        std::wstring plTitleLine;
        std::wstring fileNameLine;


        std::getline(inSS, infoLine);
        while (!infoLine.empty()) {
            std::getline(inSS, plTitleLine);
            std::getline(inSS, fileNameLine);

            if (fileNameLine[fileNameLine.length() - 1] == '\r' || fileNameLine[fileNameLine.length() - 1] == '\n')
                fileNameLine[fileNameLine.length() - 1] = '\0';

            CustomItemInfo currentItem = {
                _wcsdup(infoLine.c_str()),
                _wcsdup(plTitleLine.c_str()),
                _wcsdup(fileNameLine.c_str())
            };
            retItems.push_back(currentItem);

            std::getline(inSS, infoLine);
        }
    }
    else
    {
        std::wstring authorLine;
        std::wstring titleLine;
        std::wstring infoLine;
        std::wstring fileNameLine;

        std::getline(inSS, authorLine);
        while (!authorLine.empty()) {
            std::getline(inSS, titleLine);
            std::getline(inSS, infoLine);
            std::getline(inSS, fileNameLine);

            if (fileNameLine[fileNameLine.length() - 1] == '\r' || fileNameLine[fileNameLine.length() - 1] == '\n')
                fileNameLine[fileNameLine.length() - 1] = '\0';

            wchar_t ciiInfo[1024];
            wsprintf(ciiInfo, L"%s\t%s\t%s", authorLine.c_str(), titleLine.c_str(), infoLine.c_str());
            wchar_t plTitle[1024];
            wsprintf(plTitle, L"%s - %s", authorLine.c_str(), titleLine.c_str());

            CustomItemInfo currentItem = {
                _wcsdup(ciiInfo),
                _wcsdup(plTitle),
                _wcsdup(fileNameLine.c_str())
            };
            retItems.push_back(currentItem);

            std::getline(inSS, authorLine);
        }
    }

    return retItems;
}

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
    if (customDialogSupported == -1)
    {
        wchar_t cmdLine[1024];
        wsprintf(cmdLine, L"%s CanGetCustomDialog", _exeName);
        std::string s1 = ReadProcessOutput(cmdLine);
        std::wstring ws1 = std::wstring(s1.begin(), s1.end());
        std::wistringstream inSS(ws1);

        std::wstring line;
        std::getline(inSS, line);
        if (!line.empty())
        {
            if (line.c_str()[0] == '1')
                customDialogSupported = 1;
            else
                customDialogSupported = 0;
        }
        else
        {
            customDialogSupported = 0;
        }
    }

    if (customDialogSupported == 1)
    {
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        wchar_t skinPath[MAX_PATH];
        SendMessage(_hwndWinampParent, WM_WA_IPC, (WPARAM)skinPath, IPC_GETSKINW);

        wchar_t cmdLine[1024];
        wsprintf(cmdLine, L"%s GetCustomDialog %d %d %d \"%s\"", _exeName, _hwndWinampParent, _hwndLibraryParent, hwndParentControl, skinPath);
        if (CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            customDialogPID = pi.dwProcessId;
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
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
*/
