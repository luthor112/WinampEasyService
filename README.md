# WinampEasyService
Making it easier to integrate services with good old WinAmp

Bugs:
* Reference feature is broken
    * WACUP always calls GetFileInfo in in_easyfngetter with file==NULL
    * Feature is currently disabled using #define DISABLE_REFERENCE_FEATURE
* WinAmp crashes in list_OnNotify in ml_easysrv
    * WACUP works
