/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "library.h"
#include "graphics.h"

extern void		TwosComplement(const int input[8], int output[8]) ;
extern float	Bin2Dec(const int bin[8]) ;
extern void		Dec2Bin(const float x, int bin[8]) ;

typedef int BOOL ;
#define	FALSE	0
#define	TRUE	1

typedef struct
{
    unsigned	input ;
    unsigned	output ;
    unsigned	correct ;
} DISPLAY_ROWS ;

typedef struct
{
    const uint8_t *table ;
    const uint16_t Width ;
    const uint16_t Height ;
} sFONT;

extern sFONT Font8, Font12, Font16, Font20, Font24 ;

static int8_t	Bin2Int(int *binary) ;
static void		Delay(uint32_t msec) ;
static int8_t	Round(float input) ;
static void		Int2Bin(int8_t integer_in, int *binary) ;
static void		InitializeDisplay(void) ;
static void		PutArrayAt(int pxlrow, int pxlcol, int binary[8]) ;
static void		PutFixedAt(int pxlrow, int pxlcol, int8_t fixed_pt) ;
static void		PutFloatAt(int pxlrow, int pxlcol, float decimal) ;
static void		PutStatusMessage(char *msg) ;
static int		PutStringAt(int pxlrow, int pxlcol, char *format, ...) ;
static void		SetFontSize(sFONT *pFont) ;

static sFONT *font ;

#define	CPU_CLOCK_SPEED_MHZ			168
#define	TWOSCOMP_LABEL_ROW			58
#define	BIN2DEC_LABEL_ROW			(TWOSCOMP_LABEL_ROW + 76)
#define	DEC2BIN_LABEL_ROW			(BIN2DEC_LABEL_ROW + 76)
#define	STATUS_MSG_ROW				(DEC2BIN_LABEL_ROW + 76)

static DISPLAY_ROWS twoscomp_row, bin2dec_row, dec2bin_row ;
static unsigned value_column ;

int main(void)
{
    static int binary_in[8] = {0} ;
    int8_t integer_in, first_int ;
    unsigned new_errors = 0 ;
    unsigned old_errors = 0 ;
    BOOL all_binary = FALSE ;
    BOOL all_decimal = FALSE ;
    float decimal_in ;

    InitializeHardware(HEADER, "Lab 1: Binary Number Systems") ;
    InitializeDisplay() ;

    first_int = (GetRandomNumber() % 256) - 128 ;
    integer_in = first_int ;
    decimal_in = 0.0 ;
    for (;;)
    {
        float decimal_out, floating ;
        int integer, binary_out[8] ;

        // Convert integer_in into bit pattern
        Int2Bin(integer_in, binary_in) ;

        // Two's Complement Test
        SetForeground(COLOR_BLACK) ;
        SetBackground(COLOR_LIGHTYELLOW) ;
        PutArrayAt(twoscomp_row.input, value_column, binary_in) ;
        TwosComplement(binary_in, binary_out) ;
        PutFixedAt(twoscomp_row.correct, value_column, -integer_in) ;
        if (Bin2Int(binary_out) != (int8_t) -integer_in)
        {
            SetForeground(COLOR_WHITE) ;
            SetBackground(COLOR_RED) ;
            new_errors++ ;
        }
        PutArrayAt(twoscomp_row.output, value_column, binary_out) ;

        // Bin2Dec Test
        SetForeground(COLOR_BLACK) ;
        SetBackground(COLOR_LIGHTYELLOW) ;
        PutArrayAt(bin2dec_row.input, value_column, binary_in) ;
        decimal_out = Bin2Dec(binary_in) ;
        floating = integer_in / 128.0 ;
        PutFloatAt(bin2dec_row.correct, value_column, floating) ;
        if (decimal_out != floating)
        {
            SetForeground(COLOR_WHITE) ;
            SetBackground(COLOR_RED) ;
            new_errors++ ;
        }
        PutFloatAt(bin2dec_row.output, value_column, decimal_out) ;

        // Dec2Bin Test
        SetForeground(COLOR_BLACK) ;
        SetBackground(COLOR_LIGHTYELLOW) ;
        PutFloatAt(dec2bin_row.input, value_column, decimal_in) ;
        Dec2Bin(decimal_in, binary_out) ;
        integer = Round(128 * decimal_in) ;
        PutFixedAt(dec2bin_row.correct, value_column, integer) ;
        if (Bin2Int(binary_out) != integer)
        {
            SetForeground(COLOR_WHITE) ;
            SetBackground(COLOR_RED) ;
            new_errors++ ;
        }
        PutArrayAt(dec2bin_row.output, value_column, binary_out) ;

        if (new_errors != old_errors)
        {
            PutStatusMessage("Press blue button") ;
            WaitForPushButton() ;
            old_errors = new_errors ;
        }

        if (all_binary && all_decimal && new_errors == 0)
        {
            PutStatusMessage("No Errors!") ;
        }

        if (++integer_in == first_int) all_binary = TRUE ;
        if ((decimal_in += 0.01) >= 1.0)
        {
            decimal_in = -1.0 ;
            all_decimal = TRUE ;
        }
        Delay(100) ;
    }

    return 0 ;
}

static void Delay(uint32_t msec)
{
    uint32_t cycles = 1000 * msec * CPU_CLOCK_SPEED_MHZ ;
    uint32_t timeout = GetClockCycleCount() + cycles ;
    while ((int) (timeout - GetClockCycleCount()) > 0) ;
}

