/// \file CMain.cpp
/// \brief Code for the main class CMain.

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
#include "WindowsHelpers.h"

///////////////////////////////////////////////////////////////////////////////
// Constructors and destructors

#pragma region Constructors and destructors

/// Initialize GDI+, create a bitmap and a graphics object open on that bitmap,
/// create the menus, initialize the check marks on the `L-System` menu,
/// gray out the `Generate` entry in the `File` menu if the initial L-system
/// create the L-system rules for the initial L-system type, generate and
/// draw the initial string.
/// \param hwnd Window handle.

CMain::CMain(const HWND hwnd):
  m_hWnd(hwnd)
{
  m_gdiplusToken = InitGDIPlus(); 
  
  const int w = 3000; //bitmap width
  const int h = 3000; //bitmap height

  m_pBitmap = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);  
  m_pGraphics = new Gdiplus::Graphics(m_pBitmap);
  m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

  SetRules(); //create the first set of rules

  //create and init menus

  CreateMenus(); //create the drop-down menu
  SetLSystemMenuChecks(); //set menu checkmarks
  EnableGenerateMenuEntry(); //enable or disable the generate menu entry 
  
  const UINT bShowRules = m_bShowRules? MF_CHECKED: MF_UNCHECKED;
  CheckMenuItem(m_hViewMenu, IDM_VIEW_RULES, bShowRules);

  if(m_eLineThickness == LineThickness::Thin)
    CheckMenuItem(m_hViewMenu, IDM_VIEW_THINLINES, MF_CHECKED); 
  else if(m_eLineThickness == LineThickness::Thick)
    CheckMenuItem(m_hViewMenu, IDM_VIEW_THICKLINES, MF_CHECKED); 

  //generate and draw the first object
  
  Generate();
  Draw();
} //constructor

/// Delete the graphics object and the bitmap, then shut down GDI+.

CMain::~CMain(){
  delete m_pGraphics;
  delete m_pBitmap;
  Gdiplus::GdiplusShutdown(m_gdiplusToken);
} //destructor

#pragma endregion Constructors and destructors

///////////////////////////////////////////////////////////////////////////////
// Drawing functions

#pragma region Drawing functions

/// Draw the bitmap to the window client area. If the window is smaller than
/// the bitmap, then the top left of the bitmap is used. If the window is
/// larger than the bitmap, then the bitmap is centered in the window. This
/// function is intended to be called in response to a WM_PAINT message.
/// \param hdc Device context for window to be painted.

void CMain::OnPaint(HDC hdc){
  Gdiplus::Graphics graphics(hdc);

  //dirty rectangle width and height
  
  const int nDirtyWidth = m_rectDirty.right  - m_rectDirty.left; 
  const int nDirtyHt    = m_rectDirty.bottom - m_rectDirty.top; 

  //get client rectangle

  RECT rectClient; //for client rectangle
  GetClientRect(m_hWnd, &rectClient); //get client rectangle
  const int nClientWidth = rectClient.right - rectClient.left; //client width
  const int nClientHt = rectClient.bottom - rectClient.top; //client height

  //fill client area with white background

  Gdiplus::SolidBrush brush(Gdiplus::Color::White);
  graphics.FillRectangle(&brush, 0, 0, nClientWidth, nClientHt);

  //draw the dirty rectangle of the bitmap to the center of the client area

  const int x = max(0, (nClientWidth - nDirtyWidth)/2);
  const int y = max(0, (nClientHt - nDirtyHt)/2);

  graphics.DrawImage(m_pBitmap, x, y, m_rectDirty.left, m_rectDirty.top,
    nDirtyWidth, nDirtyHt, Gdiplus::UnitPixel);

 //draw the rules on the screen (note: not the bitmap)

  if(m_bShowRules)DrawRules(graphics); //draw rule text, if required
} //OnPaint

/// Draw the L-system rules text to a GDI+ graphics object.
/// \param graphics Reference to a GDI+ graphics object.

