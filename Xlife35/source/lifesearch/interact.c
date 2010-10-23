/*
 * Life search program - user interactions module.
 * Author: David I. Bell.
 */

#include "lifesrc.h"


/*
 * Local procedures
 */
static void	usage();
static void	getsetting();
static void	clearunknowns();
static void	writegen();
static STATUS	loadstate();
static STATUS	readfile();
static BOOL	confirm();
static long	getnum();
static char	*getstr();


main(argc, argv)
	char	**argv;
{
	char	*str;
	STATUS	status;

	if (--argc <= 0) {
		usage();
		exit(1);
	}
	argv++;

	while (argc-- > 0) {
		str = *argv++;
		if (*str++ != '-') {
			usage();
			exit(1);
		}
		switch (*str++) {
			case 'r':			/* rows */
				rowmax = atoi(str);
				break;

			case 'c':			/* columns */
				colmax = atoi(str);
				break;

			case 'g':			/* generations */
				genmax = atoi(str);
				break;

			case 't':			/* translation */
				switch (*str++) {
					case 'r':
						rowtrans = atoi(str);
						break;
					case 'c':
						coltrans = atoi(str);
						break;
					default:
						fprintf(stderr, "Bad translate\n");
						exit(1);
				}
				break;

			case 's':			/* symmetry */
				switch (*str++) {
					case 'r':
						rowsym = TRUE;
						break;
					case 'c':
						colsym = TRUE;
						break;
					default:
						fprintf(stderr, "Bad symmetry\n");
						exit(1);
				}
				break;

			case 'd':			/* dump frequency */
				dumpfreq = atol(str) * DUMPMULT;
				dumpfile = DUMPFILE;
				if ((argc > 0) && (**argv != '-')) {
					argc--;
					dumpfile = *argv++;
				}
				break;

			case 'v':			/* view frequency */
				viewfreq = atol(str) * VIEWMULT;
				break;

			case 'l':			/* load file */
				if (*str == 'n')
					nowait = TRUE;
				if ((argc <= 0) || (**argv == '-')) {
					fprintf(stderr, "Missing load file name\n");
					exit(1);
				}
				loadfile = *argv++;
				argc--;
				break;

			case 'i':			/* initial file */
				if (*str == 'a')
					setall = TRUE;
				if ((argc <= 0) || (**argv == '-')) {
					fprintf(stderr, "Missing initial file name\n");
					exit(1);
				}
				initfile = *argv++;
				argc--;
				break;

			case 'o':			/* output file */
				if ((argc <= 0) || (**argv == '-')) {
					fprintf(stderr, "Missing output file name\n");
					exit(1);
				}
				outputfile = *argv++;
				argc--;
				break;

			case 'm':			/* max cell count */
				maxcount = atoi(str);
				break;

			case 'p':			/* find parents only */
				parent = TRUE;
				break;

			case 'a':
				allobjects = TRUE;	/* find all objects */
				break;

			case 'D':			/* debugging output */
				debug = TRUE;
				break;

			default:
				fprintf(stderr, "Unknown option -%c\n",
					str[-1]);
				exit(1);
		}
	}

	if (parent && (rowtrans || coltrans)) {
		fprintf(stderr, "Cannot specify -tr or -tc with -p\n");
		exit(1);
	}

	if (ttyopen()) {
		fprintf(stderr, "Cannot initialize terminal\n");
		exit(1);
	}

	/*
	 * Check for loading state from file or reading initial
	 * object from file.
	 */
	if (loadfile) {
		if (loadstate(loadfile) != OK) {
			ttyclose();
			exit(1);
		}
	} else {
		initcells();
		if (initfile) {
			if (readfile(initfile) != OK) {
				ttyclose();
				exit(1);
			}
			baseset = nextset;
		}
	}

	/*
	 * If we are looking for parents, then set the current generation
	 * to the last one so that it can be input easily.  Then get the
	 * commands to initialize the cells, unless we were told to not wait.
	 */
	if (parent)
		curgen = genmax - 1;

	if (nowait)
		printgen(0);
	else
		getcommands();

	/*
	 * Initial commands are complet, now look for the object.
	 */
	while (TRUE) {
		if (curstatus == OK)
			curstatus = search();

		if (dumpfreq) {
			dumpcount = 0;
			dumpstate(dumpfile);
		}

		if ((curstatus == FOUND) && !allobjects && subperiods()) {
			curstatus = OK;
			continue;
		}

		curgen = 0;

		if (outputfile == NULL) {
			getcommands();
			continue;
		}

		/*
		 * Here if results are going to a file.
		 */
		if (curstatus == FOUND) {
			curstatus = OK;
			printgen(0);
			ttystatus("Object %ld found.\n", ++foundcount);
			writegen(outputfile, TRUE);
			continue;
		}

		if (foundcount == 0) {
			ttyclose();
			fprintf(stderr, "No objects found\n");
			exit(1);
		}

		ttyclose();
		printf("Search completed, file \"%s\" contains %ld object%s\n",
			outputfile, foundcount, (foundcount == 1) ? "" : "s");
		exit(0);
	}
}


