.syntax		unified
.cpu		cortex-m4
.text

.global UseLDRB
.thumb_func
.align
//USELDRB: copies 512 bytes, 1 byte at a time
UseLDRB:	.rept 	512
		ldrb	R2, [R1], #1
		strb	R2, [R0], #1
		.endr
		bx	LR

.global UseLDRH
.thumb_func
.align
//UseLDRH: copies 512 bytes in 256 loops using LDRH and STRH
UseLDRH:	.rept 	256
		ldrh	R2, [R1], #2
		strh	R2, [R0], #2
		.endr
		bx	LR

.global UseLDR
.thumb_func
.align
//useLDR: copies 512 bytes in 128 loops using LDR and STR
UseLDR:		.rept	128
		ldr	R2, [R1], #4
		str	R2, [R0], #4
		.endr
		bx	LR
	
.global UseLDRD
.thumb_func
.align
//UseLDRD: copies 512 bytes in 64 loops using LRDR and STRD
UseLDRD:	.rept 	64
		ldrd	R2, R3, [R1], #8
		strd	R2, R3, [R0], #8
		.endr
		bx	LR

.global UseLDMIA
.thumb_func
.align
//useLDMIA: copies 512 bytes in 16 loops using LDMIA and STMIA
UseLDMIA:	push 	{R4-R9}
		
		.rept 	16
		ldmia	R1!, {R2-R9}
		stmia	R0!, {R2-R9}
		.endr
		
		pop 	{R4-R9}
		bx	LR

		



