/////////////////////////////////////////////////////////////////////
//
// X11Lib
// Win32 X11 library
//
// X11Wnd.cpp : Implementation of the window class
//
/////////////////////////////////////////////////////////////////////

#include "X11App.h"
#include "X11Wnd.h"

X11Wnd::X11Wnd(X11Wnd* wnd)
 : m_hWnd(0), m_parentWnd(wnd), m_backColour(0),
   m_dc(0), m_bitmap(0), m_oldBitmap(0), m_context(NULL)
{
}

X11Wnd::~X11Wnd()
{
  // If a Windows handle is attached, destroy it
  if (m_hWnd != 0)
  {
    ::OutputDebugString("Calling ::DestroyWindow() in X11Wnd::~X11Wnd()");
    ::DestroyWindow(m_hWnd);
  }
}

// Once only class initialization
void X11Wnd::init(void)
{
  // Register the window class
  WNDCLASSEX wndClass;
  ::ZeroMemory(&wndClass,sizeof(WNDCLASSEX));
  wndClass.cbSize = sizeof(WNDCLASSEX);
  wndClass.style = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
  wndClass.lpfnWndProc = X11Wnd::wndProc;
  wndClass.hInstance = X11App::getApp().getAppInstance();
  wndClass.hCursor = ::LoadCursor(NULL,IDC_ARROW);
  wndClass.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
  wndClass.lpszClassName = "X11Wnd";
  ::RegisterClassEx(&wndClass);
}

static int CALLBACK enumFontFunc(const LOGFONT*, const TEXTMETRIC*, DWORD, LPARAM data)
{
  (*(int*)data)++;
  return 1;
}

// Test for a font
bool X11Wnd::testFont(const char* name)
{
  int count = 0;
  HWND wnd = ::GetDesktopWindow();
  if (wnd)
  {
    HDC dc = ::GetDC(wnd);
    if (dc)
    {
      ::EnumFonts(dc,name,enumFontFunc,(LPARAM)&count);
      ::ReleaseDC(wnd,dc);
    }
  }

  return (count > 0);
}

// Create a font
HFONT X11Wnd::createFont(const char** names, int width, int height, int weight)
{
  const char* name = NULL;
  for (int i = 0; names[i] != NULL; i++)
  {
    if (testFont(names[i]))
    {
      name = names[i];
      break;
    }
  }
  if (name == NULL)
    name = "Courier";

  return ::CreateFont(height,width,
    0,0,weight,FALSE,FALSE,FALSE,ANSI_CHARSET,
    OUT_TT_ONLY_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,
    DEFAULT_PITCH|FF_DONTCARE,name);
}

// Create the Windows window
bool X11Wnd::createWindow(int x, int y, int w, int h)
{
  // Return if the window already exists
  if (m_hWnd != 0)
    return false;

  // Create the window
  m_hWnd = ::CreateWindowEx(0,"X11Wnd","",
    WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,x,y,w,h,
    m_parentWnd->getHwnd(),0,X11App::getApp().getAppInstance(),NULL);
  if (m_hWnd == 0)
    return false;

  // Create a memory device context and bitmap
  m_dc = ::CreateCompatibleDC(::GetDC(m_hWnd));
  createMemoryBitmap();

  // Store in the map
  m_windowMap.insert(WindowMap::value_type(m_hWnd,this));

  return true;
}

// Show or hide the window
void X11Wnd::showWindow(bool show)
{
  ::ShowWindow(m_hWnd,show ? SW_SHOW : SW_HIDE);
}

// Move the window
void X11Wnd::moveWindow(int x, int y)
{
  ::SetWindowPos(m_hWnd,0,x,y,0,0,
    SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER);
}

// Resize the window
void X11Wnd::resizeWindow(int w, int h)
{
  ::SetWindowPos(m_hWnd,0,0,0,w,h,
    SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOMOVE|SWP_NOZORDER);
}

// Alter the Z order window position
void X11Wnd::orderWindow(bool top)
{
  ::SetWindowPos(m_hWnd,top ? HWND_TOP : HWND_BOTTOM,0,0,0,0,
    SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER|SWP_NOMOVE|SWP_NOSIZE);
}