/*
 * Get one or more user commands.
 * Commands are ended by a blank line.
 */
void
getcommands()
{
	char	*cp;
	char	*cmd;
	char	buf[LINESIZE];

	dumpcount = 0;
	viewcount = 0;
	printgen(curgen);

	while (TRUE) {
		ttyread("> ", buf, LINESIZE);
		cp = buf;
		while (isblank(*cp))
			cp++;
		cmd = cp;
		if (*cp)
			cp++;
		while (isblank(*cp))
			cp++;

		switch (*cmd) {
			case 'p':		/* print previous generation */
				printgen((curgen + genmax - 1) % genmax);
				break;

			case 'n':		/* print next generation */
				printgen((curgen + 1) % genmax);
				break;

			case 's':		/* add a cell setting */
				getsetting(cp);
				break;

			case 'c':		/* clear rest of generation */
				if (confirm("Clear all unknown cells? "))
					clearunknowns();
				break;

			case 'v':		/* set view frequency */
				viewfreq = atol(cp) * VIEWMULT;
				printgen(curgen);
				break;

			case 'w':		/* write generation to file */
				writegen(cp, FALSE);
				break;

			case 'd':		/* dump state to file */
				dumpstate(cp);
				break;

			case 'N':		/* find next object */
				if (curstatus == FOUND)
					curstatus = OK;
				return;

			case 'q':		/* quit program */
			case 'Q':
				if (confirm("Really quit? ")) {
					ttyclose();
					exit(0);
				}
				break;

			case '\n':		/* return from commands */
			case '\0':
				return;

			default:
				if (isdigit(*cmd)) {
					getsetting(cmd);
					break;
				}
				ttystatus("Unknown command\n");
				break;
		}
	}
}


/*
 * Get a cell to be set in the current generation.
 * The state of the cell is defaulted to ON.
 * Warning: Use of this routine invalidates backing up over
 * the setting, so that the setting is permanent.
 */
static void
getsetting(cp)
	char	*cp;
{
	int	row;
	int	col;
	STATE	state;

	cp = getstr(cp, "Cell to set (row col [state]): ");
	if (*cp == '\0')
		return;

	row = getnum(&cp, -1);
	if (*cp == ',')
		cp++;
	col = getnum(&cp, -1);
	if (*cp == ',')
		cp++;
	state = getnum(&cp, 1);

	while (isblank(*cp))
		cp++;
	if (*cp != '\0') {
		ttystatus("Bad input line format\n");
		return;
	}

	if ((row <= 0) || (row > rowmax) || (col <= 0) || (col > colmax) ||
		((state != 0) && (state != 1)))
	{
		ttystatus("Illegal cell value\n");
		return;
	}

	if (proceed(findcell(row, col, curgen), state, FALSE) != OK) {
		ttystatus("Inconsistent state for cell\n");
		return;
	}

	baseset = nextset;
	printgen(curgen);
}


/*
 * Clear all remaining unknown cells in the current generation.
 * This is used when searching for parents of a configuration.
 */
static void
clearunknowns()
{
	int	row;
	int	col;
	CELL	*cell;

	for (row = 1; row <= rowmax; row++) {
		for (col = 1; col <= colmax; col++) {
			cell = findcell(row, col, curgen);
			if (cell->state != UNK)
				continue;

			if (proceed(cell, OFF, FALSE) != OK)
			{
				ttystatus("Inconsistent state for cell\n");
				return;
			}
		}
	}

	baseset = nextset;
	printgen(curgen);
}


