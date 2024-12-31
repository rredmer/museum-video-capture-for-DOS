*
*	gspasm.asm : Assembler routine for HRCAP
*

	.include	"d:\gsptools\gspregs.hdr"

	.global	_clr,_fill,_fillXY
	.global	_PixBltXY
	.global	_PixBltL,_PixBltXYL,_PixBltLXY,_PixBltBXY
	.global	_PixBltL32to24,_PixBltL24to32,_PixBltL24to24
	.global	_SetPitch,_GetPitch
	.global	_SetOffset,_GetOffset,_SetControl,_GetControl
	.global	_GetPix,_PutPix,_GetPXY,_PutPXY,_GetPMask,_SetPMask
	.global	_GetColor,_SetColor,_GetPSize,_SetPSize

	.global	_movemem16,_movemem24,_movemem32
	.global	_mv24to32
	.global	__Cursor16,__Cursor32


src	.set	a0
dst	.set	a1
crs	.set	a2
dydx	.set	a3

; _Cursor16(int src, int x, int y);
__Cursor16:
	mmtm	SP,src,dst,dydx
	setf	16,0,0
	move	-*STK,src,1	;cursor pattern pointer
	move	src,SADDR
	move	-*STK,dst,1	;destination address (XY)
	move	-*STK,a8,1
	sll	16,a8
	movy	a8,dst
	move	dst,DADDR
	movi	16*65536+16,DYDX
	movi	0BDEFH,a8
	move	a8,@PMASK,1
	movk	16,a8
	move	a8,SPTCH
	lmo	a8,a8
	move	a8,@CONVSP
	pixblt	b,xy
	mmfm	SP,src,dst,dydx
	rets	2

; _Cursor32(int src, int x, int y);
__Cursor32:
	mmtm	SP,src,dst,dydx
	setf	16,0,0
	move	-*STK,src,1	;cursor pattern pointer
	move	src,SADDR
	move	-*STK,dst,1	;destination address (XY)
	add	dst,dst
	move	-*STK,a8,1
	sll	16,a8
	movy	a8,dst
	move	dst,DADDR
	movi	16*65536+32,DYDX
	movi	7F7FH,a8
	move	a8,@PMASK,1
	movk	32,a8
	move	a8,SPTCH
	lmo	a8,a8
	move	a8,@CONVSP
	pixblt	b,xy
	mmfm	SP,src,dst,dydx
	rets	2

; _Cursor16(int src, int x, int y);
;__Cursor16:
;	mmtm	SP,src,dst,crs,dydx
;	setf	16,0,0
;	move	-*STK,src,1	;cursor pattern pointer
;	move	-*STK,dst,1	;destination address (XY)
;	move	-*STK,a8,1
;	sll	16,a8
;	movy	a8,dst
;	movi	16*65536+16,crs
;
;	movi	0BDEFH,a8
;	move	a8,@PMASK,1
;	movk	16,a8
;	move	a8,SPTCH
;	lmo	a8,a8
;	move	a8,@CONVSP
;
;	move	src,SADDR
;	move	dst,DADDR
;	move	crs,DYDX
;	move	@CONTROL,a8
;	andi	0FF3FH,a8	;set W=01
;	ori	0040H,a8
;	move	a8,@CONTROL
;	pixblt	b,xy
;	move	DADDR,dydx
;	
;	subxy	dst,dydx
;	movx	dydx,a8
;	srl	12,dydx		;y*16
;	add	dydx,a8		;+x
;	add	a8,src
;	move	src,SADDR
;	
;	move	@CONTROL,a8
;	ori	00C0H,a8
;	move	a8,@CONTROL
;	pixblt	b,xy
;
;	mmfm	SP,src,dst,crs,dydx
;	rets	2
;
; _Cursor32(int src, int x, int y);
;__Cursor32:
;	mmtm	SP,src,dst,crs,dydx
;	setf	16,0,0
;	move	-*STK,src,1	;cursor pattern pointer
;	move	-*STK,dst,1	;destination address (XY)
;	sll	1,dst		;x*2
;	move	-*STK,a8,1
;	sll	16,a8
;	movy	a8,dst
;	movi	16*65536+32,crs
;
;	movi	07F7FH,a8
;	move	a8,@PMASK,1
;	movk	32,a8
;	move	a8,SPTCH
;	lmo	a8,a8
;	move	a8,@CONVSP
;
;	move	src,SADDR
;	move	dst,DADDR
;	move	crs,DYDX
;	move	@CONTROL,a8
;	andi	0FF3FH,a8	;set W=01
;	ori	0040H,a8
;	move	a8,@CONTROL
;	pixblt	b,xy
;	move	DADDR,dydx
;	
;	subxy	dst,dydx
;	movx	dydx,a8
;	srl	11,dydx		;y*32
;	add	dydx,a8		;+x
;	add	a8,src
;	move	src,SADDR
;	
;	move	@CONTROL,a8
;	ori	00C0H,a8
;	move	a8,@CONTROL
;	pixblt	b,xy
;
;	mmfm	SP,src,dst,crs,dydx
;	rets	2

