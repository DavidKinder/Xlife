/*
 * Life search program - actual search routines.
 * Author: David I. Bell.
 * Based on the algorithms by Dean Hickerson that were
 * included with the "xlife 2.0" distribution.  Thanks!
 */

#include "lifesrc.h"


/*
 * IMPLIC flag values.
 */
typedef	unsigned char	FLAGS;
#define	N0IC0	((FLAGS) 0x01)	/* new cell 0 ==> current cell 0 */
#define	N0IC1	((FLAGS) 0x02)	/* new cell 0 ==> current cell 1 */
#define	N1IC0	((FLAGS) 0x04)	/* new cell 1 ==> current cell 0 */
#define	N1IC1	((FLAGS) 0x08)	/* new cell 1 ==> current cell 1 */
#define	N0ICUN0	((FLAGS) 0x10)	/* new cell 0 ==> current unknown neighbors 0 */
#define	N0ICUN1	((FLAGS) 0x20)	/* new cell 0 ==> current unknown neighbors 1 */
#define	N1ICUN0	((FLAGS) 0x40)	/* new cell 1 ==> current unknown neighbors 0 */
#define	N1ICUN1	((FLAGS) 0x80)	/* new cell 1 ==> current unknown neighbors 1 */


/*
 * Table of transitions.
 * Given the state of a cell and its neighbors in one generation,
 * this table determines the state of the cell in the next generation.
 * The table is indexed by the descriptor value of a cell.
 */
static STATE	transit[256];


/*
 * Table of implications.
 * Given the state of a cell and its neighbors in one generation,
 * this table determines deductions about the cell and its neighbors
 * in the previous generation.
 * The table is indexed by the descriptor value of a cell.
 */
static FLAGS	implic[256];


/*
 * Data about the cells.
 */
CELL	*celltable[MAXCELLS];	/* table of usual cells */
CELL	*auxtable[AUXCELLS];	/* table of auxillary cells */
CELL	*settable[MAXCELLS];	/* table of cells whose value is set */
CELL	**newset;		/* where to add new cells into setting table */
CELL	**nextset;		/* next cell in setting table to examine */
CELL	**baseset;		/* base of changeable part of setting table */
CELL	*searchlist;		/* current list of cells to search */
CELL	*fullsearchlist;	/* complete list of cells to search */
CELL	*newcells;		/* cells ready for allocation */
CELL	*deadcell;		/* boundary cell value */
int	newcellcount;		/* number of cells ready for allocation */
int	auxcellcount;		/* number of cells in auxillary table */


/*
 * Current parameter values for the program.
 */
BOOL	debug;			/* enable debugging output (if compiled so) */
BOOL	nowait;			/* don't wait for commands after loading */
BOOL	setall;			/* set all cells from initial file */
BOOL	rowsym;			/* enable row symmetry */
BOOL	colsym;			/* enable column symmetry */
BOOL	parent;			/* only look for parents */
BOOL	allobjects;		/* look for all objects including subperiods */
STATUS	curstatus;		/* current status for display */
int	curgen;			/* current generation for display */
int	rowmax;			/* maximum number of rows */
int	colmax;			/* maximum number of columns */
int	genmax;			/* maximum number of generations */
int	rowtrans;		/* translation of rows */
int	coltrans;		/* translation of columns */
int	maxcount;		/* maximum number of cells in generation 0 */
int	cellcount;		/* number of live cells in generation 0 */
long	dumpfreq;		/* how often to perform dumps */
long	dumpcount;		/* counter for dumps */
long	viewfreq;		/* how often to view results */
long	viewcount;		/* counter for viewing */
long	foundcount;		/* number of objects found */
char	*dumpfile;		/* dump file name */
char	*initfile;		/* file containing initial cells */
char	*loadfile;		/* file to load state from */
char	*outputfile;		/* file to output results to */


static STATE	states[NSTATES] = {OFF, ON, UNK};


/*
 * Local procedures
 */
static void	inittransit();
static void	initimplic();
static void	linkcell();
static STATE	transition();
static STATE	choose();
static FLAGS	implication();
static CELL	*symcell();
static CELL	*allocatecell();
static CELL	*backup();
static CELL	*getunknown();
static STATUS	setcell();
static STATUS	consistify();
static STATUS	consistify10();
static STATUS	examinenext();
static STATUS	go();
static int	getdesc();
static int	sumtodesc();


