////////////////////////////////////////////////////////////
//
// CGA GRABBING
//
// NOTE: THE LOADED CGA SCREEN AT "LBMSCREEN" & "MASKSCREEN"
// IS NOT "CGA-MUNGED", IT'S LINEARLY ARRANGED! YAY!
//
////////////////////////////////////////////////////////////
#include "igrab.h"
#pragma hdrstop

////////////////////////////////////////////////////////////
//
// CGAblit : blits a CGA shape from memory to the CGA screen
//
////////////////////////////////////////////////////////////
void CGAblit(int x,int y,int width,int height,char huge *buffer)
{
 unsigned ESreg,DIreg,SIreg,DSreg,where,owidth,addx;

 if (noshow || SkipToStart)
   return;
 // Preclipping
 if (x<0 || y<0)
   return;

 if (x>79 || y>199)
   globalx=globaly=globalmaxh=x=y=0;

 addx=0;
 if (x+width>79)
   {
    owidth=width;
    width=80-x;
    addx=owidth-width;
   }
 if (y+height>199)
   {
    height=200-y;
   }

 DSreg=FP_SEG(buffer);
 SIreg=FP_OFF(buffer);
 where=(y/2)*80+0x2000*(y&1)+x;

	__asm {
		push	si
		push	di
		push	ds
		pushf

		cld
		mov	ax,0xb800
		mov	es,ax
		mov	si,[SIreg]
		mov	ax,[DSreg]
		mov	ds,ax

		mov	bx,[y]
		mov	di,[where]
		mov	dx,[height]
		LOOP1:
		mov	cx,[width]
		rep movsb
		mov	cx,[addx]	// any to finish up horizontally?
		jcxz	LOOP1a
		rep lodsb

		LOOP1a:
		sub	di,[width]
		xor	di,0x2000	// calculate next CGA line
		inc	bx
		and	bx,1
		or	bx,bx
		jnz	LOOP1a0
		add	di,80

		LOOP1a0:
		dec	dx
		jnz	LOOP1

		popf
		pop	ds
		pop	di
		pop	si
	}
}


////////////////////////////////////////////////////////////
//
// DoCGAblit : handles output of CGA grabs to the screen
//
////////////////////////////////////////////////////////////
void DoCGAblit(int x,int y,int width,int height)
{
 if (noshow || SkipToStart)
   return;

 if (nostacking)
   {
    globalx=x;
    globaly=y;
   }

 CGAblit(globalx,globaly,width,height,databuffer+offset);
 if (!nostacking)
   {
    globalx+=width;
    if (globalmaxh<height)
      globalmaxh=height;
    if (globalx>80-width)
      {
       globaly+=globalmaxh;
       globalx=globalmaxh=0;
      }
   }
}


////////////////////////////////////////////////////////////
//
// CGAgrab : grabs any CGA shape from the screen buffer in main memory
//           with INLINE asm for *FAST* grab!
//	     Of course, by John Romero!
//
// NOTE: I expect X & WIDTH to be in BYTE values, not pixels!
//
////////////////////////////////////////////////////////////
void CGAgrab(int x,int y,int width,int height,unsigned offset)
{
 unsigned ESreg,DIreg,SIreg,DSreg,scrnwid,loc;

 scrnwid=CurrentLBM.width/4;
 loc=y*scrnwid+x;

 // FROM
 SIreg=FP_OFF(lbmscreen)+(loc&15);
 DSreg=FP_SEG(lbmscreen)+(loc/16);

 // TO
 ESreg=FP_SEG(databuffer+offset);
 DIreg=FP_OFF(databuffer+offset);

	__asm {
		push	si
		push	di
		push	ds

		mov	bx,height
		mov	dx,width

		mov	es,ESreg
		mov	di,DIreg
		mov	si,SIreg
		mov	ax,DSreg
		mov	ds,ax
		cld

		LOOP1:

		mov	cx,dx
		rep movsb

		sub	si,dx
		add	si,scrnwid
		dec	bx
		jnz	LOOP1

		pop	ds
		pop	di
		pop	si
	}

 offset+=width*height;
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
// CGAMblit : blits a CGA masked shape from memory to the CGA screen
//
////////////////////////////////////////////////////////////
void CGAMblit(int x,int y,int width,int height,char huge *buffer)
{
 unsigned MASKoff,DATAoff,ESreg,DIreg,SIreg,DSreg,where,oheight,owidth,addx;

 if (noshow || SkipToStart)
   return;
 // Preclipping
 if (x<0 || y<0)
   return;

 addx=0;
 owidth=width;
 oheight=height;

 if (x+width>79)
   {
    width=80-x;
    addx=owidth-width;
   }
 if (y+height>199)
   {
    height=200-y;
   }

 DSreg=FP_SEG(buffer);
 MASKoff=FP_OFF(buffer);
 DATAoff=FP_OFF((char far *)buffer+owidth*oheight);
 where=(y/2)*80+0x2000*(y&1)+x;

	__asm {
		push	si
		push	di
		push	ds
		pushf

		mov	dx,[DATAoff]
		mov	ds,[DSreg]

		mov	si,[MASKoff]
		mov	di,[where]
		mov	ax,0xb800
		mov	es,ax

		mov	bl,[BYTE PTR y]
		mov	bh,[BYTE PTR height]
		LOOP1:
		mov	cx,[width]
		LOOP1c:
		mov	al,[es:di]
		and	al,[si]		// get mask byte (SI=mask data)
		inc	si
		xchg	si,dx		// SI now = CGA data
		or	al,[si]
		inc	si
		xchg	si,dx		// SI now = MASK data
		stosb
		loop LOOP1c
		mov	cx,[addx]	// any to finish up horizontally?
		jcxz	LOOP1a
		add	si,cx
		add	dx,cx

		LOOP1a:
		sub	di,[width]	// calculate next CGA line
		xor	di,0x2000
		inc	bl
		and	bl,1
		or	bl,bl
		jnz	LOOP1a0
		add	di,80

		LOOP1a0:
		dec	bh
		jnz	LOOP1

		popf
		pop	ds
		pop	di
		pop	si
	}
}


////////////////////////////////////////////////////////////
//
// DoCGAMblit : handles output of masked EGA grabs to the screen
//
////////////////////////////////////////////////////////////
void DoCGAMblit(int x,int y,int width,int height,int yadd,int hadd)
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
    char huge *CGAscrn=MK_FP(0xb800,0);

    for (j=globaly;j<globaly+yadd;j++)
      for (i=globalx;i<globalx+width;i++)
	*(CGAscrn+(j/2)*80+(j&1)*0x2000+i)=0x5555;

    for (j=globaly+yadd+height;j<height+globaly+yadd+hadd;j++)
      for (i=globalx;i<globalx+width;i++)
	*(CGAscrn+(j/2)*80+(j&1)*0x2000+i)=0x5555;
   }

 CGAMblit(globalx,globaly+yadd,width,height,databuffer+offset);
 if (!nostacking)
   {
    globalx+=width;
    if (globalmaxh<height)
      globalmaxh=height;
    if (globalx>80-width)
      {
       globaly+=globalmaxh;
       globalx=globalmaxh=0;
      }
   }
}


