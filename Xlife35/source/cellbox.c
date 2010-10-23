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

/*
 * This module encapsulates cell-box handling. Everything else in the program
 * except possibly the evolution function should go through these functions
 * to change the board state. Entry points are:
 *
 * void initcells()   -- initialize board
 *
 * void addcell(x, y) -- set cell on
 *
 * void deletecell(x, y) -- set cell off , no cellbox creation any more
 *
 * void checkcell(x, y, inbox, alive)  get cell status
 *
 * void center() -- center visible board on barycenter of pattern
 *
 * void median() -- center visible board on median of pattern
 *
 * void clear() -- blank the board, freeing all storage
 *
 * void redisplay() -- update visible display of current board
 *
 * void redrawscreen() -- redraw display, assume nothing about previous state
 *
 * void saveall() -- save entire board state to file
 *
 * This code knows that cells are packed in 8x8 cell boxes on a hash list, but
 * it doesn't know anything about the internals of cell representation within
 * cell boxes. It relies on the existence of a triad of functions (see cell.c)
 *
 * int getcell(ptr, dx, dy)       -- get state of cell at x, y in box *ptr
 * void setcell(ptr, dx, dy, val) -- set cell at x, y in box *ptr to state val
 * void forget(ptr)            -- cause a box to forget last generation's state
 *
 * to access cell state. It also relies on the existence of a function
 *
 * void displaybox(x, y, ptr) -- post box state to Xrect arrays
 */

#include "defs.h"
#include "data.h"
#include "cellbox.h"
#include <pwd.h>
#include <stdlib.h>

extern char *ctime();
extern cellbox *tentative;

#define HASH(x,y) 	(((x>>3) & 0x7f)<< 7) + ((y>>3) & 0x7f)
#define SPACE		'.'
#define	USL_MAX		((unsigned long)~0L) /* max decimal value of "unsigned" */

#ifndef NULL
#define NULL 0
#endif
 
cellbox *head;
cellbox *freep;
cellbox *boxes[HASHSIZE];
XPoint onpoints[MAXON],offpoints[MAXON];
XRectangle onrects[MAXOFF],offrects[MAXOFF];
unsigned long maxdead;
unsigned long universe;
 
void initcells()
/* initialize the cell hash list */
{
int i;

  bzero((char*)boxes,HASHSIZE*sizeof(cellbox *));   

}

void addcell(UNS32 x, UNS32 y)
/* turn a cell to state ON */
{
    unsigned long ydx,xdx;
    cellbox *ptr;

    ptr = lifelink(x,y);
    ydx = y - ptr->y;
    xdx = x - ptr->x;
    ptr->dead=0;

    setcell(ptr, xdx, ydx, 1);
}

void deletecell(UNS32 x, UNS32 y)
/* turn a cell to state OFF */
{
    unsigned long ydx,xdx;
    cellbox *ptr;
    
    if (ptr = looklink(x,y)){
    ydx = y - ptr->y;
    xdx = x - ptr->x;
    setcell(ptr, xdx, ydx, 0);
	}
}

void checkcell(UNS32 x, UNS32 y, UNS32 *inbox, UNS32 *alive)
/* return back status of cell */
{
    unsigned long ydx,xdx;
    cellbox *ptr;
    
    if (ptr = looklink(x,y)){
	*inbox= 1;
    ydx = y - ptr->y;
    xdx = x - ptr->x;
    *alive= getcell(ptr, xdx, ydx);
	}
	else{
	*inbox= 0;
	*alive= 0;
	}
}

void killbox(cellbox *ptr)
{
    unsigned long hv=HASH(ptr->x,ptr->y);

    if(ptr != head){
	ptr->fore->next=ptr->next;
    }
    else{
	head = ptr->next;
    }
    if(ptr == boxes[hv]){
	boxes[hv] = ptr->hnext;
    }
    else{
	ptr->hfore->hnext=ptr->hnext;
    }
    if(ptr->next) ptr->next->fore=ptr->fore;
    if(ptr->hnext) ptr->hnext->hfore=ptr->hfore;
    if(ptr->rt) ptr->rt->lf=NULL;
    if(ptr->lf) ptr->lf->rt=NULL;
    if(ptr->up) ptr->up->dn=NULL;
    if(ptr->dn) ptr->dn->up=NULL;
    ptr->next=freep;
    freep=ptr;
    numboxes--;
}

