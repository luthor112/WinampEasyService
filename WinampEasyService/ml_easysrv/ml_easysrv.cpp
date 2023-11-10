#include "easysrv_internal.h"

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
#include <thread>
#include <mutex>
#include <fstream>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

// Uncomment to disable features
//#define DISABLE_REFERENCE_FEATURE
//#define DISABLE_MAIN_PAGE
//#define DISABLE_SRV_DLL
//#define DISABLE_MSRV_DLL
//#define DISABLE_ESRV_EXE

//////////////////////////
// FORWARD DECLARATIONS //
//////////////////////////

// These are callback functions/events which will be called by Winamp
int  init(void);
void quit(void);
INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

// Service handling
void loadServices(void);

// ListView Dialog
void view_addLineToList(HWND hwnd, int index, const wchar_t* info);
LRESULT CALLBACK viewDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Empty Dialog
LRESULT CALLBACK emptyDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/////////////////////
// LOADED SERVICES //
/////////////////////

std::map<UINT_PTR, EasyService*> serviceMap;
std::map<HWND, UINT_PTR> serviceHwndMap;
std::map<UINT_PTR, std::vector<ItemInfo>> serviceListItemMap;
std::mutex serviceListItemMapMutex;

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

// Global variables
wchar_t configFileName[MAX_PATH];
wchar_t pluginDir[MAX_PATH];
bool tracingEnabled = FALSE;
std::wfstream traceStream;

// Helper function for tracing
void trace(const wchar_t* part1, const wchar_t* part2 = NULL)
{
	if (tracingEnabled)
		traceStream << part1 << (part2 ? part2 : L"") << std::endl;
}

// Called by WinAmp after loading
int init()
{
	char* iniDir = (char*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORY);
	wsprintf(configFileName, L"%S\\easysrv.ini", iniDir);

	wchar_t tracePath[MAX_PATH];
	GetPrivateProfileString(L"easysrv", L"trace", L"", tracePath, MAX_PATH, configFileName);
	if (wcslen(tracePath) > 0)
	{
		tracingEnabled = TRUE;
		traceStream = std::wfstream(tracePath, std::ios::out | std::ios::ate);
		traceStream << L"init()" << std::endl;
	}

	char* pluginDirCS = (char*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORY);
	wsprintf(pluginDir, L"%S", pluginDirCS);

    loadServices();

    return 0;
}

// Called by WinAmp when quitting
void quit()
{
	if (tracingEnabled)
		traceStream << L"quit()" << std::endl;
}

// Called by WinAmp to talk to our plugin
INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
    if (message_type == ML_MSG_TREE_ONCREATEVIEW && serviceMap.count(param1))
    {
		trace(L"Activated node: ", serviceMap[param1]->GetNodeDesc().NodeName);

		bool supportsCustomDialog = serviceMap[param1]->GetNodeDesc().Capabilities & CAP_CUSTOMDIALOG;
		if (supportsCustomDialog)
		{
			HWND customWnd = serviceMap[param1]->GetCustomDialog(plugin.hwndWinampParent, plugin.hwndLibraryParent, (HWND)(LONG_PTR)param2);
			if (customWnd != NULL)
			{
				serviceHwndMap[customWnd] = param1;

				return (INT_PTR)customWnd;
			}
			else
			{
				HWND dialogWnd = CreateDialogParam(plugin.hDllInstance, MAKEINTRESOURCE(IDD_VIEW_EMPTY), (HWND)(LONG_PTR)param2, (DLGPROC)emptyDialogProc, (LPARAM)param1);
				serviceHwndMap[dialogWnd] = param1;

				return (INT_PTR)dialogWnd;
			}
		}
		else
		{
			HWND dialogWnd = CreateDialogParam(plugin.hDllInstance, MAKEINTRESOURCE(IDD_VIEW_EASYSRV), (HWND)(LONG_PTR)param2, (DLGPROC)viewDialogProc, (LPARAM)param1);
			serviceHwndMap[dialogWnd] = param1;

			std::lock_guard<std::mutex> guard(serviceListItemMapMutex);
			std::vector<ItemInfo>& itemsToAdd = serviceListItemMap[serviceHwndMap[dialogWnd]];
			int index = 0;
			for (ItemInfo& info : itemsToAdd)
			{
				view_addLineToList(dialogWnd, index, info.info);
				index++;
			}

			return (INT_PTR)dialogWnd;
		}

		return 1;	// Message is ours, stop processing it
    }

    return 0;		// Message is not ours, pass it along
}

////////////////////////
// EXPORTED FUNCTIONS //
////////////////////////

