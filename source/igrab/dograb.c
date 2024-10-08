////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//
// GRABBING ROUTINES
//
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#include "igrab.h"
#pragma hdrstop

////////////////////////////////////////////////////////////
//
// FONTS - need VGA still
//
// FONT/M format:
// unsigned numchars       - 2 bytes
// unsigned charoffs[256]  - 512 bytes
// char charwidths[256]    - 256 bytes
// data  - masked: mask first, then data
//       - nomask: data
//
// EGA note: plane 0-3, linear storage
//
////////////////////////////////////////////////////////////
int charoff[256];
char cwidths[256];

void GrabFont(grabtype type)
{
 int grabamt,loop3,loop2,loop1,loop,fheight,fascii,x,y;
 char *str1,*str,*comp=", ",*comp1="0123456789",name[20];


 for (loop=0;loop<256;loop++)
   cwidths[loop]=charoff[loop]=0;

 str=string+5;

 grabamt=atoi(str);
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 fheight=atoi(str);
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 fascii=atoi(str);

 if (!fheight)
   errout("ERROR!: You didn't define the height of your font!\n"
	  "Your scriptfile needs to say 'GRAB <amt>,<height>,<asciistart>'");

 if (fascii+grabamt>256)
   {
    settext();
    printf("ERROR! You can't grab more than 256 chars in a FONT!  You're starting\n");
    printf("       at ASCII %d and grabbing %d chars! TOO MUCH!\n",fascii,grabamt);
    nosound();
    exit(1);
   }

 x=y=16;
 offset=0x302;

 //
 // Info:
 // * Fonts always start at 2,2 on the screen (grid coords)
 //

 globalx=globaly=0;
 for (loop1=fascii;loop1<fascii+grabamt;loop1++)
   {
    int found;
    //
    // first, find width of character OR next char
    //
    found=0;

    while(!found)
      {
       for (loop=x+1;loop<320;loop++)
	 if (CharEdgeCheck(loop,y+fheight))
	   {
	    cwidths[loop1]=loop-x;
	    found++;
	    break;
	   }

       //
       // if we reached the right edge of the screen
       // without finding a character, reset the xcoord
       // to search the next row!
       //
       if (!found)
	 {
	  x=16;
	  //
	  // next row. if we're off the bottom of the
	  // screen, give an error!
	  //
	  y=(y+fheight+7)&0xf8;
	  if (y>200)
	    {
	     settext();
	     printf("ERROR!  You specified %d FONT CHARS to grab, but I could\n",grabamt);
	     printf("        only find %d!\n",loop1-fascii);
	     nosound();
	     exit(1);
	    }
	 }
      }

    //
    // store offset to this character in the table
    //
    charoff[loop1]=offset;

    //
    // Grab mask first if we're grabbing a FONTM
    //
    if (type==FONTMTYPE)
      {
       switch(format[0])
	 {
	  case 'C':
	    for (loop2=y;loop2<y+fheight;loop2++)
	      for (loop3=x/4;loop3<x/4+(cwidths[loop1]+3)/4;loop3++)
		*(databuffer+offset++)=(*(maskscreen+loop2*(CurrentLBM.width/4)+loop3))^0xff;
	    break;

	  //
	  // next, grab mask in maskscreen, plane 0 and xor-0xff it
	  //
	  case 'E':
	    for (loop2=y;loop2<y+fheight;loop2++)
	      for (loop3=x/8;loop3<x/8+(cwidths[loop1]+7)/8;loop3++)
		*(databuffer+offset++)=(*(maskscreen+loop2*(CurrentLBM.width/8)+loop3))^0xff;
	    break;
	  case 'V':
	    for (loop2=y;loop2<y+fheight;loop2++)
	      for (loop3=x;loop3<x+cwidths[loop1];loop3++)
		*(databuffer+offset++)=(*(maskscreen+loop2*CurrentLBM.width+loop3))^0xff;
	 }
       CheckBuffer();
      }

    //
    // Now, grab character data
    //
    switch(format[0])
      {
       case 'C':
	 for (loop2=y;loop2<y+fheight;loop2++)
	   for (loop3=x/4;loop3<x/4+(cwidths[loop1]+3)/4;loop3++)
	     *(databuffer+offset++)=*(lbmscreen+loop2*(CurrentLBM.width/4)+loop3);
	 x=(x+cwidths[loop1]+7)&0xfff8;
	 break;

       //
       // next, grab char data in lbmscreen, plane 0
       //
       case 'E':
	 for (loop2=y;loop2<y+fheight;loop2++)
	   for (loop3=x/8;loop3<x/8+(cwidths[loop1]+7)/8;loop3++)
	     *(databuffer+offset++)=*(lbmscreen+loop2*(CurrentLBM.width/8)+loop3);
	 x=(x+cwidths[loop1]+7)&0xfff8;
	 break;
       case 'V':
	 for (loop2=y;loop2<y+fheight;loop2++)
	   for (loop3=x;loop3<x+cwidths[loop1];loop3++)
	     *(databuffer+offset++)=*(lbmscreen+loop2*CurrentLBM.width+loop3);
	 x=(x+cwidths[loop1]+7)&0xfff8;
      }

    //
    // Time to BLIT the data to the screen!
    //
    if (!noshow && !SkipToStart)
      switch(format[0])
	{
	 //
	 // CGA
	 //
	 case 'C':
	   if (type==FONTMTYPE)
	     {
	      unsigned loop4,datoff,charsize=fheight*((cwidths[loop1]+3)/4),tmp;
	      unsigned char c,huge *CGAscrn=MK_FP(0xb800,0);

	      datoff=charoff[loop1];

	      for (loop2=0;loop2<fheight;loop2++)
		for (loop3=0;loop3<(cwidths[loop1]+3)/4;loop3++)
		  {
		   tmp=globaly+loop2;
		   c=*(CGAscrn+0x2000*(tmp&1)+(tmp/2)*(CurrentLBM.width/4)+globalx+loop3);
		   c=(c)&(*(databuffer+datoff)^0xff)|(*(databuffer+charsize+datoff++));
		   *(CGAscrn+0x2000*(tmp&1)+(tmp/2)*(CurrentLBM.width/4)+globalx+loop3)=c;
		  }
	     }
	   else
	     {
	      unsigned loop4,datoff,tmp;
	      unsigned char c,huge *CGAscrn=MK_FP(0xb800,0);

	      datoff=charoff[loop1];

	      for (loop2=0;loop2<fheight;loop2++)
		for (loop3=0;loop3<(cwidths[loop1]+3)/4;loop3++)
		  {
		   tmp=globaly+loop2;
		   c=*(CGAscrn+0x2000*(tmp&1)+(tmp/2)*(CurrentLBM.width/4)+globalx+loop3);
		   c=(c)|(*(databuffer+datoff++));
		   *(CGAscrn+0x2000*(tmp&1)+(tmp/2)*(CurrentLBM.width/4)+globalx+loop3)=c;
		  }
	     }

	   globalx+=(cwidths[loop1]+3)/4;
	   if (globalx>78)
	     {
	      globalx=0;
	      globaly+=fheight;
	      if (globaly>200-fheight)
		globaly=0;
	     }
	   break;
	 //
	 // EGA
	 //
	 case 'E':
	   if (type==FONTMTYPE)
	     {
	      unsigned loop4,datoff,charsize=fheight*((cwidths[loop1]+7)/8);
	      unsigned char c,huge *EGAscrn=MK_FP(0xa000,0);

	      outport(GCindex,GCmode);
	      for (loop4=0;loop4<4;loop4++)
		{
		 outport(GCindex,GCreadmap | (loop4*256));
		 outport(SCindex,SCmapmask | (1<<loop4)*256);
		 datoff=charoff[loop1];

		 for (loop2=0;loop2<fheight;loop2++)
		   for (loop3=0;loop3<(cwidths[loop1]+7)/8;loop3++)
		     {
		      c=*(EGAscrn+(globaly+loop2)*(CurrentLBM.width/8)+globalx+loop3);
		      c=(c)&(*(databuffer+datoff)^0xff)|(*(databuffer+charsize+datoff++));
		      *(EGAscrn+(globaly+loop2)*(CurrentLBM.width/8)+globalx+loop3)=c;
		     }
		 }
	     }
	   else
	     {
	      unsigned loop4,datoff;
	      unsigned char c,huge *EGAscrn=MK_FP(0xa000,0);

	      outport(GCindex,GCmode);
	      for (loop4=0;loop4<4;loop4++)
		{
		 outport(GCindex,GCreadmap | (loop4*256));
		 outport(SCindex,SCmapmask | (1<<loop4)*256);
		 datoff=charoff[loop1];

		 for (loop2=0;loop2<fheight;loop2++)
		   for (loop3=0;loop3<(cwidths[loop1]+7)/8;loop3++)
		     {
		      c=*(EGAscrn+(globaly+loop2)*(CurrentLBM.width/8)+globalx+loop3);
		      c=(c)|(*(databuffer+datoff++));
		      *(EGAscrn+(globaly+loop2)*(CurrentLBM.width/8)+globalx+loop3)=c;
		     }
		 }
	     }

	   globalx+=(cwidths[loop1]+7)/8;
	   if (globalx>38)
	     {
	      globalx=0;
	      globaly+=fheight;
	      if (globaly>200-fheight)
		globaly=0;
	     }
	   break;
	 //
	 // VGA
	 //
	 case 'V':
	   if (type==FONTMTYPE)
	     {
	      unsigned loop4,datoff,charsize=fheight*cwidths[loop1];
	      unsigned char c,huge *VGAscrn=MK_FP(0xa000,0);

	      datoff=charoff[loop1];

	      for (loop2=0;loop2<fheight;loop2++)
		for (loop3=0;loop3<cwidths[loop1];loop3++)
		  {
		   c=*(VGAscrn+(globaly+loop2)*CurrentLBM.width+globalx+loop3);
		   c=(c)&(*(databuffer+datoff)^0xff)|(*(databuffer+charsize+datoff++));
		   *(VGAscrn+(globaly+loop2)*CurrentLBM.width+globalx+loop3)=c;
		  }
	     }
	   else
	     {
	      unsigned loop4,datoff;
	      unsigned char c,huge *VGAscrn=MK_FP(0xa000,0);

	      datoff=charoff[loop1];

	      for (loop2=0;loop2<fheight;loop2++)
		for (loop3=0;loop3<cwidths[loop1];loop3++)
		  {
		   c=*(VGAscrn+(globaly+loop2)*CurrentLBM.width+globalx+loop3);
		   c=(c)|(*(databuffer+datoff++));
		   *(VGAscrn+(globaly+loop2)*CurrentLBM.width+globalx+loop3)=c;
		  }
	     }

	   globalx+=cwidths[loop1];
	   if (globalx>304)
	     {
	      globalx=0;
	      globaly+=fheight;
	      if (globaly>200-fheight)
		globaly=0;
	     }
	}

    CheckBuffer();
   }

 //
 // move the completed arrays into the start of the font area
 //
 movedata(FP_SEG(charoff),FP_OFF(charoff),
	  FP_SEG(databuffer+2),FP_OFF(databuffer+2),512);
 movedata(FP_SEG(cwidths),FP_OFF(cwidths),
	  FP_SEG(databuffer+514),FP_OFF(databuffer+514),256);
 *(int huge *)databuffer=fheight;

 //
 // save file out
 //
 CountBytes(databuffer,offset);

 switch(type)
   {
    case FONTTYPE:
      FontOffs[Data[FONT].num]=Data[FONT].offset;
      FontOffs[Data[FONT].num+1]=Data[FONT].offset+offset;
      Data[FONT].offset+=offset;
      Data[FONT].num++;
      AddDataToFile("FONT");
      break;
    case FONTMTYPE:
      FontMOffs[Data[FONTM].num]=Data[FONTM].offset;
      FontMOffs[Data[FONTM].num+1]=Data[FONTM].offset+offset;
      Data[FONTM].offset+=offset;
      Data[FONTM].num++;
      AddDataToFile("FONTM");
   }
}