cellbox *create(UNS32 x, UNS32 y, UNS32 hv)
{
    cellbox *ptr;

#ifdef PROF
    create_called++;
#endif

    if(freep == NULL){
#ifdef PROF
	create_null++;
#endif
	if((ptr= (cellbox *)malloc(sizeof(cellbox))) ==NULL){
	    perror("create: malloc error: ");
	    exit(-1);
	}
    }
    else{
	ptr=freep;
	freep=freep->next;
    }
    bzero((char*)ptr,sizeof(cellbox));
    
    ptr->next=head;
    head=ptr;
    
    if(ptr->next != NULL){
	ptr->next->fore=ptr;
    }	
    ptr->hnext=boxes[hv];
    boxes[hv]= ptr;
    
    if(ptr->hnext != NULL){
	ptr->hnext->hfore=ptr;
    }
    ptr->x = x;
    ptr->y = y;
    numboxes++;
    return(ptr);
}


cellbox *lifelink(UNS32 x, UNS32 y)
{
    cellbox *ptr;
    unsigned long hv;    
    
#ifdef  BOUNDED
    x |= ~universe;
    y |= ~universe;
#endif
    x &= ~7;
    y &= ~7;
    hv=HASH(x,y);
    ptr = boxes[hv]; 
#ifdef PROF
    link_called++;
#endif
    if(freep){
	if (ptr==NULL){
	    ptr=freep;
	    boxes[hv]= ptr;
	    freep=freep->next;
	    bzero((char*)ptr,sizeof(cellbox));
	    ptr->x = x;
	    ptr->y = y;
	    ptr->next=head;
	    head=ptr;    

	    if(ptr->next){
		ptr->next->fore=ptr;
	    }	
	    numboxes++;
	    return(ptr);
	}
	else{
	    do{
#ifdef PROF
		link_search++;
#endif
		if ((ptr->x == x) && (ptr->y == y)){
		    return(ptr);
		}
		ptr=ptr->hnext;
	    }while(ptr!=NULL);
	    
	    return(create(x,y,hv));
	}
    }
    else{
	if (ptr==NULL) return(create(x,y,hv));
	do{
#ifdef PROF
	    link_search++;
#endif
	    if ((ptr->x == x) && (ptr->y == y)){
		return(ptr);
	    }
	    ptr=ptr->hnext;
	}while(ptr!=NULL);
	
	return(create(x,y,hv));
    }
}


cellbox *looklink(UNS32 x, UNS32 y)
{
    cellbox *ptr;
    unsigned long hv;    
    
#ifdef  BOUNDED
    x |= ~universe;
    y |= ~universe;
#endif
    x &= ~7;
    y &= ~7;

    hv=HASH(x,y);
    ptr = boxes[hv]; 
#ifdef PROF
    link_called++;
#endif
    if(freep){
	if (ptr==NULL){
	    ptr=freep;
	    boxes[hv]= ptr;
	    freep=freep->next;
	    bzero((char*)ptr,sizeof(cellbox));
	    ptr->x = x;
	    ptr->y = y;
	    ptr->next=head;
	    head=ptr;    

	    if(ptr->next){
		ptr->next->fore=ptr;
	    }	
	    numboxes++;
	    return(ptr);
	}
	else{
	    do{
#ifdef PROF
		link_search++;
#endif
		if ((ptr->x == x) && (ptr->y == y)){
		    return(ptr);
		}
		ptr=ptr->hnext;
	    }while(ptr!=NULL);
	    
	    return( ptr );
	}
    }
    else{
	if (ptr==NULL) return( ptr );
	do{
#ifdef PROF
	    link_search++;
#endif
	    if ((ptr->x == x) && (ptr->y == y)){
		return(ptr);
	    }
	    ptr=ptr->hnext;
	}while(ptr!=NULL);
	
	return( ptr );
    }
}


void displaystats()
{
	sprintf(inpbuf,"Generations: %6lu, Boxes: %6lu",generations,numboxes);
    if(dispcells){
	sprintf(inpbuf+strlen(inpbuf),", Cells: %7lu ",numcells);
    }
    if(dispchang){
	sprintf(inpbuf+strlen(inpbuf),", Changes: %7lu ",numchang);
    }
    XClearWindow(disp,inputw);
    XDrawString(disp, inputw, ntextgc,ICOORDS(0,0),inpbuf, strlen(inpbuf));

}

