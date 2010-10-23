/////////////////////////////////////////////////////////////////////
//
// X11Lib
// Win32 X11 library
//
// X11App.cpp : Implementation of the application class
//
/////////////////////////////////////////////////////////////////////

#include "X11App.h"
#include "X11Wnd.h"

#include <string>

// Process Windows messages
void X11App::processMessages(void)
{
  MSG msg;
  while (::PeekMessage(&msg,0,0,0,PM_REMOVE))
  {
    if (msg.message == WM_QUIT)
      ::ExitProcess(0);

    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }
}

// Get the Windows display depth
int X11App::getDisplayDepth(void)
{
  HWND wnd = ::GetDesktopWindow();
  if (wnd)
  {
    HDC dc = ::GetDC(wnd);
    if (dc)
    {
      int depth = ::GetDeviceCaps(dc,BITSPIXEL);
      ::ReleaseDC(wnd,dc);
      return depth;
    }
  }
  return 0;
}

// Put the current message on the message queue
void X11App::addToMsgQueue(X11Wnd* wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  MsgData data(wnd,msg);

  switch (msg)
  {
  case WM_KEYDOWN:
    {
      data.m_param1 = wParam;

      // Store the ASCII value of the keypress
      static BYTE state[256];
      static WORD ascii[32];
      ::GetKeyboardState(state);
      if (::ToAscii(wParam,0,state,ascii,0) == 1)
      {
        if (isprint((int)ascii[0]))
          data.m_param2 = ascii[0];
      }

      // Store the Shift key state
      if (::GetKeyState(VK_SHIFT) & 0x8000)
        data.m_param2 |= 0x10000;

      // Store the cursor position
      data.m_param3 = ::GetMessagePos();
    }
    break;
  default:
    data.m_param1 = wParam;
    data.m_param2 = lParam;
    break;
  }

  m_msgQueue.push_back(data);
}

// Pull a message from the message queue
bool X11App::getFromMsgQueue(const MsgSet& msgs, MsgData& msg)
{
  for (MsgVector::iterator it = m_msgQueue.begin(); it != m_msgQueue.end(); it++)
  {
    if (msgs.find(it->m_msg) != msgs.end())
    {
      msg = *it;
      m_msgQueue.erase(it);
      return true;
    }
  }
  return false;
}

// Generate output as if from /bin/ls
void X11App::cmd_ls(FILE* output, const char* pattern)
{
  // Find all files matching the pattern
  std::set<std::string> files;
  WIN32_FIND_DATA find;
  HANDLE h = ::FindFirstFile(pattern,&find);
  if (h != INVALID_HANDLE_VALUE)
  {
    bool more = true;
    do
    {
      files.insert(std::string(find.cFileName));
      if (::FindNextFile(h,&find) == 0)
      {
        ::FindClose(h);
        more = false;
      }
    }
    while (more);
  }

  // Output all the file names
  for (std::set<std::string>::const_iterator it = files.begin(); it != files.end(); it++)
    fprintf(output,"%s\n",it->c_str());
}

// Singleton app
X11App X11App::m_app;