// Set the window's title
void X11Wnd::setWindowTitle(const char* title)
{
  ::SetWindowText(m_hWnd,title);
}

// Get the window rectangle
void X11Wnd::getWindowRect(RECT& r)
{
  ::GetWindowRect(m_hWnd,&r);
}

// Get the window's client rectangle
void X11Wnd::getClientRect(RECT& r)
{
  ::GetClientRect(m_hWnd,&r);
}

// Convert a point to the window's client co-ordinates
void X11Wnd::screenToClient(POINT& p)
{
  ::ScreenToClient(m_hWnd,&p);
}

// Set up the memory bitmap and device context
void X11Wnd::createMemoryBitmap(void)
{
  // Clear out the current bitmap
  deleteMemoryBitmap();

  // Get the size of the window
  RECT client;
  ::GetClientRect(m_hWnd,&client);

  // Create a memory bitmap
  m_bitmap = ::CreateCompatibleBitmap(::GetDC(m_hWnd),
    client.right,client.bottom);
  m_oldBitmap = (HBITMAP)::SelectObject(m_dc,m_bitmap);

  // Clear the bitmap
  solidRect(client,m_backColour);

  m_context = NULL;
}

// Delete the current bitmap
void X11Wnd::deleteMemoryBitmap(void)
{
  if (m_dc)
  {
    if (m_oldBitmap)
      ::SelectObject(m_dc,m_oldBitmap);

    if (m_bitmap)
    {
      ::DeleteObject(m_bitmap);
      m_bitmap = 0;
    }
  }
}

/////////////////////////////////////////////////////////////////////
// Graphics context
/////////////////////////////////////////////////////////////////////

X11Wnd::Context::Context()
 : m_foreColour(0), m_backColour(0), m_drawMode(R2_COPYPEN),
   m_font(0), m_pen(0)
{
}

X11Wnd::Context::~Context()
{
  // The font is stored elsewhere, but the pen must be cleared up here
  if (m_pen)
    ::DeleteObject(m_pen);
}

void X11Wnd::Context::setForegroundColour(unsigned long fore)
{
  m_foreColour = fore;

  // Update the pen
  if (m_pen)
    ::DeleteObject(m_pen);
  m_pen = ::CreatePen(PS_SOLID,0,fore);
}

// Set up a device context
void X11Wnd::Context::setContext(HDC dc)
{
  ::SetTextAlign(dc,TA_BOTTOM);
  ::SetTextColor(dc,m_foreColour);
  ::SetBkMode(dc,TRANSPARENT);
  ::SetBkColor(dc,m_backColour);
  ::SetROP2(dc,m_drawMode);

  ::SelectObject(dc,::GetStockObject(HOLLOW_BRUSH));
  if (m_font != 0)
    ::SelectObject(dc,m_font);
  if (m_pen != 0)
    ::SelectObject(dc,m_pen);
}

void X11Wnd::setContext(Context* context)
{
  if (m_context != context)
  {
    context->setContext(m_dc);
    m_context = context;
  }
}

/////////////////////////////////////////////////////////////////////
// Drawing operations
/////////////////////////////////////////////////////////////////////

void X11Wnd::clearWindow(void)
{
  RECT client;
  ::GetClientRect(m_hWnd,&client);
  solidRect(client,m_backColour);
  ::InvalidateRect(m_hWnd,NULL,FALSE);
}

void X11Wnd::drawLine(Context* context, int x1, int y1, int x2, int y2)
{
  setContext(context);
  ::MoveToEx(m_dc,x1,y1,NULL);
  ::LineTo(m_dc,x2,y2);
  ::InvalidateRect(m_hWnd,NULL,FALSE);
}

void X11Wnd::drawPoint(Context* context, int x, int y)
{
  setContext(context);
  ::SetPixel(m_dc,x,y,::GetTextColor(m_dc));
  ::InvalidateRect(m_hWnd,NULL,FALSE);
}