/*
 * Print out the current status of the specified generation.
 * This also sets the current generation.
 */
void
printgen(gen)
{
	int	row;
	int	col;
	int	count;
	CELL	*cell;
	char	*msg;

	curgen = gen;

	switch (curstatus) {
		case NOTEXIST:	msg = "No such object"; break;
		case FOUND:	msg = "Found object"; break;
		default:	msg = ""; break;
	}

	count = 0;
	for (row = 1; row <= rowmax; row++) {
		for (col = 1; col <= colmax; col++) {
			count += (findcell(row, col, gen)->state == ON);
		}
	}

	ttyhome();
	ttyeeop();

	ttyprintf("%s (gen %d, cells %d) -g%d", msg, gen, count, genmax);
	if (rowtrans)
		ttyprintf(" -tr%d", rowtrans);
	if (coltrans)
		ttyprintf(" -tc%d", coltrans);
	if (rowsym)
		ttyprintf(" -sr");
	if (colsym)
		ttyprintf(" -sc");
	if (parent)
		ttyprintf(" -p");
	if (allobjects)
		ttyprintf(" -a");
	if (maxcount)
		ttyprintf(" -m%d", maxcount);
	if (viewfreq)
		ttyprintf(" -v%ld", viewfreq / VIEWMULT);
	if (dumpfreq)
		ttyprintf(" -d%ld %s", dumpfreq / DUMPMULT, dumpfile);
	if (outputfile)
		ttyprintf(" -o %s", outputfile);
	ttyprintf("\n");

	for (row = 1; row <= rowmax; row++) {
		for (col = 1; col <= colmax; col++) {
			cell = findcell(row, col, gen);
			switch (cell->state) {
				case OFF:	msg = ". "; break;
				case ON:	msg = "O "; break;
				case UNK:	msg = "? "; break;
			}

			/*
			 * If wide output, print only one character,
			 * else print both characters.
			 */
			ttywrite(msg, (colmax < 40) + 1);
		}
		ttywrite("\n", 1);
	}

	ttyhome();
	ttyflush();
}


/*
 * Write the current generation to the specified file.
 * Empty rows and columns are not written.
 * If no file is specified, it is asked for.
 */
static void
writegen(file, append)
	char	*file;		/* file name (or NULL) */
	BOOL	append;		/* TRUE to append instead of create */
{
	FILE	*fp;
	CELL	*cell;
	int	row;
	int	col;
	int	ch;
	int	minrow, maxrow, mincol, maxcol;

	file = getstr(file, "Write object to file: ");
	if (*file == '\0')
		return;

	fp = fopen(file, append ? "a" : "w");
	if (fp == NULL) {
		ttystatus("Cannot create \"%s\"\n", file);
		return;
	}

	/*
	 * First find the minimum bounds on the object.
	 */
	minrow = rowmax;
	mincol = colmax;
	maxrow = 1;
	maxcol = 1;

	for (row = 1; row <= rowmax; row++) {
		for (col = 1; col <= colmax; col++) {
			cell = findcell(row, col, curgen);
			if (cell->state == OFF)
				continue;
			if (row < minrow)
				minrow = row;
			if (row > maxrow)
				maxrow = row;
			if (col < mincol)
				mincol = col;
			if (col > maxcol)
				maxcol = col;
		}
	}

	if (minrow > maxrow) {
		minrow = 1;
		maxrow = 1;
		mincol = 1;
		maxcol = 1;
	}

	/*
	 * Now write out the bounded area.
	 */
	for (row = minrow; row <= maxrow; row++) {
		for (col = mincol; col <= maxcol; col++) {
			cell = findcell(row, col, curgen);
			switch (cell->state) {
				case OFF:	ch = '.'; break;
				case ON:	ch = '*'; break;
				case UNK:	ch = '?'; break;
			}
			fputc(ch, fp);
		}
		fputc('\n', fp);
	}

	if (append)
		fprintf(fp, "\n");

	if (fclose(fp))
		ttystatus("Error writing \"%s\"\n", file);
	else
		ttystatus("\"%s\" written\n", file);
}


/*
 * Dump the current state of the search in the specified file.
 * If no file is specified, it is asked for.
 */