////////////////////////////////////////////////////////////
//
// Find the right edge of a character.
// Pass pixel coords, please.
//
char CharEdgeCheck(int x,int y)
{
 char value;

 switch(format[0])
 {
  case 'C':
    value=(lbmscreen[y*(CurrentLBM.width/4)+x/4]>>(6-2*(x%4)))&3;
    break;
  case 'E':
    value=(lbmscreen[y*(CurrentLBM.width/8)+x/8]>>(7-(x%8)))&1;
    break;
  case 'V':
    value=lbmscreen[y*CurrentLBM.width+x];
 }

 return value;
}



////////////////////////////////////////////////////////////
//
// Grab info from the screen for any graphics mode
// NON-MASKED
//
////////////////////////////////////////////////////////////
void DoGrab(int x,int y,int width,int height,unsigned offset)
{
 switch(format[0])
   {
    case 'C': CGAgrab(x/4,y,width/4,height,offset); break;
    case 'E': EGAgrab(x/8,y,width/8,height,offset); break;
    case 'V': VGAgrab(x,y,width,height,offset); break;
   }
}



////////////////////////////////////////////////////////////
//
// Grab info from the screen for any graphics mode
// MASKED
//
////////////////////////////////////////////////////////////
void DoMGrab(int x,int y,int width,int height,unsigned offset,int opt)
{
 switch(format[0])
   {
    case 'C': CGAMgrab(x/4,y,width/4,height,offset,opt); break;
    case 'E': EGAMgrab(x/8,y,width/8,height,offset,opt); break;
    case 'V': VGAMgrab(x,y,width,height,offset,opt);
   }
}



