
#ifndef _BORLAND_H_
#define _BORLAND_H_

#define FP_OFF(p) p
#define FP_SEG(p) p

void settext(void);
void nosound(void);
void outport(int port, int value);

#ifndef _MSC_VER
char *strupr(char *s);
char *itoa(int value, char *string, int radix);
#endif

#endif /* _BORLAND_H_ */