void
dumpstate(file)
	char	*file;
{
	FILE	*fp;
	CELL	**set;
	CELL	*cell;

	file = getstr(file, "Dump state to file: ");
	if (*file == '\0')
		return;

	fp = fopen(file, "w");
	if (fp == NULL) {
		ttystatus("Cannot create \"%s\"\n", file);
		return;
	}

	fprintf(fp, "V %d\n", DUMPVERSION);
	fprintf(fp, "I %d %d %d %d %d %d %d %d %d %d %d %d\n", rowmax, colmax,
		genmax, rowtrans, coltrans, rowsym, colsym, parent,
		allobjects, maxcount, cellcount, curstatus);

	set = settable;
	while (set != nextset) {
		cell = *set++;
		fprintf(fp, "S %d %d %d %d %d\n", cell->row, cell->col,
			cell->gen, cell->state, cell->free);
	}

	fprintf(fp, "T %d %d\n", baseset - settable, nextset - settable);
	fprintf(fp, "E\n");

	if (fclose(fp)) {
		ttystatus("Error writing \"%s\"\n", file);
		return;
	}

	ttystatus("State dumped to \"%s\"\n", file);
}


/*
 * Load a previously dumped state from a file.
 * Warning: Almost no checks are made for validity of the state.
 * Returns OK on success, ERROR on failure.
 */
static STATUS
loadstate(file)
	char	*file;
{
	FILE	*fp;
	char	*cp;
	int	row;
	int	col;
	int	gen;
	STATE	state;
	BOOL	free;
	CELL	*cell;
	char	buf[LINESIZE];

	file = getstr(file, "Load state from file: ");
	if (*file == '\0')
		return OK;

	fp = fopen(file, "r");
	if (fp == NULL) {
		ttystatus("Cannot open state file \"%s\"\n", file);
		return ERROR;
	}

	buf[0] = '\0';
	fgets(buf, LINESIZE, fp);
	if (buf[0] != 'V') {
		ttystatus("Missing version line in file \"%s\"\n", file);
		fclose(fp);
		return ERROR;
	}

	cp = &buf[1];
	if (getnum(&cp, 0) != DUMPVERSION) {
		ttystatus("Unknown version in state file \"%s\"\n", file);
		fclose(fp);
		return ERROR;
	}

	fgets(buf, LINESIZE, fp);
	if (buf[0] != 'I') {
		ttystatus("Missing init line in state file\n");
		fclose(fp);
		return ERROR;
	}
	cp = &buf[1];
	rowmax = getnum(&cp, 0);
	colmax = getnum(&cp, 0);
	genmax = getnum(&cp, 0);
	rowtrans = getnum(&cp, 0);
	coltrans = getnum(&cp, 0);
	rowsym = getnum(&cp, 0);
	colsym = getnum(&cp, 0);
	parent = getnum(&cp, 0);
	allobjects = getnum(&cp, 0);
	maxcount = getnum(&cp, 0);
	cellcount = getnum(&cp, 0);
	curstatus = getnum(&cp, 0);

	initcells();

	newset = settable;
	for (;;) {
		buf[0] = '\0';
		fgets(buf, LINESIZE, fp);
		if (buf[0] != 'S')
			break;

		cp = &buf[1];
		row = getnum(&cp, 0);
		col = getnum(&cp, 0);
		gen = getnum(&cp, 0);
		state = getnum(&cp, 0);
		free = getnum(&cp, 0);

		cell = findcell(row, col, gen);
		cell->state = state;
		cell->free = free;
		*newset++ = cell;
	}

	if (buf[0] != 'T') {
		ttystatus("Missing table line in state file\n");
		fclose(fp);
		return ERROR;
	}
	cp = &buf[1];
	baseset = &settable[getnum(&cp, 0)];
	nextset = &settable[getnum(&cp, 0)];

	fgets(buf, LINESIZE, fp);
	if (buf[0] != 'E') {
		ttystatus("Missing end of file line in state file\n");
		fclose(fp);
		return ERROR;
	}

	if (fclose(fp)) {
		ttystatus("Error reading \"%s\"\n", file);
		return ERROR;
	}

	ttystatus("State loaded from \"%s\"\n", file);
	return OK;
}


/*
 * Read a file containing initial settings for either gen 0 or the last gen.
 * If setall is TRUE, both the ON and the OFF cells will be set.
 * Returns OK on success, ERROR on error.
 */