UNS32 checkcellsperiod()
{   unsigned long  i, j=0;
	if (!numcellsperiod)  return  0;
	if (numcells != numbercells[lastindex]){
       numbercells[lastindex] = numcells;
	   matchedcounter = 0;
    }
    else if (++matchedcounter >= numcellsperiod  &&  matchedcounter >= 12) {
	   for (j=1;j<=numcellsperiod;j++)
       {  for (i=0;i<numcellsperiod-j;i++)
			 if (numbercells[i] != numbercells[i+j])  break;
          if (i!=numcellsperiod-j)  continue;
          for (   ;i<numcellsperiod;i++)
			 if (numbercells[i] != numbercells[i+j-numcellsperiod])  break;
          if (i==numcellsperiod)  break;
       }
	}
	if (++lastindex == numcellsperiod)
	   lastindex = 0;
    return  j;
}

void newrules()
{
    int i,k;
    char *ptr;
    
    ptr=inpbuf;
    strcpy(inpbuf,"Rules:   ");
    ptr=inpbuf+7;
    k=live;
    for(i=0; i<9; i++)
    {	if(k&1)
	{   *ptr=i+'0';
	    ptr++;
	}
	k>>=1;
    }
    *ptr='/';
    ptr++;
    k=born;
    for(i=0; i<9; i++)
    {	if(k&1)
	{   *ptr=i+'0';
	    ptr++;
	}
	k>>=1;
    }
    strcpy(ptr,"   New Rules:  ");
    minbuflen=strlen(inpbuf);
    XClearWindow(disp,inputw);
    XDrawString(disp, inputw, ntextgc,ICOORDS(0,0),inpbuf, strlen(inpbuf));
    
    getxstring();
    inpbuf[0]=0;
    
    k=0;
    ptr=inpbuf+minbuflen;
	while (*ptr)
	    if (*ptr > ' ')  break; else  ptr++;
    if (! *ptr)  return;
    while((*ptr>='0')&&(*ptr<'0'+10))
    {   k|=(1<<(*ptr-'0'));
	ptr++;
    }
    live=k;
    k=0;
    if(*ptr=='/')
    {   ptr++;
	while((*ptr>='0')&&(*ptr<'0'+10))
	{   k|=(1<<(*ptr-'0'));
	    ptr++;
	}
	born=k;
    }
    XClearWindow(disp,inputw);
    gentab();
}


void getboundingbox(UNS32 *xmin, UNS32 *xmax, UNS32 *ymin, UNS32 *ymax)
{
    cellbox *ptr;
    int dx, dy;
	*xmax = *ymax = 0;
	*xmin = *ymin = USL_MAX;
    for (ptr = head; ptr != NULL; ptr = ptr->next){
	if(!ptr->dead)
	    for (dx = 0; dx < BOXSIZE; dx++)
		for (dy = 0; dy < BOXSIZE; dy++)
		    if (getcellinbounds(ptr, dx, dy, 0, 0, USL_MAX, USL_MAX))
		    {
			if (ptr->x+dx < *xmin)
			    *xmin = ptr->x+dx;
			if (ptr->y+dy < *ymin)
			    *ymin = ptr->y+dy;
			if (ptr->x+dx > *xmax)
			    *xmax = ptr->x+dx;
			if (ptr->y+dy > *ymax)
			    *ymax = ptr->y+dy;
		    }
    }
}


void center()
/* center the display on the `center of mass' of the live boxes */
{
    double x,y;
    int ctr=0;
    cellbox *ptr;
    x=0.0;
    y=0.0;
    XClearWindow(disp,lifew);
    for (ptr = head; ptr != NULL; ptr = ptr->next){
	if(!ptr->dead){
	    x+= ptr->x;
	    y+= ptr->y;
	    ctr++;
	}
    } 
    if (ctr>0) {
       x/=ctr;
       y/=ctr;
    }
    else {
       x=xorigin;
       y=yorigin;
    }
    xpos=x- SCALE(width/2);
    ypos=y- SCALE(height/2);
    redrawscreen();
}

