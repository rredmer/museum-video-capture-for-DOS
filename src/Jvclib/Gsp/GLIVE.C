/*
**	glive.c :
*/

#include	"ghrcap.h"

#define	DEB

#define	AUP		0x4800		/* arrow key */
#define	ADOWN	0x5000
#define	ARIGHT	0x4D00
#define	ALEFT	0x4B00

static VideoMode LiveReg16 = {	/* 16 bit/pixel, pal live */
	0x0310,	/* VIDEO CLKDIV 4x (14.18750MHz), Zoom 1x */
	0x15F4	/* MODE  16-bit live, PAL, overlay independent */
};
static VideoMode LiveReg32 = {	/* 32 bit/pixel, pal live */
	0x0300,	/* VIDEO CLKDIV 4x (14.18750MHz), Zoom 1x */
	0x05F4	/* MODE  32-bit live, PAL, overlay independent */
};
static VideoMode PalLiveReg16 = { /* 16 bit/pixel, pal live */
	0x0510,	/* VIDEO CLKDIV 6x (9.45833MHz), Zoom 1x */
	0x11F4	/* MODE 16-bit live, PAL */
};
static VideoMode PalLiveReg32 = {	/* 32 bit/pixel, 246x576 capture */
	0x0511,	/* VIDEO CLKDIV 6x (9.45833MHz), Zoom 2x */
	0x01F4	/* MODE  32-bit live, PAL */
};

static	GPort	livePort = {
	VRAMEND - PalResV * PITCH,	/* baseAddr */
	PITCH,						/* rowBits */
	0,							/* psize by devPort */
	{ 0,0,PalResH,PalResV }		/* portRect */
};

static PutCapRect(Rect *rp) {
	*(Rect *)VistaEnv.commbuf = *rp;
	*(Wptr)HSTCTLL = INTOUT;
}

static InitLiveFrame() {
	register int i,addr,control;

	SetPMask(0x7FFF);
	control = GetControl();
	SetControl((control & 0x83FF) + 0x3000);	/* set 1 */

	WaitVBlnk();

	if (livePort.psize == 32) {
		addr = VRAMEND - PITCH * PalResV + 16;
		for (i=0; i<PalResH; i++) {
			fill(addr,PalResV*65536+1,0);
			addr += 32;
		}
	} else {
		addr = VRAMEND - PITCH * PalResV;
		for (i=0; i<PalResH; i++) {
			fill(addr,PalResV*65536+1,0);
			addr += 16;
		}
	}
}

static SetLiveFrame(Rect *r) {
	register int width,height;
	register int left,top,right,bottom;
	register int addr,xy,dydx,xdata,w;

	width = r->right - r->left;
	height = r->bottom - r->top;

	r->left = min(max(r->left,0),PalCapH*DspCapInfo.capMode-width);
	r->top = min(max(r->top,0),PalCapV*DspCapInfo.capMode-height);

	r->right  = r->left + width;
	r->bottom = r->top + height;

	left = r->left/DspCapInfo.capMode;
	top = r->top/DspCapInfo.capMode;
	right = r->right/DspCapInfo.capMode;
	bottom = r->bottom/DspCapInfo.capMode;
	
	w = top;
	top = PalResV - bottom;
	bottom = PalResV - w;

	if (livePort.psize == 32) {
		SetPMask(0x7F7F);
		xdata = 0x8080;
		left *= 2;
		right *= 2;
	} else {
		SetPMask(0x3DEF);
		xdata = 0xC210;
	}
		
	SetControl((GetControl() & 0x83FF) + 0x2800);	/* xor */

	WaitVBlnk();

	/* horizontal line */
	dydx = 65536*2 + (right - left);
	xy = top * 65536 + left;
	fillXY(xy,dydx,xdata);
	xy = (bottom - 2) * 65536 + left;
	fillXY(xy,dydx,xdata);

	/* vertical line */
	w = livePort.psize * 2 / 16;
	dydx = (bottom - top - 4) * 65536 + w;
	xy = (top + 2) * 65536 + left;
	fillXY(xy,dydx,xdata);
	xy = (top + 2) * 65536 + (right - w);
	fillXY(xy,dydx,xdata);
}

#define	MINHV	32
static CheckLiveRect(register Rect *r) {
	register int width,height;
	register int dh,dv;
	register int MinHV;

	width = r->right - r->left;
	height = r->bottom - r->top;
	MinHV = MINHV*DspCapInfo.capMode;

	dh = MinHV - width;
	if (dh > 0)
		InsetRect(r,dh*(-1)/2,0);
	dv = MinHV - height;
	if (dv > 0)
		InsetRect(r,0,dv*(-1)/2);

	dh = width - DspCapInfo.horzRes;
	if (dh > 0)
		InsetRect(r,dh/2,0);
	dv = height - DspCapInfo.vertRes;
	if (dv > 0)
		InsetRect(r,0,dv/2);
}

DoMasterLive(register VideoMode *reg) {

	WaitVBlnk();

/* clear tap register */
	*(Wptr)DPYTAP  = 0;	/* set display tap to zero */

/* set row table */
	InitRowTable(PalResV);
	
/* set Video & Mode registers */
	SetVideo(reg->video);
	SetMode(reg->mode);

/* make synchronous to the video signal */
	*(Wptr)VCOUNT = 0;
	*(Wptr)HCOUNT = 0;

/* set Vista registers */
	SetVstaRegs(CaptureRegs);
}

