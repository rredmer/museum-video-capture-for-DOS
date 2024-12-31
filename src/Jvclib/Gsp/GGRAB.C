/*
**	ggrab.c :
*/

#include	"ghrcap.h"

#define	PPMSK	0x83df		/* control ppop mask */
#define	PPOR	0x20c0		/* pixblt opecode */

#define	DEB


static VideoMode CapReg16 = {	/* 16 bit/pixel, 492x576 capture */
	0x0510,	/* VIDEO CLKDIV 6x (9.45833MHz), Zoom 1x */
	0x11F4	/* MODE 16-bit live, PAL */
};
static VideoMode CapReg32 = {	/* 32 bit/pixel, 246x576 capture */
	0x0511,	/* VIDEO CLKDIV 6x (9.45833MHz), Zoom 2x */
	        /* The sampling rate of TK-F7300U is 14.1875MHz/3 = 4.7291MHz */
	        /* ATVista can make this sampling rate with the combination of */
	        /* CLKDIV and Zoom factor. */
	0x01F4 	/* MODE  32-bit live, PAL */
};

GPort	capPort;	/* capture device graf port */

static WaitV() {
	register Wptr intpend;

	intpend = (Wptr)INTPEND;
	*intpend = 0;
	while((*intpend & 0x0400) == 0);
}

static WaitOddField() {
	register Word dpystrt;
	register Wptr dpyadr;
	
	dpystrt = *(Wptr)DPYSTRT;
	dpyadr = (Wptr)DPYADR;
	while(dpystrt != *dpyadr) ;	/* wait odd field */
}		

GrabFrame() {
	register Wptr mode,modes;

	mode = (Wptr)MODE;
	modes = (Wptr)MODES;

	WaitOddField();
	*mode = *modes | 0x8000;
	WaitV();
	*mode = *modes & 0x7FFF;
	*mode = *modes | 0x8000;
	WaitV();
	*mode = *modes & 0x7FFF;
}

static ThrowOneFrame() {
	register Wptr mode,modes;

	mode = (Wptr)MODE;
	modes = (Wptr)MODES;

	WaitOddField();
	WaitV();
	WaitV();
}

static int Capture32(GPtr sport,GPtr dport,Rect *r) {
	register int src,dst,array;
	register int src0,dst0;
	register int k,hcnt,psize;
	register int width;
	int i,j,height,vcnt,capm,doff;
	int sbase;
	Rect sr,dr;

	psize = dport->psize;
	capm = DspCapInfo.capMode;
	hcnt = capm * 3;
	vcnt = capm;

	SetPSize(8);
	SetPitch(sport->rowBits, dport->rowBits * capm);

	height = dport->portRect.bottom - dport->portRect.top;
	sbase = dport->baseAddr;	/* save base address */
	dport->baseAddr += 
		(height - (r->bottom - r->top)) * dport->rowBits - capm * 2 * psize;

	src = sport->baseAddr + 
		r->top/capm * sport->rowBits + r->left / hcnt * 32;

	width = min((r->right - r->left + hcnt) / hcnt + 1, 246);
	height = (r->bottom - r->top + vcnt - 1) / vcnt;
	array = (height << 16) + 3;

	for (i=1; i<=vcnt; i++) {
		dst = dport->baseAddr + (vcnt - i) * dport->rowBits;
		if ((i&1) == 0) {
			dst += (hcnt-1) * psize;
			doff = psize * (-1);
		} else {
			doff = psize;
		}

		for (j=1; j<=hcnt; j++) {
			while(!(*(Wptr)INTPEND & 0x0200));
			ThrowOneFrame();
			GrabFrame();
			*(Wptr)HSTCTLL &= 0xFFF7;
			if ((*(Wptr)HSTCTLL & 0x0007) == CANMSG)
				return -1;

			for (k=0,src0=src,dst0=dst; k<width; k++) {
				PixBltL(src0,dst0,array);
				src0 += 32;
				dst0 += hcnt * psize;
			}
			dst += doff;
		}
	}

	SetPitch(dport->rowBits, dport->rowBits);

	width = (r->right - r->left) + capm * 2;
	height = r->bottom - r->top;

	/* move blue data */
	src = dport->baseAddr + (width - capm * 2 - 1) * psize;
	dst = dport->baseAddr + (width - 1) * psize;
	array = (height << 16) + 1;

	for (k=0; k<width-capm*2; k++) {
		PixBltL(src,dst,array);
		src -= psize;
		dst -= psize;
		*VistaEnv.commbuf = k/2;
	}

	/* move green data */
	src = dport->baseAddr + (width - capm - 1) * psize + 8;
	dst = dport->baseAddr + (width - 1) * psize + 8;
	for (k=0; k<width-capm; k++) {
		PixBltL(src,dst,array);
		src -= psize;
		dst -= psize;
		*VistaEnv.commbuf = (width+k)/2;
	}

	dport->baseAddr = sbase;

	SetPSize(16);
	return 0;
}

