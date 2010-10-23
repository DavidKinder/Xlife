/////////////////////////////////////////////////////////////////////
//
// X11Lib
// Win32 X11 library
//
// X11Lib.cpp : Implementation of entry point routines
//
/////////////////////////////////////////////////////////////////////

#include "X11App.h"
#include "X11FrameWnd.h"
#include "X11Wnd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <string>

extern "C"
{
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/time.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

/////////////////////////////////////////////////////////////////////
// System Initialization
/////////////////////////////////////////////////////////////////////

namespace
{
  char moduleName[256];
  char* progArgs[1];
}

extern "C" void Win32X11Init(HINSTANCE hInstance)
{
  // Initialize the application
  X11App::getApp().setAppInstance(hInstance);

  // Initialize the window classes
  X11Wnd::init();
  X11FrameWnd::init();

  // Turn off Windows errors
  ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

  // Set up the environment
  _putenv("DISPLAY=X11Lib");
}

/////////////////////////////////////////////////////////////////////
// Unix user account functions
/////////////////////////////////////////////////////////////////////

namespace
{
  struct passwd userPassword = { "winuser",1,1,"","","winuser" };
}

extern "C" struct passwd* getpwuid(int uid)
{
  return &userPassword;
}

extern "C" struct passwd* getpwnam(const char *name)
{
  return &userPassword;
}

extern "C" uid_t getuid(void)
{
  return 1;
}

/////////////////////////////////////////////////////////////////////
// Unix directory functions
/////////////////////////////////////////////////////////////////////

namespace
{
  struct dirent dirEntry;
}

extern "C" int closedir(DIR* dir)
{
  ::FindClose((HANDLE)dir);
  return 0;
}

extern "C" DIR* opendir(const char *dirname)
{
  std::string pattern = dirname;
  pattern += "*.*";

  WIN32_FIND_DATA find;
  HANDLE h = ::FindFirstFile(pattern.c_str(),&find);
  if (h != INVALID_HANDLE_VALUE)
    return (DIR*)h;
  return NULL;
}

extern "C" struct dirent* readdir(DIR *dir)
{
  WIN32_FIND_DATA find;
  if (::FindNextFile((HANDLE)dir,&find))
  {
    strcpy(dirEntry.d_name,find.cFileName);
    dirEntry.d_namlen = strlen(dirEntry.d_name);
    return &dirEntry;
  }
  else
    ::FindClose((HANDLE)dir);
  return NULL;
}

/////////////////////////////////////////////////////////////////////
// Unix time function
/////////////////////////////////////////////////////////////////////

extern "C" int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
  // Get the system time for the millisecond value
  SYSTEMTIME SystemTime;
  ::GetSystemTime(&SystemTime);

  // Use time() for the other time values
  tp->tv_sec = time(NULL);
  tp->tv_usec = SystemTime.wMilliseconds * 1000;
  tzp->tz_minuteswest = 0;
  tzp->tz_dsttime = 0;
  return 0;
}

/////////////////////////////////////////////////////////////////////
// Other Unix functions
/////////////////////////////////////////////////////////////////////

namespace
{
  const char* hostName = "localhost";
}

extern "C" long random(void)
{
  return rand();
}

extern "C" int srandom(int seed)
{
  srand(seed);
  return 0;
}

extern "C" int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)
{
  if (readfds != NULL)
    ::OutputDebugString("X11Lib: select() readfds not null\n");
  if (writefds != NULL)
    ::OutputDebugString("X11Lib: select() writefds not null\n");
  if (exceptfds != NULL)
    ::OutputDebugString("X11Lib: select() exceptfds not null\n");

  // Wait for timeout
  ::Sleep((timeout->tv_sec * 1000) + (timeout->tv_usec / 1000));
  return 0;
}

extern "C" int gethostname(char *buf, int size)
{
  if (size < (int)strlen(hostName)+1)
    return 1;
  strcpy(buf,hostName);
  return 0;
}

extern "C" void bzero(void *ptr, size_t len)
{
  ::ZeroMemory(ptr,len);
}

