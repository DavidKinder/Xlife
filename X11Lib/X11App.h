/////////////////////////////////////////////////////////////////////
//
// X11Lib
// Win32 X11 library
//
// X11App.h : Declaration of the application class
//
/////////////////////////////////////////////////////////////////////

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <set>
#include <vector>

class X11Wnd;

class X11App
{
public:
  ~X11App() {}

public:
  // Get and set the application instance
  void setAppInstance(HINSTANCE hInstance) { m_hAppInstance = hInstance; }
  HINSTANCE getAppInstance(void) const { return m_hAppInstance; }

  // Get the bit depth of the display
  static int getDisplayDepth(void);

  // Access to singleton app
  static X11App& getApp(void) { return m_app; }

  // Process Windows messages
  void processMessages(void);

  // Generate output as if from /bin/ls
  void cmd_ls(FILE* output, const char* pattern);

  // Message queue

  class MsgData
  {
  public:
    MsgData() : m_wnd(NULL), m_msg(0), m_param1(0), m_param2(0), m_param3(0) {}
    MsgData(X11Wnd* wnd, UINT msg) : m_wnd(wnd), m_msg(msg), m_param1(0), m_param2(0), m_param3(0) {}

    X11Wnd* m_wnd;
    UINT m_msg;
    unsigned long m_param1;
    unsigned long m_param2;
    unsigned long m_param3;
  };

  typedef std::set<UINT> MsgSet;
  typedef std::vector<MsgData> MsgVector;

  void addToMsgQueue(X11Wnd* wnd, UINT msg, WPARAM wParam, LPARAM lParam);
  bool getFromMsgQueue(const MsgSet& msgs, MsgData& msg);

protected:
  // Hidden constructor
  X11App() {}

  // Singleton app
  static X11App m_app;

  HINSTANCE m_hAppInstance;
  MsgVector m_msgQueue;
};
