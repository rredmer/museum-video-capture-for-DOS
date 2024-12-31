/*
**	gimage.c
*/

#include	"ghrcap.h"

/*
_enhance32(src,p0,p1,p2,cnt)
register Pix32 *src,p0[],p1[],p2[]; register int cnt;
{
	register int i;

	src++;
	
	for (i=1; i<=cnt; i++) {
		register int dr,dg,db,r,g,b;

	dr = (
	(int)p1[i].r*26
		-(int)(p0[i-1].r+p0[i+1].r+p1[i-1].r+p1[i+1].r+p2[i-1].r+p2[i+1].r)*3
		-(int)(p0[i].r+p2[i].r)*4
	);
	dg = (
	(int)p1[i].g*26
		-(int)(p0[i-1].g+p0[i+1].g+p1[i-1].g+p1[i+1].g+p2[i-1].g+p2[i+1].g)*3
		-(int)(p0[i].g+p2[i].g)*4
	);
	db = (
	(int)p1[i].b*26
		-(int)(p0[i-1].b+p0[i+1].b+p1[i-1].b+p1[i+1].b+p2[i-1].b+p2[i+1].b)*3
		-(int)(p0[i].b+p2[i].b)*4
	);

		r = ((r = src->r) * 52 + dr) / 52;
		if (r > 255) r = 255; else if (r < 0 ) r = 0;
		g = ((g = src->g) * 52 + dg) / 52;
		if (g > 255) g = 255; else if (g < 0 ) g = 0;
		b = ((b = src->b) * 52 + db) / 52;
		if (b > 255) b = 255; else if (b < 0 ) b = 0;

		src->r = r;
		src->g = g;
		src->b = b;

		src++;
	}
}
*/
/*
_enhance24(src,p0,p1,p2,cnt)
register Pix24 *src,p0[],p1[],p2[]; register int cnt;
{
	register int i;
	
	src++;

	for (i=1; i<=cnt; i++) {
		register int dr,dg,db,r,g,b;

		dr = (
				-(int)p0[i-1].r *10
				-(int)p0[i+1].r *10
				-(int)p0[i].r   *13
				-(int)p1[i-1].r *10
				+(int)p1[i].r   *86
				-(int)p1[i+1].r *10
				-(int)p2[i].r   *13
				-(int)p2[i-1].r *10
				-(int)p2[i+1].r *10
			 );
		dg = (
				-(int)p0[i-1].g *10
				-(int)p0[i+1].g *10
				-(int)p0[i].g   *13
				-(int)p1[i-1].g *10
				+(int)p1[i].g   *86
				-(int)p1[i+1].g *10
				-(int)p2[i].g   *13
				-(int)p2[i-1].g *10
				-(int)p2[i+1].g *10
			 );
		db = (
				-(int)p0[i-1].b *10
				-(int)p0[i+1].b *10
				-(int)p0[i].b   *13
				-(int)p1[i-1].b *10
				+(int)p1[i].b   *86
				-(int)p1[i+1].b *10
				-(int)p2[i].b   *13
				-(int)p2[i-1].b *10
				-(int)p2[i+1].b *10
			 );

		r = ((r = src->r) * 172 + dr) / 172;
		if (r > 255) r = 255; else if (r < 0 ) r = 0;
		g = ((g = src->g) * 172 + dg) / 172;
		if (g > 255) g = 255; else if (g < 0 ) g = 0;
		b = ((b = src->b) * 172 + db) / 172;
		if (b > 255) b = 255; else if (b < 0 ) b = 0;

		src->r = r;
		src->g = g;
		src->b = b;

		src++;
	}
}
*/
/*
_enhance16(src,p0,p1,p2,cnt)
register Pix16 *src,p0[],p1[],p2[]; register int cnt;
{
	register int i;

	src++;
	
	for (i=1; i<=cnt; i++) {
		register int dr,dg,db,r,g,b;

	dr = (
	(int)p1[i].r*26
		-(int)(p0[i-1].r+p0[i+1].r+p1[i-1].r+p1[i+1].r+p2[i-1].r+p2[i+1].r)*3
		-(int)(p0[i].r+p2[i].r)*4
	);
	dg = (
	(int)p1[i].g*26
		-(int)(p0[i-1].g+p0[i+1].g+p1[i-1].g+p1[i+1].g+p2[i-1].g+p2[i+1].g)*3
		-(int)(p0[i].g+p2[i].g)*4
	);
	db = (
	(int)p1[i].b*26
		-(int)(p0[i-1].b+p0[i+1].b+p1[i-1].b+p1[i+1].b+p2[i-1].b+p2[i+1].b)*3
		-(int)(p0[i].b+p2[i].b)*4
	);

		r = ((r = src->r) * 52 + dr) / 52;
		if (r > 31) r = 31; else if (r < 0 ) r = 0;
		g = ((g = src->g) * 52 + dg) / 52;
		if (g > 31) g = 31; else if (g < 0 ) g = 0;
		b = ((b = src->b) * 52 + db) / 52;
		if (b > 31) b = 31; else if (b < 0 ) b = 0;

		src->r = r;
		src->g = g;
		src->b = b;

		src++;
	}
}
*/

