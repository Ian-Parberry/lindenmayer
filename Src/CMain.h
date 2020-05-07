/// \file CMain.h
/// \brief Interface for the main class CMain.

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
#include "Types.h"

#include "WindowsHelpers.h"
#include "Lsystem.h"

/// \brief The main class.
///
/// The interface between I/O from Windows (input from the drop-down menus,
/// output to the client area of the window), the L-system string generator,
/// turtle graphics, and the GDI+ graphics interface.

class CMain{
  private:
    HWND m_hWnd = nullptr; ///< Window handle.
    HMENU m_hFileMenu = nullptr; ///< Handle to the `File` menu.
    HMENU m_hLSMenu = nullptr; ///< Handle to the `L-System` menu.
    HMENU m_hViewMenu = nullptr; ///< Handle to the `View` menu.
    
    ULONG_PTR m_gdiplusToken = 0; ///< GDI+ token.

    Gdiplus::Bitmap* m_pBitmap = nullptr; ///< Pointer to a bitmap.
    RECT m_rectDirty = {0, 0, 0, 0}; ///< Dirty rectangle in bitmap.
    Gdiplus::Graphics* m_pGraphics; ///< Pointer to a graphics object.

    LSystem m_cLSystem; ///< The L-system.

    UINT m_nType = IDM_LSYS_PLANT_A; ///< Current L-system type.
    bool m_bThickLines = false; ///< Line thickness flag.
    bool m_bShowRules = true; ///< Whether to show the rules.

    Gdiplus::FontFamily* m_pFontFamily = nullptr; ///< Font family.
    Gdiplus::Font* m_pFont = nullptr; ///< Font.

    void SetRules(); ///< Create L-system rules.
    
    void Draw(const TurtleDesc& d); ///< Draw turtle graphics.
    void DrawRules(Gdiplus::Graphics& graphics, Gdiplus::PointF p); ///< Draw rules.

    void CreateMenus(); ///< Create menus.
    void SetLSystemMenuChecks(); ///< Set L-system menu checkmarks.
    void EnableGenerateMenuEntry(); ///< Enable `Generate` in `File` menu.

    int GetRuleStrWidth(Gdiplus::Graphics& graphics); ///< Get rule string width.

  public:
    CMain(const HWND hwnd); ///< Constructor.
    ~CMain(); ///< Destructor.

    void Draw(); ///< Draw turtle graphics.
    void Generate(); ///< Generate L-system string.

    void OnPaint(); ///< Paint the client area.
    void SetType(UINT t); ///< Set type.
    //void SetLineThickness(LineThickness width); ///< Set line thickness.
    bool SaveImage(); ///< Save image to a file.

    const bool IsStochastic() const; ///< Is a stochastic L-system.
    void ToggleShowRules(); ///< Toggle the show rules flag.
    void ToggleLineThickness(); ///< Toggle the line thickness flag.
}; //CMain
