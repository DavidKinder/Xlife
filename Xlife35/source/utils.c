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

void boxoutline(UNS32 x1, UNS32 y1, UNS32 x2, UNS32 y2)
{
unsigned long t;

   
   if (x1> x2) {t= x1; x1=x2; x2=t;}
   if (y1> y2) {t= y1; y1=y2; y2=t;}

   x2++;  
   y2++;

   x1= RXPOS(x1,xpos);
   x2= RXPOS(x2,xpos);
   y1= RYPOS(y1,ypos);
   y2= RYPOS(y2,ypos);

   XDrawLine(disp,lifew,xorgc,x1,y1,x1,y2);
   XDrawLine(disp,lifew,xorgc,x2,y1,x2,y2);
   XDrawLine(disp,lifew,xorgc,x1+1,y1,x2-1,y1);
   XDrawLine(disp,lifew,xorgc,x1+1,y2,x2-1,y2);

}

void displaycoords() {
   unsigned long x,y;
   char pstring[50];

   x=XPOS(event.xmotion.x,xpos)-xorigin; 
   y=YPOS(event.xmotion.y,ypos)-yorigin; 
   if (lastx!=x || lasty!=y) {
      lastx=x;
      lasty=y; 
      sprintf(pstring,"(%ld,%ld) S=%ld",x,y,scale);
      XClearWindow(disp,coordw);
      XDrawString(disp,coordw,ntextgc,CWCOORDS(0,0), pstring,strlen(pstring));
   }
}

int getcoords(UNS32 *x, UNS32 *y, UNS32 ox, UNS32 oy, int dobox)
{
   *x=ox;
   *y=oy;
   for (;;) {
      if (dobox) boxoutline(*x,*y,ox,oy);
      XMaskEvent(disp, ButtonPressMask | Button1MotionMask | Button3MotionMask |
                       PointerMotionMask | KeyPressMask ,
                 &event); 
      if (event.type == KeyPress) {
		 XComposeStatus status;
         if (XLookupString(&event.xkey, keybuf, KEYBUFLEN, &ks, &status)) {
			if (ks == XK_Escape) {
			   keybuf[0]='\0';
	           return  1;
            }
         }
		 keybuf[0]='\0';
      }
      if (dobox) boxoutline(*x,*y,ox,oy);

      *x=XPOS(event.xmotion.x,xpos);
      *y=YPOS(event.xmotion.y,ypos);

      if (event.type==ButtonPress) break;
      if (dispcoord) displaycoords();

   }
   return  0;

}


void getbounds(UNS32 *lx, UNS32 *ly, UNS32 *ux, UNS32 *uy)
{
unsigned long t;
static char msg1[]="Click on first box corner   or  hit Escape to abort";
static char msg2[]="Click on second box corner   or  hit Escape to abort";
static char msg3[]="Please wait ...";

   XClearWindow(disp, inputw);
   XDrawString(disp,inputw, ntextgc, ICOORDS(0,0), msg1, strlen(msg1)); 
   if (getcoords(lx,ly,0,0,0)){
      XClearWindow(disp, inputw);
	  *ux= *lx;  *uy= *ly;
	  return;
   }
   XClearWindow(disp, inputw);
   XDrawString(disp,inputw, ntextgc, ICOORDS(0,0), msg2, strlen(msg2)); 
   if (getcoords(ux,uy,*lx,*ly,1)){
      XClearWindow(disp, inputw);
	  *ux= *lx;  *uy= *ly;
	  return;
   }

   if (*lx> *ux) {t= *lx; *lx= *ux; *ux=t;}
   if (*ly> *uy) {t= *ly; *ly= *uy; *uy=t;}
   if ((*ux - *lx) * (*uy - *ly) > 10000){
      XClearWindow(disp, inputw);
      XDrawString(disp,inputw, ntextgc, ICOORDS(0,0), msg3, strlen(msg3)); 
	  XFlush(disp);
   } 
}

void fatal(char *s)
{
	fprintf(stderr, s);
	exit(-22);
}

int ClassifyWin(Window win)
{
    if(win == lifew)
	return LIFEWIN;
    if(win == inputw)
	return INPUTWIN;
    if(win == mainw)
	return MAINWIN;
    if(win == coordw)
	return COORDWIN;
	return  -1;
}
/*
char *itoa(int n)
{
    static char buf[16];
    char sign;
    char *ptr;

    if (n >= 0)
	sign = ' ';
    else {
	n = -n;
	sign = '-';
    }
    buf[15] = 0;
    ptr = buf + 14;
    do {
	*--ptr = n % 10 + '0';
	if (ptr <= buf)
	    return(buf);
	n /= 10;
    } while (n > 0);
    *--ptr = sign;
    return(ptr);
}
*/
void drawpoint(int x, int y, int c)
{
    if(c){
	XDrawPoint(disp,lifew,whitegc,x,y);
    }
    else{
	XDrawPoint(disp,lifew,blackgc,x,y);
    }
}

void drawbox(int x, int y, int c)
{
    int sc;
    sc = (1 << scale) - (scale > 1);
    if(c){
	XFillRectangle(disp,lifew,whitegc,x,y,sc,sc);
    }
    else{
	XFillRectangle(disp,lifew,blackgc,x,y,sc,sc);
    }
}

