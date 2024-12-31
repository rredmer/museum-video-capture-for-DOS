	title	videoasm

.model	large,c

.data

savmode	DB	1
savcol	DB	1
savpage	DB	1
savcur	DW	1
curcol	DB	1		;current cursor position
curlin	DB	1
attr	DW	1		;character attribute

.code

; curoff();
curoff	proc
	mov	cx,2000h	;ch bit 5 on
	mov	ah,1
	int	10h
	ret
curoff	endp

; curon();
curon	proc
	mov	cx,savcur
	mov	ah,1
	int	10h
	ret
curon	endp

; initscr();
initscr	proc
	mov	ah,15
	int	10h
	mov	word ptr savmode,ax	;save current display mode & column
	mov	savpage,bh
	mov	ah,3
	int	10h		;get cursor size and save it
	mov	savcur,cx
	mov	ax,0003h	;set video mode 3 (80x25)
	int	10h
	xor	ax,ax
	mov	word ptr curcol,ax	;init cursor position
	ret
initscr	endp

; endwin();
endwin	proc
	mov	al,savmode	;restore video mode, etc.
	mov	ah,0
	int	10h
	mov	al,savpage
	mov	ah,5
	int	10h
	mov	cx,savcur
	mov	ah,1
	int	10h
	ret
endwin	endp

; getcur();
getcur	proc
	mov	ax,word ptr curcol
	ret
getcur	endp

; move(int line, int column);
move	proc	line:word, column:word
	mov	dh,byte ptr (line)	;line
	mov	dl,byte ptr (column)	;col
	mov	word ptr curcol,dx	;save current cursor position
	mov	bh,0			;set page 0
	mov	ah,2
	int	10h
	ret	
move	endp

readch	proc
	mov	ah,8		;int readch();
	int	10h
	ret
readch	endp

; setatr(int attribute)
setatr	proc	attribute:word
	mov	ax,attribute
	mov	attr,ax		;save attribute
	ret
setatr	endp

; addch(int ch);
addch	proc	character:word
	mov	bx,attr		;attribute
	mov	cx,1		;1 byte only
	mov	al,byte ptr (character);ch
	mov	ah,9
	int	10h
	inc	curcol
	mov	dx,word ptr curcol	;set next cursor position
	mov	bh,0		;set page 0
	mov	ah,2
	int	10h
	ret
addch	endp

; repch(int ch, int len);
repch	proc	character:word, len:word
	mov	bx,attr		;attr
	mov	cx,len		;len
	or	cx,cx
	jz	repch0
	mov	al,byte ptr (character);ch
	mov	ah,9
	int	10h
	mov	dx,len
	add	dx,word ptr curcol
	mov	word ptr curcol,dx
	mov	bh,0		;set page 0
	mov	ah,2
	int	10h
repch0:
	ret
repch	endp

; addstr(char *str);
addstr	proc	string:ptr byte
	les	bx,string
@@:
	mov	al,es:[bx]	;check string end
	or	al,al
	jz	@F
	push	bx

	mov	bx,attr		;set character attribute
	mov	cx,1
	mov	ah,9		;addch
	int	10h

	mov	dx,word ptr curcol ;increment col
	inc	curcol
	inc	dl
	mov	bh,0
	mov	ah,2
	int	10h

	pop	bx
	inc	bx
	jmp	short @B
@@:
	ret
addstr	endp

MDA	EQU	0
CGA	EQU	1
EGA	EQU	2
VGA	EQU	3

;int chkcrt();	returns 0:MDA,1:CGA,2:EGA,3:VGA
chkcrt	PROC
	PUSH	ES
	MOV	AX,1A00H
	INT	10H
	CMP	AL,1AH
	JE	FOUND_VGA
;
	MOV	AH,12H
	MOV	BL,10H
	INT	10H
	CMP	BL,10H
	JNE	FOUND_EGA
;
	MOV	AX,40H
	MOV	ES,AX
	MOV	AX,ES:[63H]		;CRTC BASE ADDRESS
	AND	AX,0FFF0H
	CMP	AX,3D0H
	JE	FOUND_CGA
;
FOUND_MDA:
	MOV	AX,MDA
	JMP	SHORT OWARI
