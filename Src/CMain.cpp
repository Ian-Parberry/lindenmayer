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

/// Initialize GDI+, create a font for drawing text on the window, create a
/// bitmap and a graphics object open on that bitmap, create the menus, 
/// initialize the check marks on the various menu entries, gray out the
/// `Generate` entry in the `File` menu if necessary, create the initial
/// L-system rules, generate the initial string from those rules, then draw
/// the corresponding line drawing to the bitmap.
/// \param hwnd Window handle.

CMain::CMain(const HWND hwnd):
  m_hWnd(hwnd)
{
  m_gdiplusToken = InitGDIPlus(); 

  m_pFontFamily = new Gdiplus::FontFamily(L"Consolas");
  m_pFont = new Gdiplus::Font(m_pFontFamily, 14, Gdiplus::FontStyleRegular,
    Gdiplus::UnitPixel);

  //make the bitmap much larger than it needs to be
  
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

  if(m_bThickLines)
    CheckMenuItem(m_hViewMenu, IDM_VIEW_THICKLINES, MF_CHECKED);

  //generate and draw the first object
  
  Generate();
  Draw();
} //constructor

/// Delete all GDI+ objects, then shut down GDI+.

CMain::~CMain(){
  delete m_pGraphics;
  delete m_pBitmap;
  delete m_pFontFamily;
  delete m_pFont;

  Gdiplus::GdiplusShutdown(m_gdiplusToken);
} //destructor

#pragma endregion Constructors and destructors

///////////////////////////////////////////////////////////////////////////////
// Drawing functions

#pragma region Drawing functions

/// Draw the dirty rectangle from the bitmap to the window client area, scaled
/// down if necessary. This function will only be called in response to a
/// WM_PAINT message.

void CMain::OnPaint(){
  PAINTSTRUCT ps; //paint structure
  HDC hdc = BeginPaint(m_hWnd, &ps); //device context
  Gdiplus::Graphics graphics(hdc); //GDI+ graphics object

  //dirty rectangle width and height
  
  const int nDirtyWidth = m_rectDirty.right  - m_rectDirty.left; 
  const int nDirtyHt    = m_rectDirty.bottom - m_rectDirty.top; 

  //get client rectangle

  RECT rectClient; //for client rectangle
  GetClientRect(m_hWnd, &rectClient); //get client rectangle
  const int nClientWidth = rectClient.right - rectClient.left; //client width
  const int nClientHt = rectClient.bottom - rectClient.top; //client height

  //fill client area with white background

  graphics.Clear(Gdiplus::Color::White); //white background

  //draw the dirty rectangle of the bitmap to the center of the client area

  int textwidth = m_bShowRules? (int)std::ceil(GetRuleStrWidth(graphics)): 0;

  const int margin = 10; //default margin

  const float xscale = float(nClientWidth
    - (textwidth > 0? 3: 2)*margin - textwidth)/nDirtyWidth;
  const float yscale = float(nClientHt - 2*margin)/nDirtyHt;
  const float scale = min(min(xscale, yscale), 1);

  Gdiplus::Rect rectDest;

  rectDest.Width  = (int)std::floor(scale*nDirtyWidth);
  rectDest.Height = (int)std::floor(scale*nDirtyHt);

  rectDest.X = max(2*margin + textwidth, 
    (nClientWidth - rectDest.Width + textwidth)/2);
  rectDest.Y = max(margin, (nClientHt - rectDest.Height)/2);
  
  graphics.DrawImage(m_pBitmap, rectDest, m_rectDirty.left, m_rectDirty.top,
    nDirtyWidth, nDirtyHt, Gdiplus::UnitPixel);

 //draw the rules on the screen (note: NOT on the bitmap)

  if(m_bShowRules)
    DrawRules(graphics, Gdiplus::PointF(margin, margin)); 

  EndPaint(m_hWnd, &ps); //this must be done last
} //OnPaint

/// Draw the L-system rules text to a GDI+ graphics object.
/// \param graphics Reference to a GDI+ graphics object.
/// \param p Point at which to draw the rules (the top left pixel).

void CMain::DrawRules(Gdiplus::Graphics& graphics, Gdiplus::PointF p){
  Gdiplus::SolidBrush brush(Gdiplus::Color::DarkCyan);

  std::wstring temp = m_cLSystem.GetRuleString();
  temp += std::to_wstring(m_cLSystem.GetGenerations()) + L" generations\n";

  graphics.DrawString(temp.c_str(), -1, m_pFont, p, &brush);
} //DrawRules

