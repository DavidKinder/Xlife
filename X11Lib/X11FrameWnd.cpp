/////////////////////////////////////////////////////////////////////
//
// X11Lib
// Win32 X11 library
//
// X11FrameWnd.cpp : Implementation of the frame window class
//
/////////////////////////////////////////////////////////////////////

#include "X11App.h"
#include "X11FrameWnd.h"

X11FrameWnd::X11FrameWnd() : X11Wnd(NULL)
{
}

X11FrameWnd::~X11FrameWnd()
{
}

// Once only class initialization
void X11FrameWnd::init(void)
{
  // Register the window class
  WNDCLASSEX wndClass;
  ::ZeroMemory(&wndClass,sizeof(WNDCLASSEX));
  wndClass.cbSize = sizeof(WNDCLASSEX);
  wndClass.style = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
  wndClass.lpfnWndProc = X11FrameWnd::wndProc;
  wndClass.hInstance = X11App::getApp().getAppInstance();
  wndClass.hIcon = ::LoadIcon(wndClass.hInstance,MAKEINTRESOURCE(100));
  wndClass.hCursor = ::LoadCursor(NULL,IDC_ARROW);
  wndClass.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
  wndClass.lpszClassName = "X11FrameWnd";
  ::RegisterClassEx(&wndClass);
}

// Create the Windows window
bool X11FrameWnd::createWindow(int x, int y, int w, int h)
{
  // Return if the window already exists
  if (m_hWnd != 0)
    return false;

  // Create the window
  m_hWnd = ::CreateWindowEx(WS_EX_CLIENTEDGE,"X11FrameWnd","",
    WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
    CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
    0,0,X11App::getApp().getAppInstance(),NULL);
  if (m_hWnd == 0)
    return false;

  // Create a memory device context and bitmap
  m_dc = ::CreateCompatibleDC(::GetDC(m_hWnd));
  createMemoryBitmap();

  // Store in the map
  m_windowMap.insert(WindowMap::value_type(m_hWnd,this));

  // Get the window and client rectangles
  RECT wndRect, clientRect;
  ::GetWindowRect(m_hWnd,&wndRect);
  ::GetClientRect(m_hWnd,&clientRect);

  // Adjust for the size of the non-client area
  w += (wndRect.right - wndRect.left) - (clientRect.right - clientRect.left);
  h += (wndRect.bottom - wndRect.top) - (clientRect.bottom - clientRect.top);

  // Keep within the work area
  RECT work;
  if (::SystemParametersInfo(SPI_GETWORKAREA,0,&work,0))
  {
    if (x < work.left)
      x = work.left;
    if (y < work.top)
      y = work.top;
    if (x+w > work.right)
      w = work.right-x;
    if (y+h > work.bottom)
      h = work.bottom-y;
  }

  // Resize the window
  ::MoveWindow(m_hWnd,x,y,w,h,FALSE);
  return true;
}

// Window procedure
LRESULT CALLBACK X11FrameWnd::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
      ::PostQuitMessage(0);
      break;
    }

    // Should the message be used to create an event for the active window?
    if (m_activeWindow)
    {
      if (m_activeWindow->findMessage(msg))
        X11App::getApp().addToMsgQueue(m_activeWindow,msg,wParam,lParam);
    }
  }
  return X11Wnd::wndProc(hWnd,msg,wParam,lParam);
}