extern "C" FILE* popen(const char* command, const char* mode)
{
  // Is this read mode?
  if (strcmp(mode,"r") == 0)
  {
    // Find the temporary directory
    char path[_MAX_PATH];
    if (::GetTempPath(_MAX_PATH,path) > 0)
    {
      // Create a temporary file
      FILE* temp = fopen(_tempnam(path,"x11lib"),"w+bD");
      if (temp != NULL)
      {
        // Is this a recognised command?
        if (strncmp(command,"/bin/ls ",8) == 0)
        {
          // Find the first argument
          std::string pattern;
          int i = 8;
          while ((command[i] != '\0') && (command[i] != ' '))
          {
            pattern += command[i];
            i++;
          }

          // Call the equivalent of /bin/ls
          X11App::getApp().cmd_ls(temp,pattern.c_str());
        }
        else
          ::OutputDebugString("X11Lib: popen() unsupported command\n");

        // Return the file pointer, set back to the start of the file
        rewind(temp);
        return temp;
      }
    }
  }
  else
    ::OutputDebugString("X11Lib: popen() unsupported mode\n");
  return NULL;
}

extern "C" int pclose(FILE* pf)
{
  return fclose(pf);
}

/////////////////////////////////////////////////////////////////////
// X functions
/////////////////////////////////////////////////////////////////////

#define NUMBER_COLOURS 8
#define NUMBER_FONTS 4
#define NUMBER_KEYS 30
namespace
{
  Display display;
  Screen screen;

  // Colours
  struct NamedColour
  {
    char* name;
    unsigned long pixel;
    unsigned short red;
    unsigned short green;
    unsigned short blue;
  };
  NamedColour namedColours[NUMBER_COLOURS] = {
    "black", RGB(0x00,0x00,0x00),0x0000,0x0000,0x0000,
    "blue",  RGB(0x00,0x00,0xFF),0x0000,0x0000,0xFFFF,
    "brown", RGB(0xA0,0x70,0x20),0xA000,0x7000,0x2000,
    "green", RGB(0x00,0xFF,0x00),0x0000,0xFFFF,0x0000,
    "purple",RGB(0xFF,0x00,0xFF),0xFFFF,0x0000,0xFFFF,
    "red",   RGB(0xFF,0x00,0x00),0xFFFF,0x0000,0x0000,
    "white", RGB(0xFF,0xFF,0xFF),0xFFFF,0xFFFF,0xFFFF,
    "yellow",RGB(0xFF,0xFF,0x00),0xFFFF,0xFFFF,0x0000,
  };

  // Fonts
  struct NamedFont
  {
    const char* name;
    const char** windowsNames;
    int width;
    int height;
    int weight;
  };
  const char* fixedFontNames[4] = {
    "Lucida Console","Consolas","Courier New",NULL
  };
  NamedFont namedFonts[NUMBER_FONTS] = {
    "8x13",fixedFontNames,8,13,FW_NORMAL,
    "8x13bold",fixedFontNames,8,13,FW_BOLD,
    "9x15",fixedFontNames,9,15,FW_NORMAL,
    "9x15bold",fixedFontNames,9,15,FW_BOLD,
  };