; mv24to32(int src,int dst,int cnt);
_mv24to32:
	mmtm	SP,a0,a1,a2
	move	-*STK,a0,1	;src
	move	-*STK,a1,1	;dst
	move	-*STK,a2,1	;cnt
	setf	24,0
MV24TO32:
	move	*a0+,*a1+
	addk	8,a1
	dsjs	a2,MV24TO32

	mmfm	SP,a0,a1,a2
	rets	2

; movemem(int src,int dst,int cnt);
_movemem16:
	mmtm	SP,a0,a1,a2
	move	-*STK,a0,1	;src
	move	-*STK,a1,1	;dst
	move	-*STK,a2,1	;cnt
	setf	16,0
MV16:
	move	*a0+,*a1+
	dsjs	a2,MV16

	mmfm	SP,a0,a1,a2
	rets	2

_movemem24:
	mmtm	SP,a0,a1,a2
	move	-*STK,a0,1	;src
	move	-*STK,a1,1	;dst
	move	-*STK,a2,1	;cnt
	setf	24,0
MV24:
	move	*a0+,*a1+
	dsjs	a2,MV24

	mmfm	SP,a0,a1,a2
	rets	2

_movemem32:
	mmtm	SP,a0,a1,a2
	move	-*STK,a0,1	;src
	move	-*STK,a1,1	;dst
	move	-*STK,a2,1	;cnt
	setf	32,0
MV32:
	move	*a0+,*a1+
	dsjs	a2,MV32

	mmfm	SP,a0,a1,a2
	rets	2

; int GetPix(int address);
_GetPix:
	move	-*STK,a8,1
	move	*a8,a8,1
	rets	2

; int PutPix(int address,int data);
_PutPix:
	move	a0,-*SP,1
	move	-*STK,a8,1	;address
	move	-*STK,a0,1	;data
	move	a0,*a8,1
	move	*SP+,a0,1
	rets	2

; int GetPSize();
_GetPSize:
	setf	16,0,0
	move	@PSIZE,a8
	rets

; SetPSize(int size);
_SetPSize:
	setf	16,0,0
	move	-*STK,a8,1
	move	a8,@PSIZE
	rets	2

; GetColor(int *col0, int *col1);
_GetColor:
	move	a0,-*SP,1
	move	-*STK,a8,1	;col0
	move	COLOR0,a0
	move	a0,*a8,1
	move	-*STK,a8,1	;col1
	move	COLOR1,a0
	move	a0,*a8,1
	move	*SP+,a0,1
	rets	2

; SetColor(int col0, int col1);
_SetColor:
	move	-*STK,a8,1
	move	a8,COLOR0
	move	-*STK,a8,1
	move	a8,COLOR1
	rets	2