;
FOUND_CGA:
	MOV	AX,CGA
	JMP	SHORT OWARI
;
FOUND_EGA:
	MOV	AX,EGA
	JMP	SHORT OWARI
;
FOUND_VGA:
	MOV	AX,VGA
;
OWARI:
	POP	ES
	RET
;
chkcrt	ENDP


; keywrite(int code);
keywrite	proc	keycode:word
	mov	cx,keycode
	mov	ah,5
	int	16h
	ret
keywrite	endp

; int keyread();
keyread	proc
	xor	ah,ah
	int	16h
	ret
keyread	endp

; int keyhit();
keyhit	proc
	mov	ah,1
	int	16h
	jnz	short @F
	xor	ax,ax
@@:
	ret
keyhit	endp

; keyflush();
keyflush	proc
@@:
	mov	ah,1
	int	16h
	jz	short @F
	xor	ah,ah
	int	16h
	jmp	short @B
@@:
	ret
keyflush	endp

; int shftstat();
shftstat	proc
	mov	ah,2
	int	16h
	and	al,0FH
	ret
shftstat	endp

; shftrls(int maskBit); --- while(shftstat() & mask) ;
shftrls	proc	maskBit:word
@@:
	mov	ah,2
	int	16h
	and	al,byte ptr (maskBit)
	jnz	@B
	ret
shftrls	endp

;long getticks();
getticks	proc
	xor	ah,ah
	int	1ah
	mov	ax,dx
	mov	dx,cx
	ret
getticks	endp


.data

extrn	VistaEnv:word
buffer	dd	0		;buffer pointer
datacnt	dw	0		;data counter

.code

;outpw(int portNum,int writeData);
outpw	proc	portNum:word, writeData:word
	mov	dx,portNum
	mov	ax,writeData
	out	dx,ax
	ret
outpw	endp

;inpw(int portNum);
inpw	proc	portNum:word
	mov	dx,portNum
	in	ax,dx
	ret
inpw	endp

;void movemem(char *src, char *dst, int len);
movemem	proc	src:ptr, dst:ptr, len:word
	push	ds
	push	es
	push	di
	push	si

	mov	cx,len
	or	cx,cx
	jz	@F
	cld
	lds	si,src
	les	di,dst
	rep movsb
@@:
	pop	si
	pop	di
	pop	es
	pop	ds
	ret
movemem	endp

;long GetCommAddr();		;get GSP Communication buffer pointer
GetCommAddr	proc
	mov	dx,VistaEnv
	in	ax,dx
	mov	bx,ax		;save it
	add	dx,2
	in	ax,dx
	mov	dx,ax		;set high
	mov	ax,bx		;load low
	ret
GetCommAddr 	endp

;int ReadGSPData();
ReadGSPData	proc
	mov	dx,VistaEnv
	add	dx,4
	in	ax,dx
	ret
ReadGSPData	endp

;void WriteGSPData(int writeData);
WriteGSPData	proc	writeData:word
	mov	ax,writeData
	mov	dx,VistaEnv
	add	dx,4
	out	dx,ax
	ret
WriteGSPData	endp

;void SetGSPMsg(int message);	;Set GSP message register
SetGSPMsg	proc	message:word
	mov	dx,VistaEnv
	add	dx,6
	in	ax,dx
	and	ax,0FF00h	;mask control
	or	ax,message
	out	dx,ax
	ret
SetGSPMsg	endp

;int GetGSPMsg();		;Return GSP message register value
GetGSPMsg	proc
	mov	dx,VistaEnv
	add	dx,6
	in	ax,dx
	and	ax,00FFh	;mask message
	ret
GetGSPMsg	endp

;SetGSPCtl(int ctl);		;Set specified value int GSP control register
SetGSPCtl	proc	ctl:word
	mov	dx,VistaEnv
	add	dx,6
	in	ax,dx
	and	ax,00FFh	;mask message
	or	ax,ctl
	out	dx,ax
	ret
SetGSPCtl	endp

;int GetGSPCtl();		;Return current GSP control register value
GetGSPCtl	proc
	mov	dx,VistaEnv
	add	dx,6
	in	ax,dx
	and	ax,0FF00h	;mask control
	ret
GetGSPCtl	endp