  // Keys
  struct KeyTranslation
  {
    unsigned long windowsCode;
    KeySym x11Code;
    KeySym x11ShiftCode;
  };
  KeyTranslation keys[NUMBER_KEYS] = {
    '0',XK_0,XK_parenright,
    '1',XK_1,XK_exclam,
    '2',XK_2,XK_quotedbl,
    '3',XK_3,XK_sterling,
    '4',XK_4,XK_dollar,
    '5',XK_5,XK_percent,
    '6',XK_6,XK_asciicircum,
    '7',XK_7,XK_ampersand,
    '8',XK_8,XK_asterisk,
    '9',XK_9,XK_parenleft,
    VK_NUMPAD0,XK_KP_0,XK_KP_0,
    VK_NUMPAD1,XK_KP_1,XK_End,
    VK_NUMPAD2,XK_KP_2,XK_Down,
    VK_NUMPAD3,XK_KP_3,XK_Next,
    VK_NUMPAD4,XK_KP_4,XK_Left,
    VK_NUMPAD5,XK_KP_5,XK_KP_5,
    VK_NUMPAD6,XK_KP_6,XK_Right,
    VK_NUMPAD7,XK_KP_7,XK_Begin,
    VK_NUMPAD8,XK_KP_8,XK_Up,
    VK_NUMPAD9,XK_KP_9,XK_Prior,
    VK_LEFT,XK_Left,XK_Left,
    VK_RIGHT,XK_Right,XK_Right,
    VK_UP,XK_Up,XK_Up,
    VK_DOWN,XK_Down,XK_Down,
    VK_RETURN,XK_Return,XK_Return,
    VK_BACK,XK_BackSpace,XK_BackSpace,
    VK_DELETE,XK_Delete,XK_Delete,
    VK_TAB,XK_Tab,XK_Tab,
    VK_ESCAPE,XK_Escape,XK_Escape,
  };
}

extern "C" Status XAllocNamedColor(Display* display, Colormap colormap, _Xconst char*  color_name, XColor* screen_def_return, XColor* exact_def_return)
{
  // Initialize the structure
  screen_def_return->flags = 0;
  screen_def_return->pad = 0;
  screen_def_return->pixel = 0;
  screen_def_return->red = 0;
  screen_def_return->green = 0;
  screen_def_return->blue = 0;

  // Check for a recognized colour
  bool found = false;
  int i = 0;
  while ((i < NUMBER_COLOURS) && (found == false))
  {
    if (stricmp(color_name,namedColours[i].name) == 0)
    {
      screen_def_return->pixel = namedColours[i].pixel;
      screen_def_return->red = namedColours[i].red;
      screen_def_return->green = namedColours[i].green;
      screen_def_return->blue = namedColours[i].blue;
      found = true;
    }
    i++;
  }

  if (found == false)
    ::OutputDebugString("X11Lib: XAllocNamedColor() color_name not recognized\n");

  // Copy to the exact colour
  if (screen_def_return != exact_def_return)
    ::CopyMemory(exact_def_return,screen_def_return,sizeof(XColor));

  return True;
}

extern "C" int XChangeWindowAttributes(Display* display, Window w, unsigned long valuemask, XSetWindowAttributes* attributes)
{
  ::OutputDebugString("X11Lib: XChangeWindowAttributes() does nothing\n");
  return 0;
}

