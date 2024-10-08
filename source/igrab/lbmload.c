/////////////////////////////////////////////////////////////
/*
** ILBM file loader by John Romero (C) 1991 Id Software
**
** Loads and decompresses an ILBM-format file to the
** screen in either CGA, EGA or MCGA -- compressed or
** or unpacked!
**
** Merely pass the filename of the image to LoadLBM
** and sit back! The proper graphics mode is initialized
** and the screen (or brush) is loaded and displayed!
**
*/
/////////////////////////////////////////////////////////////
#include "igrab.h"
#pragma hdrstop

//
// LBMload PROTOs
//

void GetChunkID(char huge *buffer,char *tempstr);
int  NextChunkID(char huge *buffer);
void huge *Decompress(char huge *buffer,char *unpacked,int bpwidth,char planes);
void huge *SetupLBM(char *filename);
void Do_CGA_Screen(char huge *buffer,char compress,char planes,int width,int height,char huge *scrnmem);
void Do_EGA_Screen(char huge *buffer,char compress,char planes,int width,int height,char huge *scrnmem);
void Do_MCGA_Screen(char huge *buffer,char compress,int width,int height,char huge *scrnmem);

char typestr[5],huge *startbuff;


//
// Here goes!
//
char huge *LoadLBM(char *filename,LBMtype *thelbm)
{
 char huge *buffer,huge *cmap,huge *scrnmem;
 char planes,tempstr[5],compress;
 unsigned handle,width,height;

  if((buffer = SetupLBM(filename))==NULL) errout("LBMLOAD error");

  /*
  ** Need to get BMHD info, like:
  ** - screen width, height
  ** - # of bitplanes
  ** - compression flag (YES/NO)
  */

  width  = (*(buffer+9)&0xFF)+(*(buffer+8)*256);
  height = (*(buffer+11)&0xFF)+(*(buffer+10)*256);
  planes = *(buffer+16);
  if (buffer[17])
    {
     char string[]="The ILBM screen '";

     strcat(string,filename);
     strcat(string,"' is in STENCIL mode!\nGo back into DeluxePaint and turn it off!");
     errout(string);
    }
  compress = *(buffer+18);

  thelbm->width=width;	// fill the struct
  thelbm->height=height;
  thelbm->planes=planes;

  if ((scrnmem=(char huge *)farmalloc((long)(width/8)*height*planes))==NULL)
    {
     char str[100]="Not enough memory for loading the ILBM screen '";

     strcat(str,filename);
     strcat(str,"'!");
     errout(str);
    }

  /*
  ** Find the CMAP chunk so I can remap the registers...
  */

  movedata(FP_SEG(buffer),FP_OFF(buffer),_DS,(unsigned)tempstr,4);
  tempstr[4]=0;
  while (strcmp(tempstr,"CMAP")!=0)
     {
      buffer += NextChunkID(buffer);
      movedata(FP_SEG(buffer),FP_OFF(buffer),_DS,(unsigned)tempstr,4);
     }

  cmap = buffer+8;

  /*
  ** Now, find the BODY chunk...
  */

  movedata(FP_SEG(buffer),FP_OFF(buffer),_DS,(unsigned)tempstr,4);
  while (strcmp(tempstr,"BODY")!=0)
     {
      buffer += NextChunkID(buffer);
      movedata(FP_SEG(buffer),FP_OFF(buffer),_DS,(unsigned)tempstr,4);
     }

  /*
  ** Found the BODY chunk! Here we go!
  */

  buffer += 8; /* point to actual data */

  switch (planes)
    {
      case 2: Do_CGA_Screen(buffer,compress,planes,width,height,scrnmem);
	      break;
      case 4: Do_EGA_Screen(buffer,compress,planes,width,height,scrnmem);
	      break;
      case 8: {
	       unsigned int i;

	       if (!noshow)
		 {
		  for (i=0;i<0x300;i++) (unsigned char)*cmap++ >>= 2;

		  cmap -= 0x300; // reset to beginning again

		  _BX = 0;
		  _CX = 0x100;
		  _ES = FP_SEG(cmap);
		  _DX = FP_OFF(cmap);
		  _AX = 0x1012;
		  geninterrupt(0x10);
		 }

	       Do_MCGA_Screen(buffer,compress,width,height,scrnmem);

	       break;
	      }

      default: {
		char msg[80]="This screen has ",temp[10];

		itoa(planes,temp,10);
		strcat(msg,temp);
		strcat(msg," bitplanes. I don't unnerstand that sorta stuff.");
		errout(msg);
	       }
    }
  farfree((void far *)startbuff);
 return scrnmem;
}


