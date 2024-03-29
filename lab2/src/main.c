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
#include <stdarg.h>
#include "library.h"
#include "graphics.h"

extern unsigned Ten32(void) ;
extern unsigned long long Ten64(void) ;
extern unsigned Incr(unsigned n) ;
extern unsigned Nested1(void) ;
extern unsigned Nested2(void) ;
extern void PrintTwo(char *format, unsigned number) ;

#define	MS32BITS(a)	((unsigned *) &a)[1]
#define	LS32BITS(a)	((unsigned *) &a)[0]
#define	ROW_HEIGHT	14
#define	FIRST_ROW	70
#define	LABEL_COL	20
#define	DWORD_COL	(LABEL_COL + 74)
#define	WORD_COL	(DWORD_COL + 56)

static void	PaintBackground(int row, int rows) ;
static int	PutStringAt(int row, int col, char *fmt, ...) ;
static int	PutValueAt(int row, int col, int ok, char *fmt, ...) ;

int main(void)
{
    unsigned seed, a32, b32, row ;
    uint64_t c64 ;

    InitializeHardware(HEADER, "Lab 2: Functions & Parameters") ;

    row = FIRST_ROW ;

    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    row = PutStringAt(row, LABEL_COL, "Functions returning constant:") ;
    PaintBackground(row, 2) ;
    row += 4 ;

    a32 = Ten32() ;
    PutStringAt(row, LABEL_COL, "Ten32() =") ;
    row = PutValueAt(row, WORD_COL, (a32 == 10), "0x%08X", a32) ;

    c64 = Ten64() ;
    PutStringAt(row, LABEL_COL, "Ten64() =") ;
    row = PutValueAt(row, DWORD_COL, (c64 == 10ULL), "0x%08X%08X", MS32BITS(c64), LS32BITS(c64)) ;

    row += ROW_HEIGHT ;

    seed = GetRandomNumber() ;
    srand(seed) ;

    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    row = PutStringAt(row, LABEL_COL, "Function returning N+1:") ;
    PaintBackground(row, 1) ;
    row += 4 ;

    a32 = rand() % 1000000000 ;
    b32 = Incr(a32) ;
    PutStringAt(row, LABEL_COL, "Incr(%9u) =", a32) ;
    row = PutValueAt(row, WORD_COL, (b32 == a32 + 1), "%10u", b32) ;

    row += ROW_HEIGHT ;

    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    row = PutStringAt(row, LABEL_COL, "Functions calling functions:") ;
    PaintBackground(row, 2) ;
    row += 4 ;

    seed = GetRandomNumber() ;
    srand(seed) ;
    a32 = rand() + 1 ;
    srand(seed) ;
    b32 = Nested1() ;
    PutStringAt(row, LABEL_COL, "Nested1() =") ;
    row = PutValueAt(row, WORD_COL, (b32 == a32), "%010u", b32) ;

    seed = GetRandomNumber() ;
    srand(seed) ;
    a32 = rand() + rand() ;
    srand(seed) ;
    b32 = Nested2() ;
    PutStringAt(row, LABEL_COL, "Nested2() =") ;
    row = PutValueAt(row, WORD_COL, (b32 == a32), "%010u", b32) ;
    row += ROW_HEIGHT ;

    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    row = PutStringAt(row, LABEL_COL, "Function PrintTwo:") ;
    PaintBackground(row, 2) ;
    a32 = GetRandomNumber() % 10000000 ;
    PrintTwo("number = %u", a32) ;

    return 0 ;
}

static void PaintBackground(int row, int rows)
{
    SetForeground(COLOR_LIGHTYELLOW) ;
    FillRect(LABEL_COL - 2, row, 240 - 2*LABEL_COL + 3, rows*ROW_HEIGHT + 2) ;
    SetForeground(COLOR_RED) ;
    DrawRect(LABEL_COL - 3, row - 1, 240 - 2*LABEL_COL + 5, rows*ROW_HEIGHT + 4) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_LIGHTYELLOW) ;
}

static int PutStringAt(int row, int col, char *fmt, ...)
{
    va_list args ;
    char text[100] ;

    va_start(args, fmt) ;
    vsprintf(text, fmt, args) ;
    va_end(args) ;

    DisplayStringAt(col, row, text) ;

    return row + ROW_HEIGHT ;
}

static int PutValueAt(int row, int col, int ok, char *fmt, ...)
{
    va_list args ;
    char text[100] ;

    va_start(args, fmt) ;
    vsprintf(text, fmt, args) ;
    va_end(args) ;

    if (!ok)
    {
        SetForeground(COLOR_WHITE) ;
        SetBackground(COLOR_RED) ;
    }

    DisplayStringAt(col, row, text) ;

    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_LIGHTYELLOW) ;

    return row + ROW_HEIGHT ;
}

int printf(const char *fmt, ...)
{
    static unsigned row = 255 ;
    static int called = 0 ;
    static unsigned arg1 ;
    unsigned arg2 ;
    va_list args ;
    char text[100] ;

    va_start(args, fmt) ;
    vsprintf(text, fmt, args) ;
    if (called) arg2 = va_arg(args, unsigned) ;
    else arg1 = va_arg(args, unsigned) ;
    va_end(args) ;

    if (called && arg2 != arg1 + 1)
    {
        SetForeground(COLOR_WHITE) ;
        SetBackground(COLOR_RED) ;
    }
    DisplayStringAt(LABEL_COL, row, text) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_LIGHTYELLOW) ;

    DisplayStringAt(WORD_COL, row, (called ? "sequential" : "Must be")) ;

    row += ROW_HEIGHT ;
    called = 1 ;
    return 0 ;
}