void heapsort(UNS32 *data, UNS32 n)
{
   unsigned long  help;
   register unsigned long  i, j, k;
   data --;
#define  swap(x,y)  {  help=data[x];  data[x]=data[y];  data[y]=help;  }
   for (i=2;i<=n;i++)
   {  while (j = i/2)
      {  if (data[i] > data[j])  swap(i,j)
         else  break;
         i = j;
      }
   }
   for (k=n;k>1;k--)
   {  help = data[1];
      for (i=2,j=1;i<k;i+=i)
      {  if (data[i+1] > data[i])  i++;
         data[j] = data[i];
         j = i;
      }
      if (i==k)
      {  data[j] = data[i];
         j = i;
      }
      data[j] = data[k];
      data[k] = help;
      if (j < k)
      {  while ( i = j/2 )
         {  if (data[j] > data[i])  swap(i,j)
            else  break;
            j = i;
         }
      }
   }
#undef  swap
   data ++;
}

void median()
/* center the display on the median of the alive boxes */
{
    unsigned long  ctr=0, *coordxlist, *coordylist;
    cellbox *ptr;
	if (!(coordxlist= (unsigned long*) malloc(sizeof(unsigned long)*numboxes)))
	   return;
	if (!(coordylist= (unsigned long*) malloc(sizeof(unsigned long)*numboxes)))
	   return;
    for (ptr = head; ptr != NULL; ptr = ptr->next){
	if(!ptr->dead  &&  (ptr->live1 || ptr->live2)){
	    coordxlist[ctr] = ptr->x;
	    coordylist[ctr] = ptr->y;
		ctr++;
	}
    } 
    XClearWindow(disp,lifew);
    if (ctr>0) {
	   heapsort(coordxlist,ctr);
	   heapsort(coordylist,ctr);
       xpos=coordxlist[ctr/2];
       ypos=coordylist[ctr/2];
    }
    else {
       xpos=xorigin;
       ypos=yorigin;
    }
	free(coordylist);
	free(coordxlist);
    xpos -= SCALE(width/2);
    ypos -= SCALE(height/2);
    redrawscreen();
}

void clear()
/* clear the board, freeing all storage */
{
    cellbox *ptr,*nptr;
   
    initcells();
    ptr=head;

    while(ptr){
	nptr=ptr->next;
	free(ptr);
	ptr=nptr;
    }
    /* free tentative pattern */
    ptr=tentative;
    while(ptr){
	nptr=ptr->next;
	free(ptr);
	ptr=nptr;
    }
    /* restart load script */
    free_loadscript();    

    head=tentative=NULL;
    state=STOP;
    generations=0;
    numboxes=0;
    numcells=0;
    XClearWindow(disp,lifew);
    XClearWindow(disp,inputw);
    displaystats();
}

#define ONSCREEN(x,y)  (((x>xpos-BOXSIZE) && (x<(xpos+BOXSIZE+SCALE(width)))) &&  \
						((y>ypos-BOXSIZE) && (y<(ypos+BOXSIZE+SCALE(height)))) )

#define POINTS	4000	/* accumulate this many point changes before writing */
#define RECTS	64	/* accumulate this many box changes before writing */