void huge *SetupLBM(char *filename)
{
 long filesize;
 int handle;
 char huge *buffer;
 char tempstr[64],errst[120];



 filesize = filelen(filename);
 buffer = startbuff = (char huge *)farmalloc(filesize);
 if (buffer==NULL)
 {
   strcpy(errst,"Not enough memory to load ILBM file! Size=");
   ltoa(filesize,tempstr,10);
   strcat(errst,tempstr);
   errout(errst);
 }

 strupr(filename);
 if (strstr(filename,".")==NULL)
	strcat(filename,".LBM");

 if (access(filename,0))
   {
	char msg[80]="The ILBM file '";

	strcat(msg,filename);
	strcat(msg,"' doesn't exist!");
	errout(msg);
   }

 LoadFile(filename,buffer,0,0);

 GetChunkID(buffer,tempstr);
 if (strcmp(tempstr,"FORM")!=0)
   errout("This isn't an ILBM FORM format file!");

  /*
  ** point past the FORM chunk
  ** and see if this really IS
  ** and ILBM file
  */

  buffer += 8;
  GetChunkID(buffer,tempstr);
  strcpy(typestr,tempstr); // save file type
  if ((strcmp(tempstr,"ILBM")!=0) && (strcmp(tempstr,"PBM ")!=0) )
    errout("This isn't an ILBM format file!");

  /*
  ** point to BMHD chunk, the first NORMAL chunk!
  */

  buffer += 4;
  GetChunkID(buffer,tempstr);
  if (strcmp(tempstr,"BMHD")!=0)
    errout("What kind of ILBM is this? There's no BMHD chunk!");

  return(buffer);
}



void GetChunkID(char huge *buffer,char *tempstr)
{
  movedata(FP_SEG(buffer),FP_OFF(buffer),_DS,(unsigned)tempstr,4);
  tempstr[4]=0;
}




int NextChunkID(char huge *buffer)
{
  unsigned int newoffset;

  newoffset = (*(buffer+7)&0xFF) + (*(buffer+6)*256);
  if ((newoffset & 1)==1) newoffset += 1;
  return(newoffset+8); /* +8 because chunk + offset = 8 bytes! */
}


/////////////////////////////////////////////////////////////
/*
** CGA loader
*/
/////////////////////////////////////////////////////////////
void Do_CGA_Screen(char huge *buffer,char compress,char planes,int width,int height,char huge *scrnmem)
{
 unsigned int bpwidth,loopY,loopX,loopB,offset,data;
 char huge *screen,b1,b2,unpacked[1280];

 bpwidth = width/8;

 for (loopY=0;loopY<height;loopY++)
     {
      if (compress==1)
	 buffer=Decompress(buffer,unpacked,bpwidth,planes);

      offset=0;
      screen=scrnmem+(loopY*((width/8)*planes));
      // screen = MK_FP(0xb800,(0x2000*(loopY&1))+(80*(loopY/2)));
      for (loopX=0;loopX<bpwidth;loopX++)
	  {
	   if (compress==1)
	      {
	       b1 = *(unpacked+offset);
	       b2 = *(unpacked+bpwidth+offset);
	      }
	   else
	      {
	       b1 = *(buffer+offset);
	       b2 = *(buffer+bpwidth+offset);
	      }
	   offset++;

	   // This loop should be in INLINE(!) assembler!

                asm       mov   cx,8
                asm       mov   bh,b1
                asm       mov   bl,b2
                asm       xor   dx,dx
                LoopTop:
                asm       test  bh,1
                asm       jz    NoOR
                asm       or    dx,4000h
                NoOR:
                asm       test  bl,1
                asm       jz    NoOR1
                asm       or    dx,8000h
                NoOR1:
                asm       cmp   cx,1
                asm       je    NoShift
                asm       shr   dx,1
                asm       shr   dx,1
                NoShift:
                asm       shr   bh,1
                asm       shr   bl,1
                asm       loop  LoopTop

                asm       mov   data,dx

	   /* first draft of above loop:

           for (loopB=0;loopB<8;loopB++)
               {
                if (b1 & 1==1) data |= 0x4000;
                if (b2 & 1==1) data |= 0x8000;
                if (loopB<7) data >>= 2;
                b1 >>= 1;
                b2 >>= 1;
               }
           */

           *screen = data >> 8;
           *(screen+1) = data;
           screen += 2;
          }

      if (!compress)
         buffer += bpwidth*planes;
     }
}


