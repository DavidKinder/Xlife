/////////////////////////////////////////////////////////////////////
//
// X11Lib
// Win32 X11 library
//
// X11Wnd.h : Declaration of the window class
//
/////////////////////////////////////////////////////////////////////

#pragma once
#pragma warning(disable : 4786)

#include "X11App.h"

#include <map>
#include <set>
#include <string>
#include <vector>

class X11Wnd
{
public:
  X11Wnd(X11Wnd* parentWnd);
  virtual ~X11Wnd();

  HWND getHwnd(void) { return m_hWnd; }

  static X11Wnd* getActiveWindow(void) { return m_activeWindow; }
  static void setActiveWindow(X11Wnd* wnd) { m_activeWindow = wnd; }

  unsigned long getBackgroundColour(void) { return m_backColour; }
  void setBackgroundColour(unsigned long back) { m_backColour = back; }

  static void init(void);
  static bool testFont(const char* name);
  static HFONT createFont(const char** names, int width, int height, int weight);

  void createMemoryBitmap(void);
  void deleteMemoryBitmap(void);

  virtual bool createWindow(int x, int y, int w, int h);
  void showWindow(bool show);
  void moveWindow(int x, int y);
  void resizeWindow(int w, int h);
  void orderWindow(bool top);
  void setWindowTitle(const char* title);

  void getWindowRect(RECT& r);
  void getClientRect(RECT& r);
  void screenToClient(POINT& p);

  // Message handlers
  static LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  virtual void onDestroy(void);
  virtual void onMouseMove(void);
  virtual void onPaint(HDC dc);
  virtual void onSize(void);

  // Graphics context
  class Context
  {
  public:
    Context();
    ~Context();

    void setContext(HDC dc);

    void setForegroundColour(unsigned long fore);
    void setBackgroundColour(unsigned long back) { m_backColour = back; }
    void setDrawMode(int mode) { m_drawMode = mode; }
    void setFont(HFONT font) { m_font = font; }

  protected:
    unsigned long m_foreColour;
    unsigned long m_backColour;
    int m_drawMode;

    HFONT m_font;
    HPEN m_pen;
  };
  void setContext(Context* context);

  // Drawing operations
  void clearWindow(void);
  void drawLine(Context* context, int x1, int y1, int x2, int y2);
  void drawPoint(Context* context, int x, int y);
  void drawPoints(Context* context, POINT* points, int numPoints);
  void drawRect(Context* context, int x, int y, int w, int h);
  void drawString(Context* context, int x, int y, const char* s, int l);
  void fillRect(Context* context, int x, int y, int w, int h);
  void fillRects(Context* context, RECT* rects, int numRects);

  // Messages
  void clearMessages(void) { m_msgs.clear(); }
  void addMessage(UINT msg) { m_msgs.insert(msg); }
  bool findMessage(UINT msg) { return m_msgs.find(msg) != m_msgs.end(); }

protected:
  void solidRect(const RECT& r, COLORREF col);

protected:
  X11Wnd* m_parentWnd;
  HWND m_hWnd;
  unsigned long m_backColour;

  HDC m_dc;
  HBITMAP m_bitmap;
  HBITMAP m_oldBitmap;
  Context* m_context;

  X11App::MsgSet m_msgs;
  static X11Wnd* m_activeWindow;

  typedef std::map<HWND,X11Wnd*> WindowMap;
  static WindowMap m_windowMap;
};
