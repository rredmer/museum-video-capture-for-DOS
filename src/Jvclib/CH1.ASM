	title	ch1

.model	large,c

APORT	equ	03f8h		;COM1 base address, use 02f8h for COM2
BPORT	equ	02f8h
AGETIV	equ	350ch		;COM1 vectors, use 0bh for COM2
BGETIV	equ	350bh
APUTIV	equ	250ch
BPUTIV	equ	250bh
AIMRMSK	equ	00010000b	;COM1 mask, use 00001000b for COM2
BIMRMSK	equ	00001000b

.data

BD_DV	dw	0417h		;0 baud rate divisor (0=110 bps)
	dw	0300h		;1 150 bps
	dw	0180h		;2 300 bps
	dw	00c0h		;3 600 bps
	dw	0060h		;4 1200 bps
	dw	0030h		;5 2400 bps
	dw	0018h		;6 4800 bps
	dw	000ch		;7 9600 bps

port	dw	0
getiv	dw	0
putiv	dw	0
imrmsk	db	0

oiv_o	dw	0		;old int vector save space
oiv_s	dw	0

bf_pp	dw	in_bf		;put pointer (last used)
bf_gp	dw	in_bf		;get pointer (next to use)
bf_bg	dw	in_bf		;start of buffer
bf_fi	dw	b_last		;end of buffer

in_bf	db	512 dup (?)	;input buffer

b_last	equ	$		;address just past buffer end

.code

w_tmr	proc	near
	mov	bl,1		;wait timer, bouble loop
w_tm1:	sub	cx,cx		;set up inner loop
w_tm2:	in	al,dx		;check for the requested response
	mov	ah,al		;save what came in
	and	al,bh		;mask with desired bits
	cmp	al,bh		;then compare
	jz	w_tm3		;got it, return with ZF set
	loop	w_tm2		;else keep trying
	dec	bl
	jnz	w_tm1
	or	bh,bh		;timed out, return NZ
w_tm3:
	ret
w_tmr	endp

; set_mdm(int parm);
set_mdm	proc	parm:word	;replace BIOS 'init' function
	mov	ah,byte ptr (parm)	;get parameter passed by C
	mov	dx,port		;point to line control register
	add	dx,3
	mov	al,80h		;set DLAB bit
	out	dx,al
	mov	dl,ah		;shift param to BAUD field
	mov	cl,4
	rol	dl,cl
	and	dx,00001110b	;mask out all other bits
	mov	di,offset BD_DV
	add	di,dx
	mov	dx,port		;set to high byte first
	add	dx,1
	mov	al,[di+1]
	out	dx,al		;put high byte into UART
	mov	dx,port		;then to low byte
	mov	al,[di]
	out	dx,al
	mov	al,ah		;now use rest of patameter
	and	al,00011111b	;to set Line Control Reg
	mov	dx,port
	add	dx,3
	out	dx,al
	mov	dx,port		;interrupt enable register
	add	dx,2
	mov	al,1		;receive type only
	out	dx,al
	ret
set_mdm	endp

; wrtmdm(int data);
wrtmdm	proc	writeData:word	;write char to modem
	mov	dx,port		;establish DTR,RTS and OUT2
	add	dx,4
	mov	al,0bh
	out	dx,al
	mov	dx,port		;check for on line,CTS
	add	dx,6
;	mov	bh,30h
	mov	bh,10h
	call	w_tmr
	jnz	@L		;timed out
	mov	dx,port		;check for UART ready
	add	dx,5
	mov	bh,20h
	call	w_tmr
	jnz	@L		;timed out
	mov	dx,port		;send out to UART port
	mov	al,byte ptr (writeData)	;get char passed from C
	out	dx,al
@L:
	ret
wrtmdm	endp

; int rdmdm();
rdmdm	proc			;reads byte from buffer
	mov	ax,0ffffh	;set for EOF flag
	mov	bx,bf_gp	;use "get" ptr
	cmp	bx,bf_pp	;compare to "put"
	jz	@L		;same, empty
	inc	bx		;else char available
	cmp	bx,bf_fi	;at end of bfr ?
	jnz	@F		;no
	mov	bx,bf_bg	;yes,set to begining