/////////////////////////////////////////////////////////////
//
// EGA loader
//
/////////////////////////////////////////////////////////////
void Do_EGA_Screen(char huge *buffer,char compress,char planes,int width,int height,char huge *scrnmem)
{
 unsigned bpwidth,loopY,loopX,loopB,offset,data,dest,j;
 char huge *screen,b1,b2,unpacked[1280], // 8 screens wide max!!!
	  huge *plane[4];
 long size;


 bpwidth = width/8;
 size=bpwidth*height;

 plane[0]=scrnmem;
 plane[1]=plane[0]+size;
 plane[2]=plane[1]+size;
 plane[3]=plane[2]+size;

 for (loopY=0;loopY<height;loopY++)
     {
      if (compress==1)
	 {
	  buffer=Decompress(buffer,unpacked,bpwidth,planes);
	  dest=loopY*bpwidth;
	  for (i=0;i<4;i++)
	    {
	     movedata(FP_SEG(unpacked+bpwidth*i),
		      FP_OFF(unpacked+bpwidth*i),
		      FP_SEG(plane[i])+dest/16,
		      FP_OFF(plane[i])+(dest&15),
		      bpwidth);
	    }
	 }
      else
	 {
	  dest=loopY*bpwidth;
	  for (i=0;i<4;i++)
	    {
	     movedata(FP_SEG(buffer+bpwidth*i),FP_OFF(buffer+bpwidth*i),
		      FP_SEG(plane[i])+dest/16,FP_OFF(plane[i])+(dest&15),
		      bpwidth);
	    }
	  buffer += bpwidth*planes;
	 }
     }
}

/////////////////////////////////////////////////////////////
/*
** MCGA loader
*/
/////////////////////////////////////////////////////////////
void Do_MCGA_Screen(char huge *buffer,char compress,int width,int height,char huge *scrnmem)
{
 unsigned int bpwidth,loopY,loopX,loopB,offset,data;
 char huge *screen,b1,b2,unpacked1[1280],unpacked[1280];


 screen=scrnmem;

 for (loopY=0;loopY<height;loopY++)
     {
      if (compress==1)
         {
	  buffer=Decompress(buffer,unpacked,width,1);
	  if (strcmp(typestr,"ILBM")==0)
	     {
	      int tloop;

	      memset(unpacked1,0,320);
	      for (tloop=0;tloop<40;tloop++)
		  {
		   int tloop1,tloop2;
		   unsigned char mask[8] = { 0x80,0x40,0x20,0x10,8,4,2,1 };

		   for (tloop1=0;tloop1<8;tloop1++)
		     for (tloop2=0;tloop2<8;tloop2++)
		       unpacked1[tloop*8+tloop1]|=
			  ((( (unsigned)unpacked[tloop+(7-tloop2)*40]
			  &mask[tloop1])
			  <<tloop1)
			  >>tloop2);
		  }
	      movedata(_DS,(unsigned)unpacked1,FP_SEG(screen),FP_OFF(screen),width);
	     }
	  else movedata(_DS,(unsigned)unpacked,FP_SEG(screen),FP_OFF(screen),width);
         }
      else
         {
	  movedata(FP_SEG(buffer),FP_OFF(buffer),FP_SEG(screen),FP_OFF(screen),width);
          buffer += width;
         }
      screen+=width;
     }
}