extern "C" Bool XCheckMaskEvent(Display* display, long event_mask, XEvent* event_return)
{
  // Make sure any outstanding Windows messages are handled
  X11App::getApp().processMessages();

  // Create message set
  X11App::MsgSet msgs;
  if (event_mask & KeyPressMask)
    msgs.insert(WM_KEYDOWN);
  if (event_mask & (PointerMotionMask|Button1MotionMask|Button2MotionMask|Button3MotionMask))
    msgs.insert(WM_MOUSEMOVE);
  if (event_mask & StructureNotifyMask)
    msgs.insert(WM_SIZE);
  if (event_mask & ButtonPressMask)
  {
    msgs.insert(WM_LBUTTONDOWN);
    msgs.insert(WM_RBUTTONDOWN);
    msgs.insert(WM_MBUTTONDOWN);
  }
  if (event_mask & ButtonReleaseMask)
  {
    msgs.insert(WM_LBUTTONUP);
    msgs.insert(WM_RBUTTONUP);
    msgs.insert(WM_MBUTTONUP);
  }

  X11App::MsgData msg;
  if (X11App::getApp().getFromMsgQueue(msgs,msg))
  {
    // Initialize the event header
    ::ZeroMemory(event_return,sizeof(XEvent));
    event_return->xany.display = display;
    event_return->xany.window = (Window)msg.m_wnd;

    // Check the event type
    switch (msg.m_msg)
    {
    case WM_KEYDOWN:
      {
        event_return->xany.type = KeyPress;

        // Translate the Windows code to a keyboard symbol
        for (int i = 0; i < NUMBER_KEYS; i++)
        {
          if (msg.m_param1 == keys[i].windowsCode)
          {
            if (msg.m_param2 & 0x10000)
              event_return->xkey.keycode = keys[i].x11ShiftCode;
            else
              event_return->xkey.keycode = keys[i].x11Code;
          }
        }

        // Store the ASCII value in the high word
        event_return->xkey.keycode |= (msg.m_param2 & 0xFF) << 16;

        // Store the mouse position
        POINT point;
        point.x = GET_X_LPARAM(msg.m_param3);
        point.y = GET_Y_LPARAM(msg.m_param3);
        msg.m_wnd->screenToClient(point);
        event_return->xkey.x = point.x;
        event_return->xkey.y = point.y;
      }
      break;
    case WM_SIZE:
      {
        event_return->xany.type = ConfigureNotify;

        RECT inner, outer;
        msg.m_wnd->getClientRect(inner);
        msg.m_wnd->getWindowRect(outer);

        event_return->xconfigure.x = outer.left;
        event_return->xconfigure.y = outer.top;
        event_return->xconfigure.width = inner.right;
        event_return->xconfigure.height = inner.bottom;
      }
      break;
    case WM_MOUSEMOVE:
      event_return->xany.type = MotionNotify;
      event_return->xmotion.x = LOWORD(msg.m_param2);
      event_return->xmotion.y = HIWORD(msg.m_param2);

      if (msg.m_param1 & MK_LBUTTON)
      {
        if (msg.m_param1 & MK_SHIFT)
          event_return->xmotion.state |= Button3MotionMask;
        else
          event_return->xmotion.state |= Button1MotionMask;
      }
      else if (msg.m_param1 & MK_RBUTTON)
        event_return->xmotion.state |= Button2MotionMask;
      else if (msg.m_param1 & MK_MBUTTON)
        event_return->xmotion.state |= Button3MotionMask;
      break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      if (msg.m_msg == WM_LBUTTONUP)
        event_return->xany.type = ButtonRelease;
      else
        event_return->xany.type = ButtonPress;
      event_return->xbutton.x = LOWORD(msg.m_param2);
      event_return->xbutton.y = HIWORD(msg.m_param2);
      if (msg.m_param1 & MK_SHIFT)
        event_return->xbutton.button = Button3;
      else
        event_return->xbutton.button = Button1;
      break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      if (msg.m_msg == WM_RBUTTONUP)
        event_return->xany.type = ButtonRelease;
      else
        event_return->xany.type = ButtonPress;
      event_return->xbutton.x = LOWORD(msg.m_param2);
      event_return->xbutton.y = HIWORD(msg.m_param2);
      event_return->xbutton.button = Button2;
      break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      if (msg.m_msg == WM_MBUTTONUP)
        event_return->xany.type = ButtonRelease;
      else
        event_return->xany.type = ButtonPress;
      event_return->xbutton.x = LOWORD(msg.m_param2);
      event_return->xbutton.y = HIWORD(msg.m_param2);
      event_return->xbutton.button = Button3;
      break;
    }
    return True;
  }
  return False;
}

extern "C" int XClearWindow(Display* display, Window w)
{
  ((X11Wnd*)w)->clearWindow();
  return 0;
}

extern "C" Pixmap XCreateBitmapFromData(Display* display, Drawable d, _Xconst char* data, unsigned int width, unsigned int height)
{
  char* windowsData = new char[width*height];
  for (unsigned int i = 0; i < width*height; i++)
  {
    int v1 = data[i];
    int v2 = 0;
    for (int j = 0; j < 8; j++)
    {
      v2 <<= 1;
      if (v1 & 1)
        v2 |= 1;
      v1 >>= 1;
    }
    windowsData[i] = v2;
  }

  HBITMAP bitmap = ::CreateBitmap(width,height,1,1,windowsData);
  delete windowsData;
  return (Pixmap)bitmap;
}

