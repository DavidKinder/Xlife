/*
 * Smart terminal output routine.
 * Uses curses to display the object.
 */
#include <signal.h>
#include <curses.h>
#include <varargs.h>


#define	STATUSLINE	22
#define	INPUTLINE	23


static	int	inputready;

static void	gotinput();
extern void	ttywrite();


/*
 * Open the terminal and enable for detecting terminal input.
 */
ttyopen()
{
    struct sgttyb s;

    initscr();

    signal(SIGINT, gotinput);

    /*
     * Get terminal modes.
     */
    ioctl(2, TIOCGETP, &s);
    
    /*
     * Set the modes to the way we want them.
     */
    s.sg_flags |= CBREAK;
    s.sg_flags &= ~(ECHO|XTABS);

    ioctl(2, TIOCSETN, &s);

    return 0;
}


static void
gotinput()
{
	signal(SIGINT, gotinput);
	inputready = 1;
}


/*
 * Close the terminal.
 */
void
ttyclose()
{
	refresh();
	endwin();
}


/*
 * Test to see if a keyboard character is ready.
 * Returns nonzero if so (and clears the ready flag).
 */
ttycheck()
{
	int	result;

	result = inputready;
	inputready = 0;
	return result;
}


/*
 * Print a formatted string to the terminal.
 * The string length is limited to 256 characters.
 */
void
ttyprintf(char *fmt, ...)
{

    va_list ap;
    static char	buf[256];

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    addstr(buf);
}


/*
 * Print a status line, similar to printf.
 * The string length is limited to 256 characters.
 */
void
ttystatus(char *fmt, ...)
{
	va_list ap;
	static char	buf[256];

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	move(STATUSLINE, 0);
	addstr(buf);
	clrtobot();
	move(0, 0);
	refresh();
}


void
ttywrite(cp, len)
	char	*cp;
{
	while (len-- > 0) {
		addch(*cp);
		cp++;
	}
}


void
ttyhome()
{
	move(0, 0);
}


void
ttyeeop()
{
	clrtobot();
}


void
ttyflush()
{
	refresh();
}


/*
 * Return a NULL terminated input line (without the final newline).
 * The specified string is printed as a prompt.
 * Returns nonzero (and an empty buffer) on EOF or error.
 */
ttyread(prompt, buf, buflen)
	char	*prompt;
	char	*buf;
{
	int	c;
	char	*cp;

	move(INPUTLINE, 0);
	addstr(prompt);
	clrtoeol();

	cp = buf;
	for (;;)
	{
		refresh();
		c = fgetc(stdin);
		switch (c)
		{
		case EOF:
			buf[0] = '\0';
			move(INPUTLINE, 0);
			clrtoeol();
			move(0, 0);
			refresh();
			return -1;

		default:
			*cp++ = c;
			addch(c);
			if (cp < buf + buflen - 1)
				break;
			/* fall through... */

		case '\n':
		case '\r':
			*cp = 0;
			move(INPUTLINE, 0);
			clrtoeol();
			move(0, 0);
			refresh();
			return 0;

		case '\b':
			if (cp == buf)
				break;
			--cp;
			addch('\b');
			clrtoeol();
			break;
		}
	}
}

/* END CODE */