static int Capture16(GPtr sport,GPtr dport,Rect *r) {
	register int src,dst,array;
	register int src0,dst0;
	register int k,hcnt,psize;
	register int width;
	int i,j,height,vcnt,capm,doff;
	int sbase;
	Rect sr,dr;

	psize = dport->psize;
	capm = DspCapInfo.capMode;
	hcnt = capm * 3;
	vcnt = capm;

	SetPitch(sport->rowBits, dport->rowBits * capm);

	height = dport->portRect.bottom - dport->portRect.top;
	sbase = dport->baseAddr;	/* save base address */
	dport->baseAddr += 
		(height - (r->bottom - r->top)) * dport->rowBits - capm * 2 * psize;
/*
	src = sport->baseAddr + 
		r->top/capm * sport->rowBits + r->left / hcnt * 16;
*/
	src = sport->baseAddr + 
		r->top/capm * sport->rowBits + r->left / hcnt * 16 * 2;

	width = min((r->right - r->left + hcnt) / hcnt + 1, 246);
	height = (r->bottom - r->top + vcnt - 1) / vcnt;
	array = (height << 16) + 1;

	for (i=1; i<=vcnt; i++) {
		dst = dport->baseAddr + (vcnt - i) * dport->rowBits;
		if ((i&1) == 0) {
			dst += (hcnt-1) * psize;
			doff = psize * (-1);
		} else {
			doff = psize;
		}

		for (j=1; j<=hcnt; j++) {
			while(!(*(Wptr)INTPEND & 0x0200));
			ThrowOneFrame();
			GrabFrame();
			*(Wptr)HSTCTLL &= 0xFFF7;
			if ((*(Wptr)HSTCTLL & 0x0007) == CANMSG)
				return -1;

			for (k=0,src0=src,dst0=dst; k<width; k++) {
				PixBltL(src0,dst0,array);
				src0 += 16*2;
				dst0 += hcnt * psize;
			}
			dst += doff;
		}
	}

	SetPitch(dport->rowBits, dport->rowBits);

	width = (r->right - r->left) + capm * 2;
	height = r->bottom - r->top;

	/* move blue data */
	src = dport->baseAddr + (width - capm * 2 - 1) * psize;
	dst = dport->baseAddr + (width - 1) * psize;
	array = (height << 16) + 1;

	SetPMask(0xFFE0);

	for (k=0; k<width-capm*2; k++) {
		PixBltL(src,dst,array);
		src -= psize;
		dst -= psize;
		*VistaEnv.commbuf = k/2;
	}

	/* move green data */
	src = dport->baseAddr + (width - capm - 1) * psize + 8;
	dst = dport->baseAddr + (width - 1) * psize + 8;

	SetPMask(0xFC1F);

	for (k=0; k<width-capm; k++) {
		PixBltL(src,dst,array);
		src -= psize;
		dst -= psize;
		*VistaEnv.commbuf = (width+k)/2;
	}

	SetPMask(0x0000);

	dport->baseAddr = sbase;
	return 0;
}

static Word hsblnk,vsblnk;
SetBlnkForCap() {
	register Wptr videos;
	register Word clkdiv;

	videos = (Wptr)VIDEOS;
	SetVideo((*videos & 0x0FFF) | 0x7000);	/* set hoffset 7 */

	hsblnk = *(Wptr)HSBLNK;		/* change hsblnk */
	/* increment hsblnk for capture offset */
	clkdiv = (*videos & 0x0700) >> 8;
	*(Wptr)HSBLNK = hsblnk + 7 * (clkdiv+1) / 8;

	vsblnk = *(Wptr)VSBLNK;
	*(Wptr)VSBLNK = vsblnk + 1;	/* increment vsblnk value */
	*(Wptr)DPYINT = *(Wptr)VSBLNK;
}

