/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"
#include "graphics.h"

#define	LINE_HEIGHT	12		// Number of pixels per line of text
#define	XPOS_PARAMS	13		// Horizontal pixel position of test case values
#define	XPOS_ANSWER	92		// Horizontal pixel position of your answer
#define XPOS_RESULT 122		// Horizontal pixel position of your execution time or error message

#define	TESTS		5		// Number of test cases (do not change)

// Functions to be implemented in ARM assembly
extern uint32_t MyProduct(uint32_t a, uint32_t b) ;
extern uint32_t MyFibonacci(uint32_t n) ;
extern uint32_t MyAckermann(uint32_t m, uint32_t n) ;

typedef struct
{
	uint32_t	params[2] ;
	uint32_t	result ;
	uint32_t	cycles ;
} DATA ;

// Functions implemented in C in this file
static uint32_t Ackermann(uint32_t m, uint32_t n) ;
static void		ChooseParams(DATA data[], uint32_t numb, uint32_t mod) ;
static int		Compare(const void *p1, const void *p2) ;
static uint32_t Fibonacci(uint32_t n) ;
static void		PaintBackground(int ypos) ;
static uint32_t Product(uint32_t a, uint32_t b) ;
static uint32_t Result(uint32_t ypos, int correct, uint32_t nsec) ;
static uint32_t TestAckermann(uint32_t ypos, uint32_t ovhd) ;
static uint32_t TestFibonacci(uint32_t ypos, uint32_t ovhd) ;
static uint32_t TestProduct(uint32_t ypos, uint32_t ovhd) ;
static uint32_t TestTitle(uint32_t ypos, char *title) ;

int main(void)
{
	uint32_t ypos, ovhd, dummy[2] ;

	InitializeHardware(HEADER, "Lab 5b: Recursive Functions") ;

	ovhd = CountCycles(CallReturnOverhead, dummy, dummy, dummy) ;
	ypos = 58 ;	// Vertical pixel position of 1st line of text

	ypos = TestProduct(ypos, ovhd) ;	// If you are having trouble with one of your assembly
	ypos = TestFibonacci(ypos, ovhd) ;	// language functions, use "//" to comment out the
	ypos = TestAckermann(ypos, ovhd) ;	// corresponding line here while you test the others.

	return 0 ;
}

// Function to randomly select two unsigned integer parameters
static void ChooseParams(DATA data[], uint32_t numb, uint32_t mod)
{
	uint32_t k ;

	for (k = 0; k < TESTS; k++)
	{
		uint32_t n1 = 1 ;
		uint32_t n2 = 1 ;

		for (;;)
		{
			uint32_t i ;

			n1 = 1 + GetRandomNumber() % mod ;
			n2 = 1 + GetRandomNumber() % mod ;

			for (i = 0; i < k; i++)
			{
				if (numb == 1 && n1 == data[i].params[0]) break ;
				if (numb == 2 && n1 == data[i].params[0] && n2 == data[i].params[1]) break ;
			}
			if (i == k) break ;
		}

		data[k].params[0] = n1 ;
		data[k].params[1] = n2 ;
	}
}

static void PaintBackground(int ypos)
{
	SetForeground(COLOR_LIGHTYELLOW) ;
	FillRect(XPOS_PARAMS - 1, ypos - 1, 240 - 2*XPOS_PARAMS + 1, 5*LINE_HEIGHT + 1) ;
	SetForeground(COLOR_RED) ;
	DrawRect(XPOS_PARAMS - 2, ypos - 2, 240 - 2*XPOS_PARAMS + 2, 5*LINE_HEIGHT + 2) ;
	SetForeground(COLOR_BLACK) ;
	SetBackground(COLOR_LIGHTYELLOW) ;
}

// Function to test your MyProduct function
static uint32_t TestProduct(uint32_t ypos, uint32_t ovhd)
{
	uint32_t k, results[2] ;
	DATA data[TESTS] ;

	ChooseParams(data, 2, 9) ;
	for (k = 0; k < TESTS; k++)
	{
		data[k].cycles = CountCycles(MyProduct, data[k].params, data[k].params, results) - ovhd ;
		data[k].result = results[0] ;
	}
	qsort(data, TESTS, sizeof(DATA), Compare) ;

	ypos = TestTitle(ypos, "Test Recursive Product ...") + 1 ;
	PaintBackground(ypos) ;
	for (k = 0; k < TESTS; k++)
	{
		char text[40] ;
		int correct ;

		sprintf(text, "Prod(%u,%u) =", (unsigned) data[k].params[0], (unsigned) data[k].params[1]) ;
		DisplayStringAt(XPOS_PARAMS, ypos, text) ;
		sprintf(text, "%3u", (unsigned) data[k].result) ;
		DisplayStringAt(XPOS_ANSWER, ypos, text) ;
		correct = data[k].result == Product(data[k].params[0], data[k].params[1]) ;
		ypos = Result(ypos, correct, data[k].cycles) ;
	}

	return ypos + LINE_HEIGHT/2 ;
}