;void SetGSPAddr(long address);
SetGSPAddr	proc	address:dword
	mov	ax,word ptr (address)	;get address low
	mov	dx,VistaEnv	;set it
	out	dx,ax
	mov	ax,word ptr (address+2)	;get address high
	add	dx,2
	out	dx,ax
	ret
SetGSPAddr	endp

;void WriteGSPWord(long address,int writeData);
WriteGSPWord	proc	address:dword, writeData:word
	mov	ax,word ptr (address)	;get address low
	mov	dx,VistaEnv	;set it
	out	dx,ax
	mov	ax,word ptr (address+2)	;get address high
	add	dx,2
	out	dx,ax
	mov	ax,writeData	;get data
	add	dx,2
	out	dx,ax
	ret
WriteGSPWord	endp

;void WriteGSPLong(long address, long writeData);
WriteGSPLong	proc	address:dword, writeData:dword
	mov	ax,word ptr (address)	;get address low
	mov	dx,VistaEnv	;set it
	out	dx,ax
	mov	ax,word ptr (address+2)	;get address high
	add	dx,2
	out	dx,ax
	add	dx,4
	in	ax,dx
	mov	bx,ax		;save current control register
	or	ax,0800h	;set auto-increment write bit
	out	dx,ax
	mov	ax,word ptr (writeData)	;get low data word
	mov	dx,VistaEnv
	add	dx,4
	out	dx,ax		;write low word to data register
	mov	ax,word ptr (writeData+2)	;get high data word
	out	dx,ax
	add	dx,2
	mov	ax,bx
	out	dx,ax		;restore control register
	ret
WriteGSPLong	endp

;int ReadGSPWord(long address);
ReadGSPWord	proc	address:dword
	mov	ax,word ptr (address)	;get address low
	mov	dx,VistaEnv	;set it
	out	dx,ax
	mov	ax,word ptr (address+2)	;get address high
	add	dx,2
	out	dx,ax
	add	dx,2
	in	ax,dx
	ret
ReadGSPWord	endp

;long ReadGSPLong(long address);
ReadGSPLong	proc	address:dword
	mov	ax,word ptr (address)	;get address low
	mov	dx,VistaEnv	;set it
	out	dx,ax
	mov	ax,word ptr (address+2)	;get address high
	add	dx,2
	out	dx,ax
	add	dx,4
	in	ax,dx
	mov	bx,ax		;save current control register
	or	ax,0800h	;set auto-increment write bit
	out	dx,ax
	mov	dx,VistaEnv
	add	dx,4
	in	ax,dx		;get low data word
	push	ax		;save it
	in	ax,dx		;get high data word
	push	ax		;save it
	add	dx,2
	mov	ax,bx
	out	dx,ax		;restore control register
	pop	dx		;restore high data
	pop	ax		;restore low data
	ret
ReadGSPLong	endp

;void _GSPHostPortOutBuf(int *bufPtr, int cnt);
GSPHostPortOutBuf	proc	bufPtr:ptr, cnt:word
	mov	dx,VistaEnv
	add	dx,4
	les	bx,bufPtr	;set pointer
	mov	cx,cnt		;data counter
@L:
	mov	ax,es:[bx]	;load data word
	out	dx,ax
	inc	bx		;point up
	inc	bx
	loop	@L
	ret
GSPHostPortOutBuf	endp

; void GSPHostPortInBuf(int *bufPtr, int cnt);	
GSPHostPortInBuf	proc	bufPtr:ptr, cnt:word
	mov	dx,VistaEnv
	add	dx,4
	les	bx,bufPtr	;set pointer
	mov	cx,cnt		;data counter
@L:
	in	ax,dx
	mov	es:[bx],ax	;save data word
	inc	bx		;point up
	inc	bx
	loop	@L
	ret
GSPHostPortInBuf	endp

;void WriteGSPBuffer(int *bufPtr, long address, int cnt);
WriteGSPBuffer	proc	bufPtr:ptr, address:dword, cnt:word
	call	GetGSPCtl	;get current control value
	push	ax		;save it
	or	ax,1800h	;set auto-inc read/write
	push	ax
	call	SetGSPCtl
	push	word ptr (address+2)	;set address into stk frame
	push	word ptr (address)
	call	SetGSPAddr
	push	word ptr cnt	;set cnt into stk frame
	les	bx,bufPtr
	push	es
	push	bx
	call	GSPHostPortOutBuf
	add	sp,12
	call	SetGSPCtl	;restore control register
	pop	ax
	ret
