/*
 * XLife Copyright 1989 Jon Bennett jb7m+@andrew.cmu.edu, jcrb@cs.cmu.edu
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "defs.h"
#include "macros.h"

void getxstring()
{
    XComposeStatus status;
    int offset=0, buflen, windowlen;
    char tmpinpbuf[INPBUFLEN];
    state = STOP;
    for(;;){
       XMaskEvent(disp, KeyPressMask | ButtonPressMask | Button1MotionMask 
                      | PointerMotionMask | Button3MotionMask | ExposureMask 
                      | StructureNotifyMask,&event);
       /* handle other kinds of events if no key is pressed  */ 
       strcpy(tmpinpbuf,inpbuf);
       switch(event.type) {
          case MotionNotify:
            Motion();
	    break;
	  case ButtonPress:
	    Button();
	    break;
	  case ConfigureNotify:
	    DoResize();
	    break;
	  case Expose:
	    DoExpose(ClassifyWin(event.xexpose.window));
	    default:
	    break;
	} 
    strcpy(inpbuf,tmpinpbuf);

    if (dispcoord) displaycoords();

	if (event.type != KeyPress){
           /* non KeyPress events might write on inputw */
           XClearWindow(disp, inputw);
	   XDrawString(disp, inputw, ntextgc, ICOORDS(0,0), 
                    inpbuf + offset, strlen(inpbuf));
        }
        else {
	      if (!XLookupString(&event.xkey, keybuf, KEYBUFLEN, &ks, &status))
			continue;  /* Bug in Ultrix 4.3 at least, depends on the
			length of the last XDrawString % 4 before the XMaskEvent :(  
			XLookupString starts after keymodifier! No idea to reset keyboard */
	    
	    if(IsModifierKey(ks)){
		    continue;
	    }
	    /* compute window length for rescrolling */
            windowlen=(width-dispcoord*(COORDW+BORDERWIDTH)-BORDERWIDTH*2)
                       /FONTWIDTH;
	    switch(ClassifyWin(event.xkey.window)) {
	      case INPUTWIN:
	      case LIFEWIN:
		if((ks != XK_Return) && (ks != XK_Linefeed)) {
		    if((ks == XK_BackSpace) || (ks == XK_Delete)) {
			buflen = strlen(inpbuf);
			if(buflen>minbuflen) {
			    inpbuf[buflen - 1] = 0;
			    XClearWindow(disp, inputw);
			    offset = (buflen > windowlen) ? buflen - windowlen : 0;
			    XDrawString(disp, inputw, ntextgc, ICOORDS(0,0), inpbuf + offset, buflen);
			}
		    }
		    else {
			if(ks == '~'){
			    inpbuf[minbuflen] = 0;
			    XClearWindow(disp,inputw);
			}
			strcat(inpbuf, keybuf);
			buflen = strlen(inpbuf);
			if (buflen > INPBUFLEN) inpbuf[INPBUFLEN-1] = 0;
			offset = (buflen > windowlen) ? buflen - windowlen : 0;
			if (offset) XClearWindow(disp, inputw);
			XDrawString(disp, inputw, ntextgc, ICOORDS(0,0), inpbuf + offset, buflen);
		    }
		}
		else {
		    XClearWindow(disp, inputw);
		    return;
		}
	    }
	}
	}
}