void CMain::DrawRules(Gdiplus::Graphics& graphics){
  Gdiplus::FontFamily ff(L"Consolas");
  Gdiplus::Font font(&ff, 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
  Gdiplus::SolidBrush brush(Gdiplus::Color::DarkCyan);
  Gdiplus::PointF point(10.0f, 10.0f);

  std::wstring temp = m_cLSystem.GetRuleString();
  temp += std::to_wstring(m_cLSystem.GetGenerations()) + L" generations\n";

  graphics.DrawString(temp.c_str(), -1, &font, point, &brush);
} //DrawRules

/// Use turtle graphics to draw the shape corresponding to the generated string
/// to the bitmap. The *dirty rectangle* (the smallest rectangle that contains
/// all of the non-transparent pixels) is calculated and stored also.
/// \param d Turtle graphics descriptor.

void CMain::Draw(const TurtleDesc& d){
  std::stack<StackFrame> stack; //stack frame

  //prepare to draw
  
  m_pGraphics->Clear(Gdiplus::Color::Transparent); //transparent background

  Gdiplus::PointF ptCurPos = d.m_ptStart; //current position
  float angle = 0; //current orientation
  float len = d.m_fLength; //current branch length
    
  Gdiplus::Pen pen(Gdiplus::Color::Black);
  pen.SetWidth(d.m_fPointSize);
  
  //initialize the dirty rectangle to the start pixel

  m_rectDirty.left   = int(std::floor(ptCurPos.X)); 
  m_rectDirty.right  = int(std::ceil (ptCurPos.X)); 
  m_rectDirty.top    = int(std::floor(ptCurPos.Y)); 
  m_rectDirty.bottom = int(std::ceil (ptCurPos.Y)); 

  const std::wstring& s = m_cLSystem.GetString(); //shorthand for generated string

  //loop through characters of the string s, performing turtle graphics

  for(size_t i=0; i<s.size(); i++){ 
    Gdiplus::PointF ptNextPos;

    switch(s[i]){ 
      case 'L':
      case 'R':
      case 'F':
        ptNextPos = ptCurPos - Gdiplus::PointF(-len*sinf(angle), len*cosf(angle));
        m_pGraphics->DrawLine(&pen, ptCurPos, ptNextPos);
        ptCurPos = ptNextPos;

        AddPointToRect(m_rectDirty, ptNextPos);
      break; 

      case '+': angle -= d.m_fAngleDelta; break;
      case '-': angle += d.m_fAngleDelta; break;

      case '[': 
        stack.push(StackFrame(ptCurPos, angle, len)); 
        len *= d.m_fLenMultiplier;
      break;

      case ']': {
        const StackFrame& sf = stack.top();
          
        ptCurPos = sf.m_ptPos;
        angle    = sf.m_fAngle;
        len      = sf.m_fLength;

        stack.pop(); //this must be last, obviously
      } //case
      break;
    } //switch
  } //for

  //make the dirty rectangle slightly larger to include lines on the edge

  const int delta = (int)std::ceil(d.m_fPointSize/2.0f); //amount to add
  m_rectDirty.right  += delta;
  m_rectDirty.bottom += delta;
} //Draw

/// Draw the image from the generated string to the bitmap using turtle graphics.
/// The *dirty rectangle* (the smallest rectangle that contains all of the
/// non-transparent pixels) is calculated and stored also. This version of the
/// function constructs a hard-coded turtle graphics appropriate to the
/// current type.

void CMain::Draw(){
  TurtleDesc d; //turtle graphics descriptor
  const float fDegToRad = (float)M_PI/180.0f; //degrees to radians

  switch(m_eLineThickness){
    case LineThickness::Thin:   d.m_fPointSize = 1.0f; break;
    case LineThickness::Thick:  d.m_fPointSize = 2.0f; break;

    default: d.m_fPointSize = 1.0f; break; 
  } //switch
  
  const float midx = 0.5f*m_pBitmap->GetWidth();
  const float midy = 0.5f*m_pBitmap->GetHeight();
  const float bottomy = m_pBitmap->GetHeight() - 64.0f;
  
  const Gdiplus::PointF ptBottomCenter(midx, bottomy);
  const Gdiplus::PointF ptMidCenter(midx, midy);

  switch(m_nType){ //the angle deltas are cribbed from ABOP
    case IDM_LSYS_PLANT_A:
      d = TurtleDesc(ptBottomCenter, fDegToRad*22.7f, 8.0f);
    break;

    case IDM_LSYS_PLANT_B:   
      d = TurtleDesc(ptBottomCenter, fDegToRad*20.0f, 20.0f);
    break;

    case IDM_LSYS_PLANT_C:
      d = TurtleDesc(ptBottomCenter, fDegToRad*22.5f, 12.0f);
    break;

    case IDM_LSYS_PLANT_D:
      d = TurtleDesc(ptBottomCenter, fDegToRad*20.0f, 5.0f);
    break;

    case IDM_LSYS_PLANT_E: 
      d = TurtleDesc(ptBottomCenter, fDegToRad*25.7f, 5.0f);
    break;

    case IDM_LSYS_PLANT_F:     
      d = TurtleDesc(ptBottomCenter, fDegToRad*22.5f, 16.0f);
    break;

    case IDM_LSYS_BRANCHING:
      d = TurtleDesc(ptBottomCenter, 0.37f, 8.0f);
    break;

    case IDM_LSYS_HEXGOSPER:
      d = TurtleDesc(ptMidCenter, float(M_PI/3), 12.0f);
    break;
  } //switch
  
  // line thickness

  switch(m_eLineThickness){
    case LineThickness::Thin:   d.m_fPointSize = 1.0f; break;
    case LineThickness::Thick:  d.m_fPointSize = 2.0f; break;

    default: d.m_fPointSize = 1.0f; break; 
  } //switch
  
  Draw(d);
  InvalidateRect(m_hWnd, nullptr, TRUE);
} //Draw

#pragma endregion Drawing functions

///////////////////////////////////////////////////////////////////////////////
// Menu functions

#pragma region Menu functions

/// Add menus to the menu bar and store the menu handles, which will
/// be needed later to set checkmarks and such.

void CMain::CreateMenus(){
  HMENU hMenubar = CreateMenu(); //handle to the menu bar

  //set the FILE menu

  m_hFileMenu = CreateMenu();
  AppendMenuW(m_hFileMenu, MF_STRING, IDM_FILE_GENERATE, L"&Generate");
  AppendMenuW(m_hFileMenu, MF_STRING, IDM_FILE_SAVE, L"&Save...");
  AppendMenuW(m_hFileMenu, MF_STRING, IDM_FILE_QUIT, L"&Quit");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)m_hFileMenu, L"&File");

  //set the LSYS menu
  
  m_hLSMenu = CreateMenu();
  
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_A,
    L"&Plant-like (Fig. 1.24a)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_B,
    L"&Plant-like (Fig. 1.24b)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_C,
    L"&Plant-like (Fig. 1.24c)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_D,
    L"&Plant-like (Fig. 1.24d)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_E,
    L"&Plant-like (Fig. 1.24e)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_F,
    L"&Plant-like (Fig. 1.24f)");
  
  AppendMenuW(m_hLSMenu, MF_SEPARATOR, 0, nullptr);

  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_BRANCHING,
    L"&Stochastic branching (Fig. 1.27)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_HEXGOSPER, 
    L"&Hexagonal Gosper curve (Fig. 1.11a)");
  
  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)m_hLSMenu, L"&L-System");

  //set the VIEW menu
  
  m_hViewMenu = CreateMenu();

  AppendMenuW(m_hViewMenu, MF_STRING, IDM_VIEW_THINLINES,   L"&Thin lines");
  AppendMenuW(m_hViewMenu, MF_STRING, IDM_VIEW_THICKLINES,  L"T&hick lines");
  
  AppendMenuW(m_hViewMenu, MF_SEPARATOR, 0, nullptr);

  AppendMenuW(m_hViewMenu, MF_STRING, IDM_VIEW_RULES, L"&Show rules");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)m_hViewMenu, L"&View");

  //set the menu bar

  SetMenu(m_hWnd, hMenubar);
} //CreateMenus

