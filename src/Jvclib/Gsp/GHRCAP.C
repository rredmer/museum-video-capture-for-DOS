/*
**	ghrcap.c :
*/

#include	"ghrcap.h"

#define	MINHV	128

#define	DEB

/* GSP register value for Interlace PAL mode;
				see A-3 in Tec.Manu for detail */
static Word HiRes32Regs[] = {
	20,		/* HESYNC  0000 */
	30,		/* HEBLNK  0010 */
	286,	/* HSBLNK  0020 */
	297,	/* HTOTAL  0030 */
	2,		/* VESYNC  0040 */
	6,		/* VEBLNK  0050 */
	394, 	/* VSBLNK  0060 */
	400, 	/* VTOTAL  0070 */
	0xB020,	/* DPYCTL  0080 (interlace) */
	0xFFFC,	/* DPYSTR  0090 */
	394,	/* DPYINT  00A0 */
	0x0100,	/* video */
	0x04F0	/* mode */
};
static Word HiRes16Regs[] = {
	20,		/* HESYNC  0000 */
	30,		/* HEBLNK  0010 */
	286,	/* HSBLNK  0020 */
	297,	/* HTOTAL  0030 */
	2,		/* VESYNC  0040 */
	6,		/* VEBLNK  0050 */
	394, 	/* VSBLNK  0060 */
	400, 	/* VTOTAL  0070 */
	0xB020,	/* DPYCTL  0080 (interlace) */
	0xFFFC,	/* DPYSTR  0090 */
	394,	/* DPYINT  00A0 */
	0x0110,	/* video */
	0x14F0	/* mode */
};
static Word NonIntPal32Regs[] = {
	21,		/* HESYNC  0000 */
	35,		/* HEBLNK  0010 */
	219,	/* HSBLNK  0020 */
	226,	/* HTOTAL  0030 */
	5,		/* VESYNC  0040 */
	14,		/* VEBLNK  0050 */
	600, 	/* VSBLNK  0060 */
	624, 	/* VTOTAL  0070 */
	0xF010,	/* DPYCTL  0080 (Non-Interlaced) */
	0xFFFC,	/* DPYSTR  0090 */
	600,	/* DPYINT  00A0 */
	0x0100,	/* video */
	0x04F4	/* mode */
};
static Word NonIntPal16Regs[] = {
	21,		/* HESYNC  0000 */
	35,		/* HEBLNK  0010 */
	219,	/* HSBLNK  0020 */
	226,	/* HTOTAL  0030 */
	5,		/* VESYNC  0040 */
	14,		/* VEBLNK  0050 */
	600, 	/* VSBLNK  0060 */
	624, 	/* VTOTAL  0070 */
	0xF010,	/* DPYCTL  0080 (Non-Interlaced) */
	0xFFFC,	/* DPYSTR  0090 */
	600,	/* DPYINT  00A0 */
	0x0110,	/* video */
	0x14F4	/* mode */
};
static Word StdPal32Regs[] = {
	207,	/* HESYNC  0000 */
	69,		/* HEBLNK  0010 */
	437,	/* HSBLNK  0020 */
	453,	/* HTOTAL  0030 */
	2,		/* VESYNC  0040 */
	17,		/* VEBLNK  0050 */
	310, 	/* VSBLNK  0060 */
	312, 	/* VTOTAL  0070 */
	0xB020,	/* DPYCTL  0080 (Interlaced) */
	0xFFFC,	/* DPYSTR  0090 */
	310,	/* DPYINT  00A0 */
	0x0300,	/* video */
	0x04F4	/* mode */
};
static Word StdPal16Regs[] = {
	207,	/* HESYNC  0000 */
	69,		/* HEBLNK  0010 */
	437,	/* HSBLNK  0020 */
	453,	/* HTOTAL  0030 */
	2,		/* VESYNC  0040 */
	17,		/* VEBLNK  0050 */
	310, 	/* VSBLNK  0060 */
	312, 	/* VTOTAL  0070 */
	0xB020,	/* DPYCTL  0080 (Interlaced) */
	0xFFFC,	/* DPYSTR  0090 */
	310,	/* DPYINT  00A0 */
	0x0310,	/* video */
	0x14F4	/* mode */
};

