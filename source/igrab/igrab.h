#include <memory.h>
#include <io.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <alloc.h>
#include <bios.h>
#include <fcntl.h>
#include <time.h>

#include "xms.h"

#define VERSION		"0.40"
#define TITLESTR	"IGRAB v"VERSION" by John Romero (C) 1991 Id Software\n\r"
#define NUMBITARRAY	600
#define CHKNAMELEN	20
#define NAMELEN		32
#define FNAMELEN	13

#define BUFFERSIZE	0x14000L
#define MAXCOMPSIZE	0x14000L

#define BUFFER1SIZE	0x1d000L	// for REALLY huge EGA screens
#define MAXCOMP1SIZE	0x1d000L

#define MAXFONT		10
#define MAXPICS		200
#define MAXSPRITES	800
#define MAXGRID8	19*12
#define MAXGRID16	12*8
#define MAXGRID32	7*4
#define MAXALT8		36*21
#define MAXALT16	18*12
#define MAXALT32	9*6
#define GRID8H		19
#define ALT8H		36
#define GRID16H		12
#define ALT16H		18
#define GRID32H		7
#define ALT32H		9
#define MAXOFFS		6000

#define GCindex		0x3ce
#define GCmode		5
#define GCreadmap	4
#define SCindex		0x3c4
#define SCmapmask	2

