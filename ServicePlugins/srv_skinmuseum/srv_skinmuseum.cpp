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
#include <string>
#include <fstream>

#include <thread>
#include <mutex>

#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Shlwapi.lib")

GetOptionFunc GetOption;
SetOptionFunc SetOption;
const wchar_t* myDirectory;
UINT_PTR myServiceID;

HINSTANCE myself = NULL;
HWND hwndWinampParent = NULL;
HWND hwndLibraryParent = NULL;

bool keepAll = FALSE;
int currentPage = 1;
int currentSkin = -1;
wchar_t tempPath[MAX_PATH];
int pageLength = 50;
int pageSkip = 5;

struct SkinInfo
{
	const wchar_t* md5;
	const wchar_t* filename;
};

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
	NodeDescriptor desc = { L"Skins", L"Winamp Skin Museum", NULL, CAP_CUSTOMDIALOG };
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

std::vector<SkinInfo> fileList;
HIMAGELIST previewImageList;
std::mutex fileListMutex;

void clearList(HWND hwnd)
{
	fileList.clear();
	currentSkin = -1;

	HWND hwndList = GetDlgItem(hwnd, IDC_LIST);
	ListView_DeleteAllItems(hwndList);

	ImageList_Destroy(previewImageList);
	previewImageList = ImageList_Create(275, 348, ILC_COLOR24, pageLength, 0);
	ListView_SetImageList(hwndList, previewImageList, LVSIL_NORMAL);
	ListView_SetImageList(hwndList, previewImageList, LVSIL_SMALL);
}

void addItemToList(HWND hwnd, SkinInfo skinInfo)
{
	wchar_t thumbFile[1024];
	wsprintf(thumbFile, L"%swmp_museum_cache\\%s.bmp", tempPath, skinInfo.filename);

	HBITMAP hImage = (HBITMAP)LoadImage(NULL, thumbFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	ImageList_Add(previewImageList, hImage, NULL);
	DeleteObject(hImage);

	HWND hwndList = GetDlgItem(hwnd, IDC_LIST);

	LVITEM lvi = { 0, };
	lvi.mask = LVIF_IMAGE | LVIF_TEXT;
	lvi.iItem = fileList.size() - 1;
	lvi.pszText = (LPTSTR)(skinInfo.filename);
	lvi.cchTextMax = lstrlenW(skinInfo.filename);
	lvi.iImage = fileList.size() - 1;
	SendMessage(hwndList, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
}

void setToPage(HWND hwnd, int page)
{
	std::lock_guard<std::mutex> guard(fileListMutex);

	currentPage = page;
	clearList(hwnd);

	wchar_t pageText[128];
	wsprintf(pageText, L"Page %d", currentPage);

	HWND pageNumWnd = GetDlgItem(hwnd, IDC_PAGENUM);
	SetWindowText(pageNumWnd, pageText);
	RedrawWindow(pageNumWnd, NULL, NULL, RDW_INVALIDATE);


	wchar_t cacheDirName[1024];
	wsprintf(cacheDirName, L"%swmp_museum_cache", tempPath);

	wchar_t helperCmd[1024];
	wsprintf(helperCmd, L"\"%s\\skinmuseum_helper.exe\" page %d %d \"%s\"", myDirectory, currentPage, pageLength, cacheDirName);
	runProcessInBackground(helperCmd);

	wchar_t cacheFileName[1024];
	wsprintf(cacheFileName, L"%s\\page.txt", cacheDirName);
	std::wifstream pageStream(cacheFileName);
	std::wstring line;
	while (std::getline(pageStream, line))
	{
		if (line.empty())
			break;

		const wchar_t* md5 = _wcsdup(line.substr(0, 32).c_str());
		const wchar_t* filename = _wcsdup(line.substr(33).c_str());
		SkinInfo si = { md5, filename };
		fileList.push_back(si);

		addItemToList(hwnd, si);
	}
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
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_KEEPALL);
	MLSkinWindow(hwndLibraryParent, &sw);

	/* skin buttons */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_KEEP);
	MLSkinWindow(hwndLibraryParent, &sw);
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_PREV);
	MLSkinWindow(hwndLibraryParent, &sw);
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_NEXT);
	MLSkinWindow(hwndLibraryParent, &sw);

	/* skin text */
	sw.skinType = SKINNEDWND_TYPE_STATIC;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_PAGENUM);
	MLSkinWindow(hwndLibraryParent, &sw);

	return FALSE;
}

static BOOL view_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state != SIZE_MINIMIZED)
	{
		HWND listWnd = GetDlgItem(hwnd, IDC_LIST);
		HWND keepButtonWnd = GetDlgItem(hwnd, IDC_KEEP);
		HWND keepCheckWnd = GetDlgItem(hwnd, IDC_KEEPALL);
		HWND prevSkipWnd = GetDlgItem(hwnd, IDC_PREVSKIP);
		HWND prevButtonWnd = GetDlgItem(hwnd, IDC_PREV);
		HWND pageNumWnd = GetDlgItem(hwnd, IDC_PAGENUM);
		HWND nextButtonWnd = GetDlgItem(hwnd, IDC_NEXT);
		HWND nextSkipnWnd = GetDlgItem(hwnd, IDC_NEXTSKIP);

		MoveWindow(listWnd, 0, 0, cx, cy - 56, TRUE);
		MoveWindow(keepButtonWnd, 1, cy - 53, 200, 25, TRUE);
		MoveWindow(keepCheckWnd, 209, cy - 48, 150, 15, TRUE);
		MoveWindow(prevSkipWnd, 1, cy - 25, 10, 25, TRUE);
		MoveWindow(prevButtonWnd, 11, cy - 25, 107, 25, TRUE);
		MoveWindow(pageNumWnd, 126, cy - 20, 64, 15, TRUE);
		MoveWindow(nextButtonWnd, 188, cy - 25, 107, 25, TRUE);
		MoveWindow(nextSkipnWnd, 295, cy - 25, 10, 25, TRUE);

		RedrawWindow(pageNumWnd, NULL, NULL, RDW_INVALIDATE);
	}

	return FALSE;
}

