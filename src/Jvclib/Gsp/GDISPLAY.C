/*
**	gdisplay.c
*/

#include	"ghrcap.h"

#define	DEB

static short crosCrs16[] = {
	0x0140,	
	0x0140,	
	0x0140,	
	0x0140,	
	0x0140,	
	0x0140,
	0x7F7F, 
	0x0000, 
	0x7F7F,
	0x0140,	
	0x0140,	
	0x0140,	
	0x0140,	
	0x0140,	
	0x0140,
	0x0000
};

/*
static short crosCrs16[] = {
	0x0001,
	0x0003,
	0x0007,
	0x000F,
	0x001F,
	0x003F,
	0x007F,
	0x00FF,
	0x01FF,
	0x03FF,
	0x07FF,
	0x0FFF,
	0x1FFF,
	0x3FFF,
	0x7FFF,
	0xFFFF
};
*/

static int crosCrs32[] = {
	0x00033000,
	0x00033000,
	0x00033000,
	0x00033000,
	0x00033000,
	0x00033000,
	0x3FFF3FFF,
	0x00000000,
	0x3FFF3FFF,
	0x00033000,
	0x00033000,
	0x00033000,
	0x00033000,
	0x00033000,
	0x00033000,
	0x00000000,
};
static short blkCrs16[] = {
	0x7FFF,
	0x4001,
	0x4001,
	0x4001,
	0x4001,
	0x4001,
	0x4001,
	0x4001,
	0x4001,
	0x4001,
	0x4001,
	0x4001,
	0x4001,
	0x4001,
	0x7FFF,
	0x0000
};
static int blkCrs32[] = {
	0x3FFFFFFF,
	0x30000003,
	0x30000003,
	0x30000003,
	0x30000003,
	0x30000003,
	0x30000003,
	0x30000003,
	0x30000003,
	0x30000003,
	0x30000003,
	0x30000003,
	0x30000003,
	0x30000003,
	0x3FFFFFFF,
	0x00000000
};

static Word ZoomTbl16[6] = { 0x10, 0x21, 0x32, 0x43, 0x54, 0x65 };
static Word ZoomTbl32[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };

static int	crsType;
static int	ZoomFactor;
static int	ZoomDiv;

static Rect curR;
static Rect prvR;

ToLowerLeft(register GPtr port,register Rect *sr,register Rect *dr) {
	dr->left = sr->left;
	dr->top = port->portRect.bottom - sr->bottom;
	dr->right = sr->right;
	dr->bottom = port->portRect.bottom - sr->top;
}

ClearRect(register GPtr port,register Rect *r) {
	register GPtr gport;
	register int src;
	register int width,height;
	register int psize;
	
	gport = GetPort();
	psize = GetPSize();

	SetPort(port);
	width = r->right - r->left;
	height = (r->bottom - r->top) << 16;
	src = port->baseAddr + r->top*port->rowBits + r->left*port->psize;

	switch(port->psize) {
	case 16:
		clr(src,height+width);
		break;
	case 24:
		SetPSize(8);
		clr(src,height+width*3);
		break;
	case 32:
		clr(src,height+width*2);
		break;
	}

	SetPSize(psize);
	SetPort(gport);
}

CopyBits(register GPtr sport,register GPtr dport,
		register Rect *sr,register Rect *dr,int direction) {
	register int src,dst;
	register int width,height;
	int control;

	if (sport->baseAddr == dport->baseAddr)
		return;

	if ((width = sr->right - sr->left) == 0)
		return;

	if ((height = sr->bottom - sr->top) == 0)
		return;

	control = GetControl();

	if (direction) {
		SetControl(control | 0x0200);	/* V decrement */
		src = sport->baseAddr +
			(sr->bottom-1)*sport->rowBits + sr->left*sport->psize;
		dst = dport->baseAddr +
			(dr->bottom-1)*dport->rowBits + dr->left*dport->psize;
	} else {
		src = sport->baseAddr + 
			sr->top*sport->rowBits + sr->left*sport->psize;
		dst = dport->baseAddr + 
			dr->top*dport->rowBits + dr->left*dport->psize;
	}

	SetPitch(sport->rowBits,dport->rowBits);

	if (sport->psize == 16 && dport->psize == 16)
		PixBltL(src,dst,(height << 16) + width);
	else if (sport->psize == 24 && dport->psize == 24)
		PixBltL24to24(src,dst,width,height);
	else if (sport->psize == 24 && dport->psize == 32)
		PixBltL24to32(src,dst,width,height);
	else if (sport->psize == 32 && dport->psize == 24)
		PixBltL32to24(src,dst,width,height);
	else if (sport->psize == 32 && dport->psize == 32)
		PixBltL(src,dst,(height << 16) + width*2);
	
	SetControl(control);
}