// This is an export function called by winamp which returns this plugin info.
// We wrap the code in 'extern "C"' to ensure the export isn't mangled if used in a CPP file.
extern "C" __declspec(dllexport) winampMediaLibraryPlugin * winampGetMediaLibraryPlugin() {
    return &plugin;
}

// This is an export function called by in_easyfngetter which returns the real filename.
// We wrap the code in 'extern "C"' to ensure the export isn't mangled if used in a CPP file.
extern "C" __declspec(dllexport) const wchar_t* GetPluginFileName(const wchar_t* referenceName) {
	int serviceID;
	int serviceIDLen;
	wchar_t onlyRefName[1024];

	swscanf_s(referenceName, L"%d%n", &serviceID, &serviceIDLen);
	wcscpy_s(onlyRefName, 1024, referenceName + serviceIDLen + 1);
	onlyRefName[wcslen(onlyRefName) - 4] = '\0';

	return serviceMap[serviceID]->GetFileName(onlyRefName);
}

//////////////////////
// SERVICE HANDLING //
//////////////////////

UINT_PTR addTreeItem(UINT_PTR parentId, const wchar_t* title, BOOL hasChildren, int imageIndex)
{
	MLTREEITEMW treeItem = {
        sizeof(MLTREEITEMW),
        0,							// id, 0 for next available, in which case it will be filled in
        parentId,					// parentId, 0 for root
        const_cast<wchar_t*>(title),// title
		wcslen(title),		// titleLen
        hasChildren,				// hasChildren
        imageIndex					// imageIndex
    };
    SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&treeItem, ML_IPC_TREEITEM_ADDW);
	return treeItem.id;
}

bool containsToken(const wchar_t* tokenSearchList, const wchar_t* searchToken)
{
	wchar_t* tokenList = _wcsdup(tokenSearchList);
	wchar_t* context = NULL;
	wchar_t* token = wcstok_s(tokenList, L";", &context);
	while (token)
	{
		if (!wcscmp(token, searchToken))
		{
			free(tokenList);
			return TRUE;
		}

		token = wcstok_s(NULL, L";", &context);
	}

	free(tokenList);
	return FALSE;
}

UINT_PTR getCategoryNodeID(std::map<const wchar_t*, UINT_PTR>& categoryMap, const wchar_t* category)
{
	for (auto const& x : categoryMap)
	{
		if (!wcscmp(x.first, category))
			return x.second;
	}

	return 0;
}

void loadServices()
{
	UINT_PTR servicesNode = addTreeItem(0, L"Services", TRUE, MLTREEIMAGE_BRANCH);
	trace(L"Services node added");
	
	wchar_t disabledList[4096];
	GetPrivateProfileString(L"easysrv", L"disable", L"", disabledList, 4096, configFileName);
	trace(L"Disabled services: ", disabledList);

	std::map<const wchar_t*, UINT_PTR> catMap;

	wchar_t searchCriteria[1024];
	WIN32_FIND_DATA FindFileData;
	HANDLE searchHandle = INVALID_HANDLE_VALUE;

	wchar_t absoluteName[1022];

#ifndef DISABLE_MAIN_PAGE
	// Main page
	wsprintf(absoluteName, L"%s\\isrv_mainpage.dll", pluginDir);
	if (PathFileExists(absoluteName))
	{
		EasyService* service = new DLLService(absoluteName, L"easysrv");
		if (service->IsValid())
		{
			serviceMap[servicesNode] = service;
			service->InitService(servicesNode);
			trace(L"Loaded service: ", absoluteName);
		}
		else
		{
			trace(L"Invalid service: ", absoluteName);
		}
	}
#endif

#ifndef DISABLE_SRV_DLL
	// walk srv_*.dll
	wsprintf(searchCriteria, L"%s\\srv_*.dll", pluginDir);
	searchHandle = FindFirstFile(searchCriteria, &FindFileData);
	if (searchHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			wsprintf(absoluteName, L"%s\\%s", pluginDir, FindFileData.cFileName);
			if (containsToken(disabledList, absoluteName) || containsToken(disabledList, FindFileData.cFileName))
			{
				trace(L"Skipping service: ", absoluteName);
			}
			else
			{
				EasyService* service = new DLLService(absoluteName, FindFileData.cFileName);
				if (!service->IsValid())
				{
					trace(L"Invalid service: ", absoluteName);
					continue;
				}

				UINT_PTR nodeID;
				const wchar_t* catName = service->GetNodeDesc().Category;
				if (catName == NULL)
				{
					nodeID = addTreeItem(servicesNode, service->GetNodeDesc().NodeName, FALSE, MLTREEIMAGE_BRANCH);
				}
				else
				{
					UINT_PTR catNodeID = getCategoryNodeID(catMap, catName);
					if (catNodeID == 0)
					{
						catNodeID = addTreeItem(servicesNode, catName, TRUE, MLTREEIMAGE_BRANCH);
						catMap[catName] = catNodeID;
					}
					
					nodeID = addTreeItem(catNodeID, service->GetNodeDesc().NodeName, FALSE, MLTREEIMAGE_BRANCH);
				}

				serviceMap[nodeID] = service;
				service->InitService(nodeID);
				trace(L"Loaded service: ", absoluteName);
			}
		} while (FindNextFile(searchHandle, &FindFileData));
		FindClose(searchHandle);
	}
