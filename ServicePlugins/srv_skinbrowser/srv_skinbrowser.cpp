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

#include "resource.h"

#define RESCAN_TIMER_ID 111

HINSTANCE myself = NULL;
HWND hwndWinampParent = NULL;
HWND hwndLibraryParent = NULL;
bool autoApply = FALSE;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	myself = (HINSTANCE)hModule;
	return TRUE;
}

const wchar_t* GetNodeName() {
	return L"Skin Browser";
}

std::vector<const wchar_t*> fileList;

void clearList(HWND hwnd)
{
	fileList.clear();

	HWND hwndList = GetDlgItem(hwnd, IDC_LIST);
	ListView_DeleteAllItems(hwndList);
}

// Returns TRUE if the entry was new and added
bool addLineToList(HWND hwnd, const wchar_t* filename)
{
	for (const wchar_t* currFileName : fileList)
	{
		if (!wcscmp(currFileName, filename))
			return FALSE;
	}

	fileList.push_back(filename);

	HWND hwndList = GetDlgItem(hwnd, IDC_LIST);

	LVITEM lvi = { 0, };
	lvi.mask = LVIF_TEXT;
	lvi.iItem = fileList.size() - 1;
	lvi.pszText = (LPTSTR)filename;
	lvi.cchTextMax = lstrlenW(filename);
	SendMessage(hwndList, LVM_INSERTITEMW, 0, (LPARAM)&lvi);

	return TRUE;
}

// Returns the first entry that's being newly added to the list
const wchar_t* fillFileList(HWND hwnd)
{
	char* pluginDir = (char*)SendMessage(hwndWinampParent, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORY);
	const wchar_t* newFile = NULL;
	
	wchar_t searchCriteria[1024];
	wsprintf(searchCriteria, L"%S\\..\\Skins\\*", pluginDir);

	WIN32_FIND_DATA FindFileData;
	HANDLE searchHandle = FindFirstFile(searchCriteria, &FindFileData);
	if (searchHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindFileData.cFileName[0] != '.')
			{
				const wchar_t* fileNameCopy = _wcsdup(FindFileData.cFileName);
				if (addLineToList(hwnd, fileNameCopy))
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

	/* add listview columns */
	LVCOLUMN lvc = { 0, };
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = (LPTSTR)L"Filename";
	lvc.cx = 250;
	SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)0, (LPARAM)&lvc);

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
		fillFileList(hwnd);
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

		SendMessage(hwndWinampParent, WM_WA_IPC, (WPARAM)(fileList[lpnmia->iItem]), IPC_SETSKINW);
#else
		MessageBox(0, L"This should not happen...", L"", MB_OK);
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

HWND GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl)
{
	hwndWinampParent = _hwndWinampParent;
	hwndLibraryParent = _hwndLibraryParent;

	HWND dialogWnd = CreateDialog(myself, MAKEINTRESOURCE(IDD_VIEW_CUSTOM), hwndParentControl, (DLGPROC)customDialogProc);
	
	clearList(dialogWnd);
	fillFileList(dialogWnd);

	HWND checkWnd = GetDlgItem(dialogWnd, IDC_CHECK);
	if (autoApply)
		SendMessage(checkWnd, BM_SETCHECK, BST_CHECKED, 0);
	else
		SendMessage(checkWnd, BM_SETCHECK, BST_UNCHECKED, 0);

	SetTimer(dialogWnd, RESCAN_TIMER_ID, 1000, NULL);

	return dialogWnd;
}