/////////////////////////////////////////////////////////////
/*
** ILBM's RLE decompressor. Merely pass the address of the compressed
** ILBM bitplane data, where to unpack it, the # of bytes each bitplane
** takes up, and the # of bit planes to unpack.
*/
/////////////////////////////////////////////////////////////
void huge *Decompress(char huge *buffer,char *unpacked,int bpwidth,char planes)
{
 int count,offset,loopP;
 unsigned char byte,rept;

 #if 0
 for (loopP=0;loopP<planes;loopP++)
     {
      count = 0;

      do {
          rept = *(buffer);
          if (rept > 0x80)
             {
              rept = (rept^0xff)+2;
              byte = *(buffer+1);
              buffer+=2;

              memset(unpacked,byte,rept);
             }
          else if (rept < 0x80)
             {
	      rept++;
	      movedata(FP_SEG(buffer),FP_OFF(buffer)+1,_DS,(unsigned) unpacked,rept);
	      buffer += rept+1;
             }
          count += rept;
          unpacked += rept;

         } while (count<bpwidth);
     }
 #else

 unsigned PackHi,PackLo,BufHi,BufLo;

 PackHi=FP_SEG(unpacked);
 PackLo=FP_OFF(unpacked);
 BufHi=FP_SEG(buffer);
 BufLo=FP_OFF(buffer);

 asm	push	di
 asm	push	si
 asm	push	ds

 asm	mov	es,PackHi
 asm	mov	di,PackLo
 asm	mov	ds,BufHi
 asm	mov	si,BufLo
 asm	mov	cl,planes
 asm	xor	ch,ch
 asm	xor	ah,ah
 LOOP1:
 asm	push	cx
 asm	xor	dx,dx		// DX = count

 LOOP2:
 asm	lodsb
 asm	cmp	al,0x80
 asm	jbe	L2
 asm	xor	al,0xff		// rept^0xff
 asm	add	al,2		// rept+=2
 asm    add	dx,ax
 asm	mov	cl,al
 asm	xor	ch,ch
 asm	lodsb        		// AL=byte to rept
 asm	rep stosb		// repeat it!
 asm	jmp	L3

 L2:
 asm    inc	al
 asm	add	dx,ax
 asm	mov	cl,al
 asm	xor	ch,ch
 asm	rep movsb

 L3:
 asm	cmp	dx,bpwidth
 asm	jb	LOOP2

 asm	pop	cx
 asm	loop	LOOP1

 asm	mov	PackHi,es
 asm	mov	PackLo,di
 asm	mov	BufHi,ds
 asm	mov	BufLo,si

 asm	pop	ds
 asm	pop	si
 asm	pop	di

 buffer=MK_FP(BufHi,BufLo);
 unpacked=(char *)PackLo;
 #endif

 return(buffer);
}


#if 0
/////////////////////////////////////////////////////////////
//
// move an EGA bitplane
//
/////////////////////////////////////////////////////////////
void EGA_MoveBitplane(char huge *from,char far *to,int bpwidth)
{
 unsigned width;

asm		push	ds
asm		push	di
asm		push	si

asm		lds	si,from		//;DS:SI = from buffer
asm		les	di,to		//;ES:DI = screen
asm		mov	bx,bpwidth
asm		mov	width,bx

asm		mov	dx,SCindex	//;start writing to SCmapmask register
asm		mov	al,SCmapmask
asm		out	dx,al
asm		inc	dx

asm		mov	bh,4		//;4 bitplanes!
asm		mov	ah,1		//;start at bitplane 0

EGA1:

asm		mov	al,ah		//;select bitplane
asm		out	dx,al

asm		xor	ch,ch
asm		mov	cl,bl		//;cx = bitplane width
asm		rep movsb
asm		sub	di,width	//;start at beginnin' of line again...
asm		shl	ah,1
asm		dec	bh
asm		jnz	EGA1

asm		mov	al,15
asm		out	dx,al		//;write to ALL bitplanes again

asm		pop	si
asm		pop	di
asm		pop	ds
}
#endif


