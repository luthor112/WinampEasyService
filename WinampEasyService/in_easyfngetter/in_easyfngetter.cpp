#define UNICODE_INPUT_PLUGIN

#include <windows.h>
#include "Winamp/in2.h"
#include <shlwapi.h>

#include "Winamp/wa_ipc.h"

#pragma comment(lib, "Shlwapi.lib")

extern In_Module plugin; // our plugin's definition and data struct

void SetInfoHook(int bitrate, int srate, int stereo, int synched)
{
	plugin.SetInfo(bitrate, srate, stereo, synched);
}


void Config(HWND hwndParent)
{

}

void About(HWND hwndParent)
{

}

void Init()
{

}

void Quit()
{

}

// BUG: file is always NULL
void GetFileInfo(const wchar_t* file, wchar_t* title, int* length_in_ms)
{
	int plPos = SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GETLISTPOS);
	wchar_t* refFileName = (wchar_t*)SendMessage(plugin.hMainWindow, WM_WA_IPC, plPos, IPC_GETPLAYLISTFILEW);
	if (wcsncmp(refFileName + (wcslen(refFileName) - 4), L".ref", 4))
		return;

	auto easysrvdll = GetModuleHandle(L"ml_easysrv.dll");
	auto getPluginFileName = reinterpret_cast<const wchar_t* (*) (const wchar_t*)>(GetProcAddress(easysrvdll, "GetPluginFileName"));
	const wchar_t* realFileName = getPluginFileName(refFileName);
	
	SendMessage(plugin.hMainWindow, WM_WA_IPC, (WPARAM)realFileName, IPC_CHANGECURRENTFILEW);
	SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_UPDTITLE);
}

int InfoBox(const wchar_t* file, HWND hwndParent)
{
	return INFOBOX_UNCHANGED;
}

int IsOurFile(const wchar_t* file)
{
	return 0;
}

int Play(const wchar_t* file)
{
	return 0;
}

void Pause()
{

}

void UnPause()
{

}

int IsPaused()
{
	return 0;
}

void Stop()
{

}

int GetLength()
{
	return -1;
}

int GetOutputTime()
{
	return 0;
}

void SetOutputTime(int time_in_ms)
{

}

void SetVolume(int volume)
{

}

void SetPan(int pan)
{

}

void EQSet(int on, char data[10], int preamp)
{

}

In_Module plugin =
{
	IN_VER,	// defined in IN2.H
	const_cast<char*>("Reference Handler for WinampEasyService"),
	0,	// hMainWindow (filled in by winamp)
	0,  // hDllInstance (filled in by winamp)
	const_cast<char*>("ref\0Reference\0"),	// this is a double-null limited list. "EXT\0Description\0EXT\0Description\0" etc.
	0,	// is_seekable
	0,	// uses output plug-in system
	Config,
	About,
	Init,
	Quit,
	GetFileInfo,
	InfoBox,
	IsOurFile,
	Play,
	Pause,
	UnPause,
	IsPaused,
	Stop,

	GetLength,
	GetOutputTime,
	SetOutputTime,

	SetVolume,
	SetPan,

	0,0,0,0,0,0,0,0,0, // visualization calls filled in by winamp

	0,0, // dsp calls filled in by winamp

	EQSet,

	NULL,		// setinfo call filled in by winamp

	0, // out_mod filled in by winamp
};

extern "C" __declspec(dllexport) In_Module * winampGetInModule2()
{
	return &plugin;
}