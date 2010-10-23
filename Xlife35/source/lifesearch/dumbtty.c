/*
 * Dumb terminal output routine.
 * Does no cursor addressing stuff.
 */

#include <stdio.h>
#include <signal.h>
#include <varargs.h>


static	int	inputready;		/* nonzero if input now ready */

static void	gotinput();
extern void	ttywrite();


/*
 * Open the terminal and enable for detecting terminal input.
 */
ttyopen()
{
	signal(SIGINT, gotinput);
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
	ttywrite(buf, strlen(buf));
}


/*
 * Print a status message, like printf.
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
	ttywrite(buf, strlen(buf));
}


/*
 * Write the specified number of characters to the terminal.
 */
void
ttywrite(buf, count)
	unsigned char	*buf;		/* buffer to write */
	int	count;			/* number of characters */
{
	unsigned char ch;

	while (count-- > 0) {
		ch = *buf++;
		putchar(ch);
	}
}


void
ttyhome()
{
}


void
ttyeeop()
{
}


void
ttyflush()
{
	fflush(stdout);
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
	int	len;

	fputs(prompt, stdout);
	fflush(stdout);

	if (fgets(buf, buflen, stdin) == NULL) {
		buf[0] = '\0';
		return -1;
	}
	len = strlen(buf) - 1;
	if ((len >= 0) && (buf[len] == '\n'))
		buf[len] = '\0';
	return 0;
}

/* END CODE */