; GetControl();
_GetControl:
	setf	16,0,0
	move	@CONTROL,a8,0
	rets

; SetControl(int control);
_SetControl:
	setf	16,0,0
	move	-*STK,a8,1
	move	a8,@CONTROL,0
	rets	2

; GetOffset();
_GetOffset:
	move	OFFSET,a8
	rets

; SetOffset(int addr);
_SetOffset:
	move	-*STK,a8,1
	move	a8,OFFSET
	rets	2

; int GetPXY(int y, int x);
_GetPXY:
	move	a0,-*SP,1
	move	-*STK,a8,1	;y
	sll	16,a8
	move	-*STK,a0,1	;x
	movx	a0,a8
	pixt	*a8.xy,a8
	move	*SP+,a0,1
	rets	2

; PutPXY(int y, int x, int data);
_PutPXY:
	move	a0,-*SP,1
	move	-*STK,a8,1	;y
	sll	16,a8
	move	-*STK,a0,1	;x
	movx	a0,a8
	move	-*STK,a0,1	;data
	pixt	a0,*a8.xy
	move	*SP+,a0,1
	rets	2

; PixBltBXY (int src, int x, int y, DyDx dydx);
_PixBltBXY:
	move	a0,-*SP,1
	move	-*STK,a8,1	;src
	move	a8,SADDR
	move	-*STK,a8,1	;x
	move	-*STK,a0,1	;y
	sll	16,a0
	movy	a0,a8
	move	a8,DADDR
	move	-*STK,a8,1	;dydx
	move	a8,DYDX
	pixblt	b,xy
	move	*SP+,a0,1
	rets	2

; PixBltXY (DyDx src, DyDx dst, DyDx dydx);
_PixBltXY:
	move	-*STK,a8,1	;src
	move	a8,SADDR
	move	-*STK,a8,1	;dst
	move	a8,DADDR
	move	-*STK,a8,1	;dydx
	move	a8,DYDX
	pixblt	xy,xy
	rets	2

; GetPitch(int *sp, int *dp);
_GetPitch:
	move	a0,-*SP,1
	move	-*STK,a8,1	;sp
	move	SPTCH,a0
	move	a0,*a8,1
	move	-*STK,a8,1	;dp
	move	DPTCH,a0
	move	a0,*a8,1
	move	*SP+,a0,1
	rets	2

; SetPitch(int spitch, int dpitch);
_SetPitch:
	setf	16,0,0
	move	-*STK,a8,1	;spitch
	move	a8,SPTCH
	lmo	a8,a8
	move	a8,@CONVSP,0
	move	-*STK,a8,1	;dpitch
	move	a8,DPTCH
	lmo	a8,a8
	move	a8,@CONVDP,0
	rets	2

*_SetPitch:
*	setf	16,0,0
*	move	a0,-*SP,1
*	move	-*STK,a8,1	;spitch
*	move	a8,SPTCH
*	lmo	a8,a0
*	move	a0,@CONVSP,0
*	move	-*STK,a8,1	;dpitch
*	move	a8,DPTCH
*	lmo	a8,a0
*	move	a0,@CONVDP,0
*	move	*SP+,a0,1
*	rets	2

; int GetPMask();
_GetPMask:
	move	@PMASK,a8,1
	rets

; SetPMask(int mask);
_SetPMask:
	move	-*STK,a8,1	;mask
	move	a8,@PMASK,1
	rets	2

; PixBltXYL(DyDx src, int dst, int dydx);
_PixBltXYL:
	move	-*STK,a8,1	;src
	move	a8,SADDR
	move	-*STK,a8,1	;dst
	move	a8,DADDR
	move	-*STK,a8,1	;dydx
	move	a8,DYDX
	pixblt	xy,l
	rets	2