/*
 * Initialize the table of cells.
 * Each cell in the active area is set to unknown state.
 * Boundary cells are set to zero state.
 */
void
initcells()
{
	int	row, col, gen;
	int	nrow, ncol;
	int	i;
	BOOL	edge;
	CELL	*cell;

	if ((rowmax <= 0) || (rowmax > ROWMAX) ||
		(colmax <= 0) || (colmax > COLMAX) ||
		(genmax <= 0) || (genmax > GENMAX) ||
		(rowtrans < 0) || (rowtrans > TRANSMAX) ||
		(coltrans < 0) || (coltrans > TRANSMAX))
	{
		ttyclose();
		fprintf(stderr, "ROW, COL, GEN, or TRANS out of range\n");
		exit(1);
	}

	/*
	 * The first allocation of a cell MUST be deadcell.
	 * Then allocate the cells in the cell table.
	 */
	deadcell = allocatecell();
	for (i = 0; i < MAXCELLS; i++)
		celltable[i] = allocatecell();

	/*
	 * Link the cells together.
	 */
	for (col = 0; col <= colmax+1; col++) {
		for (row = 0; row <= rowmax+1; row++) {
			for (gen = 0; gen < genmax; gen++) {
				edge = ((row == 0) || (col == 0) ||
					(row > rowmax) || (col > colmax));

				cell = findcell(row, col, gen);
				cell->gen = gen;
				cell->row = row;
				cell->col = col;

				/*
				 * If this is not an edge cell, then its state
				 * is unknown and it needs linking to its
				 * neighbors.
				 */
				if (!edge) {
					linkcell(cell);
					cell->state = UNK;
					cell->free = TRUE;
				}

				/*
				 * Map time forwards and backwards,
				 * wrapping around at the ends.
				 */
				cell->past = findcell(row, col,
					(gen+genmax-1) % genmax);
				cell->future = findcell(row, col,
					(gen+1) % genmax);

				/*
				 * If this is not an edge cell, then
				 * set up symmetry for it.
				 */
				if ((rowsym || colsym) && !edge)
					cell->csym = symcell(cell);

			}
		}
	}

	/*
	 * If there is a translation, then implement it.
	 */
	if (rowtrans || coltrans) {
		for (col = 0; col <= colmax+1; col++) {
			for (row = 0; row <= rowmax+1; row++) {
				cell = findcell(row, col, genmax-1);
				nrow = row + rowtrans;
				ncol = col + coltrans;
				cell->future = findcell(nrow, ncol, 0);
				linkcell(cell->future);

				cell = findcell(row, col, 0);
				nrow = row - rowtrans;
				ncol = col - coltrans;
				cell->past = findcell(nrow, ncol, genmax-1);
				linkcell(cell->past);
			}
		}
	}

	/*
	 * Build the search table list.
	 * This list is built backwards from the intended search order.
	 * Do searches from the middle row outwards, and from the left
	 * to the right columns.  However, since we try OFF cells first,
	 * reverse the row order again to try to make thin objects.
	 */
	searchlist = NULL;
	for (gen = genmax - 1; gen >= 0; gen--) {
		for (col = colmax; col > 0; col--) {
			for (row = (rowmax + 1) / 2; row > 0; row--) {
				cell = findcell(row, col, gen);
				cell->search = searchlist;
				searchlist = cell;
				if (rowsym || (row * 2 == rowmax + 1))
					continue;
				cell = findcell(rowmax + 1 - row, col, gen);
				cell->search = searchlist;
				searchlist = cell;
			}
		}
	}
	fullsearchlist = searchlist;

	newset = settable;
	nextset = settable;
	baseset = settable;

	curgen = 0;
	curstatus = OK;
	inittransit();
	initimplic();
}


/*
 * Set the state of a cell to the specified state.
 * The state is either ON or OFF.
 * Returns ERROR if the setting is inconsistent.
 * If the cell is newly set, then it is added to the set table.
 */