Boolean SectRect(register Rect *sr1, register Rect *sr2, register Rect *dr) {
	dr->left = max(sr1->left,sr2->left);
	dr->top = max(sr1->top,sr2->top);
	dr->right = min(sr1->right,sr2->right);
	dr->bottom = min(sr1->bottom,sr2->bottom);
	if ((dr->left < dr->right) && (dr->top < dr->bottom)) {
		return true;
	} else {
		*dr = *sr1;
		return false;
	}
}

static _CopyDevPort(register GPtr sport,
	register Rect *srp,register Rect *drp) {
	Rect sr,dr;

	ToLowerLeft(sport,srp,&sr);
	ToLowerLeft(&DspCapInfo.devPort,drp,&dr);
	CopyBits(sport,&DspCapInfo.devPort,&sr,&dr,0);
}

static CopyDevPort(register GPtr sport, register Rect *srp) {
	register int dh,dv,h,v,mh,mv;
	Rect dr,dr1,sr,sr1;
	
	dh = DspCapInfo.devPort.portRect.right;
	dv = DspCapInfo.devPort.portRect.bottom;

	sr = *srp;
	dr = *srp;

	h = dr.right - dr.left;
	v = dr.bottom - dr.top;
	mh = dr.left/dh + 1;
	mv = dr.top/dv + 1;

	dr.left %= dh;
	dr.top %= dv;
	
	if ((dr.left + h <= dh) && (dr.top + v <= dv)) {
		SetRect(&dr1, dr.left, dr.top, dr.left + h, dr.top + v);
		_CopyDevPort(sport,&sr,&dr1);
	}
	else if ((dr.left + h <= dh) && (dr.top + v > dv)) {
		SetRect(&dr1, dr.left, dr.top, dr.left + h, dv);
		SetRect(&sr1, sr.left, sr.top, sr.right, dv*mv);
		_CopyDevPort(sport,&sr1,&dr1);

		SetRect(&dr1, dr.left, 0, dr.left + h, dr.top + v - dv);
		SetRect(&sr1, sr.left, dv*mv, sr.right, sr.bottom);
		_CopyDevPort(sport,&sr1,&dr1);
	}
	else if ((dr.left + h > dh) && (dr.top + v <= dv)) {
		SetRect(&dr1, dr.left, dr.top, dh, dr.top + v);
		SetRect(&sr1, sr.left, sr.top, dh*mh, sr.bottom);
		_CopyDevPort(sport,&sr1,&dr1);

		SetRect(&dr1, 0, dr.top, dr.left + h - dh, dr.top + v);
		SetRect(&sr1, dh*mh, sr.top, sr.right, sr.bottom);
		_CopyDevPort(sport,&sr1,&dr1);
	}
	else if ((dr.left + h > dh) && (dr.top + v > dv)) {
		SetRect(&dr1, dr.left, dr.top, dh, dv);
		SetRect(&sr1, sr.left, sr.top, dh*mh, dv*mv);
		_CopyDevPort(sport,&sr1,&dr1);

		SetRect(&dr1, 0, dr.top, dr.left + h - dh, dv);
		SetRect(&sr1, dh*mh, sr.top, sr.right, dv*mv);
		_CopyDevPort(sport,&sr1,&dr1);

		SetRect(&dr1, dr.left, 0, dh, dr.top + v - dv);
		SetRect(&sr1, sr.left, dv*mv, dh*mh, sr.bottom);
		_CopyDevPort(sport,&sr1,&dr1);

		SetRect(&dr1, 0, 0, dr.left + h - dh, dr.top + v - dv);
		SetRect(&sr1, dh*mh, dv*mv, sr.right, sr.bottom);
		_CopyDevPort(sport,&sr1,&dr1);
	}
}

InvalDevPort() {
	if (VistaEnv.vistaMem > 4 && DspCapInfo.capMode > X1)
		CopyDevPort(&DspCapInfo.imagePort, &DspCapInfo.devPort.portRect);
}

