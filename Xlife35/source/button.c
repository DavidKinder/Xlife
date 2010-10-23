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
#include "cellbox.h"

extern cellbox *tentative;

void moveload() {
    /* Move loaded pattern to current cell */
    loadx=XPOS(event.xmotion.x,xpos);
    loady=YPOS(event.xmotion.y,ypos);
    XClearWindow(disp,lifew);
    redrawscreen();
}

void turnload() {
int t; 
    /* Turn loaded pattern 90 degrees about its origin */
    t= -tyx; tyx=txx; txx=t;
    t= -tyy; tyy=txy; txy=t;
    XClearWindow(disp,lifew);
    redrawscreen();
} 

void flipload() {
int t; 
    /* Flip pattern about its x axis */
    tyx= -tyx;
    tyy= -tyy;
    XClearWindow(disp,lifew);
    redrawscreen();
} 


void Button()
{
    if(ClassifyWin(event.xbutton.window) == LIFEWIN){
	switch(event.xbutton.button){
	  case 1:
            if (tentative) moveload();
	    else {
               if(scale <= 0){
		drawpoint(event.xbutton.x,event.xbutton.y,1);
   	    }
	       else{
		drawbox(event.xbutton.x & (0xffffffff << scale),
				event.xbutton.y & (0xffffffff << scale),1);
         	    }
	       addcell(XPOS(event.xbutton.x,xpos),YPOS(event.xbutton.y,ypos));
	       numcells++;
            }
	    break;
	  case 2:
            if (tentative) flipload();
            else {
	       XClearWindow(disp,lifew);
	       xpos += SCALE(event.xbutton.x - width/2);
	       ypos += SCALE(event.xbutton.y - height/2);
	       redrawscreen();
            }
	    break;
	  case 3:
            if (tentative) turnload();
	    else {
               if(scale <= 0){
		drawpoint(event.xbutton.x,event.xbutton.y,0);
	    }
               else{
		drawbox(event.xbutton.x & (0xffffffff << scale),
				event.xbutton.y & (0xffffffff << scale),0);
	       }
            
	       deletecell(XPOS(event.xbutton.x,xpos),YPOS(event.xbutton.y,ypos)); 
	       numcells--;
            }
	}
    }
    XFlush(disp);
}
