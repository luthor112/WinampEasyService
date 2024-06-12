#define _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR
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

#include <thread>
#include <mutex>

#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")

#define RESCAN_TIMER_ID 111

const wchar_t* myDirectory;
UINT_PTR myServiceID;

HINSTANCE myself = NULL;
HWND hwndWinampParent = NULL;
HWND hwndLibraryParent = NULL;
bool autoApply = FALSE;
wchar_t tempPath[MAX_PATH];

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	myself = (HINSTANCE)hModule;
	return TRUE;
}

void InitService(AddItemFunc addItemFunc, GetOptionFunc getOptionFunc, SetOptionFunc setOptionFunc, const wchar_t* pluginDir, UINT_PTR serviceID)
{
	myDirectory = pluginDir;
	myServiceID = serviceID;
}

NodeDescriptor GetNodeDesc()
{
	NodeDescriptor desc = { L"Skins", L"Skin Browser v2", NULL, CAP_CUSTOMDIALOG };
	return desc;
}

void runProcessInBackground(wchar_t* cmdLine)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

std::vector<const wchar_t*> fileList;
HIMAGELIST previewImageList;
std::mutex fileListMutex;

void clearList(HWND hwnd)
{
	std::lock_guard<std::mutex> guard(fileListMutex);
	fileList.clear();

	HWND hwndList = GetDlgItem(hwnd, IDC_LIST);
	ListView_DeleteAllItems(hwndList);

	ImageList_Destroy(previewImageList);
	previewImageList = ImageList_Create(275, 116, ILC_COLOR24, 20, 200);
	ListView_SetImageList(hwndList, previewImageList, LVSIL_NORMAL);
	ListView_SetImageList(hwndList, previewImageList, LVSIL_SMALL);
}

// Returns TRUE if the entry was new and added
bool addItemToList(HWND hwnd, const wchar_t* filename)
{
	for (const wchar_t* currFileName : fileList)
	{
		if (!wcscmp(currFileName, filename))
			return FALSE;
	}

	fileList.push_back(filename);

	wchar_t thumbFile[1024];
	wsprintf(thumbFile, L"%swmp_skin_thm\\%s.bmp", tempPath, filename);

	if (!PathFileExists(thumbFile))
	{
		wchar_t helperCmd[1024];
		wsprintf(helperCmd, L"\"%s\\skinbrowser2_helper.exe\" single \"%s\\..\\Skins\\%s\" \"%swmp_skin_thm\"", myDirectory, myDirectory, filename, tempPath);
		runProcessInBackground(helperCmd);
	}
	
	HBITMAP hImage = (HBITMAP)LoadImage(NULL, thumbFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	ImageList_Add(previewImageList, hImage, NULL);
	DeleteObject(hImage);

	HWND hwndList = GetDlgItem(hwnd, IDC_LIST);

	LVITEM lvi = { 0, };
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	lvi.iItem = fileList.size() - 1;
	lvi.pszText = (LPTSTR)filename;
	lvi.cchTextMax = lstrlenW(filename);
	lvi.iImage = fileList.size() - 1;
	SendMessage(hwndList, LVM_INSERTITEMW, 0, (LPARAM)&lvi);

	return TRUE;
}

// Returns the first entry that's being newly added to the list
const wchar_t* fillFileList(HWND hwnd)
{
	std::lock_guard<std::mutex> guard(fileListMutex);
	const wchar_t* newFile = NULL;

	wchar_t searchCriteria[1024];
	wsprintf(searchCriteria, L"%s\\..\\Skins\\*", myDirectory);

	WIN32_FIND_DATA FindFileData;
	HANDLE searchHandle = FindFirstFile(searchCriteria, &FindFileData);
	if (searchHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindFileData.cFileName[0] != '.')
			{
				const wchar_t* fileNameCopy = _wcsdup(FindFileData.cFileName);
				if (addItemToList(hwnd, fileNameCopy))
				{
					newFile = fileNameCopy;
				}
			}
		} while (FindNextFile(searchHandle, &FindFileData));
		FindClose(searchHandle);
	}

	return newFile;
}

typedef int (*HookDialogFunc)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HookDialogFunc ml_hook_dialog_msg = 0;

typedef void (*DrawFunc)(HWND hwndDlg, int* tab, int tabsize);
static DrawFunc ml_draw = 0;

static BOOL view_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	/* gen_ml has some helper functions to deal with skinned dialogs,
	   we're going to grab their function pointers.
		 for definition of magic numbers, see gen_ml/ml.h	 */
	if (!ml_hook_dialog_msg)
	{
		/* skinning helper functions */
		ml_hook_dialog_msg = (HookDialogFunc)SendMessage(hwndLibraryParent, WM_ML_IPC, (WPARAM)2, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_draw = (DrawFunc)SendMessage(hwndLibraryParent, WM_ML_IPC, (WPARAM)3, ML_IPC_SKIN_WADLG_GETFUNC);
	}

	HWND listWnd = GetDlgItem(hwnd, IDC_LIST);

	/* skin dialog */
	MLSKINWINDOW sw;
	sw.skinType = SKINNEDWND_TYPE_DIALOG;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = hwnd;
	MLSkinWindow(hwndLibraryParent, &sw);

	/* skin listview */
	sw.skinType = SKINNEDWND_TYPE_LISTVIEW;
	sw.style = SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_LIST);;
	MLSkinWindow(hwndLibraryParent, &sw);

	/* skin checkbox */
	sw.skinType = SKINNEDWND_TYPE_AUTO;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_CHECK);
	MLSkinWindow(hwndLibraryParent, &sw);

	/* skin button */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_RESCAN);
	MLSkinWindow(hwndLibraryParent, &sw);

	return FALSE;
}