#endif

#ifndef DISABLE_ESRV_EXE
    // walk esrv_*.exe
	wsprintf(searchCriteria, L"%s\\esrv_*.exe", pluginDir);
	searchHandle = FindFirstFile(searchCriteria, &FindFileData);
	if (searchHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			wsprintf(absoluteName, L"\"%s\\%s\"", pluginDir, FindFileData.cFileName);
			if (containsToken(disabledList, absoluteName) || containsToken(disabledList, FindFileData.cFileName))
			{
				trace(L"Skipping service: ", absoluteName);
			}
			else
			{
				EasyService* service = new EXEService(absoluteName, FindFileData.cFileName);
				if (!service->IsValid())
				{
					trace(L"Invalid service: ", absoluteName);
					continue;
				}

				UINT_PTR nodeID;
				const wchar_t* catName = service->GetNodeDesc().Category;
				if (catName == NULL)
				{
					nodeID = addTreeItem(servicesNode, service->GetNodeDesc().NodeName, FALSE, MLTREEIMAGE_BRANCH);
				}
				else
				{
					UINT_PTR catNodeID = getCategoryNodeID(catMap, catName);
					if (catNodeID == 0)
					{
						catNodeID = addTreeItem(servicesNode, catName, TRUE, MLTREEIMAGE_BRANCH);
						catMap[catName] = catNodeID;
					}

					nodeID = addTreeItem(catNodeID, service->GetNodeDesc().NodeName, FALSE, MLTREEIMAGE_BRANCH);
				}

				serviceMap[nodeID] = service;
				service->InitService(nodeID);
				trace(L"Loaded service: ", absoluteName);
			}
		} while (FindNextFile(searchHandle, &FindFileData));
		FindClose(searchHandle);
	}
#endif

#ifndef DISABLE_MSRV_DLL
    // walk msrv_*.dll
	wsprintf(searchCriteria, L"%s\\msrv_*.dll", pluginDir);
	searchHandle = FindFirstFile(searchCriteria, &FindFileData);
	if (searchHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			wsprintf(absoluteName, L"\"%s\\isrv_managed.exe\" \"%s\\%s\"", pluginDir, pluginDir, FindFileData.cFileName);
			if (containsToken(disabledList, absoluteName) || containsToken(disabledList, FindFileData.cFileName))
			{
				trace(L"Skipping service: ", absoluteName);
			}
			else
			{
				EasyService* service = new EXEService(absoluteName, FindFileData.cFileName);
				if (!service->IsValid())
				{
					trace(L"Invalid service: ", absoluteName);
					continue;
				}

				UINT_PTR nodeID;
				const wchar_t* catName = service->GetNodeDesc().Category;
				if (catName == NULL)
				{
					nodeID = addTreeItem(servicesNode, service->GetNodeDesc().NodeName, FALSE, MLTREEIMAGE_BRANCH);
				}
				else
				{
					UINT_PTR catNodeID = getCategoryNodeID(catMap, catName);
					if (catNodeID == 0)
					{
						catNodeID = addTreeItem(servicesNode, catName, TRUE, MLTREEIMAGE_BRANCH);
						catMap[catName] = catNodeID;
					}

					nodeID = addTreeItem(catNodeID, service->GetNodeDesc().NodeName, FALSE, MLTREEIMAGE_BRANCH);
				}

				serviceMap[nodeID] = service;
				service->InitService(nodeID);
				trace(L"Loaded service: ", absoluteName);
			}
		} while (FindNextFile(searchHandle, &FindFileData));
		FindClose(searchHandle);
	}
#endif
}

///////////////////////////
// FUNCTIONS FOR DIALOGS //
///////////////////////////

typedef int (*HookDialogFunc)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HookDialogFunc ml_hook_dialog_msg = 0;