static STATUS
setcell(cell, state, free)
	CELL	*cell;
	STATE	state;
	BOOL	free;
{
	if (cell->state == state) {
		DPRINTF4("setcell %d %d %d to state %s already set\n",
			cell->row, cell->col, cell->gen,
			(state == ON) ? "on" : "off");
		return OK;
	}

	if (cell->state != UNK) {
		DPRINTF4("setcell %d %d %d to state %s inconsistent\n",
			cell->row, cell->col, cell->gen,
			(state == ON) ? "on" : "off");
		return ERROR;
	}

	if ((state == ON) && (cell->gen == 0)) {
		if (maxcount && (cellcount >= maxcount)) {
			DPRINTF2("setcell %d %d 0 on exceeds maxcount\n",
				cell->row, cell->col);
			return ERROR;
		}
		cellcount++;
	}

	DPRINTF5("setcell %d %d %d to %s, %s successful\n",
		cell->row, cell->col, cell->gen,
		(free ? "free" : "forced"), ((state == ON) ? "on" : "off"));

	cell->state = state;
	cell->free = free;
	*newset++ = cell;

	return OK;
}


/*
 * Calculate the current descriptor for a cell.
 */
static int
getdesc(cell)
	register CELL	*cell;
{
	int	sum;

	sum = cell->cul->state + cell->cu->state + cell->cur->state;
	sum += cell->cdl->state + cell->cd->state + cell->cdr->state;
	sum += cell->cl->state + cell->cr->state;

	return ((sum & 0x88) ? (sum + (cell->state << 1) + 0x11) :
		((sum << 1) + cell->state));
}


/*
 * Return the descriptor value for a cell and the sum of its neighbors.
 */
static int
sumtodesc(state, sum)
	STATE	state;
{
	return ((sum & 0x88) ? (sum + state * 2 + 0x11) : (sum * 2 + state));
}


/*
 * Consistify a cell.
 * This means examine this cell in the previous generation, and
 * make sure that the previous generation can validly produce the
 * current cell.  Returns ERROR if the cell is inconsistent.
 */
static STATUS
consistify(cell)
	CELL	*cell;
{
	CELL	*prevcell;
	int	desc;
	STATE	state;
	FLAGS	flags;

	/*
	 * If we are searching for parents and this is generation 0, then
	 * the cell is consistent with respect to the previous generation.
	 */
	if (parent && (cell->gen == 0))
		return OK;

	/*
	 * First check the transit table entry for the previous
	 * generation.  Make sure that this cell matches the ON or
	 * OFF state demanded by the transit table.  If the current
	 * cell is unknown but the transit table knows the answer,
	 * then set the now known state of the cell.
	 */
	prevcell = cell->past;
	desc = getdesc(prevcell);
	state = transit[desc];
	if (state != cell->state) {
		switch (state) {
			case ON:
				if (cell->state == OFF) {
					DPRINTF3("Transit inconsistently forces cell %d %d %d on\n",
						cell->row, cell->col,
						cell->gen);
					return ERROR;
				}

				if (cell->gen == 0) {
					if (maxcount &&
						(cellcount >= maxcount))
					{
						DPRINTF2("Transit forcing cell %d %d 0 exceeds maxcount\n",
						cell->row, cell->col);
						return ERROR;
					}
					cellcount++;
				}

				DPRINTF3("Transit forces cell %d %d %d on\n",
					cell->row, cell->col, cell->gen);
				cell->state = ON;
				cell->free = FALSE;
				*newset++ = cell;
				break;

			case OFF:
				if (cell->state == ON) {
					DPRINTF3("Transit inconsistently forces cell %d %d %d off\n",
						cell->row, cell->col,
						cell->gen);
					return ERROR;
				}
				DPRINTF3("Transit forces cell %d %d %d off\n",
					cell->row, cell->col, cell->gen);
				cell->state = OFF;
				cell->free = FALSE;
				*newset++ = cell;
				break;
		}
	}

	/*
	 * Now look up the previous generation in the implic table.
	 * If this cell implies anything about the cell or its neighbors
	 * in the previous generation, then handle that.
	 */
	flags = implic[desc];
	if ((flags == 0) || (cell->state == UNK))
		return OK;

	DPRINTF1("Implication flags %x\n", flags);

	if ((flags & N0IC0) && (cell->state == OFF) &&
		(setcell(prevcell, OFF, FALSE) != OK))
			return ERROR;

	if ((flags & N1IC1) && (cell->state == ON) &&
		(setcell(prevcell, ON, FALSE) != OK))
			return ERROR;

	state = UNK;
	if (((flags & N0ICUN0) && (cell->state == OFF))
		|| ((flags & N1ICUN0) && (cell->state == ON)))
			state = OFF;

	if (((flags & N0ICUN1) && (cell->state == OFF))
		|| ((flags & N1ICUN1) && (cell->state == ON)))
			state = ON;

	if (state == UNK) {
		DPRINTF0("Implications successful\n");
		return OK;
	}

	/*
	 * For each unknown neighbor, set its state as indicated.
	 * Return an error if any neighbor is inconsistent.
	 */
	DPRINTF4("Forcing unknown neighbors of cell %d %d %d %s\n",
		prevcell->row, prevcell->col, prevcell->gen,
		((state == ON) ? "on" : "off"));

	if ((prevcell->cul->state == UNK) &&
		(setcell(prevcell->cul, state, FALSE) != OK))
			return ERROR;

	if ((prevcell->cu->state == UNK) &&
		(setcell(prevcell->cu, state, FALSE) != OK))
			return ERROR;

	if ((prevcell->cur->state == UNK) &&
		(setcell(prevcell->cur, state, FALSE) != OK))
			return ERROR;

	if ((prevcell->cl->state == UNK) &&
		(setcell(prevcell->cl, state, FALSE) != OK))
			return ERROR;

	if ((prevcell->cr->state == UNK) &&
		(setcell(prevcell->cr, state, FALSE) != OK))
			return ERROR;

	if ((prevcell->cdl->state == UNK) &&
		(setcell(prevcell->cdl, state, FALSE) != OK))
			return ERROR;

	if ((prevcell->cd->state == UNK) &&
		(setcell(prevcell->cd, state, FALSE) != OK))
			return ERROR;

	if ((prevcell->cdr->state == UNK) &&
		(setcell(prevcell->cdr, state, FALSE) != OK))
			return ERROR;

	DPRINTF0("Implications successful\n");

	return OK;
}


