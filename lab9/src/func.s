.cpu 		cortex-m4
.syntax		unified
.text


.global SIMD_USatAdd
.thumb_func
.align

SIMD_USatAdd:	// &bytes in R0
		        // count (32b) in R1
		        // amount (8b) in R2
		        push	{R4 - R11}
            	bfi	    R2, R2, 8, 8
		        bfi	    R2, R2, 16, 16
AddLoop:	    cmp	    R1, 40
		        blt	    AddCleanup
                ldmia	R0, {R3 - R12}
                uqadd8	R3, R3, R2
                uqadd8	R4, R4, R2
                uqadd8	R5, R5, R2
                uqadd8	R6, R6, R2
                uqadd8	R7, R7, R2
                uqadd8	R8, R8, R2
                uqadd8	R9, R9, R2
                uqadd8	R10, R10, R2
                uqadd8	R11, R11, R2
                uqadd8	R12, R12, R2
                stmia	R0!, {R3 - R12}
                subs	R1, R1, 40
                b	    AddLoop
AddCleanup:	    cbz	    R1, AddDone
                ldr	    R3, [R0]
                uqadd8	R3, R3, R2
                str	    R3, [R0], 4
                subs	R1, R1, 4
                b 	    AddCleanup
AddDone:	    bx	    LR
	

.global	SIMD_USatSub
.thumb_func
.align

SIMD_USatSub:   push	{R4 - R11}
		        ldr	    R3, =40
		        bfi	    R2, R2, 8, 8
		        bfi	    R2, R2, 16, 16
SubLoop:	    cmp	    R1, 40
                blt	    SubCleanup
                ldmia	R0, {R3 - R12}
                uqsub8	R3, R3, R2
                uqsub8	R4, R4, R2
                uqsub8	R5, R5, R2
                uqsub8	R6, R6, R2
                uqsub8	R7, R7, R2
                uqsub8	R8, R8, R2
                uqsub8	R9, R9, R2
                uqsub8	R10, R10, R2
                uqsub8	R11, R11, R2
                uqsub8	R12, R12, R2
                stmia	R0!, {R3 - R12}
                subs	R1, R1, 40
                b   	SubLoop
SubCleanup:	    cbz	    R1, SubDone
		        ldr	    R3, [R0]
		        uqsub8	R3, R3, R2
                str	    R3, [R0], 4
                subs	R1, R1, 4
		        b 	    SubCleanup
SubDone:	    pop {R4 - R11}
                bx	    LR
