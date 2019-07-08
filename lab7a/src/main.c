/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdio.h>
#include <stdint.h>
#include "library.h"
#include "graphics.h"

extern int SDIVby13(int dividend) ;
extern int MySDIVby13(int dividend) ;
extern unsigned UDIVby13(unsigned dividend) ;
extern unsigned MyUDIVby13(unsigned dividend) ;

#define	ENTRIES(a)	(sizeof(a)/sizeof(a[0]))
#define	LINE_HEIGHT	15

static void	DisplayResults(int test, char *label, unsigned works[], unsigned cycles[]) ;

static unsigned dividends[] =
	{
	0xFFFFFF80,	// 15
	0xFFFFF800,	// 16
	0xFFFF8000,	// 17
	0xFFF80000,	// 18
	0xFF800000,	// 19
	0xF8000000,	// 20
	0x80000000,	// 21
	0x08000000,	// 20
	0x00800000,	// 19
	0x00080000,	// 18
	0x00008000,	// 17
	0x00000800	// 16
	} ;

int main(void)
    {
	static unsigned avgs[] = {0, 0, 0, 0} ;
	unsigned ovhd, results[2], *dvnd, works[4] ;
	int which, div ;

	InitializeHardware(HEADER, "Lab 7a: Division by a Constant") ;
	ovhd = CountCycles(CallReturnOverhead, results, results, results) ;

	DisplayStringAt(5, 60, "Dividend  SDIV MySDIV UDIV MyUDIV") ;

	SetForeground(COLOR_LIGHTYELLOW) ;
	FillRect(5, 74, 230, ENTRIES(dividends)*LINE_HEIGHT + 1) ;
	SetForeground(COLOR_RED) ;
	DrawRect(4, 73, 231, ENTRIES(dividends)*LINE_HEIGHT + 2) ;

	dvnd = dividends ;
	for (which = 0; which < ENTRIES(dividends); which++, dvnd++)
		{
		unsigned qUDIV, qMyUDIV, diff, cycles[4] ;
		int qSDIV, qMySDIV ;
		char label[10] ;

		diff = CountCycles(SDIVby13, dvnd, dvnd, results) ;
		qSDIV = results[0] ;
		works[0] = (qSDIV == ((int) *dvnd / 13)) ;
		cycles[0] = diff - ovhd ;
		avgs[0] += cycles[0] ;

		diff = CountCycles(MySDIVby13, dvnd, dvnd, results) ;
		qMySDIV = results[0] ;
		works[1] = (qMySDIV == ((int) *dvnd / 13)) ;
		cycles[1] = diff - ovhd ;
		avgs[1] += cycles[1] ;

		diff = CountCycles(UDIVby13, dvnd, dvnd, results) ;
		qUDIV = results[0] ;
		works[2] = (qUDIV == ((unsigned) *dvnd / 13)) ;
		cycles[2] = diff - ovhd ;
		avgs[2] += cycles[2] ;

		diff = CountCycles(MyUDIVby13, dvnd, dvnd, results) ;
		qMyUDIV = results[0] ;
		works[3] = (qMyUDIV == ((unsigned) *dvnd / 13)) ;
		cycles[3] = diff - ovhd ;
		avgs[3] += cycles[3] ;

		sprintf(label, "%08X", (unsigned) *dvnd) ;
		SetForeground(COLOR_BLACK) ;
		SetBackground(COLOR_LIGHTYELLOW) ;
		DisplayResults(which, label, works, cycles) ;
		}

	for (div = 0; div < 4; div++)
		{
		works[div] = 1 ;
		avgs[div] = (avgs[div] + which/2) / which ;
		}

	SetForeground(COLOR_BLACK) ;
	SetBackground(COLOR_WHITE) ;
	DisplayResults(which + 1, "Average", works, avgs) ;

	return 0 ;
    }

static void DisplayResults(int test, char *label, unsigned works[], unsigned cycles[])
	{
	int x, y, div ;
	char text[10] ;

	x = 5 ;
	y = LINE_HEIGHT*test + 78 ;

	DisplayStringAt(x, y, label) ;

	x = 78 ;
	for (div = 0; div < 4; div++)
		{
		sprintf(text, "%3d", cycles[div]) ;
		if (!works[div])
			{
			SetForeground(COLOR_WHITE) ;
			SetBackground(COLOR_RED) ;
			}
		DisplayStringAt(x, y, text) ;
		x += 40 ;
		}
	}
