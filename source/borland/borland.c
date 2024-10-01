
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#include "borland.h"

/*
 * compatibility routines for the Borland C standard library
 */

void randomize(void)
{
	srand(time(NULL));
}

void settext(void)
{
	/* stub */
}

void nosound(void)
{
	/* stub */
}

void outport(int port, int value)
{
	/* stub */
}

#ifndef _MSC_VER

char *strupr(char *s)
{
	for (char *p = s; *p; p++)
		*p = toupper((unsigned char)*p);
	return s;
}

char *itoa(int value, char *string, int radix)
{
	switch (radix)
	{
		case 10:
			sprintf(string, "%d", value);
			break;

		case 16:
			sprintf(string, "%x", value);
			break;
	}

	return string;
}

#endif