extern GC XCreateGC(Display* display, Drawable d, unsigned long valuemask, XGCValues* values)
{
  unsigned long mask = GCForeground|GCBackground|GCFunction|GCPlaneMask;
  if ((valuemask & ~(mask)) != 0)
    ::OutputDebugString("X11Lib: XCreateGC() valuemask includes unsupported values\n");

  // Create a new graphics context
  X11Wnd::Context* gc = new X11Wnd::Context;

  // Fill in the graphics context
  if (valuemask & GCForeground)
    gc->setForegroundColour(values->foreground);
  if (valuemask & GCBackground)
    gc->setBackgroundColour(values->background);
  if (valuemask & GCFunction)
  {
    switch (values->function)
    {
    case GXcopy:
      gc->setDrawMode(R2_COPYPEN);
      break;
    case GXinvert:
      gc->setDrawMode(R2_XORPEN);
      break;
    default:
      ::OutputDebugString("X11Lib: XCreateGC() GCFunction not recognized\n");
      break;
    }
  }
  if (valuemask & GCPlaneMask)
    ::OutputDebugString("X11Lib: XCreateGC() GCPlaneMask ignored\n");

  return (GC)gc;
}

extern "C" Cursor XCreatePixmapCursor(Display* display, Pixmap source, Pixmap mask, XColor* foreground_color, XColor* background_color, unsigned int x, unsigned int y)
{
  ::OutputDebugString("X11Lib: XCreatePixmapCursor() does nothing\n");
  return 0;
}

extern "C" Window XCreateSimpleWindow(Display*, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background)
{
  X11Wnd* wnd = NULL;

  // If the root window (0) is the parent, open a frame window
  if (parent == 0)
    wnd = new X11FrameWnd;
  else
    wnd = new X11Wnd((X11Wnd*)parent);

  // Create the window
  if (wnd != NULL)
  {
    if (wnd->createWindow(x,y,width,height))
      wnd->setBackgroundColour(background);
    else
    {
      delete wnd;
      wnd = NULL;
    }
  }

  return (Window)wnd;
}

extern int XDefaultDepth(Display* display, int screen_number)
{
  return X11App::getDisplayDepth();
}

extern "C" int XDefineCursor(Display* display, Window w, Cursor cursor)
{
  ::OutputDebugString("X11Lib: XDefineCursor() does nothing\n");
  return 0;
}

extern "C" int XDrawLine(Display* display, Drawable d, GC gc, int x1, int y1, int x2, int y2)
{
  ((X11Wnd*)d)->drawLine((X11Wnd::Context*)gc,x1,y1,x2,y2);
  return 0;
}

extern "C" int XDrawPoint(Display* display, Drawable d, GC gc, int x, int y)
{
  ((X11Wnd*)d)->drawPoint((X11Wnd::Context*)gc,x,y);
  return 0;
}

extern "C" int XDrawPoints(Display* display, Drawable d, GC gc, XPoint* points, int npoints, int mode)
{
  POINT* p = new POINT[npoints];
  for (int i = 0; i < npoints; i++)
  {
    p[i].x = points[i].x;
    p[i].y = points[i].y;
  }

  ((X11Wnd*)d)->drawPoints((X11Wnd::Context*)gc,p,npoints);

  delete[] p;
  return 0;
}

extern "C" int XDrawRectangle(Display* display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height)
{
  ((X11Wnd*)d)->drawRect((X11Wnd::Context*)gc,x,y,width,height);
  return 0;
}

extern "C" int XDrawString(Display* display, Drawable d, GC gc, int x, int y, _Xconst char* string, int length)
{
  ((X11Wnd*)d)->drawString((X11Wnd::Context*)gc,x,y,string,length);
  return 0;
}

extern "C" int XFillRectangle(Display* display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height)
{
  ((X11Wnd*)d)->fillRect((X11Wnd::Context*)gc,x,y,width,height);
  return 0;
}