void redisplay()
/* re-display the visible part of the board */
{
    cellbox *ptr;
    unsigned long x,y,ctr=0;
    long tentx,tenty;
	GC  offgc;

    displaystats();    
    if(state==HIDE) return;
    
    onpt=offpt=0;
    onrect=offrect=0;
    if (gcmode == 2  &&  state == RUN)
	   offgc = whitegc,
	   XClearWindow(disp,lifew); 
    else
	   offgc = blackgc;
    for (ptr = head; ptr != NULL; ptr = ptr->next){
	x=ptr->x;
	y=ptr->y;
	if(ONSCREEN(x, y)){
	    displaybox(x, y, ptr);
        if (gcmode == 0)
		   offpt=offrect=0;
	    ctr++;
	    if(scale <= 0){
		if((onpt >= POINTS) || (offpt >= POINTS)){
		    XDrawPoints(disp,lifew,offgc,offpoints,offpt,CoordModeOrigin);
		    XDrawPoints(disp,lifew,whitegc,onpoints,onpt,CoordModeOrigin);
		    onpt=offpt=0;
		    ctr=0;
		}
	    }
	    else{
		if(ctr == RECTS){
		    XFillRectangles(disp,lifew,offgc,offrects,offrect);
		    XFillRectangles(disp,lifew,whitegc,onrects,onrect);
		    onrect=offrect=0;
		    ctr=0;
		}
	    }
	}
    }
    /* draw tentative points with appropriate transformation */
    for (ptr = tentative; ptr != NULL; ptr = ptr->next){
	tentx=ptr->x-STARTPOS;
	tenty=ptr->y-STARTPOS;
	if(ONSCREEN(tx(tentx,tenty,txx,txy)+loadx,
                    ty(tentx,tenty,tyx,tyy)+loady)) {
	    trdisplaybox(tentx, tenty, ptr);
	    ctr++;
	    if(scale <= 0){
		if((onpt >= POINTS) || (offpt >= POINTS)){
		    XDrawPoints(disp,lifew,blackgc,offpoints,offpt,CoordModeOrigin);
		    XDrawPoints(disp,lifew,whitegc,onpoints,onpt,CoordModeOrigin);
		    onpt=offpt=0;
		    ctr=0;
		}
	    }
	    else{
		if(ctr == RECTS){
		    XFillRectangles(disp,lifew,blackgc,offrects,offrect);
		    XFillRectangles(disp,lifew,whitegc,onrects,onrect);
		    onrect=offrect=0;
		    ctr=0;
		}
	    }
	}
    }
    if(ctr){
	if(scale <= 0){
	    XDrawPoints(disp,lifew,offgc,offpoints,offpt,CoordModeOrigin);
    	XDrawPoints(disp,lifew,whitegc,onpoints,onpt,CoordModeOrigin);
	    onpt=offpt=0;
	    ctr=0;
	}
	else{
	    XFillRectangles(disp,lifew,offgc,offrects,offrect);
	    XFillRectangles(disp,lifew,whitegc,onrects,onrect);
	    onrect=offrect=0;
	    ctr=0;
	}
    }
    /* draw pivot of tentative points */
    if (tentative  &&  scale >= 0)
		drawpivot((loadx-xpos) << scale , (loady-ypos) << scale );

    XFlush(disp);
}

void redrawscreen()
/* force redraw of entire board */
{
    cellbox *ptr;

    if (showmesh) drawmesh();
    for (ptr = head; ptr != NULL; ptr = ptr->next)
	forget(ptr);
    for (ptr = tentative; ptr != NULL; ptr = ptr->next)
	forget(ptr);
    redisplay();
}


void saveall(FILE *ofp, char mode)
/* save entire board contents */
{
   saveinbounds(ofp, mode, 0, 0, USL_MAX, USL_MAX); 
}