////////////////////////////////////////////////////////////
//
// Blit info to the screen for any graphics mode
// MASKED
//
////////////////////////////////////////////////////////////
void DoMBlit(int x,int y,int width,int height,int yadd,int hadd)
{
 switch(format[0])
   {
    case 'C': DoCGAMblit(x/4,y,width/4,height,yadd,hadd); break;
    case 'E': DoEGAMblit(x/8,y,width/8,height,yadd,hadd); break;
    case 'V': DoVGAMblit(x,y,width,height,yadd,hadd);
   }
}



////////////////////////////////////////////////////////////
//
// Blit info to the screen for any graphics mode
// NON-MASKED
//
////////////////////////////////////////////////////////////
void DoBlit(int x,int y,int width,int height)
{
 switch(format[0])
   {
    case 'C': DoCGAblit(x/4,y,width/4,height); break;
    case 'E': DoEGAblit(x/8,y,width/8,height); break;
    case 'V': DoVGAblit(x,y,width,height);
   }
}



////////////////////////////////////////////////////////////
//
// SparseTile checking, for any video mode
// Exit: 1 = sparse tile, 0 = normal tile
//
////////////////////////////////////////////////////////////
int CheckSparse(int x,int y)
{
 int i,j;
 long size;
 unsigned char c;

 switch(format[0])
   {
    case 'C':
      c=*(lbmscreen+y*(CurrentLBM.width/4)+x/4);
      if (c!=0xff)
	return 0;
      for (j=y+1;j<y+8;j++)
	{
	 c=*(lbmscreen+j*(CurrentLBM.width/4)+x/4);
	 if (c!=0xc0)
	   return 0;
	}
      break;

    case 'E':
      size=(CurrentLBM.width/8)*CurrentLBM.height;
      for (i=0;i<4;i++)
	{
	 c=*(lbmscreen+size*i+y*(CurrentLBM.width/8)+x/8);
	 if (c!=0xff)
	   return 0;
	 for (j=y+1;j<y+8;j++)
	   {
	    c=*(lbmscreen+size*i+j*(CurrentLBM.width/8)+x/8);
	    if (c!=0x80)
	      return 0;
	   }
	}
      break;

    case 'V':
      for (j=x;j<x+8;j++)
	{
	 c=*(lbmscreen+(unsigned)y*CurrentLBM.width+j)&0xf;
	 if (c!=0xf)
	   return 0;
	}

      for (j=y+1;j<y+8;j++)
	{
	 c=*(lbmscreen+(unsigned)j*CurrentLBM.width+x)&0xf;
	 if (c!=0xf)
	   return 0;
	}

   }

 return 1;
}



