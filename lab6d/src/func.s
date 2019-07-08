.syntax		unified
.cpu		cortex-m4
.text

.global 	PutNibble
.thumb_func
.align
PutNibble:	// *nibbles in R0
		// which in R1
		// value in R2
		push	{R4, R5}
		mov	R5, R1
		lsr	R1, R1, #1	// R1 <- R1 / 2
		add	R0, R0, R1	// move R0 to correct *nibbles position
		ldr	R4, [R0]	// get value in current *nibbles position
		and	R3, R5, #1	// and used to test
		cmp 	R3, #1		// test if odd
		ite	EQ		// equal if odd
		bfieq	R4, R2, #4, #4	// if odd, higher bits
		bfine	R4, R2, #0, #4	// if even, lower bits
		str	R4, [R0]	// store result in array position
		pop	{R4, R5}
		bx	LR

		
.global		GetNibble
.thumb_func
.align
GetNibble:	// *nibbles in R0
		// which in R1
		push 	{R4, R5}	
		mov	R5, R1		// which goes into R5
		lsr	R1, R1, #1	// R1 <- R1 / 2
		add	R0, R0, R1	// correct *nibbles position in R0
		ldrb 	R4, [R0]	// value of nibbles in R4
		ldr	R2, =0		// load R2 with 0
		and	R3, R5, #1	// testing if odd
		cmp 	R3, #1		// testing...
		ite	EQ		// equal if odd
		ubfxeq	R2, R4, #4, #4	// get bytes 4-8
		ubfxne	R2, R4, #0, #4	// get bytes 0-4
		mov	R0, R2		// move R2 into R0
		pop	{R4, R5}
		bx	LR