static enhance32(register GPtr port,register Rect *r) {
	EventRecord event;
	register int p0,p1,p2,tmp;
	register int addr,pitch,height,width,h;

	pitch = port->rowBits;
	addr = port->baseAddr + (port->portRect.bottom - r->top - 1) * pitch;
	height = r->bottom - r->top - 2;
	width = r->right - r->left;

	p0 = port->baseAddr - pitch;
	p1 = p0 - pitch;
	p2 = p1 - pitch;

	movemem32(addr,p0,width);
	addr -= pitch;
	movemem32(addr,p1,width);

	h = height;
	while(h--) {
		movemem32(addr-pitch,p2,width);
		_enhance32((Pix32 *)addr,(Pix32 *)p0,(Pix32 *)p1,(Pix32 *)p2,width-2);
		addr -= pitch;
		tmp = p0, p0 = p1, p1 = p2, p2 = tmp;
		*VistaEnv.commbuf = height - h;
		GetEvent(&event);
		if (event.cmdcode == CANMSG)
			return;
	}
}

static enhance24(register GPtr port,register Rect *r) {
	EventRecord event;
	register int p0,p1,p2,tmp;
	register int addr,pitch,height,width,h;

	pitch = port->rowBits;
	addr = port->baseAddr + (port->portRect.bottom - r->top - 1) * pitch;
	height = r->bottom - r->top - 2;
	width = r->right - r->left;

	p0 = port->baseAddr - pitch;
	p1 = p0 - pitch;
	p2 = p1 - pitch;

	movemem24(addr,p0,width);
	addr -= pitch;
	movemem24(addr,p1,width);

	h = height;
	while(h--) {
		movemem24(addr-pitch,p2,width);
		_enhance24((Pix24 *)addr,(Pix24 *)p0,(Pix24 *)p1,(Pix24 *)p2,width-2);
		addr -= pitch;
		tmp = p0, p0 = p1, p1 = p2, p2 = tmp;
		*VistaEnv.commbuf = height - h;
		GetEvent(&event);
		if (event.cmdcode == CANMSG)
			return;
	}
}

static enhance16(register GPtr port,register Rect *r) {
	EventRecord event;
	register int p0,p1,p2,tmp;
	register int addr,pitch,height,width,h;

	pitch = port->rowBits;
	addr = port->baseAddr + (port->portRect.bottom - r->top - 1) * pitch;
	height = r->bottom - r->top - 2;
	width = r->right - r->left;

	p0 = port->baseAddr - pitch;
	p1 = p0 - pitch;
	p2 = p1 - pitch;

	movemem16(addr,p0,width);
	addr -= pitch;
	movemem16(addr,p1,width);

	h = height;
	while(h--) {
		movemem16(addr-pitch,p2,width);
		_enhance16((Pix16 *)addr,(Pix16 *)p0,(Pix16 *)p1,(Pix16 *)p2,width-2);
		addr -= pitch;
		tmp = p0, p0 = p1, p1 = p2, p2 = tmp;
		*VistaEnv.commbuf = height - h;
		GetEvent(&event);
		if (event.cmdcode == CANMSG)
			return;
	}
}

DoEnhance(register GPtr port,register Rect *r) {
	*VistaEnv.commbuf = 0;		/* clear percent display value */

	switch (port->psize) {
	case 32:	enhance32(port,r); break;
	case 24:	enhance24(port,r); break;
	case 16:	enhance16(port,r); break;
	}
	Msg2Host(ETX);
}