GPtr		thePort;	/* current graf port pointer */
DspCapINFO	DspCapInfo;	/* display and capture informations */
VistaENV	VistaEnv;	/* vista environment */

Wptr		DisplayRegs;
Word		CaptureRegs[11] = {
	207,	/* HESYNC  0000 */
	69,		/* HEBLNK  0010 */
	439,	/* HSBLNK  0020 */
	453,	/* HTOTAL  0030 */
	2,		/* VESYNC  0040 */
	17,		/* VEBLNK  0050 */
	310, 	/* VSBLNK  0060 */
	312, 	/* VTOTAL  0070 */
	0xB020,	/* DPYCTL  0080 (interlace) */
	0xFFFC,	/* DPYSTRT 0090 */
	310,	/* DPYINT  00A0 */
};

Word commbuf[128];

WaitVBlnk() {
	register Wptr vcount;
	register Word vtotal;
	
	vtotal = *(Wptr)VTOTAL;
	vcount = (Wptr)VCOUNT;
	if (*vcount == vtotal)
		while (*vcount == vtotal);
	while (*vcount != vtotal);
}

WaitNVBlnk(register int n) {
	while (n--)
		WaitVBlnk();
}

SetVideo(register Word data) {
	*(Wptr)VIDEOS = data;
	*(Wptr)VIDEO  = data;
}

SetMode(register Word data) {
	*(Wptr)MODES = data;
	*(Wptr)MODE  = data;
}

Msg2Host(register int msg) {
	register Wptr hstctll;

	hstctll = (Wptr)HSTCTLL;
	*hstctll = (msg << 4) + INTOUT;
	while(*hstctll & INTOUT);
}

GPtr GetPort() {
	return thePort;
}

SetPort(register GPtr port) {
	register int reg;

	thePort = port;

/* set B file registers in GSP 34010 */
	reg = port->baseAddr;
	asm("	move	a10,b4");
	reg = port->rowBits;
	asm("	move	a10,b1");
	asm("	move	a10,b3");
	asm("	lmo		a10,a8");
	asm("	move	a8,@0C0000130H");	/* convsp */
	asm("	move	a8,@0C0000140H");	/* convdp */
	reg = 0;
	asm("	move	a10,b5");	/* wstart */
	reg = (port->portRect.bottom - 1) * 65536 +
			(port->portRect.right - 1) * port->psize / 16;	/* wend */
	asm("	move	a10,b6");
	asm("	movk	16,a8");
	asm("	move	a8,@0C0000150H,0");	/* psize */

}

