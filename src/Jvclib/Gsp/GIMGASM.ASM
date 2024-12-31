
	.include	"d:\gsptools\gspregs.hdr"

	.global __enhance16,__enhance24,__enhance32
	.global	__Gray32,__Gray24,__Gray16

	.global	_commbuf,_GetEvent

	.bss	_event,48,16

esrc	.set	a0
ep0	.set	a2
ep1	.set	a4
ep2	.set	a6
cnt	.set	a10
rgbc	.set	a5
drgb	.set	a3
etmp	.set	a1
work	.set	a8

;_enhance32(esrc,ep0,ep1,ep2,cnt)
__enhance32:
	mmtm	SP,esrc,ep0,ep1,ep2,cnt,drgb,rgbc,etmp
	move	-*STK,esrc,1
	move	-*STK,ep0,1
	move	-*STK,ep1,1
	move	-*STK,ep2,1
	move	-*STK,cnt,1

	addk	32,esrc
	setf	8,0
ENH320:
	movk	3,rgbc
ENH3200:
	move	*ep0,drgb
	move	*ep0(64),work
	add	work,drgb
	move	*ep1,work
	add	work,drgb
	move	*ep1(64),work
	add	work,drgb
	move	*ep2,work
	add	work,drgb
	move	*ep2(64),work
	add	work,drgb
	move	drgb,work

	add	drgb,drgb	;*3
	add	work,drgb
	move	drgb,etmp

	move	*ep0(32),drgb
	move	*ep2(32),work
	add	work,drgb
	sll	2,drgb		;*4

	add	drgb,etmp

	move	*ep1(32),work
	sll	1,work		;*26
	move	work,drgb
	sll	2,work
	add	work,drgb
	sll	1,work
	add	work,drgb

	sub	etmp,drgb
	jrz	ENH323

	move	*esrc,work
	sll	2,work		;*52
	move	work,etmp
	sll	2,work
	add	work,etmp
	sll	1,work
	add	work,etmp
	add	drgb,etmp
	jrp	ENH321
	clr	etmp
	jruc	ENH322
ENH321:
	movi	52,work
	divu	work,etmp
	cmpi	255,etmp
	jrle	ENH322
	movi	255,etmp
ENH322:
	move	etmp,*esrc
ENH323:
	addk	8,esrc
	addk	8,ep0
	addk	8,ep1
	addk	8,ep2
	dsj	rgbc,ENH3200

	addk	8,esrc
	addk	8,ep0
	addk	8,ep1
	addk	8,ep2
	dsj	cnt,ENH320

	mmfm	SP,esrc,ep0,ep1,ep2,cnt,drgb,rgbc,etmp
	rets	2
	
;_enhance24(esrc,ep0,ep1,ep2,cnt)
__enhance24:
	mmtm	SP,esrc,ep0,ep1,ep2,cnt,drgb,rgbc,etmp
	move	-*STK,esrc,1
	move	-*STK,ep0,1
	move	-*STK,ep1,1
	move	-*STK,ep2,1
	move	-*STK,cnt,1

	addk	24,esrc
	setf	8,0
ENH240:
	movk	3,rgbc
ENH2400:
	move	*ep0,drgb
	move	*ep0(48),work
	add	work,drgb
	move	*ep1,work
	add	work,drgb
	move	*ep1(48),work
	add	work,drgb
	move	*ep2,work
	add	work,drgb
	move	*ep2(48),work
	add	work,drgb
	move	drgb,work

	add	drgb,drgb	;*3
	add	work,drgb
	move	drgb,etmp

	move	*ep0(24),drgb
	move	*ep2(24),work
	add	work,drgb
	sll	2,drgb		;*4

	add	drgb,etmp

	move	*ep1(24),work
	sll	1,work		;*26
	move	work,drgb
	sll	2,work
	add	work,drgb
	sll	1,work
	add	work,drgb

	sub	etmp,drgb
	jrz	ENH243

	move	*esrc,work
	sll	2,work		;*52
	move	work,etmp
	sll	2,work
	add	work,etmp
	sll	1,work
	add	work,etmp
	add	drgb,etmp
	jrp	ENH241
	clr	etmp
	jruc	ENH242
ENH241:
	movi	52,work
	divu	work,etmp
	cmpi	255,etmp
	jrle	ENH242
	movi	255,etmp
ENH242:
	move	etmp,*esrc
ENH243:
	addk	8,esrc
	addk	8,ep0
	addk	8,ep1
	addk	8,ep2
	dsj	rgbc,ENH2400
	dsj	cnt,ENH240

	mmfm	SP,esrc,ep0,ep1,ep2,cnt,drgb,rgbc,etmp
	rets	2
	
;_enhance16(esrc,ep0,ep1,ep2,cnt)
__enhance16:
	mmtm	SP,esrc,ep0,ep1,ep2,cnt,drgb,rgbc,etmp
	move	-*STK,esrc,1
	move	-*STK,ep0,1
	move	-*STK,ep1,1
	move	-*STK,ep2,1
	move	-*STK,cnt,1

	addk	16,esrc
	setf	5,0
ENH160:
	movk	3,rgbc
ENH1600:
	move	*ep0,drgb
	move	*ep0(32),work
	add	work,drgb
	move	*ep1,work
	add	work,drgb
	move	*ep1(32),work
	add	work,drgb
	move	*ep2,work
	add	work,drgb
	move	*ep2(32),work
	add	work,drgb
	move	drgb,work

	add	drgb,drgb	;*3
	add	work,drgb
	move	drgb,etmp

	move	*ep0(16),drgb
	move	*ep2(16),work
	add	work,drgb
	sll	2,drgb		;*4

	add	drgb,etmp

	move	*ep1(16),work
	sll	1,work		;*26
	move	work,drgb
	sll	2,work
	add	work,drgb
	sll	1,work
	add	work,drgb

	sub	etmp,drgb
	jrz	ENH163

	move	*esrc,work
	sll	2,work		;*52
	move	work,etmp
	sll	2,work
	add	work,etmp
	sll	1,work
	add	work,etmp
	add	drgb,etmp
	jrp	ENH161
	clr	etmp
	jruc	ENH162
