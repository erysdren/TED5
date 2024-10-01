
#include <ctype.h>
#include "igrab.h"
#pragma hdrstop

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