static void InitializeDisplay(void)
{
    int label_pxlrow, pxlcol, width, height ;

    SetFontSize(&Font16) ;

    width = 20 * font->Width ;
    height = 3 * font->Height + 1 ;

    pxlcol = (240 - width) / 2 ;
    value_column = pxlcol + 10 * font->Width ;

    label_pxlrow = TWOSCOMP_LABEL_ROW + font->Height + 1 ;
    SetForeground(COLOR_LIGHTYELLOW) ;
    FillRect(pxlcol, label_pxlrow, width, height) ;
    SetForeground(COLOR_RED) ;
    DrawRect(pxlcol - 1, label_pxlrow - 1, width + 1, height + 1) ;

    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    twoscomp_row.input = PutStringAt(TWOSCOMP_LABEL_ROW, pxlcol, "TwosComplement:") + 2 ;
    SetBackground(COLOR_LIGHTYELLOW) ;
    twoscomp_row.output = PutStringAt(twoscomp_row.input , pxlcol, "   Input:") ;
    twoscomp_row.correct = PutStringAt(twoscomp_row.output, pxlcol, "  Output:") ;
    PutStringAt(twoscomp_row.correct, pxlcol, "  Answer:") ;

    label_pxlrow = BIN2DEC_LABEL_ROW + font->Height + 1 ;
    SetForeground(COLOR_LIGHTYELLOW) ;
    FillRect(pxlcol, label_pxlrow, width, height) ;
    SetForeground(COLOR_RED) ;
    DrawRect(pxlcol - 1, label_pxlrow - 1, width + 1, height + 1) ;

    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    bin2dec_row.input = PutStringAt(BIN2DEC_LABEL_ROW, pxlcol, "Bin2Dec:") + 2 ;
    SetBackground(COLOR_LIGHTYELLOW) ;
    bin2dec_row.output = PutStringAt(bin2dec_row.input , pxlcol, "   Input:") ;
    bin2dec_row.correct = PutStringAt(bin2dec_row.output, pxlcol, "  Output:") ;
    PutStringAt(bin2dec_row.correct, pxlcol, "  Answer:") ;

    label_pxlrow = DEC2BIN_LABEL_ROW + font->Height + 1 ;
    SetForeground(COLOR_LIGHTYELLOW) ;
    FillRect(pxlcol, label_pxlrow, width, height) ;
    SetForeground(COLOR_RED) ;
    DrawRect(pxlcol - 1, label_pxlrow - 1, width + 1, height + 1) ;

    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    dec2bin_row.input = PutStringAt(DEC2BIN_LABEL_ROW, pxlcol, "Dec2Bin:") + 2 ;
    SetBackground(COLOR_LIGHTYELLOW) ;
    dec2bin_row.output = PutStringAt(dec2bin_row.input, pxlcol, "   Input:") ;
    dec2bin_row.correct = PutStringAt(dec2bin_row.output, pxlcol, "  Output:") ;
    PutStringAt(dec2bin_row.correct, pxlcol, "  Answer:") ;
}

static void SetFontSize(sFONT *pFont)
{
    extern void BSP_LCD_SetFont(sFONT *) ;
    BSP_LCD_SetFont(font = pFont) ;
}

static void PutFloatAt(int pxlrow, int pxlcol, float decimal)
{
    PutStringAt(pxlrow, pxlcol, "%+f", decimal) ;
}

static void PutFixedAt(int pxlrow, int pxlcol, int8_t fixed_pt)
{
    int binary[8] ;

    Int2Bin(fixed_pt, binary) ;
    PutArrayAt(pxlrow, pxlcol, binary) ;
}

static void PutArrayAt(int pxlrow, int pxlcol, int binary[8])
{
    int bit ;

    for (bit = 7; bit >= 0; bit--)
    {
        DisplayChar(pxlcol, pxlrow, '0' + binary[bit]) ;
        pxlcol += font->Width ;
        if (bit < 7) continue ;
        DisplayChar(pxlcol, pxlrow, '.') ;
        pxlcol += font->Width ;
    }
}

static int PutStringAt(int pxlrow, int pxlcol, char *format, ...)
{
    va_list args ;
    char text[100] ;

    va_start(args, format) ;
    vsprintf(text, format, args) ;
    va_end(args) ;

    DisplayStringAt(pxlcol, pxlrow, text) ;
    return pxlrow + font->Height ;
}

static void PutStatusMessage(char *msg)
{
    unsigned pxlcol = (240 - strlen(msg) * font->Width) / 2 ;
    SetForeground(COLOR_WHITE) ;
    FillRect(0, STATUS_MSG_ROW, 240, font->Height) ;
    SetForeground(COLOR_BLACK) ;
    SetBackground(COLOR_WHITE) ;
    PutStringAt(STATUS_MSG_ROW, pxlcol, msg) ;
}

static int8_t Bin2Int(int *binary)
{
    static int32_t integer ;
    int32_t *p ;
    int bit ;

    p = (int32_t *) (32 * (((unsigned) &integer) - 0x20000000) + 0x22000000) ;
    for (bit = 0; bit < 8; bit++) { *p++ = *binary++ ; Delay(1) ; }
    return (int8_t) integer ;
}

static void Int2Bin(int8_t integer_in, int *binary)
{
    static int32_t integer ;
    int32_t *p ;
    int bit ;

    integer = (int32_t) integer_in ;
    p = (int32_t *) (32 * (((unsigned) &integer) - 0x20000000) + 0x22000000) ;
    for (bit = 0; bit < 8; bit++) { *binary++ = *p++ ; Delay(1) ; }
}

static int8_t Round(float input)
{
    uint32_t output ;

    asm(
    "VCVTR.S32.F32	%[input],%[input]		\n\t"
    "VMOV			%[output],%[input]		\n\t"
    :	[input]			"+w"	(input),
    [output]		"=r"	(output)
    ) ;

    return output ;
}
