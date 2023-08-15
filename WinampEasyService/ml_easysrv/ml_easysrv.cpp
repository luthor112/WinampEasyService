#include "easyservice.h"

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

#include <map>

// Uncomment to disable reference handling
//#define DISABLE_REFERENCE_FEATURE

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

/////////////////////
// LOADED SERVICES //
/////////////////////

std::map<UINT_PTR, EasyService*> serviceMap;
std::map<HWND, UINT_PTR> serviceHwndMap;
std::map<UINT_PTR, std::vector<ItemInfo>> serviceListItemMap;

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
int init()
{
    loadServices();

    return 0;
}

// Called by WinAmp when quitting
void quit() {}

// Called by WinAmp to talk to our plugin
INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
    if (message_type == ML_MSG_TREE_ONCREATEVIEW && param1 > 1)
    {
		HWND dialogWnd = CreateDialog(plugin.hDllInstance, MAKEINTRESOURCE(IDD_VIEW_EASYSRV), (HWND)(LONG_PTR)param2, (DLGPROC)viewDialogProc);
		serviceHwndMap[dialogWnd] = param1;
		return (INT_PTR)dialogWnd;
    }

    return 0;
}

// This is an export function called by winamp which returns this plugin info.
// We wrap the code in 'extern "C"' to ensure the export isn't mangled if used in a CPP file.
extern "C" __declspec(dllexport) winampMediaLibraryPlugin * winampGetMediaLibraryPlugin() {
    return &plugin;
}

extern "C" __declspec(dllexport) const wchar_t* GetPluginFileName(const wchar_t* referenceName) {
	int serviceID;
	int serviceIDLen;
	wchar_t onlyRefName[1024];

	swscanf_s(referenceName, L"%d%n", &serviceID, &serviceIDLen);
	wcscpy_s(onlyRefName, 1024, referenceName + serviceIDLen + 1);
	onlyRefName[wcslen(onlyRefName) - 4] = '\0';

	return serviceMap[serviceID]->GetFileName(onlyRefName);
}

//////////////
// SERVICES //
//////////////

void addTreeItem(UINT_PTR parentId, UINT_PTR id, const wchar_t* title, BOOL hasChildren, int imageIndex)
{
	MLTREEITEMW treeItem = {
        sizeof(MLTREEITEMW),
        id,							// id
        parentId,					// parentId, 0 for root
        const_cast<wchar_t*>(title),// title
		wcslen(title),			// titleLen
        hasChildren,				// hasChildren
        imageIndex					// imageIndex
    };
    SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&treeItem, ML_IPC_TREEITEM_ADDW);
}

void loadServices()
{
	int playerType = PLAYERTYPE_WINAMP;
	char* pluginDir = (char*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORY);
	if (strstr(pluginDir, "WACUP"))
		playerType = PLAYERTYPE_WACUP;

	addTreeItem(0, 1, L"Services", TRUE, MLTREEIMAGE_BRANCH);
	UINT_PTR index = 2;

    // walk srv_*.dll
	wchar_t searchCriteria[1024];
	wsprintf(searchCriteria, L"%S\\srv_*.dll", pluginDir);

	WIN32_FIND_DATA FindFileData;
	HANDLE searchHandle = FindFirstFile(searchCriteria, &FindFileData);
	if (searchHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t absoluteName[1024];
			wsprintf(absoluteName, L"%S\\%s", pluginDir, FindFileData.cFileName);
			serviceMap[index] = new DLLService(absoluteName, playerType);
			addTreeItem(1, index, serviceMap[index]->GetNodeName(), FALSE, MLTREEIMAGE_BRANCH);
			index++;
		} while (FindNextFile(searchHandle, &FindFileData));
		FindClose(searchHandle);
	}

    // walk esrv_*.exe
	wsprintf(searchCriteria, L"%S\\esrv_*.exe", pluginDir);
	searchHandle = FindFirstFile(searchCriteria, &FindFileData);
	if (searchHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t absoluteName[1024];
			wsprintf(absoluteName, L"\"%S\\%s\"", pluginDir, FindFileData.cFileName);
			serviceMap[index] = new EXEService(absoluteName, playerType);
			addTreeItem(1, index, serviceMap[index]->GetNodeName(), FALSE, MLTREEIMAGE_BRANCH);
			index++;
		} while (FindNextFile(searchHandle, &FindFileData));
		FindClose(searchHandle);
	}

    // walk msrv_*.dll
	wsprintf(searchCriteria, L"%S\\msrv_*.dll", pluginDir);
	searchHandle = FindFirstFile(searchCriteria, &FindFileData);
	if (searchHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t absoluteName[1024];
			wsprintf(absoluteName, L"\"%S\\isrv_managed.exe\" \"%S\\%s\"", pluginDir, pluginDir, FindFileData.cFileName);
			serviceMap[index] = new EXEService(absoluteName, playerType);
			addTreeItem(1, index, serviceMap[index]->GetNodeName(), FALSE, MLTREEIMAGE_BRANCH);
			index++;
		} while (FindNextFile(searchHandle, &FindFileData));
		FindClose(searchHandle);
	}
}