/*
 * See if a cell and its neighbors are consistent with the cell and its
 * neighbors in the next generation.
 */
static STATUS
consistify10(cell)
	CELL	*cell;
{
	if (consistify(cell) != OK)
		return ERROR;

	cell = cell->future;
	if (consistify(cell) != OK)
		return ERROR;
	if (consistify(cell->cul) != OK)
		return ERROR;
	if (consistify(cell->cu) != OK)
		return ERROR;
	if (consistify(cell->cur) != OK)
		return ERROR;
	if (consistify(cell->cl) != OK)
		return ERROR;
	if (consistify(cell->cr) != OK)
		return ERROR;
	if (consistify(cell->cdl) != OK)
		return ERROR;
	if (consistify(cell->cd) != OK)
		return ERROR;
	if (consistify(cell->cdr) != OK)
		return ERROR;
	return OK;
}


/*
 * Examine the next choice of cell settings.
 */
static STATUS
examinenext()
{
	CELL	*cell;

	/*
	 * If there are no more cells to examine, then what we have
	 * is consistent.
	 */
	if (nextset == newset)
		return CONSISTENT;

	/*
	 * Get the next cell to examine, and check it out for symmetry
	 * and for consistency with its previous and next generations.
	 */
	cell = *nextset++;

	DPRINTF4("Examining saved cell %d %d %d (%s) for consistency\n",
		cell->row, cell->col, cell->gen,
		(cell->free ? "free" : "forced"));

	if ((rowsym || colsym) &&
		(setcell(cell->csym, cell->state, FALSE) != OK))
			return ERROR;

	return consistify10(cell);
}


/*
 * Set a cell to the specified value and determine all consequences we
 * can from the choice.  Consequences are a contradiction or a consistency.
 */
STATUS
proceed(cell, state, free)
	CELL	*cell;
	STATE	state;
	BOOL	free;
{
	int	status;

	if (setcell(cell, state, free) != OK)
		return ERROR;

	for (;;) {
		status = examinenext();
		if (status == ERROR)
			return ERROR;
		if (status == CONSISTENT)
			return OK;
	}
}


/*
 * Back up the list of set cells to undo choices.
 * Returns the cell which is to be tried for the other possibility.
 * Returns NULL_CELL on an "object cannot exist" error.
 */
