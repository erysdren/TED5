////////////////////////////////////////////////////////////
//
// EGA Grabbing
//
////////////////////////////////////////////////////////////
#include "igrab.h"
#pragma hdrstop

OptStruct Optimum;

////////////////////////////////////////////////////////////
//
// EGAblit : blits an EGA shape from memory to the EGA screen
//
////////////////////////////////////////////////////////////
void EGAblit(int x,int y,int width,int height,char huge *buffer)
{
 unsigned ESreg,DIreg,SIreg,DSreg,where,oheight,owidth,addx,addy;

 if (noshow || SkipToStart)
   return;
 // Preclipping
 if (x<0 || y<0)
   return;

 if (x>39 || y>199)
   globalx=globaly=globalmaxh=x=y=0;

 addx=addy=0;
 if (x+width>39)
   {
    owidth=width;
    width=40-x;
    addx=owidth-width;
   }
 if (y+height>199)
   {
    oheight=height;
    height=200-y;
    addy=(oheight-height)*owidth;
   }

 outport(GCindex,GCmode);
 DSreg=FP_SEG(buffer);
 SIreg=FP_OFF(buffer);
 where=y*40+x;

 asm	push	ds

 asm	cld
 asm	mov	si,SIreg
 asm	mov	ax,DSreg
 asm	mov	ds,ax
 asm	mov	bx,1
 LOOP0:
 asm	mov	dx,SCindex
 asm	mov	ax,SCmapmask
 asm	mov	ah,bl
 asm	out	dx,ax		// set plane to write to

 asm	mov	di,where
 asm	mov	ax,0xa000
 asm	mov	es,ax

 asm	mov	dx,height
 LOOP1:
 asm	mov	cx,width
 asm	rep movsb
 asm	mov	cx,addx		// any to finish up horizontally?
 asm	jcxz	LOOP1a
 asm	rep lodsb

 LOOP1a:
 asm	sub	di,width
 asm	add	di,40
 asm	dec	dx
 asm	jnz	LOOP1
 asm	mov	cx,addy		// any to finish up vertically?
 asm	jcxz	LOOP1b
 asm	rep lodsb

 LOOP1b:
 asm	mov	ax,si
 asm	shr	ax,1
 asm	shr	ax,1
 asm	shr	ax,1
 asm	shr	ax,1
 asm	mov	cx,ds
 asm	add	cx,ax
 asm	mov	ds,cx
 asm	and	si,0x0f

 asm	shl	bx,1		// make sure we write to all planes!
 asm	cmp	bx,16
 asm	jb	LOOP0

 asm	pop	ds
}


////////////////////////////////////////////////////////////
//
// DoEGAblit : handles output of EGA grabs to the screen
//
////////////////////////////////////////////////////////////
void DoEGAblit(int x,int y,int width,int height)
{
 if (noshow || SkipToStart)
   return;

 if (nostacking)
   {
    globalx=x;
    globaly=y;
   }

 EGAblit(globalx,globaly,width,height,databuffer+offset);
 if (!nostacking)
   {
    globalx+=width;
    if (globalmaxh<height)
      globalmaxh=height;
    if (globalx>40-width)
      {
       globaly+=globalmaxh;
       globalx=globalmaxh=0;
      }
   }
}