typedef void (*DrawFunc)(HWND hwndDlg, int* tab, int tabsize);
static DrawFunc ml_draw = 0;

/////////////////////
// LISTVIEW DIALOG //
/////////////////////

void view_addLineToList(HWND hwnd, int index, const wchar_t* info)
{
	HWND hwndList = GetDlgItem(hwnd, IDC_LIST);

	LVITEM lvi = {0,};
	lvi.mask = LVIF_TEXT;
	lvi.iItem = index;
	
	int subItemIndex = 0;
	wchar_t* infoList = _wcsdup(info);
	wchar_t* context = NULL;
	wchar_t* token = wcstok_s(infoList, L"\t", &context);
	while (token)
	{
		lvi.iSubItem = subItemIndex;
		lvi.pszText = (LPTSTR)token;
		lvi.cchTextMax = lstrlenW(token);

		if (subItemIndex == 0)
			SendMessage(hwndList, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
		else
			SendMessage(hwndList, LVM_SETITEMW, 0, (LPARAM)&lvi);

		subItemIndex++;
		token = wcstok_s(NULL, L"\t", &context);
	}

	free(infoList);
}

static BOOL view_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	UINT_PTR serviceID = lParam;

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

	int columnIndex = 0;
	wchar_t* columnList = _wcsdup(serviceMap[serviceID]->GetNodeDesc().ColumnNames);
	wchar_t* context = NULL;
	wchar_t* token = wcstok_s(columnList, L"\t", &context);
	while (token)
	{
		lvc.pszText = (LPTSTR)token;
		lvc.cx = 250;
		SendMessageW(listWnd, LVM_INSERTCOLUMNW, (WPARAM)columnIndex, (LPARAM)&lvc);

		columnIndex++;
		token = wcstok_s(NULL, L"\t", &context);
	}
	free(columnList);

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

	return FALSE;
}

static BOOL view_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state != SIZE_MINIMIZED)
	{
		HWND listWnd = GetDlgItem(hwnd, IDC_LIST);
		HWND buttonWnd = GetDlgItem(hwnd, IDC_INVOKE);

		MoveWindow(listWnd, 0, 0, cx, cy - 29, TRUE);
		MoveWindow(buttonWnd, 1, cy - 25, 100, 25, TRUE);
	}

	return FALSE;
}

static BOOL view_OnDestroy(HWND hwnd)
{
	serviceMap[serviceHwndMap[hwnd]]->DestroyingCustomDialog();
	return FALSE;
}

static void view_OnCommand_InvokeService(HWND hwnd)
{
	serviceMap[serviceHwndMap[hwnd]]->InvokeService(plugin.hwndWinampParent, plugin.hwndLibraryParent, hwnd);

	HWND hwndList = GetDlgItem(hwnd, IDC_LIST);
	ListView_DeleteAllItems(hwndList);

	std::lock_guard<std::mutex> guard(serviceListItemMapMutex);
	std::vector<ItemInfo>& itemsToAdd = serviceListItemMap[serviceHwndMap[hwnd]];
	int index = 0;
	for (ItemInfo& info : itemsToAdd)
	{
		view_addLineToList(hwnd, index, info.info);
		index++;
	}
}

static BOOL view_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id) {
	case IDC_INVOKE:
	{
		trace(L"Invoking plugin: ", serviceMap[serviceHwndMap[hwnd]]->GetNodeDesc().NodeName);

		HWND hwndList = GetDlgItem(hwnd, IDC_LIST);
		ListView_DeleteAllItems(hwndList);
		view_addLineToList(hwnd, 0, L"Loading...");

		std::thread bgThread(view_OnCommand_InvokeService, hwnd);
		bgThread.detach();
	}
	break;
	}
	return 0;
}

#define MENUID_PLAY			100
#define MENUID_ENQ			101
#define MENUID_ENQ_DEREF	102
#define MENUID_SAVEAS		103