void drawmesh() {
int x,y,sc;
    if (scale>1) {
       sc = 1 << scale ;
       for (x=0; x< width; x+=sc) 
         XDrawLine(disp, lifew, whitegc, x+sc-1,0, x+sc-1, height);
       for (y=0; y< height; y+=sc) 
         XDrawLine(disp, lifew, whitegc, 0, y+sc-1, width, y+sc-1);
    }
    if (scale>2) {
       sc = 5*(1 << scale);
       for (x=0; x< width; x+=sc) 
         XDrawLine(disp, lifew, whitegc, x+sc-2,0, x+sc-2, height);
       for (y=0; y< height; y+=sc) 
         XDrawLine(disp, lifew, whitegc, 0, y+sc-2, width, y+sc-2);
    }
}

void drawpivot(int x, int y)
{
int sc;
    if (scale>1) {
       sc = (1 << scale) - 1;
       XDrawLine(disp,lifew,xorgc,x,y,x+sc-1,y+sc-1);
       XDrawLine(disp,lifew,xorgc,x,y+sc-1,x+sc-1,y);
       XDrawPoint(disp,lifew,xorgc,x+sc/2,y+sc/2);
    }
    else if (scale==0)
	   XDrawPoint(disp,lifew,whitegc,x,y);
}

#define  B31  0x80000000
static UNS32  p_2 = 656623456UL, p_1 = 747247571UL;
/* unsigned long  must be 32 bits */
UNS32 random32()
{ /* Randomgenerator nach R. C. Tausworthe           */
  /* f(x) = 1 + x^52 + x^63 irreduzibel ueber GF(2)  */
  /* Certificat: Angewandte Informatik 9/83 p404-409 */
  register UNS32  p1=p_1, p2=p_2, b31=B31;
  /* shift um 63 bit  */
  p2 ^= (p2<<11) ^ (p1>>21);
  p1 ^= p1<<11;
  p2 <<= 1;
  p1 ^= p2 >> 21;
  /* shift um ein bit */
  p2 |= !!(p1&b31);
  p_2 = p2;
  p1 <<= 1;
  p2 ^= p2<<11;
  p1 |= !!(p2&b31);
  p_1 = p1;
  return p1;   /*** p2 contains 31 bits, 31. bit undefined ***/
}

void randomset(UNS32 x2, UNS32 x1)
{
  if (!(x2&~B31 | x1)) x1 = 1;
  p_1 = x1; p_2 = x2;
}

void randomize(UNS32 lx, UNS32 ly, UNS32 dx, UNS32 dy)
{
    unsigned long num;
    
    for(num = (dx*dy) >> (7-scale) ;num;num--){
	  addcell( random32()%dx+lx, random32()%dy+ly);
    }
    redisplay();
}

void randomseed()
{  unsigned long  x1, x2;
   getinputwstring("Number for randomseed: ");
   sscanf(inpbuf+1,"%lu",&x1);
   x2=0;
   sscanf(inpbuf+10,"%lu",&x2);
   randomset(x2,x1);
}


void settimeout(UNS32 ms)
{
    timeout.tv_sec=ms/1000;
    timeout.tv_usec=(ms%1000)*1000;
}

void benchmark()
{
    u_long num,count;
    double tm;
    struct timeval start,end;
    struct timezone
#ifdef  SYSV
{ int tz_minuteswest; int tz_dsttime; }   /* hopefully correct */
#endif
	                 tmz;
    static char msg0[]="Running ...";

    state=STOP;

    strcpy(inpbuf,"Number of generations:");
    minbuflen=strlen(inpbuf);
 
    XClearWindow(disp,inputw);
    XDrawString(disp, inputw, ntextgc,ICOORDS(0,0),inpbuf, minbuflen);

    getxstring();
    sscanf(inpbuf+minbuflen,"%ld",&count);
    inpbuf[0]=0;
    XClearWindow(disp, inputw);
    XDrawString(disp,inputw, ntextgc, ICOORDS(0,0), msg0, strlen(msg0)); 
	XFlush(disp);
    
    gettimeofday(&start,&tmz);
    for(num=0;num<count;num++){
	generate();
    }
    gettimeofday(&end,&tmz);
    tm=(((end.tv_sec * 1000000) + end.tv_usec) - ((start.tv_sec * 1000000) + start.tv_usec))/1000000.0;
    XClearWindow(disp,lifew);
    redrawscreen();

    sprintf(inpbuf," %s: %f sec    Hit Return to continue","Time",tm);
    minbuflen= strlen(inpbuf);

    XClearWindow(disp,inputw);
    XDrawString(disp, inputw, ntextgc,ICOORDS(0,0),inpbuf, strlen(inpbuf));
    getxstring();
    inpbuf[0]=0;

}

void getinputwstring(char *promptstr)
{
	int  i;
    strcpy(inpbuf,promptstr);
    minbuflen=strlen(promptstr);
 
    XClearWindow(disp,inputw);
    XDrawString(disp, inputw, ntextgc,ICOORDS(0,0),inpbuf, minbuflen);

    getxstring();
	if (minbuflen>1)
	   for (i=0;inpbuf[i+1]=inpbuf[minbuflen+i];i++);
    else if (!minbuflen)
	   for (i=strlen(inpbuf)+1;i;i--)
		  inpbuf[i]=inpbuf[i-1];
    inpbuf[0]=0;
}
