#define EASYSRV_IMPORTING
#include "easysrv.h"

#include <windows.h>
#include "Winamp/wa_ipc.h"
#include "gen_ml/ml.h"

#include <windowsx.h>
#include "shlobj.h"
#include "gen_ml/ml_ipc_0313.h"
#include "gen_ml/childwnd.h"
#include "Winamp/wa_dlg.h"
#include "resource.h"

#include "gen_tray/WINAMPCMD.H"

//////////////////////////
// FORWARD DECLARATIONS //
//////////////////////////

// These are callback functions/events which will be called by Winamp
int  init(void);
void quit(void);
INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

// Services
void loadServices(void);

// ListView
LRESULT CALLBACK viewDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

////////////
// PLUGIN //
////////////

// Plugin version (don't touch this)
#define GPPHDR_VER 0x10

// This structure contains plugin information
winampMediaLibraryPlugin plugin = {
  GPPHDR_VER,
  const_cast<char*>("UI Handler for WinampEasyService"),
  init,
  quit,
  MessageProc,
  0,    // hwndWinampParent, will be filled in
  0,    // hwndLibraryParent, will be filled in
  0     // hDllInstance
};

// Called by WinAmp after loading
int init() {
    loadServices();

    return 0;
}

// Called by WinAmp when quitting
void quit() {}

// Called by WinAmp to talk to our plugin
INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
    // TODO
    if (message_type == ML_MSG_TREE_ONCREATEVIEW && param1 == 3)
    {
        return (INT_PTR)CreateDialog(plugin.hDllInstance, MAKEINTRESOURCE(IDD_VIEW_EASYSRV), (HWND)(LONG_PTR)param2, (DLGPROC)viewDialogProc);
    }

    return 0;
}

// This is an export function called by winamp which returns this plugin info.
// We wrap the code in 'extern "C"' to ensure the export isn't mangled if used in a CPP file.
extern "C" __declspec(dllexport) winampMediaLibraryPlugin * winampGetMediaLibraryPlugin() {
    return &plugin;
}

//////////////
// SERVICES //
//////////////

void addTreeItem(UINT_PTR parentId, UINT_PTR id, char* title, BOOL hasChildren, int imageIndex) {
    MLTREEITEM treeItem = {
        sizeof(MLTREEITEM),
        id,                 // id
        parentId,           // parentId, 0 for root
        title,              // title
        strlen(title),  // titleLen
        hasChildren,        // hasChildren
        imageIndex          // imageIndex
    };
    SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&treeItem, ML_IPC_TREEITEM_ADD);
}

void loadServices() {
    // walk srv_*.dll
    // TODO

    // walk srv_*.exe
    // TODO

    // walk msrv_*.exe
    // TODO

    // Node examples / testing
    addTreeItem(0, 1, const_cast<char*>("Services"), TRUE, MLTREEIMAGE_BRANCH);
    addTreeItem(1, 2, const_cast<char*>("Level 1 Node 1"), FALSE, MLTREEIMAGE_BRANCH);
    addTreeItem(1, 3, const_cast<char*>("Level 1 Node 2"), TRUE, MLTREEIMAGE_BRANCH);
    addTreeItem(1, 4, const_cast<char*>("Level 1 Node 3"), FALSE, MLTREEIMAGE_BRANCH);
}

//////////////
// LISTVIEW //
//////////////

typedef void (*ChildResizeFunc)(HWND, ChildWndResizeItem*, int);
static ChildResizeFunc ml_childresize_init = 0, ml_childresize_resize = 0;

typedef int (*HookDialogFunc)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HookDialogFunc ml_hook_dialog_msg = 0;

typedef void (*DrawFunc)(HWND hwndDlg, int* tab, int tabsize);
static DrawFunc ml_draw = 0;

static ChildWndResizeItem srvwnd_rlist[] = {
	{IDC_LIST,0x0011},
	{IDC_INVOKE,0x0101},
};