static CELL *
backup()
{
	CELL	*cell;

	searchlist = fullsearchlist;

	while (newset != baseset) {
		cell = *--newset;

		DPRINTF5("backing up cell %d %d %d, was %s, %s\n",
			cell->row, cell->col, cell->gen,
			((cell->state == ON) ? "on" : "off"),
			(cell->free ? "free": "forced"));

		if ((cell->state == ON) && (cell->gen == 0))
			cellcount--;

		if (!cell->free) {
			cell->state = UNK;
			cell->free = TRUE;
			continue;
		}
		nextset = newset;
		return cell;
	}
	nextset = baseset;
	return NULL_CELL;
}


/*
 * Do checking based on setting the specified cell.
 * Returns ERROR if an inconsistency was found.
 */
static STATUS
go(cell, state, free)
	CELL	*cell;
	STATE	state;
	BOOL	free;
{
	STATUS	status;

	for (;;) {
		status = proceed(cell, state, free);
		if (status == OK)
			return OK;

		cell = backup();
		if (cell == NULL_CELL)
			return ERROR;

		free = FALSE;
		state = 1 - cell->state;
		cell->state = UNK;
	}
}


/*
 * Find another unknown cell.
 * Returns NULL_CELL if there are no more unknown cells.
 */
static CELL *
getunknown()
{
	register CELL 	*cell;

	for (cell = searchlist; cell; cell = cell->search) {
		if (cell->state == UNK) {
			searchlist = cell;
			return cell;
		}
	}
	return NULL_CELL;
}


/*
 * Choose a state for an unknown cell, either OFF or ON.
 * For billiard table stuff, this can be changed to choose the same state
 * as the majority of other generations.
 */
static STATE
choose(cell)
	CELL	*cell;
{
	return	OFF;
}


/*
 * The top level search routine.
 * Returns if an object is found, or is impossible.
 */
STATUS
search()
{
	CELL	*cell;
	BOOL	free;
	STATE	state;

	cell = getunknown();
	if (cell == NULL_CELL) {
		cell = backup();
		if (cell == NULL_CELL)
			return ERROR;
		free = FALSE;
		state = 1 - cell->state;
		cell->state = UNK;
	} else {
		state = choose(cell);
		free = TRUE;
	}

	for (;;) {
		if (go(cell, state, free) != OK)
			return NOTEXIST;

		if (dumpfreq && (++dumpcount >= dumpfreq)) {
			dumpcount = 0;
			dumpstate(dumpfile);
		}

		if (viewfreq && (++viewcount >= viewfreq)) {
			viewcount = 0;
			printgen(curgen);
		}

		if (ttycheck())
			getcommands();

		cell = getunknown();
		if (cell == NULL_CELL)
			return FOUND;

		state = choose(cell);
		free = TRUE;
	}
}


/*
 * Check to see if any other generation is identical to generation 0.
 * This is used to detect and weed out all objects with subperiods.
 * (For example, stable objects or period 2 objects when using -g4.)
 * Returns TRUE if there is an identical generation.
 */
BOOL
subperiods()
{
	int	row;
	int	col;
	int	gen;
	CELL	*cellg0;
	CELL	*cellgn;

	for (gen = 1; gen < genmax; gen++) {
		if (genmax % gen)
			continue;
		for (row = 1; row <= rowmax; row++) {
			for (col = 1; col <= colmax; col++) {
				cellg0 = findcell(row, col, 0);
				cellgn = findcell(row, col, gen);
				if (cellg0->state != cellgn->state)
					goto nextgen;
			}
		}
		return TRUE;
nextgen:;
	}
	return FALSE;
}


/*
 * Return a cell which is symmetric to the given cell.
 * It is not necessary to know all symmetric cells to a single cell,
 * as long as all symmetric cells are chained in a loop.  Thus a single
 * pointer is good enough even for the case of both row and column symmetry.
 * Returns NULL_CELL if there is no symmetry.
 */