////////////////////////////////////////////////////////////
//
// HANDLE ANY TILE!
// Currently supported sizes:
// 8,16,32; masked & non-masked
//
////////////////////////////////////////////////////////////
void GrabTile(grabtype type)
{
 int xadd,yadd,hw,masked,fullscreen,newtype,althsize,gridhsize,
     xmult,ymult,maxamount,amount,loop,fullwidth;
 char typestr[8][10]={"FONT","FONTM","TILE8","TILE8M","TILE16",
	"TILE16M","TILE32","TILE32M"};


 switch(type)
   {
    case TILE8MTYPE:
    case TILE8TYPE:
      xadd=yadd=16;
      maxamount=MAXGRID8;	// max amount per screen
      ymult=xmult=16;		// x/y screen spacing for grab
      althsize=ALT8H;		// horiz max per ALT line
      gridhsize=GRID8H;		// horiz max per GRID line
      newtype=TILE8;		// new indexing var
      fullwidth=masked=0;	// masked data or not
      if (type==TILE8MTYPE)
	masked=newtype=TILE8M;
      fullscreen=12*ALT8H;	// fullscreen for GRID8 type
      hw=8;			// how many pixels to grab horiz & vert
      break;

    case ALT8MTYPE:
    case ALT8TYPE:
      xadd=yadd=16;
      maxamount=MAXALT8;
      ymult=xmult=8;
      althsize=ALT8H;
      gridhsize=GRID8H;
      newtype=TILE8;
      masked=0;
      fullwidth=1;
      if (type==ALT8MTYPE)
	masked=newtype=TILE8M;
      fullscreen=maxamount;
      hw=8;
      break;

    case TILE16TYPE:
    case TILE16MTYPE:
      xadd=yadd=16;
      maxamount=MAXGRID16;
      ymult=xmult=24;
      althsize=ALT16H;
      gridhsize=GRID16H;
      newtype=TILE16;
      fullwidth=masked=0;
      if (type==TILE16MTYPE)
	masked=newtype=TILE16M;
      fullscreen=8*ALT16H;
      hw=16;
      break;

    case ALT16TYPE:
    case ALT16MTYPE:
      xadd=24;
      yadd=0;
      maxamount=MAXALT16;
      ymult=xmult=16;
      althsize=ALT16H;
      gridhsize=GRID16H;
      newtype=TILE16;
      masked=0;
      fullwidth=1;
      if (type==ALT16MTYPE)
	masked=newtype=TILE16M;
      fullscreen=maxamount;
      hw=16;
      break;

    case TILE32TYPE:
    case TILE32MTYPE:
      xadd=yadd=16;
      maxamount=MAXGRID32;
      ymult=xmult=40;
      althsize=ALT32H;
      gridhsize=GRID32H;
      newtype=TILE32;
      fullwidth=masked=0;
      if (type==TILE32MTYPE)
	masked=newtype=TILE32M;
      fullscreen=4*ALT32H;
      hw=32;
      break;

    case ALT32TYPE:
    case ALT32MTYPE:
      xadd=24;
      yadd=0;
      maxamount=MAXALT32;
      ymult=xmult=32;
      althsize=ALT32H;
      gridhsize=GRID32H;
      newtype=TILE32;
      masked=0;
      fullwidth=1;
      if (type==ALT32MTYPE)
	masked=newtype=TILE32M;
      fullscreen=maxamount;
      hw=32;
      break;
   }

 amount=atoi(string+4);
 if (amount>maxamount)
   {
    char msg[80]="ERROR: You can't grab more than ",temp[10];

    itoa(maxamount,temp,10);
    strcat(msg,temp);
    strcat(msg," TILE8s on a page!");
    errout(msg);
   }

 //
 // if keyword was GRAB with no parms, grab entire screen!
 //
 if (!amount)
   amount=fullscreen;

 //
 // Make sure that there's an even multiple of "althsize" tiles per line
 //
 if (amount%althsize)
   amount=((amount+althsize-1)/althsize)*althsize;

 for (loop=0;loop<amount;loop++)
   {
    int x,y,sparse;

    y=yadd+(loop/althsize)*ymult;
    x=(loop%althsize)*xmult+xadd;

    sparse=0;
    if (loop%althsize>=gridhsize && !fullwidth)
      sparse=1;
    else
     {
      if (CheckSparse(x,y))
	sparse=1;
      else
       {
	if (masked)
	  {
	   DoMGrab(x,y,hw,hw,offset,0);
	   if (bit)
	     {
	      if (setbit)
		{
		 DoBlit(x,y,hw,hw);
		 switch(type)
		 {
		  case TILE8MTYPE:
		  case ALT8MTYPE:
		    if (!cmpt8)
		      break;

		    offset+=Data[TILE8].graphlen[gmode];
		    break;
		  case TILE16MTYPE:
		  case ALT16MTYPE:
		    offset+=Data[TILE16].graphlen[gmode];
		    break;
		  case TILE32MTYPE:
		  case ALT32MTYPE:
		    offset+=Data[TILE32].graphlen[gmode];
		 }
		}
	      else
		{
		 DoMBlit(x,y,hw,hw,0,0);
		 offset+=Data[newtype].graphlen[gmode];
		}

	      //
	      // INC WHICHBIT
	      //
	      switch(type)
	      {
	       case TILE8MTYPE:
	       case ALT8MTYPE:
		 if (!cmpt8)
		   break;

		 T8whichbit++;
		 break;
	       case TILE16MTYPE:
	       case ALT16MTYPE:
		 T16whichbit++;
		 break;
	       case TILE32MTYPE:
	       case ALT32MTYPE:
		 T32whichbit++;
	      }
	     }
	   else
	     {
	      DoMBlit(x,y,hw,hw,0,0);
	      offset+=Data[newtype].graphlen[gmode];
	     }
	  }
	else
	  {
	   DoGrab(x,y,hw,hw,offset);
	   DoBlit(x,y,hw,hw);
	   //
	   // Munge VGA ModeX graphics?
	   //
	   if (ModeX)
	     VL_MungePic((unsigned char far *)databuffer+offset,hw,hw);

	   offset+=Data[newtype].graphlen[gmode];
	  }
       }
     }

//    if (!cmpt8 && sparse)
//      errout("You can't have any SPARSE tiles if your\nTILE8s are grabbed in one clump!");

    if (bit && sparse)
      //
      // INC WHICHBIT
      //
      switch(type)
      {
       case TILE8MTYPE:
       case ALT8MTYPE:
	 if (!cmpt8)
	   break;

	 T8whichbit++;
	 break;
       case TILE16MTYPE:
       case ALT16MTYPE:
	 T16whichbit++;
	 break;
       case TILE32MTYPE:
       case ALT32MTYPE:
	 T32whichbit++;
      }

    Sparse[newtype*totalobjects+loop+Data[newtype].num]=sparse;
    CheckBuffer();
   }

 CountBytes(databuffer,offset);
 AddDataToFile(typestr[newtype]);
 Data[newtype].num+=amount;
}



