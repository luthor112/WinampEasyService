#include "img_util.h"

#include <Windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

#pragma comment(lib, "Gdiplus.lib")

// Almost exact copy of https://stackoverflow.com/a/24645420

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }
    free(pImageCodecInfo);
    return -1;  // Failure
}

void WriteBitmap(HBITMAP bitmapData, const wchar_t* fileName)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Bitmap* image = new Bitmap(bitmapData, NULL);

    CLSID myClsId;
    int retVal = GetEncoderClsid(L"image/bmp", &myClsId);

    image->Save(fileName, &myClsId, NULL);
    delete image;

    GdiplusShutdown(gdiplusToken);
}
