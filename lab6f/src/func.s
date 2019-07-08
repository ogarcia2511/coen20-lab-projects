.cpu	cortex-m4
.syntax	unified
.text

.global DecodeMessage
.thumb_func
.align
DecodeMessage:	// &msg in R0
		// &array in R1
		push	{R4 - R9, LR}
		mov	R4, R0	// &msg
		mov	R5, R1  // &array
		ldr	R6, =0 	// bitnum
DMstart:	ldr 	R9, =0 	// k = 0
DMtop:		mov	R0, R4
		mov	R1, R6
		bl	GetBit	// bit in R0
		add	R6, R6, 1
		ldr	R8, =2		
		mul	R9, R9, R8
		adds	R0, R0, #1
		adds	R9, R9, R0 // k
		add	R0, R9, R5
		ldrb	R7, [R0] // array[k]
		cmp	R7, 0
		beq	DMtop
		cmp	R7, '$'
		beq	DMdone
		mov	R0, R7
		bl	putchar 
		b	DMstart
DMdone:		pop	{R4 - R9, LR}
		bx	LR

.global GetBit
.thumb_func
.align

GetBit:	// &msg in R0
	// bitnum in R1
.if 0
	lsr 	R2, R1, 3  	// bitnum/8
	add	R0, R0, R2	// 
	ldrb	R0, [R0]	
	and	R1, R1, 0x00000007
	lsr	R0, R0, R1
	and	R0, R0, 0x00000001
	bx	LR
.else
	sub	R0, R0, 0x20000000
	lsls.n	R0, R0, 5
	add	R0, R0, R1, lsl 2
	add	R0, R0, 0x22000000
	ldr	R0, [R0]
	bx 	LR
.endif