; PixBltLXY(int src, DyDx dst, int dydx);
_PixBltLXY:
	move	-*STK,a8,1	;src
	move	a8,SADDR
	move	-*STK,a8,1	;dst
	move	a8,DADDR
	move	-*STK,a8,1	;dydx
	move	a8,DYDX
	pixblt	l,xy
	rets	2

; PixBltL32to24(int src, int dst, int width, int height);
_PixBltL32to24:
	mmtm	SP,a0,a1,a2,a3,a4
	move	-*STK,a0,1	;src
	move	-*STK,a1,1	;dst
	move	-*STK,a2,1	;width
	move	-*STK,a3,1	;height
	setf	16,0,0
	move	@PSIZE,a4
	movk	8,a8		;set pixel size 8
	move	a8,@PSIZE

	movk	3,a8
	sll	16,a3
	movy	a3,a8
	move	a8,DYDX
PIX320:
	move	a0,SADDR
	move	a1,DADDR
	pixblt	l,l
	addk	32,a0
	addk	24,a1
	dsjs	a2,PIX320

	move	a4,@PSIZE
	mmfm	SP,a0,a1,a2,a3,a4
	rets	2

; PixBltL24to24(int src, int dst, int width, int height);
_PixBltL24to24:
	mmtm	SP,a0,a1,a2,a3,a4
	move	-*STK,a0,1	;src
	move	-*STK,a1,1	;dst
	move	-*STK,a2,1	;width
	move	-*STK,a3,1	;height
	setf	16,0,0
	move	@PSIZE,a4
	movk	8,a8		;set pixel size 8
	move	a8,@PSIZE

	move	a2,a8
	add	a2,a8		;*2
	add	a2,a8		;*3
	sll	16,a3
	movy	a3,a8
	move	a8,DYDX

	move	a0,SADDR
	move	a1,DADDR
	pixblt	l,l

	move	a4,@PSIZE
	mmfm	SP,a0,a1,a2,a3,a4
	rets	2

; PixBltL24to32(int src, int dst, int width, int height);
_PixBltL24to32:
	mmtm	SP,a0,a1,a2,a3,a4
	move	-*STK,a0,1	;src
	move	-*STK,a1,1	;dst
	move	-*STK,a2,1	;width
	move	-*STK,a3,1	;height
	setf	16,0,0
	move	@PSIZE,a4
	movk	8,a8		;set pixel size 8
	move	a8,@PSIZE

	movk	3,a8
	sll	16,a3
	movy	a3,a8
	move	a8,DYDX
PIX240:
	move	a0,SADDR
	move	a1,DADDR
	pixblt	l,l
	addk	24,a0
	addk	32,a1
	dsjs	a2,PIX240

	move	a4,@PSIZE
	mmfm	SP,a0,a1,a2,a3,a4
	rets	2

; PixBltL(int src, int dst, int dydx);
_PixBltL:
	move	-*STK,a8,1	;src
	move	a8,SADDR
	move	-*STK,a8,1	;dst
	move	a8,DADDR
	move	-*STK,a8,1	;dydx
	move	a8,DYDX
	pixblt	l,l
	rets	2

; clr(int str, DyDx dydx);
_clr:
	move	-*STK,a8,1	;str
	move	a8,DADDR
	move	-*STK,a8,1	;dydx
	move	a8,DYDX
	clr	COLOR1
	fill	l
	rets	2

; fill(int str, DyDx dydx, int color);
_fill:
	move	-*STK,a8,1	;str
	move	a8,DADDR
	move	-*STK,a8,1	;dydx
	move	a8,DYDX
	move	-*STK,a8,1	;color
	move	a8,COLOR1
	fill	l
	rets	2
	
; fill(DyDx str, DyDx dydx, int color);
_fillXY:
	move	-*STK,a8,1	;str
	move	a8,DADDR
	move	-*STK,a8,1	;dydx
	move	a8,DYDX
	move	-*STK,a8,1	;color
	move	a8,COLOR1
	fill	xy
	rets	2
	

	.end

;*** end of gspasm.asm ***

