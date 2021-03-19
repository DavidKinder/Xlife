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

extern int sys_nerr, errno;
extern char *sys_errlist[];
#define SYSERR sys_errlist[(errno > sys_nerr? 0 : errno)]

GLOBAL Display *disp;
GLOBAL Window rootw, mainw, lifew, inputw, coordw;
GLOBAL int screen;
GLOBAL UNS32 fcolor, bcolor;
GLOBAL XEvent event;
GLOBAL XFontStruct *nfont, *bfont;
GLOBAL int exposeflag;
GLOBAL int getinput;
#define KEYBUFLEN  16
#define INPBUFLEN 256
GLOBAL char inpbuf[INPBUFLEN];
GLOBAL int minbuflen;
#define DIRBUFLEN 100
GLOBAL int numcomments;
#define MAXCOMMENTS 50
#define COMMLEN  INPBUFLEN
GLOBAL char comments[MAXCOMMENTS][COMMLEN];
GLOBAL char loadirbuf[DIRBUFLEN];
GLOBAL char lisdirbuf[DIRBUFLEN];
GLOBAL char keybuf[KEYBUFLEN];
GLOBAL char lookup[0xfffff];
GLOBAL char fname[256];
GLOBAL char imageformat;
GLOBAL KeySym ks;
GLOBAL XGCValues xgcv;
GLOBAL GC ntextgc, btextgc,inputgc,blackgc,whitegc,xorgc;
GLOBAL Pixmap lifepm;
GLOBAL Cursor cursor;

/* GLOBAL char *itoa( int ); */

GLOBAL UNS32 xpos,ypos,xorigin,yorigin,lastx,lasty;

GLOBAL void initcells( void );
GLOBAL void saveinbounds( FILE*, char, UNS32, UNS32, UNS32, UNS32 );
GLOBAL void savefileinbounds( UNS32 lx, UNS32 ly, UNS32 ux, UNS32 uy );
GLOBAL void addcell( UNS32 x, UNS32 y );
GLOBAL void deletecell( UNS32 x, UNS32 y );
GLOBAL void drawpivot( int x, int y );
GLOBAL void redisplay( void );
GLOBAL void median( void );
GLOBAL void generate( void );
GLOBAL void clear( void );
GLOBAL void savefile( void );
GLOBAL void loadfile( char *loadname );
GLOBAL void confirmload( void );
GLOBAL void saveloadscript( void );
GLOBAL void free_loadscript( void );
GLOBAL void loadfileat( char *loadname, UNS32 valid, UNS32 tloadx, UNS32 tloady );
GLOBAL void undoload( void );
GLOBAL void genload( void );
GLOBAL void do_loadreq( LoadReq *loadqueue );
GLOBAL void add_loadreq( LoadReq **loadqueue, long loadtime, char *patname,
                         int relpath, UNS32 hotx, UNS32 hoty,
						 int rxx, int rxy, int ryx, int ryy );
GLOBAL void center( void );
GLOBAL void newrules( void );
GLOBAL void redrawscreen( void );
GLOBAL void help( void );
GLOBAL void randomize( UNS32 lx, UNS32 ly, UNS32 dx, UNS32 dy );
GLOBAL void settimeout( UNS32 ms );
GLOBAL void gentab( void );
GLOBAL void saveall( FILE *ofp, char mode );
GLOBAL void getxstring( void );
GLOBAL void name_file( void );
GLOBAL void comment( void );
GLOBAL void view_comments( void );
GLOBAL void benchmark( void );
GLOBAL void Motion( void );
GLOBAL void Button( void );
GLOBAL void DoResize( void );
GLOBAL void DoExpose( int win );
GLOBAL int ClassifyWin( Window win );
GLOBAL void DoKeyIn( char kbuf[KEYBUFLEN] );
GLOBAL int DoKeySymIn( KeySym keysym );
GLOBAL void getbounds( UNS32 *lx, UNS32 *ly, UNS32 *ux, UNS32 *uy );
GLOBAL void drawbox( int x, int y, int c );
GLOBAL void drawpoint( int x, int y, int c );
GLOBAL void drawmesh( void );
GLOBAL void displaycoords( void );
GLOBAL void getinputwstring( char *promptstr );
GLOBAL void listlifefiles( void );
GLOBAL void shellcommand( char *prestr, char *userstr, char *denystr, char *poststr );
GLOBAL void getboundingbox( UNS32 *xmin, UNS32 *xmax, UNS32 *ymin, UNS32 *ymax );
GLOBAL void randomseed( void );
GLOBAL void fatal( char *s );
GLOBAL long scale;
GLOBAL UNS32 random32( void );
GLOBAL UNS32 checkcellsperiod( void );
GLOBAL UNS32 dispcells;
GLOBAL UNS32 dispchang;
GLOBAL UNS32 load;
GLOBAL UNS32 save;
GLOBAL UNS32 born;
GLOBAL UNS32 live;
GLOBAL UNS32 run;
GLOBAL UNS32 hide;
GLOBAL UNS32 generations;
GLOBAL UNS32 numboxes;
GLOBAL UNS32 numcells;
GLOBAL UNS32 numchang;
GLOBAL UNS32 collisions;
#define MAXPERIOD 999
GLOBAL UNS32 numbercells[MAXPERIOD];
GLOBAL UNS32 numcellsperiod;
GLOBAL UNS32 lastindex;
GLOBAL UNS32 matchedcounter;
GLOBAL UNS32 gcmode;
GLOBAL UNS32 runcounter,stopcounter;
#define CHASHSIZE 65537
#define CHASHMULT 16383

GLOBAL UNS32 hashcnt[CHASHSIZE];
GLOBAL int width;
GLOBAL int height;
GLOBAL int inputlength;
GLOBAL int onpt,offpt,onrect,offrect;
GLOBAL int state,dispcoord,showmesh;
#define NUMMARKER 4
GLOBAL long  xmarker[NUMMARKER], ymarker[NUMMARKER];
GLOBAL struct timeval timeout;


GLOBAL int link_called;
GLOBAL int link_search;
GLOBAL int create_called;
GLOBAL int create_null;

GLOBAL int txx,txy,tyx,tyy;  /* transformation for new points */ 
GLOBAL UNS32 loadx, loady; /* location to load new points */

/* globals for tentative population */
GLOBAL UNS32 tentnumboxes,tentnumcells,tentgenerations;