////////////////////////////////////////////////////////////
//
// PICS
//
////////////////////////////////////////////////////////////
void GrabPics(grabtype type)
{
 int strptr,loop,x,y,w,h;
 char *str,*comp=", ",*comp1="0123456789",name[40],*comp2="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
 long size;


 if (Data[PIC].num==PicAmount)
   {
    settext();
    printf("ERROR: You have reached the limit of PICS that can be grabbed.\n");
    printf("       A change to IGRAB is required -- contact John Romero!\n");
    nosound();
    exit(1);
   }

 str=string+5;


 str=strpbrk(str,comp1);
 x=atoi(str);
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 y=atoi(str)*8;
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 w=atoi(str);
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 h=atoi(str);
 str=strpbrk(str,comp);
 str=strpbrk(str,comp2);
 memset(name,0,40);
 strcpy(name,str);
 for (i=0;i<strlen(name);i++)
   if (name[i]=='\r')
     name[i]=0;

 if (type==PICMTYPE)
   {
    switch(format[0])
      {
       case 'C': CGAMgrab(x*2,y,w*2,h*8,offset,0);
		 DoCGAMblit(x*2,y,w*2,h*8,0,0);
		 size=2L*((w*2)*(h*8));
		 offset+=size;
		 w*=2;
		 break;
       case 'E': EGAMgrab(x,y,w,h*8,offset,0);
		 DoEGAMblit(x,y,w,h*8,0,0);
		 size=5L*(w*(h*8));
		 offset+=size;
		 break;
       case 'V': VGAMgrab(x*8,y,w*8,h*8,offset,0);
		 DoVGAMblit(x*8,y,w*8,h*8,0,0);
		 size=16L*w*h;
		 offset+=size;
		 w*=8;
      }
    (PicmTable+Data[PICM].num)->width=w;
    (PicmTable+Data[PICM].num)->height=h*8;
    _fmemcpy((char far *)PicMNames+Data[PICM].num*NAMELEN,name,NAMELEN);
    PicMOffs[Data[PICM].num]=Data[PICM].offset;
    PicMOffs[Data[PICM].num+1]=Data[PICM].offset+size;
    Data[PICM].offset+=size;
    Data[PICM].num++;
   }
 else
   {
    switch(format[0])
      {
       case 'C': CGAgrab(x*2,y,w*2,h*8,offset);
		 DoCGAblit(x*2,y,w*2,h*8);
		 size=(w*2)*(h*8);
		 offset+=size;
		 w*=2;
		 break;
       case 'E': EGAgrab(x,y,w,h*8,offset);
		 DoEGAblit(x,y,w,h*8);
		 size=(w*(h*8))*4L;
		 offset+=size;
		 break;
       case 'V': VGAgrab(x*8,y,w*8,h*8,offset);
		 DoVGAblit(x*8,y,w*8,h*8);
		 size=64L*w*h;
		 offset+=size;
		 w*=8;
      }
    (PicTable+Data[PIC].num)->width=w;
    (PicTable+Data[PIC].num)->height=h*8;
    _fmemcpy((char far *)PicNames+Data[PIC].num*NAMELEN,(char far *)name,NAMELEN);
    PicOffs[Data[PIC].num]=Data[PIC].offset;
    PicOffs[Data[PIC].num+1]=Data[PIC].offset+size;
    Data[PIC].offset+=size;
    Data[PIC].num++;
   }

 CheckBuffer();
}