static STATUS
readfile(file)
	char	*file;
{
	FILE	*fp;
	char	*cp;
	char	ch;
	int	row;
	int	col;
	int	gen;
	STATE	state;
	char	buf[LINESIZE];

	file = getstr(file, "Read initial object from file: ");
	if (*file == '\0')
		return OK;

	fp = fopen(file, "r");
	if (fp == NULL) {
		ttystatus("Cannot open \"%s\"\n", file);
		return ERROR;
	}

	gen = (parent ? (genmax - 1) : 0);
	row = 0;
	while (fgets(buf, LINESIZE, fp)) {
		row++;
		cp = buf;
		col = 0;
		while (*cp && (*cp != '\n')) {
			col++;
			ch = *cp++;
			switch (ch) {
				case '?':
					continue;
				case '.':
				case ' ':
					if (!setall)
						continue;
					state = OFF;
					break;
				case 'O':
				case 'o':
				case '*':
					state = ON;
					break;
				default:
					ttystatus("Bad file format in line %d\n",
						row);
					fclose(fp);
					return ERROR;
			}

			if (proceed(findcell(row, col, gen), state, FALSE)
				!= OK)
			{
				ttystatus("Inconsistent state for cell %d %d\n",
					row, col);
				fclose(fp);
				return ERROR;
			}
		}
	}

	if (fclose(fp)) {
		ttystatus("Error reading \"%s\"\n", file);
		return ERROR;
	}
	return OK;
}


/*
 * Check a string for being NULL, and if so, ask the user to specify a
 * value for it.  Returned string may be static and thus is overwritten
 * for each call.  Leading spaces in the string are skipped over.
 */
static char *
getstr(str, prompt)
	char	*str;		/* string to check for NULLness */
	char	*prompt;	/* message to prompt with */
{
	static char	buf[LINESIZE];

	if ((str == NULL) || (*str == '\0')) {
		ttyread(prompt, buf, LINESIZE);
		str = buf;
	}
	while (isblank(*str))
		str++;
	return str;
}


/*
 * Confirm an action by prompting with the specified string and reading
 * an answer.  Entering 'y' or 'Y' indicates TRUE, everything else FALSE.
 */
static BOOL
confirm(prompt)
	char	*prompt;
{
	char	ch;

	ch = *getstr(NULL, prompt);
	if ((ch == 'y') || (ch == 'Y'))
		return TRUE;
	return FALSE;
}


/*
 * Read a number from a string, eating any leading or trailing blanks.
 * Returns the value, and indirectly updates the string pointer.
 * Returns specified default if no number was found.
 */
static long
getnum(cpp, defnum)
	char	**cpp;
	int	defnum;
{
	char	*cp;
	long	num;

	cp = *cpp;
	while (isblank(*cp))
		cp++;
	if (!isdigit(*cp)) {
		*cpp = cp;
		return defnum;
	}
	num = 0;
	while (isdigit(*cp))
		num = num * 10 + (*cp++ - '0');
	while (isblank(*cp))
		cp++;
	*cpp = cp;
	return num;
}


/*
 * Print usage text.
 */
static void
usage()
{
	char	**cpp;
	static char *text[] = {
	"Program to search for life oscillators or spaceships.",
	"",
"lifesrc -r# -c# -g# -tr# -tc# -m# -sr -sc -p -a -v# -i[a] file -o file -d# file",
"lifesrc -l[n] file -v# -o file -d# file",
		"",
		"   -r   Number of rows",
		"   -c   Number of columns",
		"   -g   Number of generations",
		"   -tr  Translate rows between last and first generation",
		"   -tc  Translate columns between last and first generation",
		"   -m   Maximum live cells for generation 0",
		"   -sr  Enforce symmetry on rows",
		"   -sc  Enforce symmetry on columns",
		"   -p   Only look for parents of last generation",
		"   -a   Find all objects (even those with subperiods)",
		"   -v   View object every N thousand searches",
		"   -d   Dump status to file every N thousand searches",
		"   -l   Load status from file",
		"   -ln  Load status without entering command mode",
		"   -i   Read initial object from file setting only ON cells",
		"   -ia  Read initial object setting both ON and OFF cells",
		"   -o   Output found objects to file (appending)",
		NULL
	};

	for (cpp = text; *cpp; cpp++)
		fprintf(stderr, "%s\n", *cpp);
}

/* END CODE */