static Gray32(register GPtr port,register Rect *r) {
	register int baseAddr,pitch,height,width;

	baseAddr = port->baseAddr + 
		(port->portRect.bottom - r->top - 1) * port->rowBits;
	pitch = port->rowBits * (-1);
	height = r->bottom - r->top;
	width = r->right - r->left;

	_Gray32(baseAddr,pitch,height,width);

	return;
/*
	EventRecord event;
	register int baseAddr,pitch,height,width;
	register int h,w;
	Pix32 *p,y;

	baseAddr = port->baseAddr + 
		(port->portRect.bottom - r->top - 1) * port->rowBits;
	pitch = port->rowBits * (-1);
	height = r->bottom - r->top;
	width = r->right - r->left;

	for (h=0; h<height; h++) {
		for (w=0,p=(Pix32 *)baseAddr; w<width; w++) {
			y.r = y.g = y.b = (p->r * 77 + p->g * 151 + p->b * 28) / 256;
			*p++ = y;
		}
		baseAddr += pitch;
		*VistaEnv.commbuf = h;
		GetEvent(&event);
		if (event.cmdcode == CANMSG)
			return;
	}
*/
}

static Gray24(register GPtr port,register Rect *r) {
	register int baseAddr,pitch,height,width;

	baseAddr = port->baseAddr + 
		(port->portRect.bottom - r->top - 1) * port->rowBits;
	pitch = port->rowBits * (-1);
	height = r->bottom - r->top;
	width = r->right - r->left;

	_Gray24(baseAddr,pitch,height,width);

	return;
/*
	EventRecord event;
	register int baseAddr,pitch,height,width;
	register int h,w;
	pix24 *p,y;

	baseAddr = port->baseAddr + 
		(port->portRect.bottom - r->top - 1) * port->rowBits;
	pitch = port->rowBits * (-1);
	height = r->bottom - r->top;
	width = r->right - r->left;

	for (h=0; h<height; h++) {
		for (w=0,p=(pix24 *)baseAddr; w<width; w++) {
			y.r = y.g = y.b = (p->r * 77 + p->g * 151 + p->b * 28) / 256;
			*p++ = y;
		}
		baseAddr += pitch;
		*VistaEnv.commbuf = h;
		GetEvent(&event);
		if (event.cmdcode == CANMSG)
			return;
	}
*/
}

Gray16(register GPtr port,register Rect *r) {
	register int baseAddr,pitch,height,width;

	baseAddr = port->baseAddr + 
		(port->portRect.bottom - r->top - 1) * port->rowBits;
	pitch = port->rowBits * (-1);
	height = r->bottom - r->top;
	width = r->right - r->left;

	_Gray16(baseAddr,pitch,height,width);

	return;
/*
	EventRecord event;
	register int baseAddr,pitch,height,width;
	register int h,w;
	pix16 *p,y;

	baseAddr = port->baseAddr + 
		(port->portRect.bottom - r->top - 1) * port->rowBits;
	pitch = port->rowBits * (-1);
	height = r->bottom - r->top;
	width = r->right - r->left;

	for (h=0; h<height; h++) {
		for (w=0,p=(pix16 *)baseAddr; w<width; w++) {
			y.r = y.g = y.b = (p->r * 77 + p->g * 151 + p->b * 28) / 256;
			*p++ = y;
		}
		baseAddr += pitch;
		*VistaEnv.commbuf = h;
		GetEvent(&event);
		if (event.cmdcode == CANMSG)
			return;
	}
*/
}

DoGreyScale(register GPtr port,register Rect *r) {
	*VistaEnv.commbuf = 0;		/* clear percent display value */

	switch(port->psize) {
	case 16:	Gray16(port,r); break;
	case 24:	Gray24(port,r); break;
	case 32:	Gray32(port,r); break;
	}
	Msg2Host(ETX);
}