////////////////////////////////////////////////////////////
//
// SPRITES
//
////////////////////////////////////////////////////////////
void GrabSprites(void)
{
 int oldh,ychng,strptr,loop,x,y,w,h,xl,yl,xh,yh;
 char *str1,*str,*comp=", ",*comp1="-0123456789",name[40],*comp2="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
 long size;

 if (Data[SPRITE].num==MAXSPRITES)
   {
    settext();
    printf("ERROR: You have reached the limit of SPRITES that can be grabbed.\n");
    printf("       A change to IGRAB is required -- contact John Romero!\n");
    nosound();
    exit(1);
   }

 SpriteTable[Data[SPRITE].num].orgx=0;
 SpriteTable[Data[SPRITE].num].orgy=0;
 SpriteTable[Data[SPRITE].num].shifts=shifts;

 str=string+5;

 str=strpbrk(str,comp1);
 x=atoi(str);
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 y=atoi(str)*8;
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 w=atoi(str);
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 h=atoi(str);
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 xl=frac*atoi(str);
 SpriteTable[Data[SPRITE].num].xl=xl;
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 yl=frac*atoi(str);
 SpriteTable[Data[SPRITE].num].yl=yl;
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 xh=frac*(w*8-atoi(str)-1);
 SpriteTable[Data[SPRITE].num].xh=xh;
 str=strpbrk(str,comp);
 str=strpbrk(str,comp1);
 yh=frac*(h*8-atoi(str)-1);
 SpriteTable[Data[SPRITE].num].yh=yh;
 str=strpbrk(str,comp);
 str=strpbrk(str,comp2);
 memset(name,0,40);
 str1=strpbrk(str,comp);	// scan past name

 if (str1!=NULL)
   {
    strcpy(name,str);
    for (i=0;i<strlen(name);i++)
      if (name[i]==',')
	{
	 name[i]=0;
	 break;
	}

    str1=strupr(str1);
    for (i=0;i<strlen(str1);i++)
      {
       if (!strncmp(str1+i,"OX=",3))
	 {
	  int temp;

	  temp=SpriteTable[Data[SPRITE].num].orgx=8*frac*atoi(str1+i+3);
	  SpriteTable[Data[SPRITE].num].xl+=temp;
	  SpriteTable[Data[SPRITE].num].xh+=temp;
	 }
       else
       if (!strncmp(str1+i,"OY=",3))
	 {
	  int temp;

	  temp=SpriteTable[Data[SPRITE].num].orgy=8*frac*atoi(str1+i+3);
	  SpriteTable[Data[SPRITE].num].yl+=temp;
	  SpriteTable[Data[SPRITE].num].yh+=temp;
	 }
       else
       if (!strncmp(str1+i,"SHIFTS=",7))
	 SpriteTable[Data[SPRITE].num].shifts=atoi(str1+i+7);
      }
   }
 else
   {
    strcpy(name,str);
    for (i=0;i<strlen(name);i++)
      if (name[i]=='\r')
	name[i]=0;
   }

 h*=8;
 switch(format[0])
   {
    case 'C': SpriteTable[Data[SPRITE].num].shifts/=2;
	      CGAMgrab(x*2,y,w*2,h,offset,1);
	      oldh=h;
	      ychng=Optimum.y-y;
	      if (Optimum.height)
		h=SpriteTable[Data[SPRITE].num].height=Optimum.height-ychng+1;
	      if (Optimum.y)
		{
		 SpriteTable[Data[SPRITE].num].orgy+=ychng*frac;
		 y=Optimum.y;
		}
	      DoCGAMblit(x*2,y,w*2,h,ychng,oldh-(h+ychng));
	      size=2L*(w*2)*h;
	      offset+=size;
	      w*=2;
	      break;
    case 'E': EGAMgrab(x,y,w,h,offset,1);
	      oldh=h;
	      ychng=Optimum.y-y;
	      if (Optimum.height)
		h=SpriteTable[Data[SPRITE].num].height=Optimum.height-ychng+1;
	      if (Optimum.y)
		{
		 SpriteTable[Data[SPRITE].num].orgy+=ychng*frac;
		 y=Optimum.y;
		}
	      DoEGAMblit(x,y,w,h,ychng,oldh-(h+ychng));
	      size=w*h*5L;
	      offset+=size;
	      break;
    case 'V': SpriteTable[Data[SPRITE].num].shifts=1;
	      VGAMgrab(x*8,y,w*8,h,offset,1);
	      oldh=h;
	      ychng=Optimum.y-y;
	      if (Optimum.height)
		h=SpriteTable[Data[SPRITE].num].height=Optimum.height-ychng+1;
	      if (Optimum.y)
		{
		 SpriteTable[Data[SPRITE].num].orgy+=ychng*frac;
		 y=Optimum.y;
		}
	      DoVGAMblit(x*8,y,w*8,h,ychng,oldh-(h+ychng));
	      size=16L*w*h;
	      offset+=size;
	      w*=8;
   }

 SpriteTable[Data[SPRITE].num].width=w;
 SpriteTable[Data[SPRITE].num].height=h;
 _fmemcpy((char far *)SpriteNames+Data[SPRITE].num*NAMELEN,(char far *)name,NAMELEN);
 SpriteOffs[Data[SPRITE].num]=Data[SPRITE].offset;
 SpriteOffs[Data[SPRITE].num+1]=Data[SPRITE].offset+size;
 Data[SPRITE].offset+=size;
 Data[SPRITE].num++;
 CheckBuffer();
}
