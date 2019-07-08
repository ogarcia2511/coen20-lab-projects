.syntax		unified
.cpu		cortex-m4
.text

.global		PixelAddress
.thumb_func
.align
//PixelAddress: uses 'x + 240y' t get the address of a single pixel
PixelAddress:	ldr	R2, =0xD0000000
		ldr	R3, =240
		mla	R3, R1, R3, R0
		ldr	R0, =4
		mla	R0, R3, R0, R2
		bx	LR

.global 	BitmapAddress
.thumb_func
.align
//BitmapAddress: uses the function for k and given parameters to find the address of a char bitmap
BitmapAddress:	push	{R4-R6}
		sub	R0, R0, #32	// R0 <- R0 - 32
		add	R4, R3, #7	// R4 <- (fontwidth + 7)
		asr	R5, R4, 3	// R6 <- (fontwidth + 7) / 8
		mul 	R6, R5, R2
		mla	R0, R6, R0, R1
		pop	{R4-R6}
		bx	LR

.global 	GetBitmapRow
.thumb_func
.align
//GetBitmapRow: reverses bits of address stored in R0 and returns
GetBitmapRow:	ldr	R0, [R0]
		rev	R0, R0
		bx 	LR