static CELL *
symcell(cell)
	CELL	*cell;
{
	int	row;
	int	col;
	int	nrow;
	int	ncol;

	if (!rowsym && !colsym)
		return NULL_CELL;

	row = cell->row;
	col = cell->col;
	nrow = rowmax + 1 - row;
	ncol = colmax + 1 - col;

	/*
	 * If there is symmetry on only one axis, then this is easy.
	 */
	if (!colsym)
		return findcell(nrow, col, cell->gen);

	if (!rowsym)
		return findcell(row, ncol, cell->gen);

	/*
	 * Here is there is both row and column symmetry.
	 * First see if the cell is in the middle row or middle column,
	 * and if so, then this is easy.
	 */
	if ((nrow == row) || (ncol == col))
		return findcell(nrow, ncol, cell->gen);

	/*
	 * The cell is really in one of the four quadrants, and therefore
	 * has four cells making up the symmetry.  Link this cell to the
	 * symmetrical cell in the next quadrant clockwise.
	 */
	if ((row < nrow) == (col < ncol))
		return findcell(row, ncol, cell->gen);
	else
		return findcell(nrow, col, cell->gen);
}


/*
 * Link a cell to its eight neighbors in the same generation, and also
 * link those neighbors back to this cell.
 */
static void
linkcell(cell)
	CELL	*cell;
{
	int	row;
	int	col;
	int	gen;
	CELL	*paircell;

	row = cell->row;
	col = cell->col;
	gen = cell->gen;

	paircell = findcell(row - 1, col - 1, gen);
	cell->cul = paircell;
	paircell->cdr = cell;

	paircell = findcell(row - 1, col, gen);
	cell->cu = paircell;
	paircell->cd = cell;

	paircell = findcell(row - 1, col + 1, gen);
	cell->cur = paircell;
	paircell->cdl = cell;

	paircell = findcell(row, col - 1, gen);
	cell->cl = paircell;
	paircell->cr = cell;

	paircell = findcell(row, col + 1, gen);
	cell->cr = paircell;
	paircell->cl = cell;

	paircell = findcell(row + 1, col - 1, gen);
	cell->cdl = paircell;
	paircell->cur = cell;

	paircell = findcell(row + 1, col, gen);
	cell->cd = paircell;
	paircell->cu = cell;

	paircell = findcell(row + 1, col + 1, gen);
	cell->cdr = paircell;
	paircell->cul = cell;
}


/*
 * Find a cell given its coordinates.
 * Most coordinates range from 0 to colmax+1, 0 to rowmax+1, and 0 to genmax-1.
 * Cells within this range are quickly found by indexing into celltable.
 * Cells outside of this range are handled by searching an auxillary table,
 * and are dynamically created as necessary.
 */
CELL *
findcell(row, col, gen)
{
	register CELL	*cell;
	int		i;

	/*
	 * If the cell is a normal cell, then we know where it is.
	 */
	if ((row >= 0) && (row <= rowmax + 1) &&
		(col >= 0) && (col <= colmax + 1) &&
		(gen >= 0) && (gen < genmax))
	{
		return celltable[(col * (rowmax + 2) + row) * genmax + gen];
	}

	/*
	 * See if the cell is already allocated in the auxillary table.
	 */
	for (i = 0; i < auxcellcount; i++) {
		cell = auxtable[i];
		if ((cell->row == row) && (cell->col == col) &&
			(cell->gen == gen))
				return cell;
	}

	/*
	 * Need to allocate the cell and add it to the auxillary table.
	 */
	cell = allocatecell();
	cell->row = row;
	cell->col = col;
	cell->gen = gen;

	auxtable[auxcellcount++] = cell;

	return cell;
}


/*
 * Allocate a new cell.
 * The cell is initialized as if it was a boundary cell.
 * Warning: The first allocation MUST be of the deadcell.
 */
static CELL *
allocatecell()
{
	CELL	*cell;

	/*
	 * Allocate a new chunk of cells if there are none left.
	 */
	if (newcellcount <= 0) {
		newcells = (CELL *) malloc(sizeof(CELL) * ALLOCSIZE);
		if (newcells == NULL) {
			ttyclose();
			fprintf(stderr, "Cannot allocate cell structure\n");
			exit(1);
		}
		newcellcount = ALLOCSIZE;
	}
	newcellcount--;
	cell = newcells++;

	/*
	 * If this is the first allocation, then make deadcell be this cell.
	 */
	if (deadcell == NULL)
		deadcell = cell;

	/*
	 * Fill in the cell as if it was a boundary cell.
	 */
	cell->state = OFF;
	cell->free = FALSE;
	cell->gen = -1;
	cell->row = -1;
	cell->col = -1;
	cell->past = deadcell;
	cell->future = deadcell;
	cell->cul = deadcell;
	cell->cu = deadcell;
	cell->cur = deadcell;
	cell->cl = deadcell;
	cell->cr = deadcell;
	cell->cdl = deadcell;
	cell->cd = deadcell;
	cell->cdr = deadcell;
	cell->csym = deadcell;

	return cell;
}