void saveinbounds(FILE *ofp, char mode, UNS32 lx, UNS32 ly, UNS32 ux, UNS32 uy)
/* save board contents within bounding box */
{
    cellbox *ptr;
    int dx, dy, val,x=0;
    unsigned long xmin, ymin, xmax, ymax, numcells;
	time_t  timeval;
    struct passwd *pw;
    char machine[80];
/**  mode should be one of "BbARDP\0"  **/
    if (mode!='B' && mode!='b') {
       if(fname[0] != 0){
	   int i, h=0;
	   for (i=0;fname[i-h]=fname[i];i++)
		  if (fname[i]=='\015')  h++;
          fprintf(ofp,"#N %s\n",fname);
       }
   	
       timeval=time(0);
       gethostname(machine,80);
       if( (pw = getpwuid(getuid())) != NULL){
           fprintf(ofp,"#O %s \"%s\"@%s %s",pw->pw_name,pw->pw_gecos,machine,
           ctime(&timeval));
       }
       
       while(x < numcomments){
          fprintf(ofp,"#C %s \n",comments[x]);
          x++;
       }
    }

	if (live != 1<<2 | 1<<3  ||  born != 1<<3)
	{  int  i, j, k;
	   char  rules[20];
	   j=0;
       k=live;
       for(i=0; i<9; i++)
       {  if(k&1)
	         rules[j++]=i+'0';
	      k>>=1;
       }
       rules[j++]='/';
       k=born;
       for(i=0; i<9; i++)
       {  if(k&1)
	         rules[j++]=i+'0';
	      k>>=1;
       }
	   rules[j]='\0';
	   fprintf(ofp,"#r %s\n",rules);
	}

    if (mode == 'A')	/* save in absolute mode */
    {
	fputs("#A\n", ofp);
	for (ptr = head; ptr != NULL; ptr = ptr->next)
	    if (!ptr->dead)
		for (dx = 0; dx < BOXSIZE; dx++)
		    for (dy = 0; dy < BOXSIZE; dy++)
			if (val = getcellinbounds(ptr, dx, dy, lx, ly, ux, uy))
			    (void) fprintf(ofp,
					   "%ld %ld\n", ptr->x+dx,ptr->y+dy);
	return;
    }

    /*
     * Otherwise, determine the bounding box of the live cells.
     */
    xmin = USL_MAX; ymin = USL_MAX; xmax = 0; ymax = 0; numcells = 0;
    for (ptr = head; ptr != NULL; ptr = ptr->next)
	if(!ptr->dead)
	    for (dx = 0; dx < BOXSIZE; dx++)
		for (dy = 0; dy < BOXSIZE; dy++)
		    if (getcellinbounds(ptr, dx, dy, lx, ly, ux, uy))
		    {
			numcells++;
			if (ptr->x+dx < xmin)
			    xmin = ptr->x+dx;
			if (ptr->y+dy < ymin)
			    ymin = ptr->y+dy;
			if (ptr->x+dx > xmax)
			    xmax = ptr->x+dx;
			if (ptr->y+dy > ymax)
			    ymax = ptr->y+dy;
		    }

    /* caller may want save to shortest format */
    if (mode == '\0')
	{   if (((ymax - ymin + 1) * (xmax - xmin + 1)) > numcells * 6)
	       mode = 'D';
	    else
	       mode = 'P';
    }
    /* now that we have the bounding box, we can gen the other formats */
    if (mode == 'R')
    {
	fprintf(ofp, "#R -%ld -%ld\n", (xmax+1-xmin)/2,(ymax+1-ymin)/2);
	for (ptr = head; ptr != NULL; ptr = ptr->next)
	    if (!ptr->dead)
		for (dx = 0; dx < BOXSIZE; dx++)
		    for (dy = 0; dy < BOXSIZE; dy++)
			if (val = getcellinbounds(ptr, dx, dy, lx, ly, ux, uy))
			    (void) fprintf(ofp, "%ld %ld\n",
					   ptr->x+dx-xmin, ptr->y+dy-ymin);
    }
    else if (mode == 'D')
    {  int xx=0, yy=0;
	fputs("#D\n", ofp);
	for (ptr = head; ptr != NULL; ptr = ptr->next)
	    if (!ptr->dead)
		for (dx = 0; dx < BOXSIZE; dx++)
		    for (dy = 0; dy < BOXSIZE; dy++)
			if (val = getcellinbounds(ptr, dx, dy, lx, ly, ux, uy))
			{   (void) fprintf(ofp, "%ld %ld\n",
					   ptr->x+dx-xmin -xx, ptr->y+dy-ymin -yy);
				xx = ptr->x+dx-xmin;
				yy = ptr->y+dy-ymin;
            }
    }
    else /* mode is 'P' or 'B' (for bounding box with centered origin) 
                        or 'b' (origin at center of user-selected box) */
    {
	unsigned long x, y;

	(void) fprintf(ofp, "#P");
        if (mode == 'B') fprintf(ofp, " -%ld -%ld",
                                 (xmax+1-xmin)/2,(ymax+1-ymin)/2+1);
        else if (mode == 'b') fprintf(ofp, " -%ld -%ld",
                                 (ux+1-lx)/2+lx-xmin,(uy+1-ly)/2+1+ly-ymin);
	(void) fprintf(ofp, "\n");
	for (y = ymin; y <= ymax; y++)
	{
	    for (x = xmin; x <= xmax; x++)
	    {
		cellbox *ptr;

		ptr = looklink(x,y);
		if (ptr && getcellinbounds(ptr, x - ptr->x, y - ptr->y, lx, ly, ux, uy))
		    (void) fputc(MARK, ofp);
		else
		    (void) fputc(SPACE, ofp);
	    }
	    (void) fputc('\n', ofp);
	}
    }
}