static RstrBlnk() {
	register Wptr videos;
	
	videos = (Wptr)VIDEOS;
	SetVideo(*videos);
	*(Wptr)HSBLNK = hsblnk;
	*(Wptr)VSBLNK = vsblnk;
	*(Wptr)DPYINT = vsblnk;
}

GrabOne() {
	SetBlnkForCap();
	GrabFrame();
	RstrBlnk();
	Msg2Host(ACK);
}

DoGrab() {
	register GPtr port,dev,image;
	register int width,height;
	GPort cap;
	Rect capRect,r;

	port = GetPort();

	dev = &DspCapInfo.devPort;
	image = &DspCapInfo.imagePort;
	
	capRect = DspCapInfo.capRect;

	height = capRect.bottom - capRect.top;
	capRect.top = PalResV*DspCapInfo.capMode - capRect.bottom;
	capRect.bottom = capRect.top + height;

	/* intialize capture device port */
	cap.psize = dev->psize;
	cap.rowBits = dev->rowBits;
	cap.baseAddr = VRAMEND - PalResV * cap.rowBits;
	cap.portRect.left = 0;
	cap.portRect.top = 0;
	cap.portRect.right = PalResH;
	cap.portRect.bottom = PalResV;
	SetPort(&cap);

	if (DspCapInfo.genlock == master)
		DoMasterLive((dev->psize == 32) ? &CapReg32 : &CapReg16);
	else
		DoSlaveLive((dev->psize == 32) ? &CapReg32 : &CapReg16);

	WaitNVBlnk(50);		/* get time for genlock */

	/* set GSP registers for capture */
	SetBlnkForCap();

	*VistaEnv.commbuf = 0;		/* clear percent value */

	Msg2Host(ACK);

	if (dev->psize == 32) {
		if (DspCapInfo.capMode == X1 || VistaEnv.vistaMem == 4) {
			GPort tmp;
			Rect sr,dr;

			tmp = *image;
			tmp.baseAddr += 256*32 - tmp.rowBits;

			tmp.psize = 24;
			if (Capture32(&cap,&tmp,&capRect) == -1)
				goto END;
			width = capRect.right - capRect.left;
			height = capRect.bottom - capRect.top;
			SetRect(&sr,
				0,tmp.portRect.bottom-height,width,tmp.portRect.bottom);
			SetRect(&dr,
				0,image->portRect.bottom-height,width,image->portRect.bottom);
			CopyBits(&tmp,image,&sr,&dr,1);
		}
		else {
			if (Capture32(&cap,image,&capRect) == -1)
				goto END;
		}
	} else {
		if (DspCapInfo.capMode == X1 || VistaEnv.vistaMem == 4) {
			GPort tmp;
			Rect sr,dr;

			tmp = *image;
			tmp.baseAddr += 512*16;
			
			if (Capture16(&cap,&tmp,&capRect) == -1)
				goto END;
			width = capRect.right - capRect.left;
			height = capRect.bottom - capRect.top;
			SetRect(&sr,
				0,tmp.portRect.bottom-height,width,tmp.portRect.bottom);
			SetRect(&dr,
				0,image->portRect.bottom-height,width,image->portRect.bottom);
			CopyBits(&tmp,image,&sr,&dr,1);
		}
		else {
			if (Capture16(&cap,image,&capRect) == -1)
				goto END;
		}
	}

	width = capRect.right - capRect.left;
	height = capRect.bottom - capRect.top;
	if ((width < dev->portRect.right) || (height < dev->portRect.bottom)) {
		SetRect(&r,0,0,image->portRect.right,image->portRect.bottom-height);
		ClearRect(image,&r);
		SetRect(&r,width,image->portRect.bottom-height,
			image->portRect.right,image->portRect.bottom);
		ClearRect(image,&r);
	}

END:
	SetPort(port);
	InvalDevPort();
	InitAtVista();

	Msg2Host(ETX);
}

/*** end of ggrag.c ***/