/// Use turtle graphics to draw the shape corresponding to the generated string
/// to the bitmap while calculating the *dirty rectangle* (the smallest
/// rectangle that contains all of the non-transparent pixels).
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

/// Use turtle graphics to draw the shape corresponding to the generated string
/// to the bitmap while calculating the *dirty rectangle* (the smallest
/// rectangle that contains all of the non-transparent pixels). This version of
/// the function constructs a hard-coded turtle graphics descriptor appropriate
/// to the current type and then uses Draw(const TurtleDesc&) to do the actual
/// work.

void CMain::Draw(){
  TurtleDesc d; //turtle graphics descriptor
  const float fDegToRad = (float)M_PI/180.0f; //degrees to radians
  
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
  
  d.m_fPointSize = m_bThickLines? 2.0f: 1.0f;

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
  AppendMenuW(m_hFileMenu, MF_STRING, IDM_FILE_GENERATE, L"Generate");
  AppendMenuW(m_hFileMenu, MF_STRING, IDM_FILE_SAVE, L"Save...");
  AppendMenuW(m_hFileMenu, MF_STRING, IDM_FILE_QUIT, L"Quit");

  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)m_hFileMenu, L"&File");

  //set the LSYS menu
  
  m_hLSMenu = CreateMenu();
  
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_A,
    L"Plant-like (Fig. 1.24a)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_B,
    L"Plant-like (Fig. 1.24b)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_C,
    L"Plant-like (Fig. 1.24c)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_D,
    L"Plant-like (Fig. 1.24d)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_E,
    L"Plant-like (Fig. 1.24e)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_PLANT_F,
    L"Plant-like (Fig. 1.24f)");
  
  AppendMenuW(m_hLSMenu, MF_SEPARATOR, 0, nullptr);

  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_BRANCHING,
    L"Stochastic branching (Fig. 1.27)");
  AppendMenuW(m_hLSMenu, MF_STRING, IDM_LSYS_HEXGOSPER, 
    L"Hexagonal Gosper curve (Fig. 1.11a)");
  
  AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)m_hLSMenu, L"&L-System");

  //set the VIEW menu
  
  m_hViewMenu = CreateMenu();

  AppendMenuW(m_hViewMenu, MF_STRING, IDM_VIEW_THICKLINES,  L"Thick lines");
  AppendMenuW(m_hViewMenu, MF_STRING, IDM_VIEW_RULES, L"Show rules");

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

/// Toggle the line thickness flag. Set the checkmark on the menu entry
/// and ask for a redraw of the window.

void CMain::ToggleLineThickness(){
  m_bThickLines = !m_bThickLines;
  const UINT status = m_bThickLines? MF_CHECKED: MF_UNCHECKED;
  CheckMenuItem(m_hViewMenu, IDM_VIEW_THICKLINES, status);
  Draw(); //redraw with new line thickness
} //ToggleLineThickness

/// Toggle the show rules flag. Set the checkmark on the menu entry
/// and ask for a refresh of the window.

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

/// Get the width of the widest line of the rule string in pixels. This will,
/// of course, depend on the font. Finding the pixel width of a character in
/// any given font is a black art, so the developers of GDI+ should (in my
/// humble opinion) be given a back-pat for making this process so easy,
/// not withstanding the number of new GDI+ concepts and functions that I had
/// to grok before I could get this to work.
/// \param graphics Reference to a GDI+ graphics object.
/// \return The width of the rule string in pixels.

int CMain::GetRuleStrWidth(Gdiplus::Graphics& graphics){
  const std::wstring& strRules = m_cLSystem.GetRuleString(); //shorthand
  const Gdiplus::RectF r = GetClientRectF(m_hWnd); //client rectangle
  
  const Gdiplus::CharacterRange charRange(0, (int)strRules.size());
  Gdiplus::StringFormat sf;
  sf.SetMeasurableCharacterRanges(1, &charRange);

  const int n = sf.GetMeasurableCharacterRangeCount();
  Gdiplus::Region* rg = new Gdiplus::Region[n]; //array of regions
  graphics.MeasureCharacterRanges(strRules.c_str(), -1, m_pFont, r, &sf, n, rg);

  //find the maximum region width in pixels

  int width = 0; //return result

  for(int i=0; i<n; i++){ //for each region
    Gdiplus::Rect r; //for bounding rectangle of current region
    rg[i].GetBounds(&r, &graphics); //get bounding rectangle of current region
    width = max(width, r.Width); //see if current region width is largest so far
  } //for

  //cleanup and exit

  delete [] rg;
  return width;
} //GetRuleStrWidth