WriteGSPBuffer	endp

;void ReadGSPBuffer(int *bufPtr, long address, int cnt);
ReadGSPBuffer	proc	bufPtr:ptr, address:dword, cnt:word
	call	GetGSPCtl	;get current control value
	push	ax		;save it
	or	ax,1800h	;set auto-inc read/write
	push	ax
	call	SetGSPCtl
	push	word ptr (address+2)	;set address into stk frame
	push	word ptr (address)
	call	SetGSPAddr
	push	word ptr cnt	;set buffer and cnt into stk frame
	les	bx,bufPtr
	push	es
	push	bx
	call	GSPHostPortInBuf
	add	sp,12
	call	SetGSPCtl	;restore control register
	pop	ax
	ret
ReadGSPBuffer	endp

WVEnt	equ	$
	dw	WVNoop		;0
	dw	WVNoop		;1
	dw	WVNoop		;2
	dw	WV8to16		;3
	dw	WV8to24		;4
	dw	WV8to32		;5
	dw	WV16to16	;6
	dw	WV16to24	;7
	dw	WV16to32	;8
	dw	WV24to16	;9
	dw	WV24to24	;10
	dw	WV24to32	;11
	dw	WV32to16	;12
	dw	WV32to24	;13
	dw	WV32to32	;14
WVEnd	equ	$

WVNoop	proc	near
	ret
WVNoop	endp

WV8to16	proc	near
	push	di
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	mov	al,byte ptr es:[bx]	;load data word
	mov	ah,al
	shr	ah,1
	shr	ah,1
	shr	ah,1
	shl	ax,1
	shl	ax,1
	and	ax,07FE0H
	mov	di,ax
	mov	al,byte ptr es:[bx]
	shr	al,1
	shr	al,1
	shr	al,1
	or	ax,di
	out	dx,ax
	inc	bx
	loop	@B
	pop	di
	ret
WV8to16	endp

WV8to24	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
	shr	cx,1		;1/2
@@:
	mov	al,byte ptr es:[bx]	;load data word
	mov	ah,al
	out	dx,ax
	inc	bx
	mov	ah,byte ptr es:[bx]
	out	dx,ax
	mov	al,ah
	out	dx,ax
	inc	bx
	loop	@B
	ret
WV8to24	endp

WV8to32	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	mov	al,byte ptr es:[bx]	;load data word
	mov	ah,al
	out	dx,ax
	xor	ah,ah
	out	dx,ax
	inc	bx
	loop	@B
	ret
WV8to32	endp

WV16to16	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	mov	ax,es:[bx]	;load data word
	out	dx,ax
	inc	bx		;point up
	inc	bx
	loop	@B
	ret
WV16to16	endp

WV16to24	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
	shr	cx,1		;1/2
@@:
	push	cx
	mov	ax,es:[bx]
	mov	cl,6
	shl	ax,cl
	and	ax,0F800H
	mov	al,es:[bx]
	mov	cl,3
	shl	al,cl
	out	dx,ax

	mov	ax,es:[bx]
	mov	cl,7
	shr	ax,cl

	mov	ah,es:[bx+2]	;next pixel
	mov	cl,3
	shl	ah,cl
	out	dx,ax

	mov	ax,es:[bx+2]
	mov	cl,2
	shr	ax,cl
	and	al,00F8H
	mov	ah,es:[bx+3]
	shl	ah,1
	out	dx,ax

	inc	bx		;point up
	inc	bx
	inc	bx
	inc	bx
	pop	cx
	loop	@B
	ret
WV16to24	endp

WV16to32	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	push	cx
	mov	ax,es:[bx]
	mov	cl,6
	shl	ax,cl
	and	ax,0F800H
	mov	al,es:[bx]
	mov	cl,3
	shl	al,cl
	out	dx,ax

	mov	ax,es:[bx]
	mov	cl,7
	shr	ax,cl
	out	dx,ax

	inc	bx		;point up
	inc	bx
	pop	cx
	loop	@B
	ret
