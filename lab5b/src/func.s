.syntax 	unified
.cpu		cortex-m4
.text

.global		MyProduct
.thumb_func
.align
//myProduct: returns recursive sequence for a product of a and b
MyProduct:	push	{LR}
6:		cmp	R0, #0		// label: conditional
		beq	7f
		subs	R0, R0, #1
		bl 	MyProduct
		adds	R0, R0, R1
		pop	{PC}
7:		ldr	R0, =0		// label: if (a == 0)
		pop	{PC}	

.global		MyFibonacci
.thumb_func
.align
//MyFibonacci: returns recursive sequence for Fibonacci through use of local labels
MyFibonacci:	push	{LR}		
1:		cmp	R0, #1		// label: testing conditional, runs if not true
		bls	2f
		subs	R0, R0, #1
		bl	MyFibonacci
		mov	R2, R0
		subs	R0, R0, #1
		mov 	R3, R0
		adds	R0, R2, R3
		b	1b
2:		pop	{PC}		// label: (n <= 1)

			
.global		MyAckermann
.thumb_func
.align
//MyAckermann: returns recursive sequence for Ackermann function with 2 input args
MyAckermann:	push	{LR}
		cbz	R0, 3f
		cbz	R1, 4f
		b	5f
3:		adds	R0, R1, #1	// label: if (m == 0)
		pop	{PC}
		cbz	R1, 4f		// probably removable oops
4:		ldr	R1, =1		// label: if (n == 0)
		subs	R0, R0, #1
		bl	MyAckermann
		pop	{PC}
5:		subs	R2, R0, #1 	// label: main statement ||| m - 1 in R2, m in R0
		subs	R1, R1, #1	// n - 1 in R1
		bl 	MyAckermann
		mov	R1, R0		// result in R1
		mov	R0, R2		// m - 1 in R0
		bl	MyAckermann
		pop	{PC}

		
		