ENH161:
	movi	52,work
	divu	work,etmp
	cmpi	31,etmp
	jrle	ENH162
	movk	31,etmp
ENH162:
	move	etmp,*esrc
ENH163:
	addk	5,esrc
	addk	5,ep0
	addk	5,ep1
	addk	5,ep2
	dsj	rgbc,ENH1600

	addk	1,esrc
	addk	1,ep0
	addk	1,ep1
	addk	1,ep2
	dsj	cnt,ENH160

	mmfm	SP,esrc,ep0,ep1,ep2,cnt,drgb,rgbc,etmp
	rets	2
	

baseAddr	.set	a11
rowBits		.set	a10
tmp		.set	a7
wh		.set	a5
wv		.set	a4
addr		.set	a3
y		.set	a2
height		.set	a0
width		.set	a6

** _Gray32(long baseAddr,long rowBits,int height,int width)
__Gray32
	mmtm	SP,baseAddr,rowBits,height,width,wh,wv,addr,y,tmp
	move	-*STK,baseAddr,1
	move	-*STK,rowBits,1
	move	-*STK,height,1
	move	-*STK,width,1

	clr	wv
GRY3200
	move	baseAddr,addr
	move	width,wh
GRY3201
	setf	8,0
	movi	77,a8
	move	*addr(16),tmp
	mpyu	a8,tmp
	move	tmp,y

	movi	151,a8
	move	*addr(8),tmp
	mpyu	a8,tmp
	add	tmp,y

	movi	28,a8
	move	*addr,tmp
	mpyu	a8,tmp
	add	tmp,y

	srl	8,y
	move	y,tmp
	sll	8,tmp
	or	tmp,y
	sll	8,tmp
	or	tmp,y
	
	setf	32,0
	move	y,*addr+
	dsj	wh,GRY3201

	add	rowBits,baseAddr
	addk	1,wv
	setf	16,0
	move	wv,@_commbuf

	move	STK,-*SP,1
	movi	_event,a8
	move	a8,*STK+,1
	calla	_GetEvent
	move	@_event,a8
	subk	7,a8
	jrz	GRY32E

	dsj	height,GRY3200
GRY32E
	mmfm	SP,baseAddr,rowBits,height,width,wh,wv,addr,y,tmp
	rets	2

** _Gray24(long baseAddr,long rowBits,int height,int width)
__Gray24
	mmtm	SP,baseAddr,rowBits,height,width,wh,wv,addr,y,tmp
	move	-*STK,baseAddr,1
	move	-*STK,rowBits,1
	move	-*STK,height,1
	move	-*STK,width,1

	clr	wv
GRY2400
	move	baseAddr,addr
	move	width,wh
GRY2401
	setf	8,0
	movi	77,a8
	move	*addr(16),tmp
	mpyu	a8,tmp
	move	tmp,y

	movi	151,a8
	move	*addr(8),tmp
	mpyu	a8,tmp
	add	tmp,y

	movi	28,a8
	move	*addr,tmp
	mpyu	a8,tmp
	add	tmp,y

	srl	8,y
	move	y,tmp
	sll	8,tmp
	or	tmp,y
	sll	8,tmp
	or	tmp,y
	
	setf	24,0
	move	y,*addr+
	dsj	wh,GRY2401

	add	rowBits,baseAddr
	addk	1,wv
	setf	16,0
	move	wv,@_commbuf

	move	STK,-*SP,1
	movi	_event,a8
	move	a8,*STK+,1
	calla	_GetEvent
	move	@_event,a8
	subk	7,a8
	jrz	GRY24E

	dsj	height,GRY2400
GRY24E
	mmfm	SP,baseAddr,rowBits,height,width,wh,wv,addr,y,tmp
	rets	2

** _Gray16(long baseAddr,long rowBits,int height,int width)
__Gray16
	mmtm	SP,baseAddr,rowBits,height,width,wh,wv,addr,y,tmp
	move	-*STK,baseAddr,1
	move	-*STK,rowBits,1
	move	-*STK,height,1
	move	-*STK,width,1

	clr	wv
GRY1600
	move	baseAddr,addr
	move	width,wh
GRY1601
	setf	5,0
	movi	77,a8
	move	*addr(10),tmp
	mpyu	a8,tmp
	move	tmp,y

	movi	151,a8
	move	*addr(5),tmp
	mpyu	a8,tmp
	add	tmp,y

	movi	28,a8
	move	*addr,tmp
	mpyu	a8,tmp
	add	tmp,y

	srl	8,y
	move	y,tmp
	sll	5,tmp
	or	tmp,y
	sll	5,tmp
	or	tmp,y
	
	setf	16,0
	move	y,*addr+
	dsj	wh,GRY1601

	add	rowBits,baseAddr
	addk	1,wv
	setf	16,0
	move	wv,@_commbuf

	move	STK,-*SP,1
	movi	_event,a8
	move	a8,*STK+,1
	calla	_GetEvent
	move	@_event,a8
	subk	7,a8
	jrz	GRY16E

	dsj	height,GRY1600
GRY16E
	mmfm	SP,baseAddr,rowBits,height,width,wh,wv,addr,y,tmp
	rets	2



	.end
