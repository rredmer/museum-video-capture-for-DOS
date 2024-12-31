/*
**	ghrcap.h :	Header file for HRCAP (GSP side)
*/

#include	<gspregs.h>
#include	<setjmp.h>

#ifndef	__GHRCAP__
#define	__GHRCAP__

typedef 	unsigned short	Word,*Wptr;
typedef		int	Boolean;
#define		true	1
#define		false	0


/* CRT mode */
#define	StdPal		0
#define	NonIntPal	1
#define	HiRes		2

/* Capture mode */
#define	X1			1
#define	X2			2
#define	X3			3
#define	X6			6

/* Display resolution */
#define	HiresH		1024
#define	HiresV		768

#define	PalResH		736
#define	PalResV		576


/* Capture resolution */
#define	PalCapH		736
#define	PalCapV		576

#define	HiresCapH	1024
#define	HiresCapV	768
#define	MaxV4M		1000	/* max vertical resolution with 4M Vista */


/* Communication bit */
#define	INTOUT		0x0080	/* MSG interrupt out */
#define	MSGOUT		0x0070	/* message out */
#define	INTIN		0x0008	/* MSG interrupt in */
#define	MSGIN		0x0007	/* message in */


/* Command message */
#define	CMDMSG		1		/* message from host */
#define	CURMSG		2
#define	SIZMSG		3
#define	ZOMMSG		4
#define	PIXMSG		5
#define	CANMSG		7


/* AtVista register/memory address */
#define	ROWTBL	0xF8000000
#define	ROWSTR	1013			/* row start index */

#define	VRAMSTR	0xFE000000	/* 4MB AtVista memory start */
#define	VRAMEND	0xFFFB0000

#define	LUT0	0xFFFB0000
#define	LUT1	0xFFFB8000
#define	LUT2	0xFFFC0000
#define	LUT3	0xFFFC8000
#define	LUT4	0xFFFD0000
#define	LUT5	0xFFFD8000
#define	LUT6	0xFFFE0000
#define	LUT7	0xFFFE8000

#define	PITCH	0x8000		/* default image pitch */


/* TGA file format */
#define	TGAType2	2
#define	TGAType3	3
#define	TGAType10	10
#define	TGAType11	11


/* AtVista i/o register */
#define	VIDEO	0xF8400000	/* vista video register */
#define	VIDEOS	0xFFFFDF40	/* video register shadow address */
#define	MODE	0xF8600000	/* vista mode register */
#define	MODES	0xFFFFDF60	/* mode register shadoe address */

/* host control register bit mask */
#define	HLT		0x8000		/* halt gsp */
#define	CF		0x4000		/* cache flush */
#define	LBL		0x2000		/* lower byte lost */
#define	INCR	0x1000		/* increment on read cycle */
#define	INCW	0x0800		/* increment on write cycle */
#define	NMIMODE	0x0200		/* NMI mode (no stack usage */
#define	NMI		0x0100		/* NMI int */

/* key code */
#define	STX		0x02
#define	ETX		0x03
#define	ENQ		0x05
#define	ACK		0x06
#define	BS		0x08
#define	TAB		0x09
#define	CR		0x0D
#define	NAK		0x15
#define	CAN		0x18
#define	ESC		0x1B
#define	SPACE	0x20


/*========= DATA STRUCTURE ==========*/
typedef	int	DyDx;

struct Pix32 {
	unsigned b:8;
	unsigned g:8;
	unsigned r:8;
	unsigned a:8;
};
typedef struct Pix32 Pix32;

struct Pix24 {
	unsigned b:8;
	unsigned g:8;
	unsigned r:8;
};
typedef struct Pix24 Pix24;

struct Pix16 {
	unsigned b:5;
	unsigned g:5;
	unsigned r:5;
	unsigned a:1;
};
typedef struct Pix16 Pix16;

struct Rect{
	short	left;
	short	top;
	short	right;
	short	bottom;
};
typedef struct Rect Rect;

struct Point {
	int	v;
	int	h;
};
typedef struct Point Point;

struct GPort {
	unsigned long 	baseAddr;
	unsigned long	rowBits;
	Word			psize;
	Rect			portRect;
};
typedef struct GPort GPort, *GPtr;	

struct EventRecord {
	short cmdcode;
	union {
		unsigned long lmes;
		short smes[2];
	} message;
};
typedef struct EventRecord EventRecord;

struct VistaENV {
	Word	rsv0;			/* reserved */
	Word	vistaMem;		/* Vista memory (MB) */
	unsigned long	vistaStr;		/* vista memory start address */
	unsigned long	vistaEnd;		/* vram end address */
	Word	*commbuf;		/* communication buffer address pointer */
};
typedef struct VistaENV VistaENV;

struct DspCapINFO {
	Word	genlock;		/* AtVista sync mode */
	Word	imageType;		/* image type, same as TGA file type */
	Word	pixSize;		/* pixel depth */
	Word	capMode;		/* capture mode */
	Word	horzRes;		/* horizontal resolution */
	Word	vertRes;		/* vertical resolution */
	Word	dspMode;		/* display mode */
	Word	horzCrtRes;		/* horizontal crt resolution */
	Word	vertCrtRes;		/* vertical crt resolution */
	GPort	imagePort;		/* image data port on AtVista */
	Rect	capRect;		/* capture rectangle in image port */
	Rect 	dspRect;		/* display rectangle in imagePort.portRect */
	GPort	dspPort;		/* display device graf port */
	GPort	devPort;		/* phisical device graf port */
};
typedef struct DspCapINFO DspCapINFO;

