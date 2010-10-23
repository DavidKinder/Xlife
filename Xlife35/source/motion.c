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

void Motion()
{
    if(ClassifyWin(event.xmotion.window) == LIFEWIN){

	if(event.xmotion.state & Button1MotionMask){
	    if(scale <= 0) drawpoint(event.xmotion.x,event.xmotion.y,1);
	    else drawbox(event.xmotion.x & (0xffffffff << scale),
					 event.xmotion.y & (0xffffffff << scale),1);
       	    addcell(XPOS(event.xmotion.x,xpos),YPOS(event.xmotion.y,ypos));
	}
	else{
	    if(event.xmotion.state & Button3MotionMask){
		if(scale <= 0) drawpoint(event.xmotion.x,event.xmotion.y,0);
		else drawbox(event.xmotion.x & (0xffffffff << scale),
					 event.xmotion.y & (0xffffffff << scale),0);
		deletecell(XPOS(event.xmotion.x,xpos),YPOS(event.xmotion.y,ypos)); 
	    }
	}
    }	   
    XFlush(disp);
}
