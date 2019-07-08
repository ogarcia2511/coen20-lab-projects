.syntax	    unified
.cpu 	    cortex-m4
.text

.global Ten32
.thumb_func
.align
//Ten32: returns a 32-bit int with value of "10"
Ten32:      mov     R1, #10  
            mov     R0, R1
            bx      LR
.global Ten64
.thumb_func
.align
//Ten64: returns 64-bit int with value of "10"
Ten64:      mov     R1, #0
            mov     R0, #10
            bx      LR
.global Incr
.thumb_func
.align
//Incr: takes a number as input and returns that number's value + 1
Incr:       add     R0, R0, #1
            bx      LR
.global Nested1
.thumb_func
.align
//Nested1: uses the rand() function to create a random number and increment it by 1
Nested1:    push    {LR}
            bl      rand
            add     R0, R0, #1
            pop     {PC}
.global Nested2
.thumb_func
.align
//Nested2: uses the rand() function to add together two random numbers and return the sum
Nested2:    push    {R4, LR}
            bl      rand
            mov     R4, R0
            bl      rand
  	    mov     R1, R4
            add     R0, R1, R0
            pop     {R4, PC}
.global PrintTwo
.thumb_func
.align
//PrintTwo: uses printf() to print two sequential numbers to the screen
PrintTwo:   push    {R4, R5, LR}
            mov     R4, R0
            mov     R5, R1
            bl      printf //section 3.6, page 48! printf() destorys values held in R0-R3
            mov     R0, R4
            mov     R1, R5
            add     R1, R1, #1
            bl      printf
            pop     {R4, R5, PC}