struct VideoMode {
	Word	video;
	Word	mode;
};
typedef struct VideoMode VideoMode;


enum	{
	slave, master
};


/*================ MACRO ================*/
#define	max(a,b)	(((a) > (b)) ? (a) : (b))
#define	min(a,b)	(((a) > (b)) ? (b) : (a))


/*============ GLOBAL DATA ==============*/
extern	GPtr		thePort;	/* current graf port pointer */
extern	DspCapINFO	DspCapInfo;	/* display and capture informations */
extern	VistaENV	VistaEnv;	/* vista environment */
extern	Word 		CaptureRegs[];

/*
**	function prototypes
*/

/*** from ghrcap.c ***/
extern	WaitVBlnk();
extern	WaitNVBlnk(int n);
extern	SetVReg(Word data);
extern	SetMReg(Word data);
extern	Msg2Host(int msg);
extern	GPtr GetPort();
extern	SetPort(GPtr port);
extern	InitRowTable(int rows);
extern	InitAtVista();
extern	SetVstaRegs(Wptr regs);
extern	GetEvent(EventRecord *event);

/*** from gspasm.asm ***/
extern	_Cursor32(int src, int x, int y);
extern	_Cursor16(int src, int x, int y);
extern	PixBltL24to24(int src,int dst,int width,int height);
extern	PixBltL24to32(int src,int dst,int width,int height);
extern	PixBltL32to24(int src,int dst,int width,int height);
extern	mv24to32(int src,int dst,int cnt);
extern	movemem(int src,int dst,int cnt);
extern	movemem24(int src,int dst,int cnt);
extern	movemem32(int src,int dst,int cnt);
extern	int GetPix(int address);
extern	int PutPix(int address,int data);
extern	int GetPSize();
extern	SetPSize(int size);
extern	GetColor(int *col0, int *col1);
extern	SetColor(int col0, int col1);
extern	int GetControl();
extern	SetControl(int control);
extern	int GetOffset();
extern	SetOffset(int addr);
extern	int GetPXY(int y, int x);
extern	PutPXY(int y, int x, int data);
extern	PixBltBXY(int src, int x, int y, DyDx);
extern	PixBltXY(DyDx src, DyDx dst, DyDx dydx);
extern	GetPitch(int *sp, int *dp);
extern	SetPitch(int spitch, int dpitch);
extern	int GetPMask();
extern	SetPMak(int mask);
extern	PixBltXYL(DyDx src, int dst, DyDx dydx);
extern	PixBltLXY(int src, DyDx dst, DyDx dydx);
extern	PixBltL(int src, int dst, DyDx dydx);
extern	clr(int str, DyDx dydx);
extern	fill(int str, DyDx dydx, int color);
extern	fillXY(DyDx str, DyDx dydx, int color);

/*** from gdisplay.c ***/
extern	ToLowerLeft(GPtr port,Rect *sr,Rect *dr);
extern	ClearRect(GPtr port,Rect *r);
extern	CopyBits(GPtr sport,GPtr dport,Rect *sr,Rect *dr,int direction);
extern	InvalDevPort();
extern	DGetPXY(int x,int y);
extern	DPutPXY(int x,int y,int data);
extern	Boolean PtInRect(int h,int v, Rect *r);
extern	OffsetRect(Rect *rect, int dh, int dv);
extern	Boolean SectRect(Rect *sr1,Rect *sr2,Rect *dr);
extern	InSetRect(Rect *rect, int dh, int dv);
extern	SetRect(Rect *rect, int left, int top, int right, int bottom);
extern	ClipRect(Rect *r);
extern	InitCursor(int kind);
extern	DoDisplay(int kind);

/*** from ggrab.c ***/
extern	GrabFrame();
extern	GrabOne();
extern	DoGrab();

/*** from glive.c ***/
extern	DoLive();
extern	DoCalibLive();
extern	DoGammaLive();
extern	DoColorBarLive();
extern	DoMasterLive(VideoMode *regp);
extern	DoSlaveLive(VideoMode *regp);

/*** from genhance.c ***/
extern	DoEnhance(GPtr port,Rect *r);
extern	DoGreyScale(GPtr port,Rect *r);
extern	DoRGBMatrix(GPtr port,Rect *r);

/*** from gintr.asm ***/
extern	HostIntr();

/*** from genhasm.asm ***/
extern	_enhance32(Pix32 *src, Pix32 p0[], Pix32 p1[], Pix32 p2[], int cnt);
extern	_enhance24(Pix24 *src, Pix24 p0[], Pix24 p1[], Pix24 p2[], int cnt);
extern	_enhance16(Pix16 *src, Pix16 p0[], Pix16 p1[], Pix16 p2[], int cnt);
extern	_Gray32(long baseAddr,long rowBits,int height,int width);
extern	_Gray24(long baseAddr,long rowBits,int height,int width);
extern	_Gray16(long baseAddr,long rowBits,int height,int width);

#endif


/*** end of ghrcap.h ***/