////////////////////////////////////////////////////////////
//
// CGAMgrab: grabs any masked CGA shape from the screen buffer in main memory
//           with INLINE asm for *FAST* grab!
//	     Of course, by John Romero!
//
// NOTE: I expect X & WIDTH to be in BYTE values, not pixels!
//
////////////////////////////////////////////////////////////
void CGAMgrab(int x,int y,int width,int height,unsigned offset,int optimize)
{
 unsigned j,maskoff,ESreg,DIreg,SIreg,DSreg,scrnwid,tmpset=0;

 scrnwid=CurrentLBM.width/4;

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

 asm		mov	bx,[height]
 asm		mov	dx,[width]

 asm		mov	es,[ESreg]
 asm		mov	di,[DIreg]
 asm		mov	si,[SIreg]
 asm		mov	ax,[DSreg]
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
 asm		add	si,[scrnwid]
 asm		dec	bx
 asm		jnz	LOOP0

 asm		pop	ds
 asm		pop	di
 asm		pop	si

 maskoff=DIreg;

 // FROM
 SIreg=FP_OFF(lbmscreen)+((scrnwid*y+x)&15);
 DSreg=FP_SEG(lbmscreen)+((scrnwid*y+x)/16);

 // TO
 DIreg+=width*height;

 asm		push	si
 asm		push	di
 asm		push	ds

 asm		mov	dx,[width]

 asm		mov	es,[ESreg]
 asm		mov	di,[DIreg]	// DI=CGA data offset
 asm		mov	si,[SIreg]	// SI=CGA screen offset
 asm		mov	ds,[DSreg]
 asm		mov	bx,[maskoff]	// BX=mask offset
 asm		mov	ah,[BYTE PTR height]

 LOOP1:
 asm		mov	cx,dx

 LOOP2:
 asm		lodsb			// get CGA data
 asm		mov	ch,[es:bx]	// get mask byte
 asm		xor	ch,0xff		// and invert it!
 asm		and	al,ch		// AND with mask
 asm		xor	ch,ch		// make sure LOOP keeps going!
 asm		stosb			// store
 asm		inc	bx		// next mask byte
 asm		loop LOOP2		// do all CGA bytes

 asm		sub	si,dx
 asm		add	si,[scrnwid]
 asm		dec	ah
 asm		jnz	LOOP1

 asm		pop	ds
 asm		pop	di
 asm		pop	si

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

       asm	mov	si,[DIreg]
       asm	mov	ax,[ESreg]
       asm	mov	ds,ax
       asm	mov	es,ax		// ES:DI - mask data
       asm	mov	di,si
       asm	add	si,[psize]	// DS:SI = tile data

       asm	mov	cx,[psize]	// move all tile data
       asm	cld			// move forwards
       asm	rep movsb

       asm	pop	ds
       asm	pop	di
       asm	pop	si
      }
   }

}