/// Set the `L-System` menu checkmarks. Uncheck them all, then check the one
/// corresponding to the current L-system type. This assumes the correct
/// ordering of defines in WindowsHelpers.h (see the comments there).

void CMain::SetLSystemMenuChecks(){
  for(UINT i=IDM_LSYS_BRANCHING; i<=IDM_LSYS_HEXGOSPER; i++)
    CheckMenuItem(m_hLSMenu, i, MF_UNCHECKED);

  CheckMenuItem(m_hLSMenu, m_nType, MF_CHECKED);
} //SetLSystemMenuChecks

/// Enable the `Generate` item in the `File` menu if stochastic, otherwise
/// gray it out so it can't be used.

void CMain::EnableGenerateMenuEntry(){
  if(m_nType == IDM_LSYS_BRANCHING) //the only stochastic type
    EnableMenuItem(m_hFileMenu, 0, MF_ENABLED | MF_BYPOSITION);
  else EnableMenuItem(m_hFileMenu, 0, MF_GRAYED | MF_BYPOSITION);
} //EnableGenerateMenuEntry

#pragma endregion Menu functions

///////////////////////////////////////////////////////////////////////////////
// Settings functions (functions that change CMain's state)

#pragma region Settings functions

/// Set rules for the current L-system type. The rules are hard-coded from ABOP
/// using a long switch statement.