/*
 * Initialize the implication table.
 */
static void
initimplic()
{
	STATE	state;
	int	OFFcount;
	int	ONcount;
	int	sum;
	int	desc;
	int	i;

	for (i = 0; i < NSTATES; i++) {
		state = states[i];
		for (OFFcount = 8; OFFcount >= 0; OFFcount--) {
			for (ONcount = 0; ONcount + OFFcount <= 8; ONcount++) {
				sum = ONcount + (8 - ONcount - OFFcount) * UNK;
				desc = sumtodesc(state, sum);
				implic[desc] =
					implication(state, OFFcount, ONcount);
			}
		}
	}
}


/*
 * Initialize the transition table.
 */
static void
inittransit()
{
	int	state;
	int	OFFcount;
	int	ONcount;
	int	sum;
	int	desc;
	int	i;

	for (i = 0; i < NSTATES; i++) {
		state = states[i];
		for (OFFcount = 8; OFFcount >= 0; OFFcount--) {
			for (ONcount = 0; ONcount + OFFcount <= 8; ONcount++) {
				sum = ONcount + (8 - ONcount - OFFcount) * UNK;
				desc = sumtodesc(state, sum);
				transit[desc] =
					transition(state, OFFcount, ONcount);
			}
		}
	}
}


/*
 * Determine the transition of a cell depending on its known neighbor counts.
 * The unknown neighbor count is implicit since there are eight neighbors.
 */
static STATE
transition(state, OFFcount, ONcount)
	STATE		state;
	unsigned int	OFFcount;
	unsigned int	ONcount;
{
	switch (state) {
		case OFF:
			if (OFFcount > 5)
				return OFF;
			if (ONcount > 3)
				return OFF;
			if ((ONcount == 3) && (OFFcount == 5))
				return ON;
			return UNK;

		case ON:
			if (ONcount > 3)
				return OFF;
			if (OFFcount > 6)
				return OFF;
			if ((ONcount == 2) &&
				((OFFcount == 5) || (OFFcount == 6)))
					return ON;
			if ((ONcount == 3) && (OFFcount == 5))
				return ON;
			return UNK;

		case UNK:
			if (ONcount > 3)
				return OFF;
			if (OFFcount > 6)
				return OFF;
			if ((ONcount == 3) && (OFFcount == 5))
				return ON;
			return UNK;

		default:
			return UNK;
	}
}


/*
 * Determine the implications of a cell depending on its known neighbor counts.
 * The unknown neighbor count is implicit since there are eight neighbors.
 */
static FLAGS
implication(state, OFFcount, ONcount)
	STATE		state;
	unsigned int	OFFcount;
	unsigned int	ONcount;
{
	unsigned int	UNKcount;
	FLAGS		flags;

	UNKcount = 8 - OFFcount - ONcount;
	flags = 0;
	if (ONcount == 3)
		flags |= N1ICUN0;
	if ((ONcount == 3) && (UNKcount == 1))
		flags |= N0ICUN1;

	switch (state) {
		case OFF:
			if ((ONcount == 2) && (UNKcount == 1))
				flags |= N0ICUN0;
			if (OFFcount == 5)
				flags |= N1ICUN1;
			break;

		case ON:
			if ((OFFcount == 4) && (ONcount == 2))
				flags |= N0ICUN1;
			if ((OFFcount == 5) && (ONcount == 1))
				flags |= N0ICUN0;
			if (OFFcount == 6)
				flags |= N1ICUN1;
			if ((ONcount == 1) && (UNKcount == 1))
				flags |= N0ICUN0;
			break;

		case UNK:
			if (OFFcount == 6)
				flags |= (N1ICUN1 | N1IC1);
			if ((ONcount == 2) && (UNKcount == 0))
				flags |= (N0IC0 | N1IC1);
			if ((ONcount == 2) && (OFFcount == 5))
				flags |= (N0IC0 | N0ICUN0);
			break;
	}
	if (UNKcount == 0)
		flags &= ~(N0ICUN0 | N0ICUN1 | N1ICUN0 | N1ICUN1);
	return flags;
}

/* END CODE */
