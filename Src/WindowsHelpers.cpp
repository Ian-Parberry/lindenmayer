/// \file WindowsHelpers.cpp
/// \brief Code for some helpful Windows-specific functions.
///
/// These platform-dependent functions are hidden away so that the faint-of-heart
/// don't have to see them if they're offended by them. 

// MIT License
//
// Copyright (c) 2020 Ian Parberry
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <shobjidl_core.h>
#include <atlbase.h>

#include "WindowsHelpers.h"
#include "resource.h"

#include "Includes.h"

/// Create and initialize a window.
/// \param hInst Instance handle.
/// \param nShow 1 to show window, 0 to hide.
/// \param WndProc Window procedure.

void InitWindow(HINSTANCE hInst, INT nShow, WNDPROC WndProc){
  const char appname[] = "L-System Image Generator";
   
  WNDCLASSEX wndClass = {0}; //extended window class structure

  wndClass.cbSize         = sizeof(WNDCLASSEX);
  wndClass.style          = CS_HREDRAW | CS_VREDRAW;
  wndClass.lpfnWndProc    = WndProc;
  wndClass.cbClsExtra     = 0;
  wndClass.cbWndExtra     = 0;
  wndClass.hInstance      = hInst;
  wndClass.hIcon          = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
  wndClass.hCursor        = LoadCursor(nullptr, IDC_ARROW);
  wndClass.hbrBackground  = nullptr;
  wndClass.lpszMenuName   = nullptr;
  wndClass.lpszClassName  = appname;
  wndClass.hIconSm        = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
   
  RegisterClassEx(&wndClass);

  const DWORD dwStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME | WS_SYSMENU; 
  const DWORD dwStyleEx = WS_EX_APPWINDOW | WS_EX_DLGMODALFRAME;

  const int w = 800; //window client area width.
  const int h = 800; //window client area height.
    
  RECT r;  
  r.left = 0; r.right = w; 
  r.top = 0; r.bottom = h + GetSystemMetrics(SM_CYMENU);
  AdjustWindowRectEx(&r, dwStyle, FALSE, dwStyleEx); 

  const HWND hwnd = CreateWindowEx(dwStyleEx, appname, appname, dwStyle, 
    CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, 
    nullptr, nullptr, hInst, nullptr);
  
  ShowWindow(hwnd, nShow);
  UpdateWindow(hwnd);
} //InitWindow

/// Display a `Save` dialog box for png files and return the file name that
/// the user selects. If for the usual reasons this fails, then the empty
/// string is returned. Only files with a `.png` extension are allowed. The
/// default file name is "ImageN.png", where N is the number of images saved
/// so far in the current instance of this program. This prevents any collisions
/// with files already saved by this instance. If there is a collision with a
/// file from a previous instance, then the user is prompted to overwrite or
/// rename it in the normal fashion. 
/// \param hwnd Window handle.
/// \return The selected file name (empty for failure).

std::wstring GetSaveFileName(HWND hwnd){
  COMDLG_FILTERSPEC filetypes[] = {
    {L"PNG Files", L"*.png"}
  }; //filetypes

  std::wstring wstrFileName; //result
  CComPtr<IFileSaveDialog> pDlg; //pointer to save dialog box
 
  if(SUCCEEDED(pDlg.CoCreateInstance(__uuidof(FileSaveDialog)))){
    static int count = 0; //number of images saved
    std::wstring wstrName = L"Image" + std::to_wstring(count++); //default file name

    pDlg->SetFileTypes(_countof(filetypes), filetypes);
    pDlg->SetTitle(L"Save Image");
    pDlg->SetFileName(wstrName.c_str());
    pDlg->SetDefaultExtension(L"png");
 
    if(SUCCEEDED(pDlg->Show(hwnd))){
      CComPtr<IShellItem> pItem;

      if(SUCCEEDED(pDlg->GetResult(&pItem))){
        LPOLESTR pwsz = nullptr;

        if(SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz))){
          wstrFileName = std::wstring(pwsz);
          CoTaskMemFree(pwsz);
        } //if
      } //if
    } //if
  } //if

  return wstrFileName;
} //GetSaveFileName

/// Get an encoder clsid for an image file format.
/// \param format File format using wide characters.
/// \param pClsid [OUT] Pointer to clsid.
/// \return Index of codec for image format, -1 for failure.

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid){
  UINT index = -1; //codec index to be returned
  UINT num = 0; //number of image encoders
  UINT size = 0; //size of the image encoder array in bytes

  Gdiplus::ImageCodecInfo* pImageCodecInfo = nullptr;
  Gdiplus::GetImageEncodersSize(&num, &size);

  if(size > 0){
    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));

    if(pImageCodecInfo != nullptr){
      GetImageEncoders(num, size, pImageCodecInfo);

      for(UINT j=0; j<num; j++)
        if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0){
          *pClsid = pImageCodecInfo[j].Clsid;
          index = j; //success
          break;
        } //if

      free(pImageCodecInfo);
    } //if
  } //if

  return index;
} //GetEncoderClsid

/// Initialize GDI+ and get a GDI+ token.
/// \return A GDI+ token.

ULONG_PTR InitGDIPlus(){
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
  return gdiplusToken;
} //InitGDIPlus

/// Add a point to a rectangle, that is, extend the rectangle to enclose the
/// point. Note that the rectangle has integer coefficients whereas the point
/// has floating-point coefficients, and therefore we must apply `floor` and
/// `ceil` judiciously.
/// \param r Reference to a `RECT` structure.
/// \param point A point.

void AddPointToRect(RECT& r, Gdiplus::PointF point){
  r.left   = min(r.left,   int(std::floor(point.X))); 
  r.right  = max(r.right,  int(std::ceil (point.X))); 
  r.top    = min(r.top,    int(std::floor(point.Y))); 
  r.bottom = max(r.bottom, int(std::ceil (point.Y)));
} //AddPointToRect