//////////////
// LISTVIEW //
//////////////

typedef int (*HookDialogFunc)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HookDialogFunc ml_hook_dialog_msg = 0;

typedef void (*DrawFunc)(HWND hwndDlg, int* tab, int tabsize);
static DrawFunc ml_draw = 0;

static ChildWndResizeItem srvwnd_rlist[] = {
	{IDC_LIST,0x0011},
	{IDC_INVOKE,0x0101},
};

void addLineToList(HWND hwnd, int index, const wchar_t* author, const wchar_t* title, const wchar_t* information)
{
	HWND hwndList = GetDlgItem(hwnd, IDC_LIST);

	LVITEM lvi = { 0, };
	lvi.mask = LVIF_TEXT;
	lvi.iItem = index;
	lvi.pszText = (LPTSTR)author;
	lvi.cchTextMax = lstrlenW(author);
	SendMessage(hwndList, LVM_INSERTITEMW, 0, (LPARAM)&lvi);

	lvi.iSubItem = 1;
	lvi.pszText = (LPTSTR)title;
	lvi.cchTextMax = lstrlenW(title);
	SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvi);

	lvi.iSubItem = 2;
	lvi.pszText = (LPTSTR)information;
	lvi.cchTextMax = lstrlenW(information);
	SendMessageW(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvi);
}

static BOOL view_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	/* gen_ml has some helper functions to deal with skinned dialogs,
	   we're going to grab their function pointers.
		 for definition of magic numbers, see gen_ml/ml.h	 */
	if (!ml_hook_dialog_msg)
	{
		/* skinning helper functions */
		ml_hook_dialog_msg = (HookDialogFunc)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)2, ML_IPC_SKIN_WADLG_GETFUNC);
		ml_draw = (DrawFunc)SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)3, ML_IPC_SKIN_WADLG_GETFUNC);
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

	// TODO: Implement resizing
	//ml_childresize_init(hwnd, srvwnd_rlist, sizeof(srvwnd_rlist) / sizeof(srvwnd_rlist[0]));

	return FALSE;
}

static BOOL view_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	// TODO: Implement resizing
	/*if (state != SIZE_MINIMIZED)
		ml_childresize_resize(hwnd, srvwnd_rlist, sizeof(srvwnd_rlist) / sizeof(srvwnd_rlist[0]));*/
	return FALSE;
}

static BOOL view_OnDestroy(HWND hwnd)
{
	return FALSE;
}

static BOOL view_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) {
	case IDC_INVOKE:
	{
		std::vector<ItemInfo> itemsToAdd = serviceMap[serviceHwndMap[hwnd]]->InvokeService();
		serviceListItemMap[serviceHwndMap[hwnd]] = itemsToAdd;

		HWND hwndList = GetDlgItem(hwnd, IDC_LIST);
		ListView_DeleteAllItems(hwndList);
		int index = 0;
		
		for (ItemInfo info : itemsToAdd)
		{
			addLineToList(hwnd, index, info.author, info.title, info.info);
			index++;
		}
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
		
		wchar_t playlistTitle[1024];
		wsprintf(playlistTitle, L"%s - %s", serviceListItemMap[serviceHwndMap[hwnd]][lpnmia->iItem].author, serviceListItemMap[serviceHwndMap[hwnd]][lpnmia->iItem].title);

		wchar_t playlistFN[1024];
#ifndef DISABLE_REFERENCE_FEATURE
		if (!wcsncmp(serviceListItemMap[serviceHwndMap[hwnd]][lpnmia->iItem].filename, L"ref_", 4)) {
			wsprintf(playlistFN, L"%d_%s.ref", serviceHwndMap[hwnd], serviceListItemMap[serviceHwndMap[hwnd]][lpnmia->iItem].filename);
		} else {
#endif // !DISABLE_REFERENCE_FEATURE
			wcscpy_s(playlistFN, 1024, serviceListItemMap[serviceHwndMap[hwnd]][lpnmia->iItem].filename);
#ifndef DISABLE_REFERENCE_FEATURE
		}
#endif // !DISABLE_REFERENCE_FEATURE

		enqueueFileWithMetaStructW newFile = {
			playlistFN,
			playlistTitle,
			-1
		};

		if (lpnmia->uKeyFlags == LVKF_ALT)
		{
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&newFile, IPC_ENQUEUEFILEW);
		} else {
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE);
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&newFile, IPC_ENQUEUEFILEW);
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 1, IPC_SETPLAYLISTPOS);
			SendMessage(plugin.hwndWinampParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2, 0), 0);
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