static void view_handleFile(HWND hwnd, int itemID, int command)
{
	const wchar_t* playlistTitle = serviceListItemMap[serviceHwndMap[hwnd]][itemID].plTitle;

	wchar_t playlistFN[1024];
#ifndef DISABLE_REFERENCE_FEATURE
	if (!wcsncmp(serviceListItemMap[serviceHwndMap[hwnd]][itemID].filename, L"ref_", 4)) {
		if (command == MENUID_ENQ)
		{
			wsprintf(playlistFN, L"%d_%s.ref", serviceHwndMap[hwnd], serviceListItemMap[serviceHwndMap[hwnd]][itemID].filename);
		}
		else
		{
			wchar_t refFN[1024];
			wsprintf(refFN, L"%d_%s.ref", serviceHwndMap[hwnd], serviceListItemMap[serviceHwndMap[hwnd]][itemID].filename);
			wcscpy_s(playlistFN, 1024, GetPluginFileName(refFN));
		}
	}
	else {
#endif // !DISABLE_REFERENCE_FEATURE
		wcscpy_s(playlistFN, 1024, serviceListItemMap[serviceHwndMap[hwnd]][itemID].filename);
#ifndef DISABLE_REFERENCE_FEATURE
	}
#endif // !DISABLE_REFERENCE_FEATURE

	enqueueFileWithMetaStructW newFile = {
		playlistFN,
		playlistTitle,
		-1
	};

	if (command == MENUID_PLAY)
	{
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE);
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&newFile, IPC_ENQUEUEFILEW);
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 1, IPC_SETPLAYLISTPOS);
		SendMessage(plugin.hwndWinampParent, WM_COMMAND, MAKEWPARAM(WINAMP_BUTTON2, 0), 0);
	}
	else if (command == MENUID_ENQ || command == MENUID_ENQ_DEREF)
	{
		SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&newFile, IPC_ENQUEUEFILEW);
	}
	else if (command == MENUID_SAVEAS) {
		wchar_t saveFileName[MAX_PATH];
		OPENFILENAME saveAsOptions;
		ZeroMemory(&saveAsOptions, sizeof(OPENFILENAME));
		saveAsOptions.lStructSize = sizeof(OPENFILENAME);
		saveAsOptions.hwndOwner = hwnd;
		saveAsOptions.lpstrFile = saveFileName;
		saveAsOptions.nMaxFile = MAX_PATH;
		
		if (GetSaveFileName(&saveAsOptions))
			CopyFile(playlistFN, saveFileName, FALSE);
	}
	else
	{
		trace(L"Unknown MENUID!");
	}
}

static BOOL view_OnNotify(HWND hwnd, int wParam, NMHDR* lParam)
{
	if (lParam->code == LVN_ITEMACTIVATE)
	{
#if (_WIN32_IE >= 0x0400)
		LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
		
		std::lock_guard<std::mutex> guard(serviceListItemMapMutex);
		if (lpnmia->uKeyFlags == LVKF_ALT)
			view_handleFile(hwnd, lpnmia->iItem, MENUID_ENQ);
		else
			view_handleFile(hwnd, lpnmia->iItem, MENUID_PLAY);
#else
		trace(L"This should not happen: _WIN32_IE < 0x0400");
#endif
	}

	return FALSE;
}

static void view_OnContext(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
{
	if (hwndContext != GetDlgItem(hwnd, IDC_LIST))
		return;

	HMENU hMenu = CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING, MENUID_PLAY, L"Play");
	AppendMenu(hMenu, MF_STRING, MENUID_ENQ, L"Enqueue");
	AppendMenu(hMenu, MF_STRING, MENUID_ENQ_DEREF, L"Enqueue dereferenced");
	AppendMenu(hMenu, MF_STRING, MENUID_SAVEAS, L"Save As");
	SetMenuDefaultItem(hMenu, MENUID_PLAY, FALSE);

	int menuItem = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, xPos, yPos, hwnd, NULL);
	int itemID = ListView_GetNextItem(hwndContext, -1, LVNI_SELECTED);

	std::lock_guard<std::mutex> guard(serviceListItemMapMutex);
	if (menuItem != 0 && itemID != -1)
		view_handleFile(hwnd, itemID, menuItem);

	DestroyMenu(hMenu);
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
		HANDLE_MSG(hwndDlg, WM_NOTIFY, view_OnNotify);
		HANDLE_MSG(hwndDlg, WM_CONTEXTMENU, view_OnContext);
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

//////////////////
// EMPTY DIALOG //
//////////////////

static BOOL empty_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
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

	/* skin dialog */
	MLSKINWINDOW sw;
	sw.skinType = SKINNEDWND_TYPE_DIALOG;
	sw.style = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = hwnd;
	MLSkinWindow(plugin.hwndLibraryParent, &sw);

	return FALSE;
}

static BOOL empty_OnDestroy(HWND hwnd)
{
	serviceMap[serviceHwndMap[hwnd]]->DestroyingCustomDialog();
	return FALSE;
}

LRESULT CALLBACK emptyDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		HANDLE_MSG(hwndDlg, WM_INITDIALOG, empty_OnInitDialog);
	return 0;
	HANDLE_MSG(hwndDlg, WM_DESTROY, empty_OnDestroy);

	}
	return FALSE;
}