WV16to32	endp

WV24to16	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	push	cx
	mov	al,es:[bx]
	mov	cl,3
	shr	al,cl
	and	al,1FH
	mov	ch,al
	mov	ax,es:[bx]
	mov	cl,6
	shr	ax,cl
	and	ax,03E0H
	or	al,ch
	push	ax
	mov	ah,es:[bx+2]
	and	ah,0F8H
	shr	ah,1
	mov	ch,ah
	xor	cl,cl
	pop	ax
	or	ax,cx
	out	dx,ax

	inc	bx		;point up
	inc	bx
	inc	bx
	pop	cx
	loop	@B
	ret
WV24to16	endp

WV24to24	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
	add	cx,cx
	add	cx,datacnt	;*3/2
	shr	cx,1
@@:
	mov	ax,es:[bx]	;load data word
	out	dx,ax
	inc	bx
	inc	bx
	loop	@B
	ret
WV24to24	endp

WV24to32	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	mov	ax,es:[bx]	;load data word
	out	dx,ax
	mov	al,es:[bx+2]
	xor	ah,ah
	out	dx,ax
	inc	bx
	inc	bx
	inc	bx
	loop	@B
	ret
WV24to32	endp

WV32to16	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	push	cx
	mov	al,es:[bx]
	mov	cl,3
	shr	al,cl
	and	al,1FH
	mov	ch,al
	mov	ax,es:[bx]
	mov	cl,6
	shr	ax,cl
	and	ax,03E0H
	or	al,ch
	push	ax
	mov	ah,es:[bx+2]
	and	ah,0F8H
	shr	ah,1
	mov	ch,ah
	xor	cl,cl
	pop	ax
	or	ax,cx
	out	dx,ax

	inc	bx		;point up
	inc	bx
	inc	bx
	inc	bx
	pop	cx
	loop	@B
	ret
WV32to16	endp

WV32to24	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
	shl	cx,1		;1/2
@@:
	mov	ax,es:[bx]	;load data word
	out	dx,ax
	inc	bx
	inc	bx
	mov	al,es:[bx]
	inc	bx
	inc	bx
	mov	ah,es:[bx]
	out	dx,ax
	inc	bx
	mov	ax,es:[bx]
	out	dx,ax
	inc	bx
	inc	bx
	inc	bx
	loop	@B
	ret
WV32to24	endp

WV32to32	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	mov	ax,es:[bx]	;load data word
	out	dx,ax
	inc	bx
	inc	bx
	mov	ax,es:[bx]
	out	dx,ax
	inc	bx
	inc	bx
	loop	@B
	ret
WV32to32	endp

;void _WriteVstaMem(int *bufPtr, long address, int cnt, int memtype);
_WriteVstaMem	proc	bufPtr:ptr, address:dword, cnt:word, memtype:word
	les	ax,bufPtr
	mov	word ptr buffer,ax
	mov	word ptr buffer+2,es
	mov	ax,cnt
	mov	datacnt,ax

	call	GetGSPCtl	;get current control value
	push	ax		;save it
	or	ax,1800h	;set auto-inc read/write
	push	ax
	call	SetGSPCtl
	push	word ptr (address+2)	;set address into stk frame
	push	word ptr (address)
	call	SetGSPAddr

	mov	bx,memtype	;memory type
	shl	bx,1		;*2
	call	word ptr WVEnt[bx]

	add	sp,6
	call	SetGSPCtl	;restore control register
	pop	ax
	ret
_WriteVstaMem	endp

RVEnt	equ	$
	dw	RVNoop
	dw	RVNoop
	dw	RV16to8
	dw	RV24to8
	dw	RV32to8
	dw	RV16
	dw	RV24
	dw	RV32
RVEnd	equ	$

RVNoop	proc	near
	ret
RVNoop	endp

RV16to8	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	in	ax,dx
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	byte ptr es:[bx],al
	inc	bx
	loop	@B
	ret
RV16to8	endp

RV24to8	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
	shr	cx,1		;1/2
@@:
	in	ax,dx
	mov	byte ptr es:[bx],al
	inc	bx
	in	ax,dx
	in	ax,dx
	mov	byte ptr es:[bx],al
	inc	bx
	loop	@B
	ret