static SetVistaGPort() {
	register int imagend,devstr;
	register int row;
	register int MaxH,MaxV,capH,capV;
	GPort port;
	Rect r;
	
	r.top = r.left = 0;

/* display port */
	r.right = DspCapInfo.horzCrtRes;
	r.bottom = DspCapInfo.vertCrtRes;
	port.psize = DspCapInfo.pixSize;
	port.rowBits = PITCH;
	port.baseAddr = VRAMEND - r.bottom*PITCH;
	port.portRect = r;
	DspCapInfo.dspPort = port;

	if (VistaEnv.vistaMem == 4 || DspCapInfo.capMode == X1) {
/* image port */
		r.right = DspCapInfo.horzRes;
		r.bottom = DspCapInfo.vertRes;
		port.psize = DspCapInfo.pixSize;
		port.rowBits = PITCH;
		port.baseAddr = VRAMEND - r.bottom*PITCH;
		port.portRect = r;
		DspCapInfo.imagePort = port;

/* device port */
		r.right = (DspCapInfo.pixSize == 32) ? 1024 : 2048;
		r.bottom = MaxV4M;
		port.baseAddr = VRAMEND - r.bottom*PITCH;
		port.portRect = r;
		DspCapInfo.devPort = port;
	}
	else {
/* image port */
		port.psize = (DspCapInfo.pixSize == 32) ? 24 : DspCapInfo.pixSize;
		port.rowBits = 
		((((DspCapInfo.horzRes+DspCapInfo.capMode*3*2)*port.psize)+15)/16)*16;
		port.baseAddr = VistaEnv.vistaStr + PITCH*20;

		imagend = port.baseAddr + DspCapInfo.vertRes * port.rowBits;
		devstr = VRAMEND - DspCapInfo.vertCrtRes * PITCH;
		while (imagend >= devstr) {
			imagend -= port.rowBits;
			--DspCapInfo.vertRes;
		}
		DspCapInfo.vertRes = (DspCapInfo.vertRes >> 1) << 1;
		r.right = DspCapInfo.horzRes;
		r.bottom = DspCapInfo.vertRes;
		port.portRect = r;
		DspCapInfo.imagePort = port;

/* device port */
		row = (VRAMEND-port.baseAddr-port.portRect.bottom*port.rowBits)/PITCH;

		row = min(MaxV4M,row);

		r.right = (DspCapInfo.pixSize == 32) ? 1024 : 2048;
		r.bottom = row;
		port.psize = DspCapInfo.pixSize;
		port.rowBits = PITCH;
		port.baseAddr = VRAMEND - r.bottom*PITCH;
		port.portRect = r;
		DspCapInfo.devPort = port;
	}
	MaxH = PalCapH*DspCapInfo.capMode;
	MaxV = PalCapV*DspCapInfo.capMode;

	capH = DspCapInfo.capRect.right - DspCapInfo.capRect.left;
	capH = max(min(DspCapInfo.horzRes,capH),MINHV*DspCapInfo.capMode);

	capV = DspCapInfo.capRect.bottom - DspCapInfo.capRect.top;
	capV = max(min(DspCapInfo.vertRes,capV),MINHV*DspCapInfo.capMode);

	SetRect(&DspCapInfo.capRect,
		(MaxH-capH)/2,(MaxV-capV)/2,(MaxH+capH)/2,(MaxV+capV)/2);
}

InitRowTable(register int rows) {
	register int i,j;
	register Word val,*ptr;

/* row tabe must be written during V blanking */
	WaitVBlnk();
/* set rows */
	val=ROWSTR+1;
	ptr=(Wptr)ROWTBL;
	for (i=0; i<8; i++) {
		*ptr++ = val | 0x1000;	/* lut load bit on */
		val++;
	}

	val=ROWSTR;
	ptr=(Wptr)ROWTBL+8;
	for (i=0; i<rows; i++) {
		*ptr++ = val | 0x0c00;	/* display,capture bit on */
		val--;
	}

/* clear rest of rows */
	while (ptr < (Wptr)ROWTBL+2048-10)
		*ptr++ = 0;
}

InitAtVista() {
/* set HESYNC, HEBLNK, ..., & DPYINT */
	SetVideo(DisplayRegs[11]);	/* set video/mode register */
	SetMode(DisplayRegs[12]);

	SetVstaRegs(DisplayRegs);

	SetPort(&DspCapInfo.imagePort);
	DspCapInfo.dspRect = DspCapInfo.dspPort.portRect;
	SetControl(0x0000);
	SetPMask(0);

	*(Wptr)INTPEND = 0;	/* clear intr pending */
	*(Wptr)DPYTAP  = 0;	/* set display tap to zero */

	InitRowTable(DspCapInfo.dspPort.portRect.bottom);
}

SetVstaRegs(register Wptr regs) {
	register int i;
	register Wptr ptr;
	
	for (i=0,ptr=(Wptr)HESYNC; i<11; i++)
		*ptr++ = *regs++;
}

static ClearScreen() {
	ClearRect(&DspCapInfo.devPort, &DspCapInfo.devPort.portRect);
	
	if (DspCapInfo.devPort.baseAddr != DspCapInfo.imagePort.baseAddr)
		ClearRect(&DspCapInfo.imagePort, &DspCapInfo.imagePort.portRect);
}

