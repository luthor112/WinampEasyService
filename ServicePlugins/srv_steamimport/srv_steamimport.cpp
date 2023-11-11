#include "..\..\WinampEasyService\ml_easysrv\easysrv.h"

#include <Windows.h>
#include "Winamp/wa_ipc.h"
#include "gen_ml/ml.h"

#include <windowsx.h>
#include "shlobj.h"
#include "gen_ml/ml_ipc_0313.h"
#include "gen_ml/childwnd.h"
#include "Winamp/wa_dlg.h"

#include <string.h>
#include <vector>
#include <Shlwapi.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <map>

#include "resource.h"

#pragma comment(lib, "Shlwapi.lib")

GetOptionFunc GetOption;
SetOptionFunc SetOption;
const wchar_t* myDirectory;
UINT_PTR myServiceID;

HINSTANCE myself = NULL;
HWND hwndWinampParent = NULL;
HWND hwndLibraryParent = NULL;

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
	NodeDescriptor desc = { L"Media", L"Steam Music Importer", NULL, CAP_CUSTOMDIALOG };
	return desc;
}

// std::codecvt_utf8 is deprecated in C++17
// https://stackoverflow.com/a/69410299
std::string wide_string_to_string(const std::wstring& wide_string)
{
	if (wide_string.empty())
	{
		return "";
	}

	const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &wide_string.at(0), (int)wide_string.size(), nullptr, 0, nullptr, nullptr);
	if (size_needed <= 0)
	{
		throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
	}

	std::string result(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wide_string.at(0), (int)wide_string.size(), &result.at(0), size_needed, nullptr, nullptr);
	return result;
}

enum class UpdateTarget { LocalLibrary, Playlists };

void scanFiles(UpdateTarget updateTarget)
{
	wchar_t steamDir[MAX_PATH];
	DWORD steamDirLen = MAX_PATH;
	RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Valve\\Steam", L"InstallPath", RRF_RT_REG_SZ, NULL, steamDir, &steamDirLen);

	wchar_t folderListFile[MAX_PATH];
	wsprintf(folderListFile, L"%s\\steamapps\\libraryfolders.vdf", steamDir);

	std::vector<std::wstring> libraryList;

	std::wifstream folderListStream(folderListFile);
	std::wstring line;
	while (std::getline(folderListStream, line))
	{
		size_t keyBeg = line.find(L"\"path\"");
		if (keyBeg != std::wstring::npos)
		{
			size_t libraryBeg = line.find('\"', keyBeg + 6) + 1;
			size_t libraryEnd = line.rfind('\"');

			std::wstring libraryPath;
			for (size_t i = libraryBeg; i < libraryEnd; i++)
				if (!(line[i] == '\\' && line[i - 1] == '\\'))
					libraryPath += line[i];

			libraryList.push_back(libraryPath);
		}
	}

	for (std::wstring libraryPath : libraryList)
	{
		wchar_t musicDir[1024];
		wsprintf(musicDir, L"%s\\steamapps\\music", libraryPath.c_str());

		wchar_t tempPath[MAX_PATH];
		GetTempPath(MAX_PATH, tempPath);

		using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
		if (PathFileExists(musicDir))
		{
			std::map<std::wstring, std::ofstream> plStreams;

			for (const auto& dirEntry : recursive_directory_iterator(musicDir, std::filesystem::directory_options::skip_permission_denied))
			{
				if (dirEntry.is_regular_file())
				{
					if (dirEntry.path().extension() == ".mp3" || dirEntry.path().extension() == ".wav" || dirEntry.path().extension() == ".flac")
					{
						if (updateTarget == UpdateTarget::LocalLibrary)
						{
							LMDB_FILE_ADD_INFOW newFile = {
								_wcsdup(dirEntry.path().native().c_str()),
								-1,		// metadata get mode (0 - don't use metadata, 1 - use metadata; -1 - read from user settings (ini file)
								-1		// metadata guessing mode (0 - smart, 1 - simple; 2 - no, -1 - read from user settings (ini file)
							};
							SendMessage(hwndLibraryParent, WM_ML_IPC, (WPARAM)&newFile, ML_IPC_DB_ADDORUPDATEFILEW);
						}
						else
						{
							std::wstring albumName = dirEntry.path().parent_path().filename().native();
							if (plStreams.count(albumName) == 0)
							{
								wchar_t plFileName[MAX_PATH];
								wsprintf(plFileName, L"%ssteam_pl_%s.m3u8", tempPath, albumName.c_str());
								plStreams[albumName] = std::ofstream(plFileName);
							}

							plStreams[albumName] << wide_string_to_string(dirEntry.path().native()) << "\n";
						}
					}
				}
			}

			for (auto& [key, val] : plStreams)
			{
				val.close();

				if (updateTarget == UpdateTarget::Playlists)
				{
					wchar_t plFileName[MAX_PATH];
					wsprintf(plFileName, L"%ssteam_pl_%s.m3u8", tempPath, key.c_str());

					mlAddPlaylist newPlaylist = {
						sizeof(mlAddPlaylist),
						_wcsdup(key.c_str()),	// playlistName
						_wcsdup(plFileName),		// filename
						PL_FLAGS_IMPORT,				// set to have ml_playlists make a copy (only valid for mlAddPlaylist)
						-1,								// numItems, set to -1 if you don't know
						-1								// length, in seconds, set to -1 if you don't know
					};

					SendMessage(hwndLibraryParent, WM_ML_IPC, (WPARAM)&newPlaylist, ML_IPC_PLAYLIST_ADD);
				}
			}
		}
	}

	SendMessage(hwndLibraryParent, WM_ML_IPC, 0, ML_IPC_DB_SYNCDB);
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
		scanFiles(UpdateTarget::LocalLibrary);
		MessageBox(0, L"Update finished!", L"Steam Music Importer", MB_OK);
	}
	break;
	case IDC_RESCANPL:
	{
		scanFiles(UpdateTarget::Playlists);
		MessageBox(0, L"Update finished!", L"Steam Music Importer", MB_OK);
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
	return dialogWnd;
}
