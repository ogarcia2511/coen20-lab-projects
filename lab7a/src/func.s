.cpu		cortex-m4
.syntax		unified
.text

.global		SDIVby13
.thumb_func
.align

SDIVby13:	movs.n	R1, 13
		sdiv	R0, R0, R1
		bx	LR

.global		UDIVby13
.thumb_func
.align
UDIVby13:	movs.n	R1, 13
		udiv	R0, R0, R1
		bx	LR

.global		MySDIVby13
.thumb_func
.align
// MySDIVBy13: divides by a constant in 6 clock cycles
MySDIVby13:	// dividend in R0
		ldr	R1, =0x4EC4EC4F
		smmul	R1, R1, R0
		asrs.n	R1, R1, 2
		add	R0, R1, R0, lsr 31
		bx	LR
			

.global		MyUDIVby13
.thumb_func
.align
// MyUDIVby13: divides by an unsigned constant in 5 clock cycles
MyUDIVby13:	// dividend in R0
		ldr	R1, =0x4EC4EC4F	// R1 <- (2^33) / 13
		umull	R1, R0, R0, R1
		lsrs.n	R0, R0, 2
		bx	LR
		