RV24to8	endp

RV32to8	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	in	ax,dx
	mov	byte ptr es:[bx],al
	inc	bx
	in	ax,dx
	loop	@B
	ret
RV32to8	endp

RV16	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	in	ax,dx
	mov	es:[bx],ax
	inc	bx
	inc	bx
	loop	@B
	ret
RV16	endp

RV24	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
	shr	cx,1		;1/2
@@:
	in	ax,dx
	mov	es:[bx],ax
	in	ax,dx
	mov	es:[bx+2],ax
	in	ax,dx
	mov	es:[bx+4],ax
	add	bx,6
	loop	@B
	ret
RV24	endp

RV32	proc	near
	mov	dx,VistaEnv
	add	dx,4
	les	bx,buffer	;set pointer
	mov	cx,datacnt	;data counter
@@:
	in	ax,dx
	mov	es:[bx],ax
	in	ax,dx
	mov	es:[bx+2],al
	inc	bx
	inc	bx
	inc	bx
	loop	@B
	ret
RV32	endp


;void _ReadVstaMem(int *bufPtr, long address, int cnt, int memtype);
_ReadVstaMem	proc	bufPtr:ptr, address:dword, cnt:word, memtype:word
	les	ax,bufPtr
	mov	word ptr buffer,ax
	mov	word ptr buffer+2,es
	mov	ax,cnt
	mov	datacnt,ax

	call	GetGSPCtl	;get current control value
	push	ax		;save it
	or	ax,1800h	;set auto-inc read/write
	push	ax
	call	SetGSPCtl
	push	word ptr (address+2)	;set address into stk frame
	push	word ptr (address)
	call	SetGSPAddr

	mov	bx,memtype	;memory type
	shl	bx,1		;*2
	call	word ptr RVEnt[bx]

	add	sp,6
	call	SetGSPCtl	;restore control register
	pop	ax
	ret
_ReadVstaMem	endp

; CopyPat(char *src, int cnt, int pattlen);
CopyPat	proc	src:ptr, cnt:word, pattlen:word
	push	ds
	push	es
	push	di
	push	si

	cld
	mov	dx,cnt
	dec	dx		;-1 itself
	jz	@L1

	mov	cx,pattlen	
	les	di,src
	mov	ax,di
	add	ax,cx
	mov	di,ax		;set pattern dest
@L0:
	mov	cx,pattlen	
	lds	si,src		;pattern source
	rep movsb
	dec	dx
	jnz	@L0
@L1:
	pop	si
	pop	di
	pop	es
	pop	ds
	ret

CopyPat	endp


.code

; int _InitMouse(int *button, Rect *r)
_InitMouse	proc	button:ptr,rect:ptr
	push	es
	mov	ax,0		;reset mouse driver
	int	33h
	or	ax,ax
	jz	short @F

	mov	ax,bx
	les	bx,button
	mov	es:[bx],ax	;save button state

	mov	ax,32		;effective mouse driver
	int	33h

	les	bx,rect
	mov	cx,es:[bx]	;left
	mov	dx,es:[bx+4]	;right
	mov	ax,7
	int	33h
	mov	cx,es:[bx+2]	;top
	mov	dx,es:[bx+6]	;bottom
	mov	ax,8
	int	33h
	mov	ax,1
@@:
	pop	es
	ret
_InitMouse	endp

; CloseMouse();
CloseMouse	proc
	mov	ax,31		;uneffective mouse driver
	int	33h
	ret
CloseMouse	endp

; GetMickey(Point *point);
_GetMickey	proc	point:ptr
	push	di
	push	es
	mov	ax,11
	int	33h
	les	di,point
	mov	es:[di],dx
	mov	es:[di+2],cx
	pop	es
	pop	di
	ret
_GetMickey	endp

; int _button(int buttonNum);
_button	proc	buttonNum:word
	mov	bx,buttonNum
	mov	ax,5
	int	33h
	ret
_button	endp

; int _btnrls(int buttonNum);
_btnrls	proc	buttonNum:word
	mov	bx,buttonNum
	mov	ax,6
	int	33h
	ret
_btnrls	endp

	end