DoSlaveLive(register VideoMode *reg) {
	register Wptr intpend;

	WaitVBlnk();

/* clear tap register */
	*(Wptr)DPYTAP  = 0;	/* set display tap to zero */

/* set row table */
	InitRowTable(PalResV);
	
/* set Video & Mode registers */
	SetVideo(reg->video);
	SetMode(reg->mode | 0x000A);

/* make synchronous to the video signal */
	*(Wptr)VCOUNT = 0;
	*(Wptr)HCOUNT = 0;

/* set Vista registers */
	SetVstaRegs(CaptureRegs);

/* genlocking to exeternal sync */
	intpend = (Wptr)INTPEND;

	while ((*intpend & 0x02) == 0);
	*(Wptr)VCOUNT = 0;
	*(Wptr)HCOUNT = 0;
	*(Wptr)DPYCTL |= 0x4000;
	*(Wptr)DPYCTL &= ~0x4000;
	*(Wptr)DPYADR = *(Wptr)DPYSTRT;
	while ((*intpend & 0x02) != 0);
}

LiveNuVista(register int psize) {
	if (DspCapInfo.genlock == master)
		DoMasterLive((psize == 32) ? &LiveReg32 : &LiveReg16);
	else
		DoSlaveLive((psize == 32) ? &LiveReg32 : &LiveReg16);
}

static _DoLive(register Rect *rp) {
	register int control,pmask;
	register GPtr port;
	EventRecord event;

	port = GetPort();
	pmask = GetPMask();
	control = GetControl();

	livePort.psize = DspCapInfo.devPort.psize;
	SetPort(&livePort);

	LiveNuVista(livePort.psize);

	InitLiveFrame();
	SetLiveFrame(rp);

	Msg2Host(ACK);

	for (;;) {
		GetEvent(&event);

		switch(event.cmdcode) {
		case CURMSG:
			SetLiveFrame(rp);
			OffsetRect(rp,event.message.smes[0],event.message.smes[1]);
			SetLiveFrame(rp);
			PutCapRect(rp);
			break;
		case SIZMSG:
			if (event.message.lmes) {
				SetLiveFrame(rp);
				InsetRect(rp,event.message.smes[0],event.message.smes[1]);
				CheckLiveRect(rp);
				SetLiveFrame(rp);
				PutCapRect(rp);
			}
			break;
		case CANMSG:
			SetLiveFrame(rp);
			goto END;
			break;
		}
	}
END:
	SetPort(port);
	SetPMask(pmask);
	SetControl(control);
}

DoLive() {
	_DoLive(&DspCapInfo.capRect);
}

DoPalLive() {
	if (DspCapInfo.genlock == master)
		DoMasterLive((DspCapInfo.pixSize==32) ? &PalLiveReg32 : &PalLiveReg16);
	else
		DoSlaveLive((DspCapInfo.pixSize==32) ? &PalLiveReg32 : &PalLiveReg16);

	Msg2Host(ACK);
}

DoCalibLive() {
	register int capmode;
	static Boolean firstTime = true;
	static Rect cr = { (PalResH-32)/2,(PalResV-32)/2,
					   (PalResH+32)/2,(PalResV+32)/2 };
	
	if (firstTime == true) {
		capmode = DspCapInfo.capMode;
		cr.left *= capmode;
		cr.top *= capmode;
		cr.right *= capmode;
		cr.bottom *= capmode;
		firstTime = false;
	}
	_DoLive(&cr);
}

DoGammaLive() {
	extern GPtr GetPort();
	EventRecord event;
	Rect cr;
	register int pmask,control;
	register steps,dh,dv,width,height;
	register GPtr port;
	register int i;

	port = GetPort();
	pmask = GetPMask();
	control = GetControl();

	livePort.psize = DspCapInfo.devPort.psize;
	SetPort(&livePort);

	LiveNuVista(livePort.psize);

	InitLiveFrame();

	SetRect(&cr,738*40/240,576*(55-8)/180,738*40/240+24,576*(55-8)/180+24);
	for (i=0; i<11; i++) {
		SetLiveFrame(&cr);
		cr.left += 738*15/240;
		cr.right += 738*15/240;
	}

	SetRect(&cr,738*40/240,576*(130-8)/180,738*40/240+24,576*(130-8)/180+24);
	for (i=0; i<11; i++) {
		SetLiveFrame(&cr);
		cr.left += 738*15/240;
		cr.right += 738*15/240;
	}

	Msg2Host(ACK);

	SetPort(port);
	SetPMask(pmask);
	SetControl(control);
}

DoColorBarLive() {
	extern GPtr GetPort();
	EventRecord event;
	Rect cr;
	register int pmask,control;
	register steps,dh,dv,width,height;
	register GPtr port;
	register int i;

	port = GetPort();
	pmask = GetPMask();
	control = GetControl();

	livePort.psize = DspCapInfo.devPort.psize;
	SetPort(&livePort);

	LiveNuVista(livePort.psize);

	InitLiveFrame();

	SetRect(&cr,84,280,100,296);
	for (i=0; i<7; i++) {
		SetLiveFrame(&cr);
		cr.left += 93;
		cr.right += 93;
	}

	Msg2Host(ACK);

	SetPort(port);
	SetPMask(pmask);
	SetControl(control);
}



/*** end of glive.c ***/
