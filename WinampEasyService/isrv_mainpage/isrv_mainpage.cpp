#include "..\ml_easysrv\easysrv.h"

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

#include "resource.h"

GetOptionFunc GetOption;
SetOptionFunc SetOption;
const wchar_t* myDirectory;
UINT_PTR myServiceID;

HINSTANCE myself = NULL;
HWND hwndWinampParent = NULL;
HWND hwndLibraryParent = NULL;

wchar_t configFileName[MAX_PATH] = L"";

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
	NodeDescriptor desc = { NULL, L"Services", NULL, CAP_CUSTOMDIALOG };
	return desc;
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

	/* skin button */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_CONFIGBUTTON);
	MLSkinWindow(hwndLibraryParent, &sw);

	/* skin text */
	sw.skinType = SKINNEDWND_TYPE_STATIC;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_INFOTEXT);
	MLSkinWindow(hwndLibraryParent, &sw);

	return FALSE;
}

static BOOL view_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state != SIZE_MINIMIZED)
	{
		HWND infoWnd = GetDlgItem(hwnd, IDC_INFOTEXT);
		HWND buttonWnd = GetDlgItem(hwnd, IDC_CONFIGBUTTON);

		MoveWindow(infoWnd, 1, 1, cx, 150, TRUE);
		MoveWindow(buttonWnd, 1, 152, 150, 25, TRUE);

		RedrawWindow(infoWnd, NULL, NULL, RDW_INVALIDATE);
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
	case IDC_CONFIGBUTTON:
	{
		ShellExecute(0, 0, configFileName, 0, 0, SW_SHOW);
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
	hwndWinampParent = _hwndWinampParent;
	hwndLibraryParent = _hwndLibraryParent;

	HWND dialogWnd = CreateDialog(myself, MAKEINTRESOURCE(IDD_VIEW_CUSTOM), hwndParentControl, (DLGPROC)customDialogProc);

	wchar_t tracePath[MAX_PATH];
	GetOption(myServiceID, L"trace", L"", tracePath, MAX_PATH);

	if (wcslen(configFileName) == 0)
	{
		char* iniDir = (char*)SendMessage(hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORY);
		wsprintf(configFileName, L"%S\\easysrv.ini", iniDir);
	}

	wchar_t infoText[1024];
	wsprintf(infoText, L"Hi! Thanks for checking out WinampEasyService!\r\nIf you need help, I usually lurk in the WACUP Discord.\r\n\r\n");
	wsprintf(infoText + wcslen(infoText), L"Current config file: %s\r\nCurrent trace file: %s\r\n\r\n", configFileName, tracePath);
	wsprintf(infoText + wcslen(infoText), L"After editing the config file, restart the application.");
	HWND infoWnd = GetDlgItem(dialogWnd, IDC_INFOTEXT);
	SetWindowText(infoWnd, infoText);
	RedrawWindow(infoWnd, NULL, NULL, RDW_INVALIDATE);

	return dialogWnd;
}