//
// TYPEDEFS
//
typedef struct
{
  unsigned bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;

typedef struct { unsigned width,height;
	       } PicStruct;


typedef struct { int width,height;
		 int orgx,orgy;
		 int xl,yl,xh,yh;
		 int shifts;
	       } SprStruct;

typedef struct { int width;
		 int height;
		 int planes;
	       } LBMtype;

//
// STRUCTURE OF THE "GFXINFO?.EXT" FILE
//
typedef struct {
		 int num8,num8m,num16,num16m,num32,num32m;
		 int off8,off8m,off16,off16m,off32,off32m;
		 int numpics,numpicm,numsprites;
		 int offpic,offpicm,offsprites;
		 int offpicstr,offpicmstr,offsprstr;
		 int numexterns,offexterns;
	       } InfoStruct;

typedef enum {FONTTYPE,FONTMTYPE,TILE8TYPE,ALT8TYPE,TILE8MTYPE,ALT8MTYPE,
      TILE16TYPE,ALT16TYPE,TILE16MTYPE,ALT16MTYPE,TILE32TYPE,ALT32TYPE,
      TILE32MTYPE,ALT32MTYPE,PICTYPE,PICMTYPE,SPRITETYPE }grabtype;

typedef enum {FONT,FONTM,TILE8,TILE8M,TILE16,TILE16M,TILE32,TILE32M,
	      PIC,PICM,SPRITE}datatype;

typedef enum {CGA,EGA,VGA,SGA}graphtype;

typedef struct { unsigned num,graphlen[4];
		 long offset; } DataStruct;

typedef struct { unsigned y,height; } OptStruct;

typedef enum {DATA,CODE,FARDATA} segtype;	// FOR MAKEOBJ ONLY

//
// PROTOTYPES
//

//
// IGRAB
//
char huge *LoadLBM(char *filename,LBMtype *);
char CharEdgeCheck(int x,int y);
int MakeOBJ(char *filename,char *destfilename,char *public,segtype whichseg,char *farname);
void CheckBuffer(void);
void DeleteTmpFiles(void);
void AddDataToFile(char *filename);
void FlushData(void);
void videomode(int planes);
void SaveFile(char *filename,char huge *buffer, long size,long offset);
unsigned long LoadFile(char *filename,char huge *buffer,long offset,long size);
void errout(char *string);
void settext(void);

//
// GRABCGA
//
void CGAgrab(int x,int y,int width,int height,unsigned offset);
void DoCGAblit(int x,int y,int width,int height);
void CGAblit(int x,int y,int width,int height,char huge *buffer);
void CGAMgrab(int x,int y,int width,int height,unsigned offset,int optimize);
void DoCGAMblit(int x,int y,int width,int height,int yadd,int hadd);
void CGAMblit(int x,int y,int width,int height,char huge *buffer);

//
// GRABEGA
//
void EGAgrab(int x,int y,int width,int height,long offset);
void DoEGAblit(int x,int y,int width,int height);
void EGAblit(int x,int y,int width,int height,char huge *buffer);
void EGAMgrab(int x,int y,int width,int height,long offset,int optimize);
void DoEGAMblit(int x,int y,int width,int height,int yadd,int hadd);
void EGAMblit(int x,int y,int width,int height,char huge *buffer);

//
// GRABVGA
//
void VGAgrab(int x,int y,int width,int height,unsigned offset);
void DoVGAblit(int x,int y,int width,int height);
void VGAblit(int x,int y,int width,int height,char huge *buffer);
void VGAMgrab(int x,int y,int width,int height,unsigned offset,int optimize);
void DoVGAMblit(int x,int y,int width,int height,int yadd,int hadd);
void VGAMblit(int x,int y,int width,int height,char huge *buffer);

//
// FINISH
//
void CreateHeaders(void);
void FinishUp(void);
void FindType(char *string);
void LoadKeyword(char *string,int grabbed);
void FreeXMSbuffs(int *handle1,int *handle2,char *filename,long start,long size);
void DispStatusScreen(void);
int CheckXMSamount(char *filename,int *handle1,int *handle2);
long filelen(char *filename);
void CompressData(void);
void CompressFonts(void);
void CompressPics(void);
void CompressSprites(void);
void Compress8(void);
void Compress16(void);
void Compress32(void);
void CompressMisc(void);
void SetupFinish(void);
void CreateOffsets(void);
void CompressSpecial(void);
void CreateGraphFiles(void);
void CreateOBJs(void);
void UpdateWindow(void);
void VL_MungePic (unsigned char far *source, unsigned width, unsigned height);

//
// DOGRAB
//
void GrabFont(grabtype type);
void DoGrab(int x,int y,int width,int height,unsigned offset);
void DoMGrab(int x,int y,int width,int height,unsigned offset,int opt);
void DoMBlit(int x,int y,int width,int height,int yadd,int hadd);
void DoBlit(int x,int y,int width,int height);
void GrabTile(grabtype type);
void GrabPics(grabtype type);
void GrabSprites(void);

//
// JHUFF
//
void CountBytes (unsigned char huge *start, long length);
long HuffCompress (unsigned char huge *source, long length,
  unsigned char huge *dest);

//
// COMP_A.ASM
//
long FastHuffCompress (unsigned char huge *source, long length,
  unsigned char huge *dest);

//
// VARS
//
extern time_t tblock;
extern LBMtype CurrentLBM;
extern PicStruct far *PicTable,far *PicmTable;
extern SprStruct SpriteTable[MAXSPRITES];
extern FILE *fp;
extern grabtype type;
extern graphtype gmode;
extern DataStruct Data[11];
extern OptStruct Optimum;

extern char typestr[5];

extern char picname[64],huge *Sparse;

extern char huge *PicNames,huge *SpriteNames,huge *PicMNames,
	    huge *ChunkNames,huge *MiscNames,huge *MiscFNames;

extern unsigned char format[2], scriptname[64],dest[13],string[80],ext[10],huge *lbmscreen,
     huge *databuffer,huge *maskscreen,ScreenColor,huge *T8bit,huge *T16bit,huge *T32bit,
     typelist[17][10],path[64],NumMisc;
extern long offset,size,fsize,tile8off,tile8moff,tile16off,tile16moff,
     tile32off,tile32moff,picoff,picmoff,spriteoff,fontoff,fontmoff,
     FontOffs[MAXFONT],far *PicOffs,SpriteOffs[MAXSPRITES],FontMOffs[MAXFONT],
     far *PicMOffs,bufmax,comp_size;
extern unsigned begin,j,i,gotstr,frac,temp,keycheck,compress,
	 end,gottiles,globalx,globaly,globalmaxh,nostacking,noshow,shifts,
	 fastgrab,totalobjects,leavetmp,cmpt8,genobj,ChunkStart[MAXSPRITES/8],
	 ChunkEnd[MAXSPRITES/8],whichchunk,ChunkType[MAXSPRITES/8],bit,
	 T8whichbit,T16whichbit,T32whichbit,setbit,lumpactive,SkipToStart,
	 Do4offs,ModeX,PicAmount;

extern char far SCREEN;
extern int handle;

extern long counts[256];
extern unsigned long huffstring[256];
extern huffnode nodearray[256];	// 256 nodes is worst case
