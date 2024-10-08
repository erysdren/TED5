IDEAL
MODEL	medium,C
P386


DATASEG

startdi	dw	?

EXTRN	huffstring:DWORD
EXTRN	huffbits:WORD

CODESEG

;==================================
;
; FastHuffCompress
;
; ds:si	source
; es:di	dest
; ss:bp	(huffbits / huffstring)
;
; eax		shifted bit string
; bx		source byte
; cx		biton
; dx		bytes left to compress
;
;==================================

PROC	FastHuffCompress	source:DWORD, srclength:DWORD, dest:DWORD
PUBLIC	FastHuffCompress
	uses	si,di
;
; NOTE: this will not work on >64K data!
;

;
; set up
;
	mov	si,[WORD source]
	mov	ax,[WORD source+2]
	mov	bx,si
	shr	bx,4
	add	ax,bx
	and	si,15
	mov	ds,ax					; normallize ds:si

	mov	di,[WORD dest]
	mov	ax,[WORD dest+2]
	mov	bx,di
	shr	bx,4
	add	ax,bx
	and	di,15
	mov	es,ax					; normallize es:di
	mov	[ss:startdi],di			; save initial value for size calculation

	mov	edx,[srclength]
	xor	al,al
	mov	[es:di],al				; clear the first byte

	xor	ebx,ebx					; make sure the high word of ebx is 0
	xor	ecx,ecx					; start at bit 0

;
; compress
;

shrinkbyte:
	xor	eax,eax					; clear ahead of the current spot so
	mov	[es:di+1],eax			; new bits can be ORed in

	mov	bx,[si]
	xor	bh,bh
	inc	si

	shl	bx,2
	mov	eax,[ss:huffstring+bx] ; base bit string
	shl	eax,cl					; shift the bit string to the proper place
	or	[es:di],eax				; or it into the bit stream

	shr	bx,1
	add	cx,[ss:huffbits+bx]   ; next bit string goes here
	mov	ax,cx
	and	cx,7
	shr	ax,3
	add	di,ax

	dec	dx
	jnz	shrinkbyte

;
; clean up
;
	mov	ax,ss
	mov	ds,ax					; restore C's data segment

	inc	di						; be sure to include the last partial byte
	sub	di,[startdi]
	mov	ax,di
	xor	dx,dx					; return compressed length in dx:ax
	ret

ENDP


END