DGetPXY(register int x,register int y) {
	if (thePort->psize == 32)
		x %= thePort->portRect.right * 2;
	else
		x %= thePort->portRect.right;
	y %= thePort->portRect.bottom;
	y = thePort->portRect.bottom - 1 - y;
	return GetPXY(y,x);
}

DPutPXY(register int x,register int y,register int data) {
	if (thePort->psize == 32)
		x %= thePort->portRect.right * 2;
	else
		x %= thePort->portRect.right;
	y %= thePort->portRect.bottom;
	y = thePort->portRect.bottom - 1 - y;
	return PutPXY(y,x,data);
}

Boolean PtInRect(register int h,register int v,register Rect *r) {
	if (r->left <= h && h < r->right && r->top <= v && v < r->bottom)
		return true;
	else
		return false;
}

OffsetRect(register Rect *rect, register int dh, register int dv) {
	rect->left += dh;
	rect->right += dh;
	rect->top += dv;
	rect->bottom += dv;
}

InsetRect(register Rect *rect, register int dh, register int dv) {
	rect->left += dh;
	rect->right -= dh;
	rect->top += dv;
	rect->bottom -= dv;
}

SetRect(register Rect *rect, int left, int top, int right, int bottom) {
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;
}

ClipRect(register Rect *r) {
	GPtr gptr;
	register int w;
	
	gptr = GetPort();
	w = (gptr->portRect.bottom - r->bottom) * 65536 + r->left;
	asm("	move	a9,b5");
	w = (gptr->portRect.bottom - r->top - 1) * 65536 + r->right - 1;
	asm("	move	a9,b6");
	SetPort(gptr);
}

static PutPixel(register int x, register int y) {
	register GPtr port;
	register Wptr ptr;
	
	port = GetPort();
	
	SetPort(&DspCapInfo.devPort);
	
	ptr = VistaEnv.commbuf;
	if (thePort->psize == 32) {
		x *= 2;
		*ptr++ = DGetPXY(x,y);		/* GB */
		*ptr =   DGetPXY(x+1,y);	/* AR */
	}
	else {
		*ptr = DGetPXY(x,y);
	}

	SetPort(port);
}

static PutPixelAverage(register int x,register int y) {
	register GPtr port;
	register Wptr ptr;
	register int rs,gs,bs,data,i,j;
	
	port = GetPort();
	SetPort(&DspCapInfo.devPort);

	rs = gs = bs = 0;
	
	x -= 7;
	y -= 7;
	for (i=0; i<16; i++) {
		for (j=0; j<16; j++) {
			if (thePort->psize == 32) {
				data = DGetPXY((x+j)*2,y+i);
				bs += data & 0x00FF;
				gs += data >> 8;
				data = DGetPXY((x+j)*2+1,y+i);
				rs += data & 0x00FF;
			} else {
				data = DGetPXY(x+j,y+i);
				bs += data & 0x001F;
				gs += (data >> 5) & 0x001F;
				rs += data >> 10;
			}
		}
	}

	rs /= 256; gs /= 256; bs /= 256;
	if (thePort->psize == 32)
		*(int *)VistaEnv.commbuf = (rs << 16) + (gs << 8) + bs;
	else
		*(int *)VistaEnv.commbuf = (rs << 10) + (gs << 5) + bs;

	SetPort(port);

}

int GetTotalDiff() {
	return 0;
/*
	register GPtr port;
	register int data,i;
	register unsigned long vramAddr;
	int blue[128];
	
	port = GetPort();
	SetPort(&DspCapInfo.devPort);

	if (thePort->psize == 32) {
		vramAddr = VRAMEND - PALResCrtV/2*PITCH + (PALResCrtH/3-128)/2*32;
		for (i=0; i<128; i++) {
			data = GetPix(vramAddr);
			blue[i] = data;
			vramAddr += 32;
		}
	}
	else {
		vramAddr = VRAMEND - PALResCrtV/2*PITCH + (PALResCrtH/3*2-128)/2*16;
		for (i=0; i<128; i++) {
			data = GetPix(vramAddr);
			blue[i] = data & 0x001F;
			vramAddr += 16;
		}
	}

	for (i=0; i<127; i++)
		blue[i] = abs(blue[i+1] - blue[i]);
	for (i=0,data=0; i<127; i++)
		data += blue[i];

	SetPort(port);
	return data;
*/
}