void CMain::SetRules(){
  m_cLSystem.Clear();

  switch(m_nType){
    case IDM_LSYS_BRANCHING:
      m_cLSystem.SetRoot(L"F");
      m_cLSystem.AddRule(LProduction(L'F', L"F[+F]F[-F]F", 0.33f));
      m_cLSystem.AddRule(LProduction(L'F', L"F[+F]F", 0.33f));
      m_cLSystem.AddRule(LProduction(L'F', L"F[-F]F", 0.34f));
      break;

    case IDM_LSYS_PLANT_A:
      m_cLSystem.SetRoot(L"F");
      m_cLSystem.AddRule(LProduction(L'F', L"F[+F]F[-F]F"));
      break;

    case IDM_LSYS_PLANT_B:
      m_cLSystem.SetRoot(L"F");
      m_cLSystem.AddRule(LProduction(L'F', L"F[+F]F[-F][F]"));
      break;

    case IDM_LSYS_PLANT_C:
      m_cLSystem.SetRoot(L"F");
      m_cLSystem.AddRule(LProduction(L'F', L"FF-[-F+F+F]+[+F-F-F]"));
      break;

    case IDM_LSYS_PLANT_D:
      m_cLSystem.SetRoot(L"X");
      m_cLSystem.AddRule(LProduction(L'X', L"F[+X]F[-X]+X"));
      m_cLSystem.AddRule(LProduction(L'F', L"FF"));
      break;

    case IDM_LSYS_PLANT_E:
      m_cLSystem.SetRoot(L"X");
      m_cLSystem.AddRule(LProduction(L'X', L"F[+X][-X]FX"));
      m_cLSystem.AddRule(LProduction(L'F', L"FF"));
      break;

    case IDM_LSYS_PLANT_F:
      m_cLSystem.SetRoot(L"X");  
      m_cLSystem.AddRule(LProduction(L'X', L"F-[ [X]+X]+F[+FX]-X"));
      m_cLSystem.AddRule(LProduction(L'F', L"FF"));
      break;

    case IDM_LSYS_HEXGOSPER:
      m_cLSystem.SetRoot(L"L");
      m_cLSystem.AddRule(LProduction(L'L', L"L+R++R-L--LL-R+"));
      m_cLSystem.AddRule(LProduction(L'R', L"-L+RR++R+L--L-R"));
      break;
  } //switch
} //SetRules

/// Set the L-system type, set the checkmarks on the `L-System` menu to indicate
/// the new type, enable the `Generate` entry in the `File` menu if the new
/// type is stochastic, create the rules for the new type, generate a string
/// and draw the image from that string. Does nothing if the new type is the
/// same as the previous one.
/// \param t New L-system type.

void CMain::SetType(UINT t){
  if(m_nType != t){ //if it's a change of state
    m_nType = t;

    EnableGenerateMenuEntry();
    SetLSystemMenuChecks();
    SetRules();
    Generate();
    Draw();
  } //if
} //SetType

