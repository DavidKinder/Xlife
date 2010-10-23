/*
 * Life search program include file.
 * Author: David I. Bell.
 */

#include <stdio.h>


/*
 * Maximum dimensions of the search
 */
#define	ROWMAX		21	/* maximum rows for search rectangle */
#define	COLMAX		79	/* maximum columns for search rectangle */
#define	GENMAX		6	/* maximum number of generations */
#define	TRANSMAX	3	/* largest translation value allowed */


/*
 * Build options
 */
#ifndef DEBUGFLAG
#define	DEBUGFLAG	0	/* nonzero for debugging features */
#endif


/*
 * Other definitions
 */
#define	ALLOCSIZE	100		/* chunk size for cell allocation */
#define	VIEWMULT	1000		/* viewing frequency multiplier */
#define	DUMPMULT	1000		/* dumping frequency multiplier */
#define	DUMPFILE	"lifesrc.dmp"	/* default dump file name */
#define	DUMPVERSION	1		/* version of dump file */
#define	LINESIZE	132		/* size of input lines */

#define	MAXCELLS	((COLMAX + 2) * (ROWMAX + 2) * GENMAX)
#define	AUXCELLS	(TRANSMAX * (COLMAX + ROWMAX + 4) * 2)


/*
 * Debugging macros
 */
#if DEBUGFLAG
#define	DPRINTF0(fmt)			if (debug) printf(fmt)
#define	DPRINTF1(fmt,a1)		if (debug) printf(fmt,a1)
#define	DPRINTF2(fmt,a1,a2)		if (debug) printf(fmt,a1,a2)
#define	DPRINTF3(fmt,a1,a2,a3)		if (debug) printf(fmt,a1,a2,a3)
#define	DPRINTF4(fmt,a1,a2,a3,a4)	if (debug) printf(fmt,a1,a2,a3,a4)
#define	DPRINTF5(fmt,a1,a2,a3,a4,a5)	if (debug) printf(fmt,a1,a2,a3,a4,a5)
#else
#define	DPRINTF0(fmt)
#define	DPRINTF1(fmt,a1)
#define	DPRINTF2(fmt,a1,a2)
#define	DPRINTF3(fmt,a1,a2,a3)
#define	DPRINTF4(fmt,a1,a2,a3,a4)
#define	DPRINTF5(fmt,a1,a2,a3,a4,a5)
#endif


#define	isdigit(ch)	(((ch) >= '0') && ((ch) <= '9'))
#define	isblank(ch)	(((ch) == ' ') || ((ch) == '\t'))


typedef	unsigned char	BOOL;
typedef	unsigned char	STATE;
typedef	unsigned int	STATUS;


#define	FALSE		((BOOL) 0)
#define	TRUE		((BOOL) 1)


/*
 * Status returned by routines
 */
#define	OK		((STATUS) 0)
#define	ERROR		((STATUS) 1)
#define	CONSISTENT	((STATUS) 2)
#define	NOTEXIST	((STATUS) 3)
#define	FOUND		((STATUS) 4)


/*
 * States of a cell
 */
#define	OFF	((STATE) 0x00)		/* cell is known off */
#define	ON	((STATE) 0x01)		/* cell is known on */
#define	UNK	((STATE) 0x10)		/* cell is unknown */
#define	NSTATES	3			/* number of states */


typedef	struct cell CELL;
struct cell {
	STATE	state;		/* current state */
	BOOL	free;		/* TRUE if this cell still has free choice */
	char	gen;		/* generation number of this cell */
	short	row;		/* row of this cell */
	short	col;		/* column of this cell */
	CELL	*search;	/* cell next to be searched for setting */
	CELL	*past;		/* cell in the past at this location */
	CELL	*future;	/* cell in the future at this location */
	CELL	*cul;		/* cell to up and left */
	CELL	*cu;		/* cell to up */
	CELL	*cur;		/* cell to up and right */
	CELL	*cl;		/* cell to left */
	CELL	*cr;		/* cell to right */
	CELL	*cdl;		/* cell to down and left */
	CELL	*cd;		/* cell to down */
	CELL	*cdr;		/* cell to down and right */
	CELL	*csym;		/* cell in symmetric position */
};

#define	NULL_CELL	((CELL *) 0)


/*
 * Data about the cells.
 */
extern CELL	*celltable[MAXCELLS];	/* table of usual cells */
extern CELL	*auxtable[AUXCELLS];	/* table of auxillary cells */
extern CELL	*settable[MAXCELLS];	/* table of cells whose value is set */
extern CELL	**newset;	/* where to add new cells into setting table */
extern CELL	**nextset;	/* next cell in setting table to examine */
extern CELL	**baseset;	/* base of changeable part of setting table */
extern CELL	*searchlist;	/* current list of cells to search */
extern CELL	*fullsearchlist;	/* complete list of cells to search */
extern CELL	*newcells;	/* cells ready for allocation */
extern CELL	*deadcell;	/* boundary cell value */
extern int	newcellcount;	/* number of cells ready for allocation */
extern int	auxcellcount;	/* number of cells in auxillary table */


/*
 * Current parameter values for the program.
 */
extern BOOL	debug;		/* enable debugging output (if compiled so) */
extern BOOL	nowait;		/* don't wait for commands after loading */
extern BOOL	setall;		/* set all cells from initial file */
extern BOOL	rowsym;		/* enable row symmetry */
extern BOOL	colsym;		/* enable column symmetry */
extern BOOL	parent;		/* only look for parents */
extern BOOL	allobjects;	/* look for all objects including subperiods */
extern STATUS	curstatus;	/* current status for display */
extern int	curgen;		/* current generation for display */
extern int	rowmax;		/* maximum number of rows */
extern int	colmax;		/* maximum number of columns */
extern int	genmax;		/* maximum number of generations */
extern int	rowtrans;	/* translation of rows */
extern int	coltrans;	/* translation of columns */
extern int	maxcount;	/* maximum number of cells in generation 0 */
extern int	cellcount;	/* number of live cells in generation 0 */
extern long	dumpfreq;	/* how often to perform dumps */
extern long	dumpcount;	/* counter for dumps */
extern long	viewfreq;	/* how often to view results */
extern long	viewcount;	/* counter for viewing */
extern long	foundcount;	/* number of objects found */
extern char	*dumpfile;	/* dump file name */
extern char	*initfile;	/* file containing initial cells */
extern char	*loadfile;	/* file to load state from */
extern char	*outputfile;	/* file to output results to */


/*
 * Global procedures
 */
extern void	getcommands();
extern void	initcells();
extern void	printgen();
extern void	dumpstate();
extern STATUS	search();
extern STATUS	proceed();
extern CELL	*findcell();
extern BOOL	subperiods();

extern int	ttyopen();
extern int	ttycheck();
extern int	ttyread();
extern void	ttyprintf();
extern void	ttystatus();
extern void	ttywrite();
extern void	ttyhome();
extern void	ttyeeop();
extern void	ttyflush();
extern void	ttyclose();

extern char	*malloc();
extern long	atol();

/* END CODE */
