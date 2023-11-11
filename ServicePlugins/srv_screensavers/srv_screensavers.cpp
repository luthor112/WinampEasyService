#include "..\..\WinampEasyService\ml_easysrv\easysrv.h"

#include <Windows.h>
#include "Winamp/wa_ipc.h"
#include "gen_ml/ml.h"

#include <windowsx.h>
#include "shlobj.h"
#include "gen_ml/ml_ipc_0313.h"
#include "gen_ml/childwnd.h"
#include "Winamp/wa_dlg.h"
#include "resource.h"

#include <string.h>
#include <vector>
#include <Shlwapi.h>
#include <string>
#include <fstream>

#include <thread>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")

GetOptionFunc GetOption;
SetOptionFunc SetOption;
const wchar_t* myDirectory;
UINT_PTR myServiceID;

HINSTANCE myself = NULL;
HWND hwndWinampParent = NULL;
HWND hwndLibraryParent = NULL;

wchar_t scrPath[MAX_PATH];
DWORD currentPid = -1;
std::vector<const wchar_t*> scrFiles;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	myself = (HINSTANCE)hModule;
	return TRUE;
}

void InitService(AddItemFunc addItemFunc, GetOptionFunc getOptionFunc, SetOptionFunc setOptionFunc, const wchar_t* pluginDir, UINT_PTR serviceID)
{
	GetOption = getOptionFunc;
	SetOption = setOptionFunc;
	myDirectory = pluginDir;
	myServiceID = serviceID;
}

NodeDescriptor GetNodeDesc()
{
	NodeDescriptor desc = { L"Misc", L"Screensavers", NULL, CAP_CUSTOMDIALOG };
	return desc;
}

void runProcessInBackground(wchar_t* cmdLine)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		currentPid = pi.dwProcessId;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

void stopProcess()
{
	if (currentPid != -1)
	{
		HANDLE procHandle = OpenProcess(PROCESS_TERMINATE, false, currentPid);
		TerminateProcess(procHandle, 0);
		CloseHandle(procHandle);

		currentPid = -1;
	}
}

void getScrFiles()
{
	wchar_t searchCriteria[1024];
	WIN32_FIND_DATA FindFileData;
	HANDLE searchHandle = INVALID_HANDLE_VALUE;

	wsprintf(searchCriteria, L"%s*.scr", scrPath);
	searchHandle = FindFirstFile(searchCriteria, &FindFileData);
	if (searchHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			scrFiles.push_back(_wcsdup(FindFileData.cFileName));
		} while (FindNextFile(searchHandle, &FindFileData));
		FindClose(searchHandle);
	}
}

void runScr(HWND hwnd, int fileIndex)
{
	HWND displayWnd = GetDlgItem(hwnd, IDC_DISPLAY);

	wchar_t cmdLine[1024];
	wsprintf(cmdLine, L"\"%s%s\" /p %d", scrPath, scrFiles[fileIndex], displayWnd);

	stopProcess();
	runProcessInBackground(cmdLine);
}

typedef int (*HookDialogFunc)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HookDialogFunc ml_hook_dialog_msg = 0;

static BOOL view_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	/* gen_ml has some helper functions to deal with skinned dialogs,
	   we're going to grab their function pointers.
		 for definition of magic numbers, see gen_ml/ml.h	 */
	if (!ml_hook_dialog_msg)
	{
		/* skinning helper functions */
		ml_hook_dialog_msg = (HookDialogFunc)SendMessage(hwndLibraryParent, WM_ML_IPC, (WPARAM)2, ML_IPC_SKIN_WADLG_GETFUNC);
	}

	/* skin dialog */
	MLSKINWINDOW sw;
	sw.skinType = SKINNEDWND_TYPE_DIALOG;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = hwnd;
	MLSkinWindow(hwndLibraryParent, &sw);

	/* skin combobox */
	sw.skinType = SKINNEDWND_TYPE_COMBOBOX;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_FILECOMBO);
	MLSkinWindow(hwndLibraryParent, &sw);

	return FALSE;
}

static BOOL view_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state != SIZE_MINIMIZED)
	{
		HWND displayWnd = GetDlgItem(hwnd, IDC_DISPLAY);
		HWND fileCombo = GetDlgItem(hwnd, IDC_FILECOMBO);

		MoveWindow(displayWnd, 0, 0, cx, cy - 28, TRUE);
		MoveWindow(fileCombo, 1, cy - 25, 200, 25, TRUE);
	}

	return FALSE;
}

static BOOL view_OnDestroy(HWND hwnd)
{
	stopProcess();

	return FALSE;
}

static BOOL view_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) {
	case IDC_FILECOMBO:
	{
		if (codeNotify == CBN_SELCHANGE)
		{
			HWND comboWnd = GetDlgItem(hwnd, IDC_FILECOMBO);
			int fileIndex = SendMessage(comboWnd, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

			runScr(hwnd, fileIndex);
		}
	}
	break;
	}
	return 0;
}

LRESULT CALLBACK customDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/* first, ask the dialog skinning system if it wants to do anything with the message
	   the function pointer gets set during WM_INITDIALOG so might be NULL for the first few messages
		 in theory we could grab it right here if we don't have it, but it's not necessary
		 and I wanted to put all the function pointer gathering code in the same place for this example	*/
	if (ml_hook_dialog_msg)
	{
		INT_PTR a = ml_hook_dialog_msg(hwndDlg, uMsg, wParam, lParam);
		if (a)
			return a;
	}

	switch (uMsg) {
		HANDLE_MSG(hwndDlg, WM_INITDIALOG, view_OnInitDialog);
		HANDLE_MSG(hwndDlg, WM_COMMAND, view_OnCommand);
		HANDLE_MSG(hwndDlg, WM_SIZE, view_OnSize);
	return 0;
	HANDLE_MSG(hwndDlg, WM_DESTROY, view_OnDestroy);

	}
	return FALSE;
}

HWND GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl, wchar_t* skinPath)
{
	// Save HWNDs
	hwndWinampParent = _hwndWinampParent;
	hwndLibraryParent = _hwndLibraryParent;

	// Create dialog
	HWND dialogWnd = CreateDialog(myself, MAKEINTRESOURCE(IDD_VIEW_CUSTOM), hwndParentControl, (DLGPROC)customDialogProc);

	// Get SCR files
	GetOption(myServiceID, L"scrdir", L"c:\\Windows\\System32\\", scrPath, MAX_PATH);
	getScrFiles();

	// Fill SCR combobox
	HWND comboWnd = GetDlgItem(dialogWnd, IDC_FILECOMBO);
	for (const wchar_t* scrName : scrFiles)
		SendMessage(comboWnd, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)scrName);

	// Read default SCR
	int defaultScr = 0;
	wchar_t defaultScrOpt[MAX_PATH];
	GetOption(myServiceID, L"default", L"", defaultScrOpt, MAX_PATH);
	if (wcslen(defaultScrOpt) > 0)
		defaultScr = _wtoi(defaultScrOpt);
	
	// Run SCR
	SendMessage(comboWnd, CB_SETCURSEL, (WPARAM)defaultScr, (LPARAM)0);
	runScr(dialogWnd, defaultScr);

	return dialogWnd;
}