void X11Wnd::drawPoints(Context* context, POINT* points, int numPoints)
{
  setContext(context);
  COLORREF col = ::GetTextColor(m_dc);
  for (int i = 0; i < numPoints; i++)
    ::SetPixel(m_dc,points[i].x,points[i].y,col);
  ::InvalidateRect(m_hWnd,NULL,FALSE);
}

void X11Wnd::drawRect(Context* context, int x, int y, int w, int h)
{
  setContext(context);
  ::Rectangle(m_dc,x,y,x+w,y+h);
  ::InvalidateRect(m_hWnd,NULL,FALSE);
}

void X11Wnd::drawString(Context* context, int x, int y, const char* s, int l)
{
  setContext(context);

  // Check the length of the string
  int len = l;
  for (int i = 0; i < l; i++)
  {
    if (s[i] == '\0')
    {
      len = i;
      break;
    }
  }
  ::TextOut(m_dc,x,y,s,len);
  ::InvalidateRect(m_hWnd,NULL,FALSE);
}

void X11Wnd::fillRect(Context* context, int x, int y, int w, int h)
{
  setContext(context);

  RECT r;
  r.left = x;
  r.top = y;
  r.right = x+w;
  r.bottom = y+h;
  solidRect(r,::GetTextColor(m_dc));
  ::InvalidateRect(m_hWnd,NULL,FALSE);
}

void X11Wnd::fillRects(Context* context, RECT* rects, int numRects)
{
  setContext(context);
  for (int i = 0; i < numRects; i++)
    solidRect(rects[i],::GetTextColor(m_dc));
  ::InvalidateRect(m_hWnd,NULL,FALSE);
}

void X11Wnd::solidRect(const RECT& r, COLORREF col)
{
  COLORREF back = ::GetBkColor(m_dc);
  ::SetBkColor(m_dc,col);
  ::ExtTextOut(m_dc,0,0,ETO_OPAQUE,&r,NULL,0,NULL);
  ::SetBkColor(m_dc,back);
}

/////////////////////////////////////////////////////////////////////
// Message handlers
/////////////////////////////////////////////////////////////////////

// Window procedure
LRESULT CALLBACK X11Wnd::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  // Find the window
  WindowMap::iterator it = m_windowMap.find(hWnd);
  if (it != m_windowMap.end())
  {
    X11Wnd* wnd = it->second;
    switch (msg)
    {
    case WM_DESTROY:
      wnd->onDestroy();
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case WM_MOUSEMOVE:
      wnd->onMouseMove();
      break;
    case WM_PAINT:
      {
        PAINTSTRUCT ps;
        ::BeginPaint(hWnd,&ps);
        wnd->onPaint(ps.hdc);
        ::EndPaint(hWnd,&ps);
      }
      return 0;
    case WM_SIZE:
      wnd->onSize();
      break;
    }

    // Should the message be used to create an event?
    if (wnd->findMessage(msg))
      X11App::getApp().addToMsgQueue(wnd,msg,wParam,lParam);
  }
  return ::DefWindowProc(hWnd,msg,wParam,lParam);
}

void X11Wnd::onDestroy(void)
{
  // Remove from the map of active windows
  m_windowMap.erase(m_hWnd);

  // Delete the memory bitmap and device context
  deleteMemoryBitmap();
  ::DeleteDC(m_dc);

  m_parentWnd = NULL;
  m_hWnd = 0;
  m_dc = 0;
}

void X11Wnd::onMouseMove(void)
{
  // Make sure input events are redirected to this window
  setActiveWindow(this);
}

void X11Wnd::onPaint(HDC dc)
{
  // Get the size of the window
  RECT client;
  ::GetClientRect(m_hWnd,&client);

  // Copy the memory bitmap to the display
  ::BitBlt(dc,0,0,client.right,client.bottom,m_dc,0,0,SRCCOPY);
}

void X11Wnd::onSize(void)
{
  // Resize the memory bitmap
  createMemoryBitmap();
  ::InvalidateRect(m_hWnd,NULL,FALSE);
}

/////////////////////////////////////////////////////////////////////
// Static data
/////////////////////////////////////////////////////////////////////

X11Wnd* X11Wnd::m_activeWindow = NULL;
X11Wnd::WindowMap X11Wnd::m_windowMap;