extern "C" int XFillRectangles(Display* display, Drawable d, GC gc, XRectangle* rectangles, int nrectangles)
{
  RECT* r = new RECT[nrectangles];
  for (int i = 0; i < nrectangles; i++)
  {
    r[i].left = rectangles[i].x;
    r[i].top = rectangles[i].y;
    r[i].right = rectangles[i].x + rectangles[i].width;
    r[i].bottom = rectangles[i].y + rectangles[i].height;
  }

  ((X11Wnd*)d)->fillRects((X11Wnd::Context*)gc,r,nrectangles);

  delete[] r;
  return 0;
}

extern "C" int XFlush(Display* display)
{
  X11App::getApp().processMessages();
  return 0;
}

extern "C" Status XGetWindowAttributes(Display* display, Window w, XWindowAttributes* window_attributes_return)
{
  ::ZeroMemory(window_attributes_return,sizeof(XWindowAttributes));

  RECT inner, outer;
  ((X11Wnd*)w)->getClientRect(inner);
  ((X11Wnd*)w)->getWindowRect(outer);

  window_attributes_return->x = outer.left;
  window_attributes_return->y = outer.top;
  window_attributes_return->width = inner.right;
  window_attributes_return->height = inner.bottom;
  window_attributes_return->depth = X11App::getDisplayDepth();
  return 0;
}

extern "C" XFontStruct* XLoadQueryFont(Display* display, _Xconst char* name)
{
  // Check for a recognized font
  for (int i = 0; i < NUMBER_FONTS; i++)
  {
    if (stricmp(name,namedFonts[i].name) == 0)
    {
      HFONT hFont = X11Wnd::createFont(namedFonts[i].windowsNames,
        namedFonts[i].width,namedFonts[i].height,namedFonts[i].weight);
      if (hFont)
      {
        // Create and initialize a new font structure
        XFontStruct* font = new XFontStruct;
        ::ZeroMemory(font,sizeof(XFontStruct));
        font->fid = (Font)hFont;
        return font;
      }
    }
  }

  ::OutputDebugString("X11Lib: XLoadQueryFont() font not found\n");
  return NULL;
}

extern "C" int XLookupString(XKeyEvent*  event_struct, char* buffer_return, int bytes_buffer, KeySym* keysym_return, XComposeStatus* status_in_out)
{
  bool found = false;

  // Extract the keyboard symbol
  *keysym_return = event_struct->keycode & 0xFFFF;
  if (*keysym_return != 0)
    found = true;

  // Extract the ASCII value
  if (buffer_return > 0)
  {
    buffer_return[0] = (event_struct->keycode & 0xFF0000) >> 16;
    if (buffer_return[0] != 0)
      found = true;
  }

  return found ? True : False;
}

extern "C" int XLowerWindow(Display* display, Window w)
{
  ((X11Wnd*)w)->orderWindow(false);
  return 0;
}

extern "C" int XMapWindow(Display* display, Window w)
{
  ((X11Wnd*)w)->showWindow(true);
  return 0;
}

extern "C" int XMaskEvent(Display* display, long event_mask, XEvent* event_return)
{
  bool gotEvent = false;
  while (gotEvent == false)
  {
    // Make sure any outstanding Windows messages are handled
    X11App::getApp().processMessages();

    // Check for an event
    if (XCheckMaskEvent(display,event_mask,event_return))
      gotEvent = true;
    else
      ::WaitMessage();
  }
  return 0;
}

extern "C" int XMoveWindow(Display* display, Window w, int x, int y)
{
  ((X11Wnd*)w)->moveWindow(x,y);
  return 0;
}

extern "C" Display* XOpenDisplay(_Xconst char* display_name)
{
  // Set up the display with one screen
  ::ZeroMemory(&display,sizeof(Display));
  ::ZeroMemory(&screen,sizeof(Screen));

  display.nscreens = 1;
  display.screens = &screen;

  screen.display = &display;
  screen.width = ::GetSystemMetrics(SM_CXSCREEN);
  screen.height = ::GetSystemMetrics(SM_CYSCREEN);
  screen.white_pixel = RGB(0xFF,0xFF,0xFF);
  screen.black_pixel = RGB(0x00,0x00,0x00);

  return &display;
}

