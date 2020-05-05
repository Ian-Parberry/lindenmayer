/// \file Main.cpp
/// \brief The window procedure WndProc(), and wWinMain().

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

#include "CMain.h"

static CMain* g_pMain = nullptr; ///< Pointer to the main class.

/// \brief Window procedure.
///
/// This is the handler for messages from the operating system. 
/// \param hWnd Window handle.
/// \param message Message code.
/// \param wParam Parameter for message.
/// \param lParam Second parameter for message if needed.
/// \return 0 If message is handled.

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
  switch(message){
    case WM_CREATE: 
      g_pMain = new CMain(hWnd);
      return 0;

    case WM_DESTROY:
      delete g_pMain;
      PostQuitMessage(0);
      return 0;

    case WM_SIZE:    
      InvalidateRect(hWnd, nullptr, TRUE);
      return 0;

    case WM_PAINT: {
      PAINTSTRUCT ps; //paint structure
      HDC hdc = BeginPaint(hWnd, &ps); //device context
      g_pMain->OnPaint(hdc);
      EndPaint(hWnd, &ps);

      return 0;
    } //case
 
    case WM_COMMAND: {
      const UINT idm = LOWORD(wParam);

      if(IDM_LSYS_BRANCHING <= idm && idm <= IDM_LSYS_HEXGOSPER)
        g_pMain->SetType(idm);

      else switch(idm){  
        case IDM_FILE_GENERATE:
          if(g_pMain->IsStochastic()){
            g_pMain->Generate();
            g_pMain->Draw();
          } //if
          break;         

        case IDM_FILE_SAVE:
          g_pMain->SaveImage();
          break;

        case IDM_VIEW_THINLINES:
          g_pMain->SetLineThickness(LineThickness::Thin);
          break;

        case IDM_VIEW_THICKLINES:
          g_pMain->SetLineThickness(LineThickness::Thick);
          break;

        case IDM_VIEW_RULES: 
          g_pMain->ToggleShowRules();
          break;

        case IDM_FILE_QUIT: 
          SendMessage(hWnd, WM_CLOSE, 0, 0);
          break;
      } //switch

      return 0;
    } //case

    default: return DefWindowProc(hWnd, message, wParam, lParam);
  } //switch
} //WndProc

/// \brief The main entry point for this application.  
///
/// The main entry point for this application. 
/// \param hInstance Handle to the current instance of this application.
/// \param hPrevInstance Unused.
/// \param lpCmdLine Unused.
/// \param nCmdShow Nonzero if window is to be shown.
/// \return 0 If this application terminates correctly, otherwise an error code.

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, 
  _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER(nCmdShow);

  InitWindow(hInstance, nCmdShow, WndProc);

  MSG msg; 

  while(GetMessage(&msg, nullptr, 0, 0)){
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  } //while

  return (int)msg.wParam;
} //wWinMain