static BOOL view_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state != SIZE_MINIMIZED)
	{
		HWND listWnd = GetDlgItem(hwnd, IDC_LIST);
		HWND checkWnd = GetDlgItem(hwnd, IDC_CHECK);
		HWND buttonWnd = GetDlgItem(hwnd, IDC_RESCAN);

		MoveWindow(listWnd, 0, 0, cx, cy - 46, TRUE);
		MoveWindow(checkWnd, 1, cy - 43, 300, 15, TRUE);
		MoveWindow(buttonWnd, 1, cy - 25, 300, 25, TRUE);
	}

	return FALSE;
}

static BOOL view_OnDestroy(HWND hwnd)
{
	KillTimer(hwnd, RESCAN_TIMER_ID);

	return FALSE;
}

static BOOL view_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) {
	case IDC_CHECK:
	{
		HWND checkWnd = GetDlgItem(hwnd, IDC_CHECK);
		int checkState = SendMessage(checkWnd, BM_GETCHECK, 0, 0);
		if (checkState == BST_CHECKED)
			autoApply = TRUE;
		else if (checkState == BST_UNCHECKED)
			autoApply = FALSE;
	}
	break;
	case IDC_RESCAN:
	{
		clearList(hwnd);
		std::thread bgThread(fillFileList, hwnd); //fillFileList(hwnd);
		bgThread.detach();
	}
	break;
	}
	return 0;
}

static BOOL list_OnNotify(HWND hwnd, int wParam, NMHDR* lParam)
{
	if (lParam->code == LVN_ITEMACTIVATE)
	{
#if (_WIN32_IE >= 0x0400)
		LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;

		std::lock_guard<std::mutex> guard(fileListMutex);
		SendMessage(hwndWinampParent, WM_WA_IPC, (WPARAM)(fileList[lpnmia->iItem]), IPC_SETSKINW);
#else
		MessageBox(0, L"This should not happen: _WIN32_IE < 0x0400", L"", MB_OK);
#endif
	}

	return FALSE;
}

static void view_OnTimer(HWND hwnd, UINT id)
{
	if (id == RESCAN_TIMER_ID)
	{
		const wchar_t* newFile = fillFileList(hwnd);
		if (newFile != NULL && autoApply)
		{
			SendMessage(hwndWinampParent, WM_WA_IPC, (WPARAM)newFile, IPC_SETSKINW);
		}
	}
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
		HANDLE_MSG(hwndDlg, WM_TIMER, view_OnTimer);
		HANDLE_MSG(hwndDlg, WM_COMMAND, view_OnCommand);
		HANDLE_MSG(hwndDlg, WM_SIZE, view_OnSize);
		HANDLE_MSG(hwndDlg, WM_NOTIFY, list_OnNotify);
	case WM_PAINT:
	{
		int tab[] = { IDC_LIST | DCW_SUNKENBORDER };
		ml_draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
	}
	return 0;
	HANDLE_MSG(hwndDlg, WM_DESTROY, view_OnDestroy);

	}
	return FALSE;
}

static void precacheThumbnails(HWND dialogWnd)
{
	// Precache thumbnails
	wchar_t helperCmd[1024];
	wsprintf(helperCmd, L"\"%s\\skinbrowser2_helper.exe\" all \"%s\\..\\Skins\" \"%swmp_skin_thm\"", myDirectory, myDirectory, tempPath);
	runProcessInBackground(helperCmd);

	// Clear and fill up skin list
	clearList(dialogWnd);
	fillFileList(dialogWnd);

	// Set up directory auto-rescan
	SetTimer(dialogWnd, RESCAN_TIMER_ID, 1000, NULL);
}

HWND GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl, wchar_t* skinPath)
{
	// Save HWNDs
	hwndWinampParent = _hwndWinampParent;
	hwndLibraryParent = _hwndLibraryParent;

	// Create dialog
	HWND dialogWnd = CreateDialog(myself, MAKEINTRESOURCE(IDD_VIEW_CUSTOM), hwndParentControl, (DLGPROC)customDialogProc);

	// Set auto-apply checkbox to last known value
	HWND checkWnd = GetDlgItem(dialogWnd, IDC_CHECK);
	if (autoApply)
		SendMessage(checkWnd, BM_SETCHECK, BST_CHECKED, 0);
	else
		SendMessage(checkWnd, BM_SETCHECK, BST_UNCHECKED, 0);

	GetTempPath(MAX_PATH, tempPath);

	std::thread bgThread(precacheThumbnails, dialogWnd);
	bgThread.detach();

	return dialogWnd;
}