extern "C" int XParseGeometry(_Xconst char* parsestring, int* x_return, int* y_return, unsigned int* width_return, unsigned int* height_return)
{
  if (*parsestring == '=')
    parsestring++;

  if (sscanf(parsestring,"%dx%d",width_return,height_return) == 2)
    return WidthValue|HeightValue;
  if (sscanf(parsestring,"%dX%d",width_return,height_return) == 2)
    return WidthValue|HeightValue;
  return NoValue;
}

extern "C" int XRaiseWindow(Display* display, Window w)
{
  ((X11Wnd*)w)->orderWindow(true);
  return 0;
}

extern "C" int XResizeWindow(Display* display, Window w, unsigned int width, unsigned int height)
{
  ((X11Wnd*)w)->resizeWindow(width,height);
  return 0;
}

extern "C" int XSelectInput(Display* display, Window w, long event_mask)
{
  long mask =
    ExposureMask|KeyPressMask|StructureNotifyMask|ButtonPressMask|ButtonReleaseMask|
    PointerMotionMask|Button1MotionMask|Button2MotionMask|Button3MotionMask;
  if ((event_mask & ~(mask)) != 0)
    ::OutputDebugString("X11Lib: XSelectInput() event_mask includes unsupported values\n");

  ((X11Wnd*)w)->clearMessages();
  if (event_mask & KeyPressMask)
  {
    ((X11Wnd*)w)->addMessage(WM_KEYDOWN);

    // If there is no active window, make this window the active one
    if (X11Wnd::getActiveWindow() == NULL)
      X11Wnd::setActiveWindow((X11Wnd*)w);
  }
  if (event_mask & StructureNotifyMask)
    ((X11Wnd*)w)->addMessage(WM_SIZE);
  if (event_mask & (PointerMotionMask|Button1MotionMask|Button2MotionMask|Button3MotionMask))
    ((X11Wnd*)w)->addMessage(WM_MOUSEMOVE);
  if (event_mask & ButtonPressMask)
  {
    ((X11Wnd*)w)->addMessage(WM_LBUTTONDOWN);
    ((X11Wnd*)w)->addMessage(WM_RBUTTONDOWN);
    ((X11Wnd*)w)->addMessage(WM_MBUTTONDOWN);
  }
  if (event_mask & ButtonReleaseMask)
  {
    ((X11Wnd*)w)->addMessage(WM_LBUTTONUP);
    ((X11Wnd*)w)->addMessage(WM_RBUTTONUP);
    ((X11Wnd*)w)->addMessage(WM_MBUTTONUP);
  }
  return 0;
}

extern "C" int XSetFont(Display* display, GC gc, Font font)
{
  ((X11Wnd::Context*)gc)->setFont((HFONT)font);
  return 0;
}

extern "C" void XSetWMProperties(Display* display, Window w, XTextProperty* window_name, XTextProperty* icon_name, char** argv, int argc, XSizeHints* normal_hints, XWMHints* wm_hints, XClassHint* class_hints)
{
  ((X11Wnd*)w)->setWindowTitle((char*)(window_name->value));
}

extern "C" Status XStringListToTextProperty(char** list, int count, XTextProperty* text_prop_return)
{
  // Initialize the structure
  text_prop_return->encoding = XA_STRING;
  text_prop_return->format = 8;
  text_prop_return->nitems = count;

  // Work out how long the value should be
  int len = 0;
  for (int i = 0; i < count; i++)
    len += strlen(list[i]) + 1;

  // Add space for trailing '\0'
  len++;

  // Allocate a buffer
  text_prop_return->value = new unsigned char[len];
  text_prop_return->value[0] ='\0';

  // Add the strings to buffer
  len = 0;
  for (i = 0; i < count; i++)
  {
    strcpy((char*)(text_prop_return->value + len),list[i]);
    len += strlen(list[i]) + 1;
  }

  // Add the trailing '\0'
  text_prop_return->value[len] = '\0';

  return True;
}