@@:
	mov	al,[bx]		;get the char
	mov	bf_gp,bx	;update "get" ptr
	inc	ah		;zero ah as flag
@L:
	ret
rdmdm	endp

;hardware intrrupt service routine
; rts_m();
rts_m	proc
	cli
	push	ds		;save all regs
	push	ax
	push	bx
	push	dx
	mov	ax,@data
	mov	ds,ax
	mov	dx,port
	in	al,dx
	mov	bx,bf_pp	;use "put" ptr
	inc	bx		;step to next slot
	cmp	bx,bf_fi	;past end yet ?
	jnz	nofix		;no
	mov	bx,bf_bg	;yes, set to begin
nofix:
	mov	[bx],al		;put char in buffer
	mov	bf_pp,bx	;update "put" pointer
	mov	al,20h		;send EOI to 8259 chip
	out	20h,al
	pop	dx
	pop	bx
	pop	ax
	pop	ds
	iret
rts_m	endp

; i_m(int port);
i_m	proc	portNum:word	;install modem service
	mov	dx,portNum	;get serial port
	mov	port,dx
	cmp	dx,APORT
	jnz	short im00
	mov	ax,AGETIV
	mov	getiv,ax
	mov	ax,APUTIV
	mov	putiv,ax
	mov	al,AIMRMSK
	mov	imrmsk,al
	jmp	short im01
im00:
	mov	ax,BGETIV
	mov	getiv,ax
	mov	ax,BPUTIV
	mov	putiv,ax
	mov	al,BIMRMSK
	mov	imrmsk,al
im01:
	mov	dx,port		;interrupt enable reg
	add	dx,1
	mov	al,0fh		;enable all ints
	out	dx,al

im1:	mov	dx,port		;clear junk from UART
	add	dx,2
	in	al,dx		;read IID reg of UART
	mov	ah,al		;save what came in
	test	al,1		;anything pending ?
	jnz	im5		;no,all clear now
	cmp	ah,0		;yes,modem status
	jnz	im2		;no
	mov	dx,port		;yes,read MSR to clear
	add	dx,6
	in	al,dx
im2:	cmp	ah,2		;transmit HR empty ?
	jnz	im3		;no (no action needed)
im3:	cmp	ah,4		;received data ready ?
	jnz	im4		;no
	mov	dx,port		;yes,read it to clear
	in	al,dx
im4:	cmp	ah,6		;line status ?
	jnz	im1		;no,check for more
	mov	dx,port		;yes,read LSR to clear
	add	dx,5
	in	al,dx
	jmp	im1		;then check for more

im5:	mov	dx,port		;set up working conditions
	add	dx,4
	mov	al,0bh		;DTR,RTS,OUT2 bits
	out	dx,al
	mov	al,1		;enable RCV interrupt only
	mov	dx,port
	add	dx,1
	out	dx,al
	mov	ax,getiv	;get old int vector
	int	21h
	mov	oiv_o,bx	;save for restoring later
	mov	oiv_s,es

	mov	ax,putiv
	mov	es,ax
	push	ds
	mov	ax,@code
	mov	ds,ax
	mov	dx,offset rts_m	;set in new one
	mov	ax,es
	int	21h
	pop	ds

	in	al,21h		;now enable 8259 PIC

	push	bx
	mov	bh,imrmsk
	not	bh
	and	al,bh
	pop	bx

	out	21h,al
	mov	al,20h		;then send out an EOI
	out	20h,al
	ret
i_m	endp

; u_m();
u_m	proc			;uninstall modem service
	in	al,21h		;disable COM int in 8259
	or	al,imrmsk
	out	21h,al
	mov	al,0		;disable UART ints
	mov	dx,port
	add	dx,1
	out	dx,al

	push	es
	push	ds
	mov	ax,putiv
	mov	dx,oiv_o
	mov	ds,oiv_s
	int	21h
	pop	ds
	pop	es
	ret
u_m	endp

	end
