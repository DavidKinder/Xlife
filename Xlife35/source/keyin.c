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
extern char  *getenv();

int DoKeySymIn(KeySym keysym)
{
     switch (keysym) {
	case XK_4:
	case XK_KP_4:
	case XK_Left:
	  XClearWindow(disp,lifew);
	  xpos-= SCALE(width/4);
	  redrawscreen();
	  break;
	case XK_6:
	case XK_KP_6:
	case XK_Right:
	  XClearWindow(disp,lifew);
	  xpos+= SCALE(width/4);
	  redrawscreen();
	  break;
	case XK_8:
	case XK_KP_8:
	case XK_Up:
	  XClearWindow(disp,lifew);
	  ypos-= SCALE(height/4);
	  redrawscreen();
	  break;
	case XK_2:
	case XK_KP_2:
	case XK_Down:
	  XClearWindow(disp,lifew);
	  ypos+= SCALE(height/4);
	  redrawscreen();
	  break;
	case XK_7:
	case XK_KP_7:
	  XClearWindow(disp,lifew);
	  xpos-= SCALE(width/4);
	  ypos-= SCALE(height/4);
	  redrawscreen();
	  break;
	case XK_9:
	case XK_KP_9:
	  XClearWindow(disp,lifew);
	  xpos+= SCALE(width/4);
	  ypos-= SCALE(height/4);
	  redrawscreen();
	  break;
	case XK_3:
	case XK_KP_3:
	  XClearWindow(disp,lifew);
	  xpos+= SCALE(width/4);
	  ypos+= SCALE(height/4);
	  redrawscreen();
	  break;
	case XK_1:
	case XK_KP_1:
	  XClearWindow(disp,lifew);
	  xpos-= SCALE(width/4);
	  ypos+= SCALE(height/4);
	  redrawscreen();
	  break;
	case XK_5:
	case XK_KP_5:
	  center();
	  break;
	case XK_0:
	case XK_KP_0:
	  median();
	  break;
	case XK_Help:
	  help();
	  break;
	case XK_Return:
	  if (dispcoord) lastx--; /* fake move to force coordinates to print */
	  redrawscreen();
	  break;
	case XK_Tab:
      confirmload();
	  generate();
	  XClearWindow(disp,lifew);
	  redrawscreen();
	  break;
	default:
	  return 0;
     }

     /* could process it */
     return 1;
}