static MoveImageHV() {
	register GPtr image;
	Rect r;

	image = &DspCapInfo.imagePort;

	if (prvR.top <= curR.top) {
		if (prvR.left <= curR.left) {
			SetRect(&r, prvR.right, curR.top, curR.right, curR.bottom);
			CopyDevPort(image,&r);
			SetRect(&r, curR.left, prvR.bottom, prvR.right, curR.bottom);
			CopyDevPort(image,&r);
		} else {
			SetRect(&r, curR.left, curR.top, prvR.left, curR.bottom);
			CopyDevPort(image,&r);
			SetRect(&r, prvR.left, prvR.bottom, curR.right, curR.bottom);
			CopyDevPort(image,&r);
		}
	} else {
		if (prvR.left <= curR.left) {
			SetRect(&r, prvR.right, curR.top, curR.right, curR.bottom);
			CopyDevPort(image,&r);
			SetRect(&r, curR.left, curR.top, prvR.right, prvR.top);
			CopyDevPort(image,&r);
		} else {
			SetRect(&r, curR.left, curR.top, prvR.left, curR.bottom);
			CopyDevPort(image,&r);
			SetRect(&r, prvR.left, curR.top, curR.right, prvR.top);
			CopyDevPort(image,&r);
		}
	}
}

static AdjustHV() {
	register int d,line;
	register Word str,end;
	register Wptr ptr;
	register int dir;
	register GPtr dsp,dev;
	register DspCapINFO *info;

	prvR = curR;		/* save current device rect */

	dir = 0;
	info = &DspCapInfo;
	
	if ((d = info->dspRect.bottom - curR.bottom) > 0) {
		OffsetRect(&curR,0,d);
		dir |= 1;
	} else if ((d = info->dspRect.top - curR.top) < 0) {
		OffsetRect(&curR,0,d);
		dir |= 1;
	}

	if ((d = info->dspRect.right - curR.right) > 0) {
		OffsetRect(&curR,d,0);
		dir |= 2;
	} else if ((d = info->dspRect.left - curR.left) < 0) {
		OffsetRect(&curR,d,0);
		dir |= 2;
	}
	
	if (dir) MoveImageHV();
	
	/* set row table for the new view rect */
	dsp = &DspCapInfo.dspPort;
	dev = &DspCapInfo.devPort;

	str = ROWSTR - (DspCapInfo.dspRect.top % dev->portRect.bottom);
	end = ROWSTR - dev->portRect.bottom;
	ptr = (Wptr)ROWTBL+8;
	for (line=1; line <= dsp->portRect.bottom; line++) {
		*ptr++ = str | 0x0C00;
		if ((line % ZoomDiv) == 0) {
			str--;
			if (str == end)
				str = ROWSTR;
		}
	}

	/* set tap for the new view rect */
	*(Wptr)DPYTAP = (dev->psize == 32) ? 
		info->dspRect.left/4 : info->dspRect.left/8;
}

static Zooming(x,y,zoom) register int x,y,zoom; {
	register int width,height;
	register GPtr dev,dsp,image;
	register Rect *dr;
	Word video,i;
	
	for (i=0,ZoomDiv=1; i<zoom; i++) ZoomDiv <<= 1;
	ZoomFactor = zoom;
	
	dev = &DspCapInfo.devPort;
	dsp = &DspCapInfo.dspPort;
	image = &DspCapInfo.imagePort;
	dr = &DspCapInfo.dspRect;

	video = *(Wptr)VIDEOS;
	SetVideo((video & 0xFF80) + 
		((dev->psize == 32) ? ZoomTbl32[zoom] : ZoomTbl16[zoom]));
	width = (dsp->portRect.right - dsp->portRect.left) / ZoomDiv;
	height = (dsp->portRect.bottom - dsp->portRect.top) / ZoomDiv;
	SetRect(dr,x-width/2,y-height/2,x+width/2,y+height/2);
	OffsetRect(dr,dr->left % 8 * (-1),0);
	if (dr->right > image->portRect.right) {
		dr->left = (image->portRect.right - width) & 0xFFF8;
		dr->right = dr->left + width;
	}
	else if (dr->left < 0) {
		dr->left = 0;
		dr->right = width;
	}
	if (dr->bottom > image->portRect.bottom) {
		dr->bottom = image->portRect.bottom;
		dr->top = image->portRect.bottom - height;
	}
	else if (dr->top < 0) {
		dr->top = 0;
		dr->bottom = height;
	}
	AdjustHV();

}

