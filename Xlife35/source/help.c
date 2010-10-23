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
#include "data.h"
    
static char *strs[] = { "  X L i f e   H e l p     V   "versionid,
			"",
			"Move up            : 8, Up Arrow",
			"Move down          : 2, Down Arrow",
			"Move right         : 6, Right Arrow",
			"Move left          : 4, Left Arrow",
			"Move ul            : 7",
			"Move ur            : 9",
			"Move dl            : 1",
			"Move dr            : 3",
			"Center average     : 5",
			"Center median      : 0",
			"Center on cursor   : Button 2 ('.' in Load Mode)",
			"Zoom in            : =,+",
			"Zoom out           : -",
			"",
			"Generate on/off    : g",
			"One generation     : o, Tab",
			"No of generations  : n",
			"Trace period       : t",
			"Run benchmark      : @",
			"Hide               : h",
			"",
			"Cell count on/off  : c",
			"Change count on/off: $",
			"Position on/off    : p",
			"Grid on/off        : L",
			"Toggle representati: ~",
			"New rules          : R",
			"Shell command      : !",
			"",
			" File Commands",
			"Load               : l",
			"Save (raw pattern) : S",
			"List lifefiles     : F",
			"Change patternname : N",
			"Add/View Comments  : A/V",
			"Select imageformat : i",
			" Load Script Commands",
			"Write to file      : W",
	        "Discard            : D", 
			"",
			"Clear              : C",
			"Draw               : Button 1 (Not in Load Mode)",
			"Delete             : Button 3 (Not in Load Mode)",
			"Randomize          : %",
			"Seed random        : ^",
	        "Reset origin       : O",
			"Set marker         : ( ) { }",
			"Jump marker        : < > [ ]",
			"Jump to  (x,y)     : J",
			"Show x/y-min/max   : a",
			"",
			" Copy and Paste Patterns",
			"Select pattern box : b",
			"Delete pattern     : d",
			"Copy back pattern  : B",
			"Move pattern       : M",
			"Save pattern       : T",
			"Random pattern     : *",
			"Abort operation    : Escape",
			"",
			" Manipulating Loaded Patterns (Load Mode)",
			"Move, Flip, Rotate : Buttons 1,2,3",
			"Generate           : G",
			"Undo & Clear       : U",
			"Undo manipulation  : u",
			"Incorporate        : I",
			"",
			" Speed Control",
			"Fast, Medium, Slow : f,m,s",
			"",
			"Redraw screen      : r, Return",
			"Help               : ?, Help Key",
			"Quit               : Q",    
			"",
			0 };

void help() {
    
    int  h = 0 , i = 2, j = 10, k;
    
    XClearWindow(disp,lifew);
    while (strs[h]) {
	for (k=0;strs[h][k];k++)
	   if (strs[h][k]==':')  break;
	XDrawString(disp,lifew,(strs[h][k]==':'?ntextgc:btextgc),j,i*FONTHEIGHT+1,
				strs[h],strlen(strs[h]));
	i++;  h++;
	if (FONTHEIGHT*(i+1) > height-INPUTH-BORDERWIDTH*2)
	   j += FONTWIDTH*50,  i=2;
    }
    strcpy(inpbuf,"Hit   R e t u r n   to continue                  \
Copyright 1989  Jon Bennett");
	minbuflen=strlen(inpbuf);
	XClearWindow(disp,inputw);
    XDrawString(disp, inputw,
 			    ntextgc,ICOORDS(0,0),inpbuf, strlen(inpbuf));
	exposeflag=0;
    getxstring();
    inpbuf[0]=0;
	exposeflag=1;
    XClearWindow(disp,lifew);
    redrawscreen();
	
}


void shellcommand(char *prestr, char *userstr, char *denystr, char *poststr)
{	
#define  MAXLINELEN  255
	char line[MAXLINELEN];
	int h= 0, i ,  j = 2*FONTWIDTH;
    FILE  *fp, *popen();

	strcpy(line,prestr);
	strcat(line,userstr);
    for (h=0,i=strlen(prestr);line[i];i++)
	{  for (j=strlen(denystr);j;j--)
	      if (denystr[j-1]==line[i])  break;
	   if (j)  h++;
	   else
          line[i-h]=line[i];
    }
	line[i-h]=line[i];
	strcat(line,poststr);

    if (!(fp = popen(line,"r")))  return;

    exposeflag=0;
    XClearWindow(disp,lifew);
	i = 2;
	while  ( fgets(line+1, MAXLINELEN-1, fp) ) {
	line[0]=' ';
	line[ strlen(line)-1 ] = '\0';
	XDrawString(disp,lifew,ntextgc,1,i*FONTHEIGHT,line,strlen(line));
	i++;
	if (FONTHEIGHT*(i+1) > height-INPUTH-BORDERWIDTH*2)
    {  strcpy(inpbuf,"Hit   R e t u r n   to continue");
	   minbuflen=strlen(inpbuf);
	   XClearWindow(disp,inputw);
       XDrawString(disp, inputw, ntextgc,ICOORDS(0,0),inpbuf, strlen(inpbuf));
       getxstring();
       inpbuf[0]=0;
       XClearWindow(disp,lifew);
	   i=2;
    }
	}
	if (i > 2)
    {  strcpy(inpbuf,"Hit   R e t u r n   to continue");
	   minbuflen=strlen(inpbuf);
	   XClearWindow(disp,inputw);
       XDrawString(disp, inputw, ntextgc,ICOORDS(0,0),inpbuf, strlen(inpbuf));
       getxstring();
       inpbuf[0]=0;
	}
	pclose(fp);
    exposeflag=1;
    XClearWindow(disp,lifew);
    XClearWindow(disp,inputw);
    redrawscreen();

}
