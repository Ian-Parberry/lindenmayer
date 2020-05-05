/// \file WindowsHelpers.h
/// \brief Interface for some helpful Windows-specific functions.
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

#pragma once

#include "Includes.h"

///////////////////////////////////////////////////////////////////////////////
// Menu IDs

#pragma region Menu IDs

#define IDM_FILE_GENERATE 1 ///< Menu id for Save.
#define IDM_FILE_SAVE 2 ///< Menu id for Save.
#define IDM_FILE_QUIT 3 ///< Menu id for Quit.

//Defines for the L-system menu (IDM_LSYS_*) must be numbered consecutively.
//IDM_LSYS_BRANCHING must be the smallest, `IDM_LSYS_HEXGOSPER` the largest.

#define IDM_LSYS_BRANCHING  4 ///< Menu id for branching structure.
#define IDM_LSYS_PLANT_A    5 ///< Menu id for Fig. 1.24a.
#define IDM_LSYS_PLANT_B    6 ///< Menu id for Fig. 1.24b.
#define IDM_LSYS_PLANT_C    7 ///< Menu id for Fig. 1.24c.
#define IDM_LSYS_PLANT_D    8 ///< Menu id for Fig. 1.24d.
#define IDM_LSYS_PLANT_E    9 ///< Menu id for Fig. 1.24e.
#define IDM_LSYS_PLANT_F   10 ///< Menu id for Fig. 1.24f.
#define IDM_LSYS_HEXGOSPER 11 ///< Menu id for hexagonal Gosper curve.

#define IDM_VIEW_RULES 12 ///< Menu id for showing current rules.
#define IDM_VIEW_THINLINES 13 ///< Menu id for thin lines.
#define IDM_VIEW_THICKLINES 14 ///< Menu id for thick lines.

#pragma endregion Menu IDs

///////////////////////////////////////////////////////////////////////////////
// Helper functions

#pragma region Helper functions

void InitWindow(HINSTANCE hInst, INT nShow, WNDPROC WndProc); ///< Init window.
ULONG_PTR InitGDIPlus(); ///< Initialize GDI+.

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid); ///< Get encoder CLSID.
std::wstring GetSaveFileName(HWND hwnd); ///< Get saved image file name.
void AddPointToRect(RECT& r, Gdiplus::PointF point); ///< Add point to rectangle.

#pragma endregion Helper functions