////////////////////////////////////////////////////////////
//
// EGAgrab : grabs any EGA shape from the screen buffer in main memory
//           with INLINE asm for *FAST* grab!
//	     Of course, by John Romero!
//
// NOTE: I expect X & WIDTH to be in BYTE values, not pixels!
//
////////////////////////////////////////////////////////////
void EGAgrab(int x,int y,int width,int height,long offset)
{
 unsigned ESreg,DIreg,SIreg,DSreg,scrnwid;
 long off,size;


 scrnwid=CurrentLBM.width/8;
 size=scrnwid*CurrentLBM.height;	// EGA plane calculation

 for (i=0;i<4;i++)
   {
    // FROM
    off=size*i+((CurrentLBM.width/8)*y+x);
    SIreg=FP_OFF(lbmscreen)+(off&15);
    DSreg=FP_SEG(lbmscreen)+off/16;

    // TO
    ESreg=FP_SEG(databuffer)+offset/16;
    DIreg=FP_OFF(databuffer)+(offset&15);

    asm		push	si
    asm		push	di
    asm		push	ds

    asm		mov	bx,height
    asm		mov	dx,width

    asm		mov	es,ESreg
    asm		mov	di,DIreg
    asm		mov	si,SIreg
    asm		mov	ax,DSreg
    asm		mov	ds,ax
    asm		cld

    LOOP1:

    asm		mov	cx,dx
    asm		rep movsb

    asm		sub	si,dx
    asm		add	si,scrnwid
    asm		dec	bx
    asm		jnz	LOOP1

    asm		pop	ds
    asm		pop	di
    asm		pop	si

    offset+=width*height;
   }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//
// MASKED STUFF
//
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//
// EGAMblit : blits an EGA masked shape from memory to the EGA screen
//
////////////////////////////////////////////////////////////
void EGAMblit(int x,int y,int width,int height,char huge *buffer)
{
 unsigned MASKoff,DATAoff,ESreg,DIreg,SIreg,DSreg,where,oheight,owidth,addx,addy;

 if (noshow || SkipToStart)
   return;
 // Preclipping
 if (x<0 || y<0)
   return;

 addx=addy=0;
 owidth=width;
 oheight=height;

 if (x+width>39)
   {
    width=40-x;
    addx=owidth-width;
   }
 if (y+height>199)
   {
    height=200-y;
    addy=(oheight-height)*owidth;
   }

 outport(GCindex,GCmode);
 DSreg=FP_SEG(buffer);
 MASKoff=FP_OFF(buffer);
 DATAoff=FP_OFF((char far *)buffer+owidth*oheight);
 where=y*40+x;

 asm	push	si
 asm	push	di
 asm	push	ds
 asm	pushf

 asm	mov	dx,DATAoff
 asm	mov	ds,DSreg
 asm	mov	bx,1
 asm	mov	ah,0
 LOOP0:
 asm	push	dx
 asm	push	ax

 asm	mov	dx,GCindex
 asm	mov	al,GCreadmap
 asm	out	dx,ax		// set plane to read from!
 asm	mov	dx,SCindex
 asm	mov	ax,SCmapmask
 asm	mov	ah,bl
 asm	out	dx,ax		// set plane to write to!

 asm	mov	si,MASKoff
 asm	mov	di,where
 asm	mov	ax,0xa000
 asm	mov	es,ax

 asm	pop	ax
 asm	pop	dx

 asm	mov	bh,BYTE PTR height
 LOOP1:
 asm	mov	cx,width
 LOOP1c:
 asm	mov	al,[es:di]
 asm	and	al,[si]		// get mask byte (SI=mask data)
 asm	inc	si
 asm	xchg	si,dx		// SI now = EGA data
 asm	or	al,[si]
 asm	inc	si
 asm	xchg	si,dx		// SI now = MASK data
 asm	stosb
 asm	loop LOOP1c
 asm	mov	cx,addx		// any to finish up horizontally?
 asm	jcxz	LOOP1a
 asm	add	si,cx
 asm	add	dx,cx

 LOOP1a:
 asm	sub	di,width
 asm	add	di,40
 asm	dec	bh
 asm	jnz	LOOP1
 asm	mov	cx,addy		// any to finish up vertically?
 asm	jcxz	LOOP1b
 asm	add	si,cx
 asm	add	dx,cx

 LOOP1b:
 asm	inc	ah		// read from next plane
 asm	shl	bl,1		// make sure we write to all planes!
 asm	cmp	bl,16
 asm	jb	LOOP0

 asm	popf
 asm	pop	ds
 asm	pop	di
 asm	pop	si
}


////////////////////////////////////////////////////////////
//
// DoEGAMblit : handles output of masked EGA grabs to the screen
//
////////////////////////////////////////////////////////////
void DoEGAMblit(int x,int y,int width,int height,int yadd,int hadd)
{
 int i,j;

 if (noshow || SkipToStart)
   return;

 if (nostacking)
   {
    globalx=x;
    globaly=y;
    yadd=hadd=0;
   }
 else
 if (yadd || hadd)
   {
    char huge *EGAscrn=MK_FP(0xa000,0);

    outport(GCindex,GCmode);

    for (j=globaly;j<globaly+yadd;j++)
      for (i=globalx;i<globalx+width;i++)
	{
	 outport(SCindex,SCmapmask | 0xf00);
	 *(EGAscrn+j*CurrentLBM.width/8+i)=0;
	 outport(SCindex,SCmapmask | ((ScreenColor^0xf)*256));
	 *(EGAscrn+j*CurrentLBM.width/8+i)=0xff;
	}

    for (j=globaly+yadd+height;j<height+globaly+yadd+hadd;j++)
      for (i=globalx;i<globalx+width;i++)
	{
	 outport(SCindex,SCmapmask | 0xf00);
	 *(EGAscrn+j*CurrentLBM.width/8+i)=0;
	 outport(SCindex,SCmapmask | ((ScreenColor^0xf)*256));
	 *(EGAscrn+j*CurrentLBM.width/8+i)=0xff;
	}
   }

 EGAMblit(globalx,globaly+yadd,width,height,databuffer+offset);
 if (!nostacking)
   {
    globalx+=width;
    if (globalmaxh<height)
      globalmaxh=height;
    if (globalx>40-width)
      {
       globaly+=globalmaxh;
       globalx=globalmaxh=0;
      }
   }
}


////////////////////////////////////////////////////////////
//
// EGAMgrab: grabs any masked EGA shape from the screen buffer in main memory
//           with INLINE asm for *FAST* grab!
//	     Of course, by John Romero!
//
// NOTE: I expect X & WIDTH to be in BYTE values, not pixels!
//
////////////////////////////////////////////////////////////
void EGAMgrab(int x,int y,int width,int height,long offset,int optimize)
{
 unsigned j,maskoff,ESreg,DIreg,SIreg,DSreg,size,scrnwid,tmpset=0;

 scrnwid=CurrentLBM.width/8;
 size=scrnwid*CurrentLBM.height;	// EGA plane calculation

 //
 // Does caller want vertical-seek optimization?
 //
 if (optimize)
   {
    Optimum.height=Optimum.y=0;

    for (i=y;i<y+height;i++)
      {
       for (j=x;j<x+width;j++)
	 if (*(maskscreen+i*scrnwid+j)!=0xff)
	   { Optimum.y=i; break; }
       if (Optimum.y)
	 break;
      }

    for (i=y+height-1;i>=0;i--)
      {
       for (j=x;j<x+width;j++)
	 if (*(maskscreen+i*scrnwid+j)!=0xff)
	   { Optimum.height=i-y; break; }
       if (Optimum.height)
	 break;
      }

    if (Optimum.height && Optimum.height!=height)
      height=Optimum.height-(Optimum.y-y)+1;

    if (Optimum.y && Optimum.y!=y)
      y=Optimum.y;
   }


 // FROM
 SIreg=FP_OFF(maskscreen)+((scrnwid*y+x)&15);
 DSreg=FP_SEG(maskscreen)+((scrnwid*y+x)/16);

 // TO
 ESreg=FP_SEG(databuffer+offset);
 DIreg=FP_OFF(databuffer+offset);

 asm		push	si
 asm		push	di
 asm		push	ds

 asm		mov	bx,height
 asm		mov	dx,width

 asm		mov	es,ESreg
 asm		mov	di,DIreg
 asm		mov	si,SIreg
 asm		mov	ax,DSreg
 asm		mov	ds,ax
 asm		cld

 LOOP0:

 asm		mov	cx,dx

 LOOP00:
 asm		lodsb
 asm		or	al,al
 asm		jz	LOOP01
 asm		mov	[tmpset],1

 LOOP01:
 asm		stosb
 asm		loop	LOOP00

 asm		sub	si,dx
 asm		add	si,scrnwid
 asm		dec	bx
 asm		jnz	LOOP0

 asm		pop	ds
 asm		pop	di
 asm		pop	si

 maskoff=DIreg;

 for (i=0;i<4;i++)
   {
    // FROM
    SIreg=FP_OFF(lbmscreen)+((size*i+(scrnwid*y+x))&15);
    DSreg=FP_SEG(lbmscreen)+((size*i+(scrnwid*y+x))/16);

    // TO
    DIreg+=width*height;

    asm		push	si
    asm		push	di
    asm		push	ds

    asm		mov	dx,width

    asm		mov	es,ESreg
    asm		mov	di,DIreg	// DI=EGA data offset
    asm		mov	si,SIreg	// SI=EGA screen offset
    asm		mov	ds,DSreg
    asm		mov	bx,maskoff	// BX=mask offset
    asm		mov	ah,BYTE PTR height

    LOOP1:
    asm		mov	cx,dx

    LOOP2:
    asm		lodsb			// get EGA data
    asm		mov	ch,[es:bx]	// get mask byte
    asm		xor	ch,0xff		// and invert it!
    asm		and	al,ch		// AND with mask
    asm		xor	ch,ch		// make sure LOOP keeps going!
    asm		stosb			// store
    asm		inc	bx		// next mask byte
    asm		loop LOOP2		// do all EGA bytes

    asm		sub	si,dx
    asm		add	si,scrnwid
    asm		dec	ah
    asm		jnz	LOOP1

    asm		pop	ds
    asm		pop	di
    asm		pop	si
   }

 //
 // SEE IF WE NEED TO ELIMINATE THE MASK & SET THE
 // BIT IN THE PACKED-BIT ARRAY. MASK-ELIMINATION IS
 // DONE BY MOVING THE TILE DATA BACK OVER THE MASK.
 //
 setbit=tmpset^1;

 if (bit && setbit)
   {
    char masks[8]={0x80,0x40,0x20,0x10,8,4,2,1};
    unsigned psize=width*height,domove=0;

    switch(type)
    {
     case TILE8MTYPE:
     case ALT8MTYPE:
       if (!cmpt8)
	 break;

       T8bit[T8whichbit/8]|=masks[T8whichbit%8];
       domove=1;
       break;
     case TILE16MTYPE:
     case ALT16MTYPE:
       T16bit[T16whichbit/8]|=masks[T16whichbit%8];
       domove=1;
       break;
     case TILE32MTYPE:
     case ALT32MTYPE:
       T32bit[T32whichbit/8]|=masks[T32whichbit%8];
       domove=1;
    }

    if (domove)
      {
       asm	push	si
       asm	push	di
       asm	push	ds

       asm	mov	di,[DIreg]
       asm	mov	ax,[ESreg]
       asm	mov	dx,[psize]
       asm	mov	ds,ax
       asm	mov	es,ax		// ES:DI - mask data
       asm	mov	si,di
       asm	add	si,dx		// DS:SI = tile data

       asm	mov	cx,dx		// move planesize*4 (all tile data)
       asm	shl	cx,1
       asm	shl	cx,1
       asm	cld			// move forwards
       asm	rep movsb

       asm	pop	ds
       asm	pop	di
       asm	pop	si
      }
   }

}
