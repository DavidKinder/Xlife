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

typedef struct box {
    UNS32 x,y,   /* upper left box coordinates, multiple of boxsize */
		  live1,live2, /* packed bitmap, each succesive 8 bits a row */
		  olive1,olive2, /* packed bitmap of old cells */
		  dead;  /* number of generations box is empty and not in use */
    UNS32 on[8];
    struct box *up, *dn, *lf, *rt,  /* pointer to neighbourboxes */
			   *fore, *next, *hfore,*hnext;   /* pointers for listlinks */
}
cellbox;

#define HASHSIZE	32768
#define MAXON		8192	/* max # cells we can change to on per move */
#define MAXOFF		8192	/* max # cells we can change to off per move */
#define BOXSIZE         8

extern cellbox *lifelink(UNS32 x, UNS32 y);
extern cellbox *looklink(UNS32 x, UNS32 y);
extern cellbox *head;
extern cellbox *freep;
extern cellbox *boxes[HASHSIZE];
GLOBAL XPoint onpoints[MAXON],offpoints[MAXOFF];
GLOBAL XRectangle onrects[MAXON],offrects[MAXOFF];
GLOBAL unsigned long maxdead;
GLOBAL unsigned long universe;

GLOBAL int getcell( cellbox *ptr, int xdx, int ydx );
GLOBAL void forget( cellbox *ptr );
GLOBAL void setcell( cellbox *ptr, int xdx, int ydx, int val );
GLOBAL void killbox( cellbox *ptr );
GLOBAL void displaybox( UNS32 x, UNS32 y, cellbox *ptr );
GLOBAL void trdisplaybox( long x, long y, cellbox *ptr );
GLOBAL int getcellinbounds( cellbox *ptr, int xdx, int ydx, int lx, int ly, int ux, int uy );
