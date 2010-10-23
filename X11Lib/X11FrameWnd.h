/////////////////////////////////////////////////////////////////////
//
// X11Lib
// Win32 X11 library
//
// X11FrameWnd.h : Declaration of the frame window class
//
/////////////////////////////////////////////////////////////////////

#pragma once

#include "X11Wnd.h"

class X11FrameWnd : public X11Wnd
{
public:
  X11FrameWnd();
  virtual ~X11FrameWnd();

  virtual bool createWindow(int x, int y, int w, int h);

public:
  // Static methods
  static void init(void);

  // Window procedure
  static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