/// Set the line thickness for turtle graphics and redraw.
/// \param width New line thickness.

void CMain::SetLineThickness(LineThickness width){
  if(m_eLineThickness != width){ //if change in width
    m_eLineThickness = width; //set new line width

    if(m_eLineThickness == LineThickness::Thin){  
      CheckMenuItem(m_hViewMenu, IDM_VIEW_THINLINES, MF_CHECKED);
      CheckMenuItem(m_hViewMenu, IDM_VIEW_THICKLINES, MF_UNCHECKED);
    } //if

    else if(m_eLineThickness == LineThickness::Thick){  
      CheckMenuItem(m_hViewMenu, IDM_VIEW_THINLINES, MF_UNCHECKED);
      CheckMenuItem(m_hViewMenu, IDM_VIEW_THICKLINES, MF_CHECKED);
    } //else if

    Draw(); //redraw with new line thickness
  } //if
} //SetLineThickness

/// Toggle the "show rules" flag. Set the checkmark on the menu entry
/// and ask for a redraw of the window.

void CMain::ToggleShowRules(){
  m_bShowRules = !m_bShowRules;
  const UINT status = m_bShowRules? MF_CHECKED: MF_UNCHECKED;
  CheckMenuItem(m_hViewMenu, IDM_VIEW_RULES, status);
  InvalidateRect(m_hWnd, nullptr, TRUE);
} //ToggleShowRules

#pragma endregion Settings functions

///////////////////////////////////////////////////////////////////////////////
// Other functions

/// Generate an L-system string for a hard-coded number of generations.

void CMain::Generate(){
  int nNumGenerations = 0; //number of generations

  switch(m_nType){
    case IDM_LSYS_PLANT_A:    nNumGenerations = 5; break;
    case IDM_LSYS_PLANT_B:    nNumGenerations = 5; break;
    case IDM_LSYS_PLANT_C:    nNumGenerations = 5; break;
    case IDM_LSYS_PLANT_D:    nNumGenerations = 7; break;
    case IDM_LSYS_PLANT_E:    nNumGenerations = 7; break;
    case IDM_LSYS_PLANT_F:    nNumGenerations = 5; break;
    case IDM_LSYS_BRANCHING:  nNumGenerations = 6; break;
    case IDM_LSYS_HEXGOSPER:  nNumGenerations = 5; break;  
  } //switch
  
  m_cLSystem.Generate(nNumGenerations);
} //Generate

/// Save bitmap as a PNG image file. A temporary bitmap the size of the dirty
/// rectangle is used to ensure that the saved image is cropped to exactly the
/// size needed.
/// \return true if the save succeeded.

bool CMain::SaveImage(){
  bool succeeded = false; //save succeeded

  std::wstring wstrFileName = GetSaveFileName(m_hWnd); //do the dialog box dance
  
  if(wstrFileName != L""){
    CLSID pngClsid; //class id
    GetEncoderClsid((WCHAR*)L"image/png", &pngClsid); //class id for PNG

    //copy from the dirty rectangle to a new bitmap of exactly the right size
    
    const int w = m_rectDirty.right  - m_rectDirty.left; //dirty rectangle width
    const int h = m_rectDirty.bottom - m_rectDirty.top;  //dirty rectangle height

    //draw the dirty rectangle into a bitmap of exactly the right size

    Gdiplus::Bitmap bitmap(w, h, PixelFormat32bppARGB); //bitmap for saved image
    Gdiplus::Graphics graphics(&bitmap); //for drawing on the new bitmap
    graphics.DrawImage(m_pBitmap, 0, 0, m_rectDirty.left, m_rectDirty.top,
      w, h, Gdiplus::UnitPixel); //draw the dirty rectangle
    
    //now for the actual save, at long last

    succeeded = SUCCEEDED(bitmap.Save(wstrFileName.c_str(), &pngClsid, nullptr));
  } //if

  return succeeded;
} //SaveImage

/// Stochasticity test. 
/// \return true if the L-system is stochastic.

const bool CMain::IsStochastic() const{
  return m_cLSystem.IsStochastic();
} //IsStochastic