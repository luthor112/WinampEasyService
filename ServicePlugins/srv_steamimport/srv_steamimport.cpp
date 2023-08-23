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

#include "resource.h"

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
	return L"Steam Music Importer";
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

	/* skin buttons */
	MLSKINWINDOW sw;
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_RESCANLIB);
	MLSkinWindow(hwndLibraryParent, &sw);
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_RESCANPL);
	MLSkinWindow(hwndLibraryParent, &sw);

	return FALSE;
}

static BOOL view_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state != SIZE_MINIMIZED)
	{
		HWND libWnd = GetDlgItem(hwnd, IDC_RESCANLIB);
		HWND plWnd = GetDlgItem(hwnd, IDC_RESCANPL);

		MoveWindow(libWnd, (cx / 2) - (250 / 2), (cy / 2) - (25 + 2), 250, 25, TRUE);
		MoveWindow(plWnd, (cx / 2) - (250 / 2), (cy / 2) + 2, 250, 25, TRUE);
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
	case IDC_RESCANLIB:
	{
		// TODO
		MessageBox(0, L"TODO", L"", MB_OK);
	}
	break;
	case IDC_RESCANPL:
	{
		// TODO
		MessageBox(0, L"TODO", L"", MB_OK);
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

HWND GetCustomDialog(HWND _hwndWinampParent, HWND _hwndLibraryParent, HWND hwndParentControl)
{
	hwndWinampParent = _hwndWinampParent;
	hwndLibraryParent = _hwndLibraryParent;

	HWND dialogWnd = CreateDialog(myself, MAKEINTRESOURCE(IDD_VIEW_CUSTOM), hwndParentControl, (DLGPROC)customDialogProc);

	return dialogWnd;
}