void DoKeyIn(char kbuf[KEYBUFLEN])
{
char pstring[50];
static unsigned long lx,ly,ux,uy;

    switch(kbuf[0]){
    case 'L':
	  showmesh=(showmesh)?0:1;
	  XClearWindow(disp,lifew);
	  redrawscreen();
      break;
    case 'b':
      {
	  char  outbuf[60];
      FILE *boxfile;
      confirmload();
	  XClearWindow(disp,lifew);
	  redrawscreen();

      getbounds(&lx,&ly,&ux,&uy);  
	  if (lx!=ux || ly!=uy) {
        if (ux-lx>=BUFSIZ-1) ux=lx+BUFSIZ-2;
        strcpy(outbuf,SCRATCHFILE);
        strcat(outbuf,".life");
        if (boxfile=fopen(outbuf,"w")) {
          saveinbounds(boxfile,'B',lx,ly,ux,uy);
          fclose(boxfile);
        }
      }
	  }
	  XClearWindow(disp,lifew);
	  redrawscreen();
      break; 
    case 'T':
      {
      confirmload();
	  XClearWindow(disp,lifew);
	  redrawscreen();
      getbounds(&lx,&ly,&ux,&uy);  
	  if (lx!=ux || ly!=uy) {
        if (ux-lx>=BUFSIZ-1) ux=lx+BUFSIZ-2;
		savefileinbounds(lx,ly,ux,uy);
      }
	  }
	  XClearWindow(disp,lifew);
	  redrawscreen();
      break; 
    case 'B':
      loadfile(SCRATCHFILE); 
      break;  
    case 'd':
      {
      unsigned long  x,y;

      confirmload();
      getbounds(&lx,&ly,&ux,&uy);
	  if (lx!=ux || ly!=uy) {
        for (x=lx; x<=ux; x++)  
          for (y=ly; y<=uy; y++) {  
           deletecell(x,y);
          }
        state=STOP;
	  }
      }
	  XClearWindow(disp,lifew);
	  redrawscreen();
      break;
    case 'M':
      {
      unsigned long  x,y;
	  char  outbuf[60];
      FILE *boxfile;
      confirmload();
      XClearWindow(disp,lifew);
      redrawscreen();

      getbounds(&lx,&ly,&ux,&uy);  
	  if (lx!=ux || ly!=uy) {
        if (ux-lx>=BUFSIZ-1) ux=lx+BUFSIZ-2;
        strcpy(outbuf,SCRATCHFILE);
        strcat(outbuf,".life");
        if (boxfile=fopen(outbuf,"w")) {
          saveinbounds(boxfile,'b',lx,ly,ux,uy);
          fclose(boxfile);
          for (x=lx; x<=ux; x++)  
            for (y=ly; y<=uy; y++) {  
             deletecell(x,y);
            }
          }
        XClearWindow(disp,lifew);
        redrawscreen();
        if (boxfile)
            loadfileat(SCRATCHFILE, 1,
                       (unsigned long)((ux+1+lx)/2),
                       (unsigned long)((uy+1+ly)/2));

        state=STOP;
      }
	  else {
        XClearWindow(disp,lifew);
        redrawscreen();
      }
	  }
      break; 
	case 'r':
	case '\015':
	  if (dispcoord) lastx--; /* fake move to force coordinates to print */
	  redrawscreen();
	break;
	case 'R':
	  newrules();
	  break;
	case '=':
	case '+':
	  if(scale < 7){
	      XClearWindow(disp,lifew);
	      scale++;
	      xpos += XPOS(event.xmotion.x, 0);
	      ypos += YPOS(event.xmotion.y, 0);
		  if (dispcoord) lastx--; /* fake move to force coordinates to print */
	      redrawscreen();
	  }
 	  break;
    case '.':
      XClearWindow(disp,lifew);
      xpos += SCALE(event.xbutton.x - width/2);
      ypos += SCALE(event.xbutton.y - height/2);
      redrawscreen();
      break;
	case '-':
	  if(scale > -3){
	      XClearWindow(disp,lifew);
	      xpos -= XPOS(event.xmotion.x, 0);
	      ypos -= YPOS(event.xmotion.y, 0);
	      scale--;
		  if (dispcoord) lastx--; /* fake move to force coordinates to print */
	      redrawscreen();
	  }
	  break;
	case 'g':
      confirmload();
	  state=(state==RUN)?STOP:RUN;
	  if (state==RUN)  stopcounter=runcounter;
	  XClearWindow(disp,lifew);
	  redrawscreen();
	  break;
	case 'c':
	  dispcells ^=1;
	  break;
	case '$':
	  dispchang ^=1;
	  break;
	case 'o':
      confirmload();
	  generate();
	  XClearWindow(disp,lifew);
	  redrawscreen();
	  break;
	case '\011':
      confirmload();
	  generate();
	  redisplay();
	  break;
	case 'p':
      if (dispcoord) {
          dispcoord=0;
      } else {
          lastx--; /* fake move to force coordinates to print */
          dispcoord=1;
      }
      /* force resize of input window */
      XResizeWindow(disp,inputw,
                    width-dispcoord*(COORDW+BORDERWIDTH)-BORDERWIDTH*2,
                    INPUTH);
	  break;
	case 'O':
      strcpy(inpbuf,"Origin set to current cell");
	  XClearWindow(disp,inputw);
      XDrawString(disp,inputw,ntextgc,ICOORDS(0,0),
                  inpbuf,strlen(inpbuf));
      xorigin=XPOS(event.xmotion.x,xpos);
      yorigin=YPOS(event.xmotion.y,ypos);
	  break;
	case '(':
	case ')':
	case '{':
	case '}':
	  {  unsigned  i;
	  for (i=0;kbuf[0]!="(){}"[i];i++);
      strcpy(inpbuf,"Marker set to current cell");
	  XClearWindow(disp,inputw);
      XDrawString(disp,inputw,ntextgc,ICOORDS(0,0),
                  inpbuf,strlen(inpbuf));
      xmarker[i]=XPOS(event.xmotion.x,xpos);
      ymarker[i]=YPOS(event.xmotion.y,ypos);
	  }
	  break;
	case '<':
	case '>':
	case '[':
	case ']':
	  {  unsigned  i;
	  for (i=0;kbuf[0]!="<>[]"[i];i++);
	  XClearWindow(disp,lifew);
	  xpos += xmarker[i] - XPOS(event.xmotion.x,xpos);
	  ypos += ymarker[i] - YPOS(event.xmotion.y,ypos);
	  redrawscreen();
	  }
	  break;
    case 'J':
	  {
	  long  x, y;
	  getinputwstring("Jump to x,y : ");
      if (2==sscanf(inpbuf+1,"%ld,%ld",&x,&y)) {
	    XClearWindow(disp,lifew);
	    xpos += x + xorigin - XPOS(event.xmotion.x,xpos);
	    ypos += y + yorigin - YPOS(event.xmotion.y,ypos);
	    redrawscreen();
      }
      }
	  break;
    case 'a':
	  {
	  getboundingbox(&lx,&ux,&ly,&uy);
	  if (!ux && !uy)
        sprintf(inpbuf,"   Life is extinct");
	  else
        sprintf(inpbuf,"   Life is inside   %ld <= x <= %ld  %ld <= y <= %ld",
	            lx-xorigin,ux-xorigin,ly-yorigin,uy-yorigin);
	  XClearWindow(disp,inputw);
      XDrawString(disp,inputw,ntextgc,ICOORDS(0,0),
                  inpbuf,strlen(inpbuf));
	  }
	  break;
	case 'C':
	  clear();
	  break;
	case 'S':
	  savefile();
	  break;
	case 'W':
	  saveloadscript();
	  free_loadscript();
	  break;
	case 'D':
	  free_loadscript();
          if (tentative) {
             undoload();
	     XClearWindow(disp,lifew);
	     redrawscreen();
             strcpy(inpbuf,"Load script (and latest load) discarded");
          } else strcpy(inpbuf,"Load script discarded"); 
	  XClearWindow(disp,inputw);
          XDrawString(disp,inputw,ntextgc,ICOORDS(0,0),
                      inpbuf,strlen(inpbuf));
	  break;
	case 'l':
	  loadfile("");
	  break;
	case 'h':
      confirmload();
	  if(state==HIDE){
	      state=STOP;
	      XClearWindow(disp,lifew);
	      redrawscreen();
	  }
	  else{
	      state=HIDE;
		  stopcounter=runcounter;
	  }
	  break;
	case '?':
	  help();
	  break;
	case 'F':
	  listlifefiles();
	  break;
	case 'f':
	  settimeout(DELAY_FAST);
	  break;
	case 'm':
	  settimeout(DELAY_MED);
	  break;
	case 's':
	  settimeout(DELAY_SLOW);
	  break;
	case '!':
	  {  int i;
	  getinputwstring("!");
	  for (i=strlen(inpbuf);i>1;i--)
		 if (inpbuf[i-1]>' ')  break;
      inpbuf[i]='\0';
	  for (i=1;inpbuf[i];i++)
		 if (inpbuf[i]>' ')  break;
	  if (!strcmp(inpbuf+i,"cd"))
		 chdir( getenv("HOME") );
	  else if (!strncmp(inpbuf+i,"cd ",3))
		 chdir(inpbuf+i+3);
      else
	     shellcommand("exec 2>&1 <&- ; ",inpbuf+i,"&","");
	  break;
	  }
	case '%':
	  randomize(xpos,ypos,SCALE(width-BORDERWIDTH*2),SCALE(height-INPUTH-BORDERWIDTH*3));
	  break;
	case '^':
	  randomseed();
	  break;
    case '*':
	  confirmload();
	  getbounds(&lx,&ly,&ux,&uy);
	  if (lx!=ux || ly!=uy)
	     randomize(lx,ly,ux-lx,uy-ly);
      XClearWindow(disp,lifew);
      redrawscreen();
	  break;
	case 'N':
	  name_file();
	  break;
	case 'A':
	  comment();
	  break;
	case 'V':
	  view_comments();
	  break;
	case '@':
	  benchmark();
	  break;
    case 'n':
	  runcounter=0;
	  getinputwstring("Number of generations to perform (default= infinite): ");
      sscanf(inpbuf+1,"%lu",&runcounter);
	break;
#ifdef  BOUNDED
    case '#':
      {  unsigned  i; 
	  i= 8*sizeof(int);
	  getinputwstring("Logarithmn of diameter of universe (default= infinite): ");
      sscanf(inpbuf+1,"%u",&i);
	  universe= (i>=8*sizeof(int)? ~0 : (1<<i)-1);
	  }
	break;
#endif
	case '~':
	  if (gcmode==2)  gcmode=0;
	  else  gcmode++;
	  if (gcmode==1){
	     XClearWindow(disp,lifew);
	     redrawscreen();
      }
	  break;
    case 't':
	  numcellsperiod=0;
	  getinputwstring("Period of number of cells to check (default= infinite): ");
      sscanf(inpbuf+1,"%lu",&numcellsperiod);
	  if (numcellsperiod > MAXPERIOD)
		numcellsperiod = 0;
	  if (numcellsperiod) {
	    for (lastindex=numcellsperiod;lastindex;lastindex--)
		   numbercells[lastindex-1] = 0;
	    matchedcounter=0;
      }
	  break;
    case 'i':
	  imageformat='\0';
	  getinputwstring("Imageformat for saving patterns: A R D P (default= automatic): ");
	  if (strchr("ARDP",inpbuf[1]))
	     imageformat=inpbuf[1];
      break;
	case 'Q':
	  exit(0);
	  break;
    case 'U':
      /* Get rid of loaded pattern */
	  if (!tentative)  break;
      undoload();
	  XClearWindow(disp,lifew);
	  redrawscreen();
	  break;
    case 'u':
      /* Get rid of loaded pattern */
	  if (!tentative)  break;
      undoload();
      loadfileat(SCRATCHFILE, 1,
                 (unsigned long)((ux+1+lx)/2),
                 (unsigned long)((uy+1+ly)/2));
	  XClearWindow(disp,lifew);
	  redrawscreen();
	  break;
    case 'I':
      /* Force confirm of loaded pattern */
      confirmload();
	  XClearWindow(disp,lifew);
	  redrawscreen();
	  break;
    case 'G':
      /* perform some generations on loaded pattern */ 
      genload();
	  XClearWindow(disp,lifew);
	  redrawscreen();
	  break;
    case '\0':  /* shift */
	  break;
	default:
      strcpy(inpbuf,"unknown key: ");
      strcat(inpbuf,kbuf);
	  if (kbuf[0] < 32 || kbuf[0]>126)
		 sprintf(inpbuf+13,"0x%2x",kbuf[0]);
      XClearWindow(disp,inputw);
      XDrawString(disp,inputw,ntextgc,ICOORDS(0,0),
                  inpbuf,strlen(inpbuf));
	  break;
    }
    kbuf[0]='\0'; /* get rid of old keystroke so shift doesn't bring it back */
}
