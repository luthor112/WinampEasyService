# WinampEasyService
Making it easier to integrate services with good old WinAmp

Goal:
* Make it easier to write plugins and integrate services into WinAmp and WACUP

Using plugins:
* Put the ml_easysrv and in_easyfngetter plugins in the WinAmp or WACUP `Plugins` folder
* Also put isrv_managed in the WinAmp or WACUP `Plugins` folder
* Put any service plugins (`esrv_\*.exe, msrv_\*.dll, srv_\*.dll`) in the same folder
* The services will be accessible in the `Services` tree of the Media Library
* Press the `Invoke plugin` button to interact with the selected service
* Double click an entry in the list to play it
* Alt + double click an entry in the list to enqueue it

Service plugin development:
* Managed (.NET based) DLLs have to be named `msrv_\*.dll` and contain the `msrv.EasyService` class implementing the following:
    * `public string GetNodeName()` should return the name you wish to show in the Media Library
    * `public List<List<string>> InvokeService(int PlayerType)` should return the entries to populate the Media Library ListView with
        * The inner lists should always contain four strings in this order: Artist, Title, Info, Filename
        * The Filename can be a direct filename (e.g. `e:\\example.mp3`) or a reference (e.g. `ref_examplefile`)
        * PlayerType is 0 for WinAmp, 1 for WACUP
    * Optional: `public string GetFileName(string fileID)` should return the direct filename when called with a reference
    * Full example: `WinampEasyService\\msrv_exampledll`
* Unmanaged DLLs have to be named `srv_\*.dll` and implement the functions in `WinampEasyService\\ml_easysrv\\easysrv.h`:
    * `const char\* GetNodeName()` should return the name you wish to show in the Media Library
    * `ItemInfo InvokeService(int PlayerType)` should return the first entry to populate the Media Library ListView with
        * `ItemInfo` contains the following: Artist, Title, Info, Filename
        * The Filename can be a direct filename (e.g. `e:\\example.mp3`) or a reference (e.g. `ref_examplefile`)
        * PlayerTypes are defined in `easysrv.h`
    * `ItemInfo InvokeNext(int PleyerType)` should return the next entry to populate the Media Library ListView with
        * Return and empty `ItemInfo()` to denote the end of the list
    * Optional: `const wchar_t\* GetFileName(const wchar_t\* fileID)` should return the direct filename when called with a reference
    * Full example: `WinampEasyService\\srv_exampledll`
* EXE files (doesn't matter if managed or unmanaged) have to be named `esrv_\*.exe` and respond to the following command line arguments:
    * `GetNodeName`: Print the name you wish to show in the Media Library to STDOUT
    * `InvokeService PlayerType`: Print the entries to populate the Media Library ListView with to STDOUT in the following way:
        * `Artist\\nTitle\\nInfo\\nFilename\\n`
        * Quit when done
        * `PlayerType` can be `PLAYERTYPE_WINAMP` or `PLAYERTYPE_WACUP`
    * Optional: `GetFileName FileID`: Print the direct filename corresponding to the reference `FileID` to STDOUT
    * Full example: `WinampEasyService\\esrv_exampleexe`

Bugs:
* WinAmp crashes in list_OnNotify in ml_easysrv
    * WACUP works correctly
