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
#include "library.h"
#include "graphics.h"

#define	COLOR_HWFP		COLOR_MAGENTA
#define	COLOR_SWFP		COLOR_GREEN
#define	COLOR_FXPT		COLOR_ORANGE

typedef int32_t 		Q16 ;
typedef	uint32_t		float32 ;

#define	ENTRIES(a)		(sizeof(a)/sizeof(a[0]))

#define Q16ONE			0x00010000
#define	FloatToQ16(x)	((Q16) (x * ((float) Q16ONE)))
#define Q16ToFloat(x)	(((float) x) / ((float) Q16ONE))

extern	float			HWFlPtPoly(float   x, float   coef[], int32_t numb) ;
extern	float32			SWFlPtPoly(float32 x, float32 coef[], int32_t numb) ;
extern	Q16				Q16FxdPoly(Q16     x, Q16     coef[], int32_t numb) ;

static	float			HWFlPtSine(float radians, unsigned overhead) ;
static	float			SWFlPtSine(float radians, unsigned overhead) ;
static	float			Q16FxdSine(float radians, unsigned overhead) ;
static	void			PutStringAt(int row, int col, char *fmt, ...) ;
static	void			PutErrMsgAt(int row, int col, char *fmt, ...) ;

#define	ROW1			50
#define	ROWN			(YPIXELS-50)
#define	ROWS			(ROWN - ROW1 + 1)
#define COLS			XPIXELS
#define	HEIGHT			(ROWS/7)
#define DEGREES			540
#define	TXT_YPXLS		15

#define	HWFP_GFXROW		(ROW1 + 1*HEIGHT)
#define	SWFP_GFXROW		(HWFP_GFXROW + 2*HEIGHT + TXT_YPXLS + 10)
#define	FXPT_GFXROW		(SWFP_GFXROW + 2*HEIGHT + TXT_YPXLS + 10)

#define	HWFP_TXTROW		(HWFP_GFXROW + HEIGHT + 2)
#define	SWFP_TXTROW		(SWFP_GFXROW + HEIGHT + 2)
#define	FXPT_TXTROW		(FXPT_GFXROW + HEIGHT + 2)
#define	MSG_TXTCOL		20

int main(void)
	{
	int16_t oldHWFPRow[COLS], oldSWFPRow[COLS], oldFxPtRow[COLS] ;
	unsigned dummy[2], ovhd ;
    int16_t row, col, phase ;

    InitializeHardware(HEADER, "Lab 8a: Arithmetic with Reals") ;

	ovhd = CountCycles(CallReturnOverhead, dummy, dummy, dummy) ;

	for (col = 0; col < COLS; col++)
		{
		oldHWFPRow[col]  = HWFP_GFXROW ;
		oldSWFPRow[col]  = SWFP_GFXROW ;
		oldFxPtRow[col] = FXPT_GFXROW ;
		}

	phase = 0 ;
	while (1)
		{
		int16_t *pOldHWFPRow, *pOldSWFPRow, *oOldFxPtRow ;

		pOldHWFPRow = &oldHWFPRow[0] ;
		pOldSWFPRow = &oldSWFPRow[0] ;
		oOldFxPtRow  = &oldFxPtRow[0] ;

		for (col = 0; col < COLS; col++)
			{
			float radians, HWFPSine, SWFPSine, FxPtSine ;
			int16_t acute, degrees ;

			degrees = (DEGREES * col + COLS/2) / COLS ;
			degrees = (degrees + phase) % 360 ;

			if (degrees >= 270)			acute = 360 - degrees ;
			else if (degrees >= 180)	acute = degrees - 180 ;
			else if (degrees >= 90)		acute = 180 - degrees ;
			else						acute = degrees ;

			radians = (3.14158 * acute) / 180 ;

			HWFPSine = HWFlPtSine(radians, ovhd) ;
			SWFPSine = SWFlPtSine(radians, ovhd) ;
			FxPtSine = Q16FxdSine(radians, ovhd) ;

			if (degrees >= 180)
				{
				HWFPSine = -HWFPSine ;
				SWFPSine = -SWFPSine ;
				FxPtSine = -FxPtSine ;
				}

			row = HWFP_GFXROW + HEIGHT * HWFPSine ;
			SetForeground(((row < *pOldHWFPRow) == (row < HWFP_GFXROW)) ? COLOR_HWFP : COLOR_WHITE) ;
			DrawLine(col, row, col, *pOldHWFPRow) ;
			*pOldHWFPRow++ = row ;

			row = SWFP_GFXROW + HEIGHT * SWFPSine ;
			SetForeground(((row < *pOldSWFPRow) == (row < SWFP_GFXROW)) ? COLOR_SWFP : COLOR_WHITE) ;
			DrawLine(col, row, col, *pOldSWFPRow) ;
			*pOldSWFPRow++ = row ;

			row = FXPT_GFXROW + HEIGHT * FxPtSine ;
			SetForeground(((row < *oOldFxPtRow) == (row < FXPT_GFXROW)) ? COLOR_FXPT : COLOR_WHITE) ;
			DrawLine(col, row, col, *oOldFxPtRow) ;
			*oOldFxPtRow++ = row ;
			}

		if (--phase <= 0) phase = 360 ;
		}

	return 0 ;
	}