static AdjustCrt(register int x, register int y) {
	register int d,width,height;
	register int changed;
	register Rect *dr;
	register GPtr dsp,image;

	changed = false;
	dsp = &DspCapInfo.dspPort;
	image = &DspCapInfo.imagePort;
	dr = &DspCapInfo.dspRect;

	/* adjust to the horizontal direction */

	width = dr->right - dr->left;
	if ((d = x - (dr->right - 1)) > 0) {
		OffsetRect(dr, (d/8+1)*8, 0);
		if (dr->right > image->portRect.right) {
			dr->left = image->portRect.right - width;
			dr->right = dr->left + width;
		}
		changed = true;;
	} else if ((d = x - (dr->left - 1)) < 0) {
		OffsetRect(dr,(-d/8+1)*(-8),0);
		if (dr->left < 0) {
			dr->left = 0;
			dr->right = width;
		}
		changed = true;;
	}

	/* adjust to the vertical direction */

	height = dr->bottom - dr->top;
	if ((d = y - (dr->bottom - 1)) > 0) {
		OffsetRect(dr,0,d);
		if (dr->bottom > image->portRect.bottom) {
			dr->bottom = image->portRect.bottom;
			dr->top = image->portRect.bottom - height;
		}
		changed = true;;
	} else if ((d = y - (dr->top - 1)) < 0) {
		OffsetRect(dr,0,d);
		if (dr->top < 0) {
			dr->top = 0;
			dr->bottom = height;
		}
		changed = true;;
	}
	if (changed) AdjustHV();

}

static Cursor(register int x, register int y) {
	register int width, height;
	register GPtr port;
	register int control,mask;

	if (PtInRect(x,y,&DspCapInfo.imagePort.portRect) == false)
		return;
	
	port = GetPort();
	mask = GetPMask();
	control = GetControl();

	SetPort(&DspCapInfo.devPort);

	/* adjust current display rectangle onto the phisical screen */
	AdjustCrt(x,y);

	width = DspCapInfo.devPort.portRect.right;
	height = DspCapInfo.devPort.portRect.bottom;

	/* change the virtual cursor address to the physical device address */
	x %= width;
	y = height - y%height - 1;	/* change the graphic origin */

	SetControl((control & 0x83FF) + 0x2800);	/* xor operation */
	if (DspCapInfo.devPort.psize == 32)
		_Cursor32(crsType,x-7,y-7);
	else
		_Cursor16(crsType,x-7,y-7);
	
	SetPMask(mask);
	SetControl(control);
	SetPort(port);
}

InitCursor(register int kind) {
	if (DspCapInfo.devPort.psize == 32)
		crsType = (kind==0) ? (int)&crosCrs32[0] : (int)&blkCrs32[0];
	else
		crsType = (kind==0) ? (int)&crosCrs16[0] : (int)&blkCrs16[0];
}

DoDisplay(register int kind) {
	register int control;
	EventRecord event;
	register int zoom;
	register int cx,cy;

	control = GetControl();						/* save control register */
	SetControl((control & 0x801F)+0x00C0);		/* enabel window clipping */
	SetColor(0,-1);

	cx = cy = 32767;
	DspCapInfo.dspRect = DspCapInfo.dspPort.portRect;
	curR = DspCapInfo.devPort.portRect;

	/* reset cursor position */
	ZoomFactor = zoom = 0;
	ZoomDiv = 1;

	Msg2Host(ACK);

	for (;;) {
		GetEvent(&event);

		switch(event.cmdcode) {
		case CURMSG:
			Cursor(cx,cy);
			cx = event.message.smes[0];
			cy = event.message.smes[1];
			Cursor(cx,cy);		/* draw current cursor */
			break;
		case ZOMMSG:
			zoom = event.message.smes[1];
			Cursor(cx,cy);		/* erase cursor for zooming */
			Zooming(cx,cy,zoom);
			Cursor(cx,cy);		/* display cursor again */
			break;
		case PIXMSG:
			PutPixel(cx,cy);
			Msg2Host(ETX);
			break;
		case CANMSG:
			Cursor(cx,cy);		/* erase current cursor */
			goto END;
			break;
		}
	}
END:
	SetControl(control);

}

/*** end of gdisplay.c ***/