// Function to test your MyFibonacci function
static uint32_t TestFibonacci(uint32_t ypos, uint32_t ovhd)
{
	uint32_t k, results[2] ;
	DATA data[TESTS] ;

	ChooseParams(data, 1, 9) ;
	for (k = 0; k < TESTS; k++)
	{
		data[k].cycles = CountCycles(Fibonacci, data[k].params, data[k].params, results) - ovhd ;
		data[k].result = results[0] ;
	}
	qsort(data, TESTS, sizeof(DATA), Compare) ;

	ypos = TestTitle(ypos, "Test Recursive Fibonacci ...") + 1 ;
	PaintBackground(ypos) ;
	for (k = 0; k < TESTS; k++)
	{
		char text[40] ;
		int correct ;

		sprintf(text, "Fib(%u) =", (unsigned) data[k].params[0]) ;
		DisplayStringAt(XPOS_PARAMS, ypos, text) ;
		sprintf(text, "%3u", (unsigned) data[k].result) ;
		DisplayStringAt(XPOS_ANSWER, ypos, text) ;
		correct = data[k].result == Fibonacci(data[k].params[0]) ;
		ypos = Result(ypos, correct, data[k].cycles) ;
	}

	return ypos + LINE_HEIGHT/2 ;
}

// Function to test your MyAckermann function
static uint32_t TestAckermann(uint32_t ypos, uint32_t ovhd)
{
	uint32_t k, results[2] ;
	DATA data[TESTS] ;

	ChooseParams(data, 2, 3) ;
	for (k = 0; k < TESTS; k++)
	{
		data[k].cycles = CountCycles(Ackermann, data[k].params, data[k].params, results) - ovhd ;
		data[k].result = results[0] ;
	}
	qsort(data, TESTS, sizeof(DATA), Compare) ;

	ypos = TestTitle(ypos, "Test Recursive Ackermann ...") + 1 ;
	PaintBackground(ypos) ;
	for (k = 0; k < TESTS; k++)
	{
		char text[40] ;
		int correct ;

		sprintf(text, "Ack(%u,%u) =", (unsigned) data[k].params[0], (unsigned) data[k].params[1]) ;
		DisplayStringAt(XPOS_PARAMS, ypos, text) ;
		sprintf(text, "%3u", (unsigned) data[k].result) ;
		DisplayStringAt(XPOS_ANSWER, ypos, text) ;
		correct = data[k].result == Ackermann(data[k].params[0], data[k].params[1]) ;
		ypos = Result(ypos, correct, data[k].cycles) ;
	}

	return ypos + LINE_HEIGHT/2 ;
}

// Function used to verify your MyProduct function
static uint32_t Product(uint32_t a, uint32_t b)
{
	if (a == 0) return 0 ;
	return Product(a - 1, b) + b ;
}

// Function used to verify your MyFibonacci function
static uint32_t Fibonacci(uint32_t n)
{
	if (n <= 1) return n ;
	return Fibonacci(n - 1) + Fibonacci(n - 2) ;
}

// Function used to verify your MyAckermann function
static uint32_t Ackermann(uint32_t m, uint32_t n)
{
	if (m == 0) return n + 1 ;
	if (n == 0) return Ackermann(m - 1, 1) ;
	return Ackermann(m - 1, Ackermann(m, n - 1)) ;
}

// Function to display test case title
static uint32_t TestTitle(uint32_t ypos, char *title)
{
	SetForeground(COLOR_BLACK) ;
	SetBackground(COLOR_WHITE) ;
	DisplayStringAt(XPOS_PARAMS, ypos, title) ;
	return ypos + LINE_HEIGHT ;
}

// Function to display execution time or error message
static uint32_t Result(uint32_t ypos, int correct, uint32_t cycles)
{
	char text[100] ;

	if (correct)
	{
		sprintf(text, "(%6u cycles)", (unsigned) cycles) ;
		DisplayStringAt(XPOS_RESULT, ypos, text) ;
	}
	else
	{
		SetForeground(COLOR_WHITE) ;
		SetBackground(COLOR_RED) ;
		DisplayStringAt(XPOS_RESULT, ypos, " Incorrect ") ;
		SetForeground(COLOR_BLACK) ;
		SetBackground(COLOR_WHITE) ;
	}
	return ypos + LINE_HEIGHT ;
}

static int Compare(const void *p1, const void *p2)
{
	return ((DATA *) p1)->cycles > ((DATA *) p2)->cycles ;
}