static Initialize() {
	register unsigned long w;
	register int i;
	register Wptr ptr;

	asm("	dint");			/* disable interrupt */

	*(Wptr)HSTCTLL = 0;		/* clear message out flag */

	w = (unsigned long)&commbuf[0];
	*(Wptr)HSTADRL = w;
	*(Wptr)HSTADRH = w >> 16;
	*(Wptr)HSTCTLL = INTOUT;
	while (*(Wptr)HSTCTLL & INTOUT) ;	/* check passage to host */

/* get vista environment and display/capture imformation from host */
	ptr = commbuf;
	DspCapInfo = *(DspCapINFO *)ptr;
	ptr += sizeof(DspCapInfo)/sizeof(Word);
	VistaEnv = *(VistaENV *)ptr;
	ptr += sizeof(VistaENV)/sizeof(Word);
	for (i=0; i<8; i++)
		CaptureRegs[i] = *ptr++;
	
	/* set ATVista initial register pointer */
	if (DspCapInfo.pixSize == 32) {
		switch(DspCapInfo.dspMode) {
		case StdPal:
			DisplayRegs = StdPal32Regs;
			break;
		case NonIntPal:
			DisplayRegs = NonIntPal32Regs;
			break;
		case HiRes:
			DisplayRegs = HiRes32Regs;
			break;
		}
	} else {
		switch(DspCapInfo.dspMode) {
		case StdPal:
			DisplayRegs = StdPal16Regs;
			break;
		case NonIntPal:
			DisplayRegs = NonIntPal16Regs;
			break;
		case HiRes:
			DisplayRegs = HiRes16Regs;
			break;
		}
	}

	SetVistaGPort();

/* answer information to host */
	*(DspCapINFO *)commbuf = DspCapInfo;
	*(Wptr)HSTCTLL = INTOUT;
	while (*(Wptr)HSTCTLL & INTOUT) ;	/* check passage to host */

	InitAtVista();
}

GetEvent(EventRecord *event) {
	if (*(Wptr)INTPEND & 0x0200) {
		*(Wptr)INTPEND = 0;
		event->cmdcode = *(Wptr)HSTCTLL & 0x0007;
		event->message.smes[0] = *(Wptr)HSTADRH;
		event->message.smes[1] = *(Wptr)HSTADRL;
		*(Wptr)HSTCTLL &= 0xFFF7;
	}
	else
		event->cmdcode = 0;	/* idel code */
}

main() {
	EventRecord event;
	register int msg;
	Rect r;

	Initialize();

	for (;;) {
		GetEvent(&event);
		if (event.cmdcode == CMDMSG) {
			switch(event.message.smes[1]) {
			case 'G' :		/* grab command */
				switch(event.message.smes[0]) {
				case 0:
					InitAtVista();
					DoGrab();
					break;
				case 1:
					GrabOne();
					break;
				}
				break;
			case 'D' :
				switch(event.message.smes[0]) {
				case 0:
				case 1:
					InitAtVista();
					InvalDevPort();
					InitCursor(event.message.smes[0]);
					DoDisplay(event.message.smes[0]);
					break;
				case 8:
					InvalDevPort();
					break;
				}
				break;
			case 'I':
				r.left = r.top = 0;
				r.right = DspCapInfo.capRect.right - DspCapInfo.capRect.left;
				r.bottom = DspCapInfo.capRect.bottom - DspCapInfo.capRect.top;
				switch(event.message.smes[0]) {
				case 0:
					DoEnhance(&DspCapInfo.imagePort,&r);
					break;
				case 1:
					DoGreyScale(&DspCapInfo.imagePort,&r);
					break;
				case 2:
					DoRGBMatrix(&DspCapInfo.imagePort,&r);
					break;
				}
				InvalDevPort();
				break;
			case 'F' :
				switch(event.message.smes[0]) {
				case 0:
					DoLive();
					break;
				case 1:
					DoPalLive();
					break;
				case 2:
					DoCalibLive();
					break;
				case 3:
					DoGammaLive();
					break;
				case 4:
					DoColorBarLive();
					break;
				}
				break;
			case 'W' :
				ClearScreen();
				break;
			}
		}
	}
}

/*** end of hrcap.c ***/