void addLineToList(HWND hwnd, int index, const wchar_t* filename, const wchar_t* artist, const wchar_t* title)
{
	HWND hwndList = GetDlgItem(hwnd, IDC_LIST);

	LVITEM lvi = { 0, };
	lvi.mask = LVIF_TEXT;
	lvi.iItem = index;
	lvi.pszText = (LPTSTR)filename;
	lvi.cchTextMax = lstrlenW(filename);
	SendMessage(hwndList, LVM_INSERTITEMW, 0, (LPARAM)&lvi);

	lvi.iSubItem = 1;
	lvi.pszText = (LPTSTR)artist;
	lvi.cchTextMax = lstrlenW(artist);
	SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvi);

	lvi.iSubItem = 2;
	lvi.pszText = (LPTSTR)title;
	lvi.cchTextMax = lstrlenW(title);
	SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvi);
}

static BOOL view_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	/* gen_ml has some helper functions to deal with skinned dialogs,
	   we're going to grab their function pointers.
		 for definition of magic numbers, see gen_ml/ml.h	 */
	if (!ml_childresize_init)
	{
		/* skinning helper functions */
		ml_hook_dialog_msg = (HookDialogFunc)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)2, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_draw = (DrawFunc)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)3, ML_IPC_SKIN_WADLG_GETFUNC);

		/* resizing helper functions */
		ml_childresize_init = (ChildResizeFunc)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)32, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_childresize_resize = (ChildResizeFunc)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)33, ML_IPC_SKIN_WADLG_GETFUNC);
	}

	HWND listWnd = GetDlgItem(hwnd, IDC_LIST);

	/* add listview columns */
	LVCOLUMN lvc = { 0, };
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = (LPTSTR)L"Author";
	lvc.cx = 250;
	SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)0, (LPARAM)&lvc);

	lvc.pszText = (LPTSTR)L"Title";
	lvc.cx = 150;
	SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)1, (LPARAM)&lvc);

	lvc.pszText = (LPTSTR)L"Information";
	lvc.cx = 150;
	SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)2, (LPARAM)&lvc);

	/* skin dialog */
	MLSKINWINDOW sw;
	sw.skinType = SKINNEDWND_TYPE_DIALOG;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = hwnd;
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	/* skin listview */
	sw.hwndToSkin = listWnd;
	sw.skinType = SKINNEDWND_TYPE_LISTVIEW;
	sw.style = SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	/* skin button */
	sw.skinType = SKINNEDWND_TYPE_BUTTON;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = GetDlgItem(hwnd, IDC_INVOKE);
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	ml_childresize_init(hwnd, srvwnd_rlist, sizeof(srvwnd_rlist) / sizeof(srvwnd_rlist[0]));

	return FALSE;
}

static BOOL view_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state != SIZE_MINIMIZED)
		ml_childresize_resize(hwnd, srvwnd_rlist, sizeof(srvwnd_rlist) / sizeof(srvwnd_rlist[0]));
	return FALSE;
}

// TODO
static BOOL view_OnDestroy(HWND hwnd)
{
	//m_hwnd = 0;
	return FALSE;
}

// TODO
static BOOL view_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) {
	case IDC_INVOKE:
	{
		MessageBox(0, L"IDC_INVOKE event triggered", L"", MB_OK);
	}
	break;
	}
	return 0;
}

// TODO
static BOOL list_OnNotify(HWND hwnd, int wParam, NMHDR* lParam)
{
	if (lParam->code == LVN_ITEMACTIVATE)
	{
#if (_WIN32_IE >= 0x0400)
		LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
		wchar_t msgText[1024];
		wsprintf(msgText, L"LVN_ITEMACTIVATE triggered for ID %d", lpnmia->iItem);
		MessageBox(0, msgText, L"", MB_OK);

		if (lpnmia->uKeyFlags == LVKF_ALT)
		{
			MessageBox(0, L"Alt was down", L"", MB_OK);
		}
#else
		MessageBox(0, L"This should not happen...", L"", MB_OK);
#endif
	}

	return FALSE;
}

LRESULT CALLBACK viewDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
