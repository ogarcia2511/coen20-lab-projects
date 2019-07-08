.cpu		cortex-m4
.syntax 	unified
.text

zero: 		.float	0.0
one:		.float	1.0

.global HWFlPtPoly
.thumb_func
.align

HWFlPtPoly:	// x in S0
		// &a in R0
		// n in R1
		vldr		S1, zero // poly
		vldr		S2, one	 // x2n
		ldr		R2, =0	 // i
HWloop:		cmp		R2, R1
		beq		HWdone
		vldmia		R0!, {S3}
		vmul.f32	S3, S3, S2		// a[i] * x2n
		vadd.f32	S1, S1, S3		// poly += a[i] * x2n
		vmul.f32	S2, S2, S0
		adds		R2, R2, #1
		b		HWloop
HWdone:		vmov		S0, S1
		bx		LR
		
						 

.global SWFlPtPoly
.thumb_func
.align
SWFlPtPoly:	// x in R0
		// &a in R1
		// n in R2
		push 	{R4 - R10, LR}	
		ldr 	R5, =0 		// i
		ldr 	R6, =0 		// poly
		ldr 	R7, =0x3F800000 // x2n
		mov	R8, R0 		// x
		mov	R9, R1 		// &a
		mov	R10, R2 	// n

SWloop:		cmp 	R5, R10
		beq	SWdone
		ldmia	R9!, {R0}	// a[i]
		mov	R1, R7
		bl	MulFloats	// result: a[i] * x2n in R0
		mov	R1, R6
		bl	AddFloats
		mov	R6, R0		// poly += a[i] * x2n
		mov	R0, R7
		mov	R1, R8
		bl	MulFloats
		mov	R7, R0
		adds	R5, R5, #1
		b 	SWloop
SWdone:		mov	R0, R6
		pop	{R4 - R10, LR}
		bx	LR	
		
		
.global Q16FxdPoly
.thumb_func
.align
	
Q16FxdPoly: 	push	{R4 - R7, LR}
		mov	R7, R0
		ldr 	R3, =0
		ldr	R4, =1
		lsl	R4, R4, 16
Q16outer:	cbz	R2, Q16done
		ldmia	R1!, {R5}
		smull	R6, R5, R4, R5
		lsr	R6, R6, 16
		orr	R6, R6, R5, lsl 16
		adds	R3, R3, R6
		smull	R4, R7, R4, R0
		lsr	R4, R4, 16
		orr	R4, R4, R7, lsl 16
		sub	R2, R2, #1
		b	Q16outer
Q16done:	mov	R0, R3
		pop	{R4 - R7, PC}