static BOOL view_OnDestroy(HWND hwnd)
{
	return FALSE;
}

static BOOL view_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) {
	case IDC_KEEPALL:
	{
		HWND checkWnd = GetDlgItem(hwnd, IDC_KEEPALL);
		int checkState = SendMessage(checkWnd, BM_GETCHECK, 0, 0);
		if (checkState == BST_CHECKED)
			keepAll = TRUE;
		else if (checkState == BST_UNCHECKED)
			keepAll = FALSE;
	}
	break;
	case IDC_KEEP:
	{
		if (currentSkin != -1)
		{
			std::lock_guard<std::mutex> guard(fileListMutex);

			wchar_t keptFileName[1024];
			wsprintf(keptFileName, L"%s\\..\\Skins\\%s", myDirectory, fileList[currentSkin].filename);

			if (!PathFileExists(keptFileName))
			{
				wchar_t cacheFileName[1024];
				wsprintf(cacheFileName, L"%swmp_museum_cache\\%s", tempPath, fileList[currentSkin].filename);

				CopyFile(cacheFileName, keptFileName, FALSE);
			}
		}
	}
	break;
	case IDC_PREV:
	{
		if (currentPage > 1)
		{
			std::thread bgThread(setToPage, hwnd, currentPage - 1); //setToPage(hwnd, currentPage - 1);
			bgThread.detach();
		}
	}
	break;
	case IDC_NEXT:
	{
		std::thread bgThread(setToPage, hwnd, currentPage + 1); //setToPage(hwnd, currentPage + 1);
		bgThread.detach();
	}
	break;
	case IDC_PREVSKIP:
	{
		if (currentPage - pageSkip > 0)
		{
			std::thread bgThread(setToPage, hwnd, currentPage - pageSkip); //setToPage(hwnd, currentPage - 1);
			bgThread.detach();
		}
	}
	break;
	case IDC_NEXTSKIP:
	{
		std::thread bgThread(setToPage, hwnd, currentPage + pageSkip); //setToPage(hwnd, currentPage + 1);
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

		currentSkin = lpnmia->iItem;

		fileListMutex.lock();

		wchar_t skinFile[1024];
		wsprintf(skinFile, L"%s\\..\\Skins\\%s", myDirectory, fileList[lpnmia->iItem].filename);

		if (!PathFileExists(skinFile))
		{
			// Download to temp
			wchar_t cacheFileName[1024];
			wsprintf(cacheFileName, L"%swmp_museum_cache\\%s", tempPath, fileList[lpnmia->iItem].filename);

			wchar_t helperCmd[1024];
			wsprintf(helperCmd, L"\"%s\\skinmuseum_helper.exe\" download %s \"%s\"", myDirectory, fileList[lpnmia->iItem].md5, cacheFileName);
			runProcessInBackground(helperCmd);

			// Set from temp
			SendMessage(hwndWinampParent, WM_WA_IPC, (WPARAM)cacheFileName, IPC_SETSKINW);

			fileListMutex.unlock();

			if (keepAll)
				view_OnCommand(NULL, IDC_KEEP, NULL, NULL);
		}
		else
		{
			// Set from Skins
			SendMessage(hwndWinampParent, WM_WA_IPC, (WPARAM)(fileList[lpnmia->iItem].filename), IPC_SETSKINW);

			fileListMutex.unlock();
		}
#else
		MessageBox(0, L"This should not happen: _WIN32_IE < 0x0400", L"", MB_OK);
#endif
	}

	return FALSE;
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

HWND GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl, wchar_t* skinPath)
{
	// Save HWNDs
	hwndWinampParent = _hwndWinampParent;
	hwndLibraryParent = _hwndLibraryParent;

	// Create dialog
	HWND dialogWnd = CreateDialog(myself, MAKEINTRESOURCE(IDD_VIEW_CUSTOM), hwndParentControl, (DLGPROC)customDialogProc);

	// Set Kepp All checkbox to last known value
	HWND checkWnd = GetDlgItem(dialogWnd, IDC_KEEPALL);
	if (keepAll)
		SendMessage(checkWnd, BM_SETCHECK, BST_CHECKED, 0);
	else
		SendMessage(checkWnd, BM_SETCHECK, BST_UNCHECKED, 0);

	GetTempPath(MAX_PATH, tempPath);

	wchar_t pageLengthOpt[MAX_PATH];
	GetOption(myServiceID, L"pagelength", L"50", pageLengthOpt, MAX_PATH);
	pageLength = _wtoi(pageLengthOpt);

	wchar_t pageSkipOpt[MAX_PATH];
	GetOption(myServiceID, L"pageskip", L"5", pageSkipOpt, MAX_PATH);
	pageSkip = _wtoi(pageSkipOpt);

	// Switch to Page 1
	std::thread bgThread(setToPage, dialogWnd, 1); //setToPage(dialogWnd, 1);
	bgThread.detach();

	return dialogWnd;
}