static Matrix32(GPtr port,Rect *rect) {
	EventRecord event;
	register int r,g,b,or,og,ob,h,w;
	register Pix32 *p;
	short m[9];
	int baseAddr,pitch,height,width;
	short *pp,i;

	baseAddr = port->baseAddr + 
		(port->portRect.bottom - rect->top - 1) * port->rowBits;
	pitch = port->rowBits * (-1);
	height = rect->bottom - rect->top;
	width = rect->right - rect->left;

	for (i=0,pp=(short *)VistaEnv.commbuf; i<9; i++)
		m[i] = *pp++;
		
	h = height;
	while(h--) {
	 p = (Pix32 *)baseAddr;
	 w = width;
	 while(w--) {
	  or = p->r, og = p->g, ob = p->b;
	  r = (or * (int)m[0] + og * (int)m[1] + ob * (int)m[2]) / 1000;
	  if (r < 0) r = 0; else if (r > 255) r = 255;
	  p->r = r;
	  g = (or * (int)m[3] + og * (int)m[4] + ob * (int)m[5]) / 1000;
	  if (g < 0) g = 0; else if (g > 255) g = 255;
	  p->g = g;
	  b = (or * (int)m[6] + og * (int)m[7] + ob * (int)m[8]) / 1000;
	  if (b < 0) b = 0; else if (b > 255) b = 255;
	  p->b = b;
	  *p++;
	 }
		baseAddr += pitch;
		*VistaEnv.commbuf = height - h;
		GetEvent(&event);
		if (event.cmdcode == CANMSG)
		 	return;
	}
}

static Matrix24(GPtr port,Rect *rect) {
	EventRecord event;
	register int r,g,b,or,og,ob,h,w;
	register Pix24 *p;
	short m[9];
	int baseAddr,pitch,height,width;
	short *pp,i;

	baseAddr = port->baseAddr + 
		(port->portRect.bottom - rect->top - 1) * port->rowBits;
	pitch = port->rowBits * (-1);
	height = rect->bottom - rect->top;
	width = rect->right - rect->left;

	for (i=0,pp=(short *)VistaEnv.commbuf; i<9; i++)
		m[i] = *pp++;
		
	h = height;
	while(h--) {
	 p = (Pix24 *)baseAddr;
	 w = width;
	 while(w--) {
	  or = p->r, og = p->g, ob = p->b;
	  r = (or * (int)m[0] + og * (int)m[1] + ob * (int)m[2]) / 1000;
	  if (r < 0) r = 0; else if (r > 255) r = 255;
	  p->r = r;
	  g = (or * (int)m[3] + og * (int)m[4] + ob * (int)m[5]) / 1000;
	  if (g < 0) g = 0; else if (g > 255) g = 255;
	  p->g = g;
	  b = (or * (int)m[6] + og * (int)m[7] + ob * (int)m[8]) / 1000;
	  if (b < 0) b = 0; else if (b > 255) b = 255;
	  p->b = b;
	  *p++;
	 }
		baseAddr += pitch;
		*VistaEnv.commbuf = height - h;
		GetEvent(&event);
		if (event.cmdcode == CANMSG)
		 	return;
	}
}

static Matrix16(GPtr port,Rect *rect) {
	EventRecord event;
	register int r,g,b,or,og,ob,h,w;
	register Pix16 *p;
	short m[9];
	int baseAddr,pitch,height,width;
	short *pp,i;

	baseAddr = port->baseAddr + 
		(port->portRect.bottom - rect->top - 1) * port->rowBits;
	pitch = port->rowBits * (-1);
	height = rect->bottom - rect->top;
	width = rect->right - rect->left;

	for (i=0,pp=(short *)VistaEnv.commbuf; i<9; i++)
		m[i] = *pp++;
		
	h = height;
	while(h--) {
	 p = (Pix16 *)baseAddr;
	 w = width;
	 while(w--) {
	  or = p->r, og = p->g, ob = p->b;
	  r = (or * (int)m[0] + og * (int)m[1] + ob * (int)m[2]) / 1000;
	  if (r < 0) r = 0; else if (r > 31) r = 31;
	  p->r = r;
	  g = (or * (int)m[3] + og * (int)m[4] + ob * (int)m[5]) / 1000;
	  if (g < 0) g = 0; else if (g > 31) g = 31;
	  p->g = g;
	  b = (or * (int)m[6] + og * (int)m[7] + ob * (int)m[8]) / 1000;
	  if (b < 0) b = 0; else if (b > 31) b = 31;
	  p->b = b;
	  *p++;
	 }
		baseAddr += pitch;
		*VistaEnv.commbuf = height - h;
		GetEvent(&event);
		if (event.cmdcode == CANMSG)
		 	return;
	}
}

DoRGBMatrix(register GPtr port,register Rect *r) {

	switch(port->psize) {
	case 16:	Matrix16(port,r); break;
	case 24:	Matrix24(port,r); break;
	case 32:	Matrix32(port,r); break;
	}
	Msg2Host(ETX);
}


/*** end of enhance.c ***/