static float HWFlPtSine(float radians, unsigned ovhd)
    {
	static unsigned total = 0 ;
	static unsigned count = 0 ;
	static unsigned error = 0 ;
    static float coef[] =
		{
		 0.0,					//  0/0!
		+1.0/(1),				// +1/1!
		 0.0,					//  0/2!
		-1.0/(3*2*1),			// -1/3!
		 0.0,					//  0/4!
		+1.0/(5*4*3*2*1),		// +1/5!
		 0.0,					//  0/6!
		-1.0/(7*6*5*4*3*2*1)	// -1/7!
		} ;
	static unsigned iparams[] = {(unsigned) coef, ENTRIES(coef)} ;
	float sine, results[2] ;

	total += CountCycles(HWFlPtPoly, iparams, &radians, results) ;
	count++ ;

	sine = results[1] ;
	if (sine < 0.0)
		{
		if (!error) PutErrMsgAt(HWFP_TXTROW, MSG_TXTCOL, "H/W FlPt: sin(%4.2f rad) < 0", radians) ;
		error = 1 ;
		sine = 0.0 ;
		}
	else if (sine > 1.0)
		{
		if (!error) PutErrMsgAt(HWFP_TXTROW, MSG_TXTCOL, "H/W FlPt: sin(%4.2f rad) > 1", radians) ;
		error = 1 ;
		sine = 1.0 ;
		}
	else if (!error && radians == 0.0)
		{
		PutStringAt(HWFP_TXTROW, MSG_TXTCOL, "H/W FlPt: %4u clock cycles", (total + count/2)/count - ovhd) ;
		total = count = 0 ;
		}

	return sine ;
    }

static float SWFlPtSine(float radians, unsigned ovhd)
    {
	static unsigned total = 0 ;
	static unsigned count = 0 ;
	static unsigned error = 0 ;
    static float coef[] =
		{
		 0.0,					//  0/0!
		+1.0/(1),				// +1/1!
		 0.0,					//  0/2!
		-1.0/(3*2*1),			// -1/3!
		 0.0,					//  0/4!
		+1.0/(5*4*3*2*1),		// +1/5!
		 0.0,					//  0/6!
		-1.0/(7*6*5*4*3*2*1)	// -1/7!
		} ;
	static unsigned iparams[] = {0, (unsigned) coef, ENTRIES(coef)} ;
	float32 results[2] ;
	float sine ;

	iparams[0] = *((unsigned *) &radians) ;
	total += CountCycles(SWFlPtPoly, iparams, iparams, results) ;
	count++ ;

	sine = *((float *) &results[0]) ;
	if (sine < 0.0)
		{
		if (!error) PutErrMsgAt(SWFP_TXTROW, MSG_TXTCOL, "S/W FlPt: sin(%4.2f rad) < 0", radians) ;
		error = 1 ;
		sine = 0.0 ;
		}
	else if (sine > 1.0)
		{
		if (!error) PutErrMsgAt(SWFP_TXTROW, MSG_TXTCOL, "S/W FlPt: sin(%4.2f rad) > 1", radians) ;
		error = 1 ;
		sine = 1.0 ;
		}
	else if (!error && radians == 0.0)
		{
		PutStringAt(SWFP_TXTROW, MSG_TXTCOL, "S/W FlPt: %4u clock cycles", (total + count/2)/count - ovhd) ;
		total = count = 0 ;
		}

	return sine ;
    }

static float Q16FxdSine(float radians, unsigned ovhd)
    {
	static unsigned total = 0 ;
	static unsigned count = 0 ;
	static unsigned error = 0 ;
	const Q16 one = 0x00010000 ;
    static Q16 coef[] =
		{
		0x00000000,	//  0/0!
		0x00010000,	// +1/1!
		0x00000000,	//  0/2!
		0xFFFFD555,	// -1/3!
		0x00000000,	//  0/4!
		0x00000222,	// +1/5!
		0x00000000,	//  0/6!
		0xFFFFFFF3	// -1/7!
		} ;
	static unsigned iparams[] = {0, (unsigned) coef, ENTRIES(coef)} ;
	Q16 sine, results[2] ;

	((Q16 *) iparams)[0] = FloatToQ16(radians) ;
	total += CountCycles(Q16FxdPoly, iparams, iparams, results) ;
	count++ ;

	sine = results[0] ;
	if (sine < 0)
		{
		if (!error) PutErrMsgAt(FXPT_TXTROW, MSG_TXTCOL, "Q16 FxPt: sin(%4.2f rad) < 0", radians) ;
		error = 1 ;
		sine = 0 ;
		}
	else if (sine > one)
		{
		if (!error) PutErrMsgAt(FXPT_TXTROW, MSG_TXTCOL, "Q16 FxPt: sin(%4.2f rad) > 1", radians) ;
		error = 1 ;
		sine = one ;
		}
	else if (!error && radians == 0)
		{
		PutStringAt(FXPT_TXTROW, MSG_TXTCOL, "Q16 FxPt: %4u clock cycles", (total + count/2)/count - ovhd) ;
		total = count = 0 ;
		}

	return Q16ToFloat(sine) ;
    }

static void PutStringAt(int row, int col, char *fmt, ...)
	{
	va_list args ;
	char text[100] ;

	va_start(args, fmt) ;
	vsprintf(text, fmt, args) ;
	va_end(args) ;

	SetForeground(COLOR_BLACK) ;
	DisplayStringAt(col, row, text) ;
	}

static void PutErrMsgAt(int row, int col, char *fmt, ...)
	{
	va_list args ;
	char text[100] ;

	va_start(args, fmt) ;
	vsprintf(text, fmt, args) ;
	va_end(args) ;

	SetForeground(COLOR_WHITE) ;
	SetBackground(COLOR_RED) ;
	DisplayStringAt(col, row, text) ;
	SetForeground(COLOR_BLACK) ;
	SetBackground(COLOR_WHITE) ;
	}
