/*
 * X11Lib Win32 X11 library WinMain entry point
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>

void Win32X11Init(HINSTANCE hInstance);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int cmdShow)
{
  /* Call the X11 DLL's initialization routine */
  Win32X11Init(hInstance);

  /* Call the usual entry point */
  return main(__argc,__argv);
}
