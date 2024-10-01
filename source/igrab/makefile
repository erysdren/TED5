
TARGET=igrab.exe

MODEL=s

CC=wcc
LD=wlink
CFLAGS=-omaxnet -m$(MODEL)

OBJECTS=&
	dograb.obj&
	finish.obj&
	grabcga.obj&
	grabega.obj&
	grabvga.obj&
	igrab.obj&
	jhuff.obj&
	lbmload.obj&
	xms.obj&

.c.obj:
	$(CC) $(CFLAGS) -fo=$*.obj $*.c

$(TARGET): $(OBJECTS) wlink.rsp
	$(LD) @wlink.rsp

wlink.rsp: makefile
	echo name $(TARGET) >wlink.rsp
	echo system dos >>wlink.rsp
	for %i in ($(OBJS)) do echo file %i >>wlink.rsp
