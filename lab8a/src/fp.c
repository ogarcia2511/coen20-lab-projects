/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This latest version of this software is available 
	for download from http://www.engr.scu.edu/~dlewis/book3.

	The following floating-point emulation was inspired by S. K. Raina, "FLIP: A 
	Floating-Point Library for Integer Processors", Laboratoire de l'Informatique du
	Parallélisme, Sept. 12, 2006. Although limited testing has been performed for
	correctness, no claim regarding accuracy or compliance with the IEEE standard 
	are made.
*/

#include <stdio.h>
#include <stdint.h>

// Data Types
typedef uint32_t float32 ;

typedef struct	// IEEE 754 32-bit Single Precision Format
	{
	uint32_t			sig	:23 ;	// (1).XXX...XX
	uint32_t			exp	: 8 ;	// 0-255
	uint32_t			msb	: 1 ;	// 0=>pos, 1=>neg
	} ieee32_t ;

typedef struct	// Adjusted values extracted from representation
	{
	uint32_t			sig ;		// 1.XXX...XX (normal)
	int32_t				exp ;		// -127 to + 128
	uint32_t			msb ;		// 0=>pos, 1=>neg
	} extract_t ;

// Symbolic Constants
const uint32_t 			HIDDEN_BIT	= (1 << 23) ;
const uint32_t			ONE_PT_ZRO	= (1 << 23) ;
const uint32_t			TWO_PT_ZRO	= (2 << 23) ;

const int32_t 			EXP_UND		= -127 ;
const int32_t 			EXP_MIN		= -126 ;
const int32_t 			EXP_MAX		= +127 ;
const int32_t 			EXP_OVR		= +128 ;

// Macros
#define	IEEE(f32)		((ieee32_t *) &f32)
#define	FLOAT32(s)		*((float32 *) &s)
#define	MAGNITUDE(f32)	(*((uint32_t *) &f32) & 0x7FFFFFFF)
#define	IsINF(x)		(MAGNITUDE(x) == 0x7F800000)
#define	IsZRO(x)		(MAGNITUDE(x) == 0x00000000)
#define	IsNEG(x)		(*((int32_t *) &x) < 0)

// Private Global Variables
static float32			pZRO = 0x00000000 ;
static float32			nZRO = 0x80000000 ;
static float32			pINF = 0x7F800000 ;
static float32			nINF = 0xFF800000 ;
static float32			qNaN = 0x7FC00000 ;

// Public Function Prototypes
float32					AddFloats(float32 a, float32 b) ;
float32					DivFloats(float32 a, float32 b) ;
int32_t					FloatToInt32(float32 real) ;
float32					Int32ToFloat(int32_t integer) ;
float32					MulFloats(float32 a, float32 b) ;
float32					SubFloats(float32 a, float32 b) ;

// Private Function Prototypes
static int				Category(float32 f32) ;
static void				Extract(float32 f32, extract_t *p) ;
static float32 			Float32(uint32_t msb, int32_t exp, uint32_t sig) ;
static float32			NearestEven(extract_t *p) ;
static void				Normalize(extract_t *p) ;
static void				ShiftLeft(extract_t *p, uint32_t bits) ;
static void				ShiftRight(extract_t *p, uint32_t bits) ;
static float32 *		SpecCase(float32 *tbl[6][6], float32 op1, float32 op2) ;

/* Public Functions ------------------------------------- */

float32 AddFloats(float32 augend, float32 addend)
	{
	static float32 *tbl[6][6] =
		{
		{&qNaN, &qNaN, &qNaN, &qNaN, &qNaN, &qNaN},
		{&qNaN, &pINF, &qNaN, &pINF, &pINF, &pINF},
		{&qNaN, &qNaN, &nINF, &nINF, &nINF, &nINF},
		{&qNaN, &pINF, &nINF, &pZRO, &pZRO, NULL },
		{&qNaN, &pINF, &nINF, &pZRO, &nZRO, NULL },
		{&qNaN, &pINF, &nINF, NULL,  NULL,  NULL }
		} ;
	extract_t op1, op2, sum ;
	float32 *special ;

	tbl[5][3] = tbl[5][4] = &augend ;
	tbl[3][5] = tbl[4][5] = &addend ;
	special = SpecCase(tbl, augend, addend) ;
	if (special != NULL) return *special ;

	Extract(augend, &op1) ; Extract(addend, &op2) ; 
	op1.sig <<= 3 ;			op2.sig <<= 3 ;

	if (op1.exp < op2.exp) ShiftRight(&op1, op2.exp - op1.exp) ;
	if (op2.exp < op1.exp) ShiftRight(&op2, op1.exp - op2.exp) ;

	if (op1.msb != 0) op1.sig = -op1.sig ;
	if (op2.msb != 0) op2.sig = -op2.sig ;

	sum.sig = op1.sig + op2.sig ; 
	sum.exp = op1.exp ;
	sum.msb = sum.sig >> 31 ;
	if (sum.msb != 0) sum.sig = -sum.sig ;

	Normalize(&sum) ;
	return NearestEven(&sum) ;
	}

float32 DivFloats(float32 dividend, float32 divisor)
	{
	static float32 zro = 0x00000000 ;
	static float32 inf = 0x7F800000 ;
	static float32 *tbl[6][6] =
		{
		{&qNaN, &qNaN, &qNaN, &qNaN, &qNaN, &qNaN},
		{&qNaN, &qNaN, &qNaN, &pINF, &nINF, &inf },
		{&qNaN, &qNaN, &qNaN, &nINF, &pINF, &inf },
		{&qNaN, &pZRO, &nZRO, &qNaN, &qNaN, &zro },
		{&qNaN, &nZRO, &pZRO, &qNaN, &qNaN, &zro },
		{&qNaN, &zro,  &zro,  &inf,  &inf,  NULL }
		} ;
	extract_t op1, op2, quo ;
	float32 *special ;
	uint32_t sign ;
	uint64_t uDvnd64 ;

	sign = IEEE(dividend)->msb ^ IEEE(divisor)->msb ;
	IEEE(zro)->msb = IEEE(inf)->msb = sign ;
	special = SpecCase(tbl, dividend, divisor) ;
	if (special != NULL) return *special ;

	Extract(dividend, &op1) ; Extract(divisor,  &op2) ;
	while (op1.sig < ONE_PT_ZRO) ShiftLeft(&op1, 1) ;
	while (op2.sig < ONE_PT_ZRO) ShiftLeft(&op2, 1) ;

	quo.msb = sign ; quo.exp = op1.exp - op2.exp ;
	uDvnd64 = ((uint64_t) op1.sig) << (23 + 3) ;
	quo.sig = (uint32_t) (uDvnd64 / (uint64_t) op2.sig) ;
	if ((uint32_t) (uDvnd64 % op2.sig) != 0) quo.sig |= 1 ;

	Normalize(&quo) ;
	return NearestEven(&quo) ;
	}

int32_t FloatToInt32(float32 f32)
	{
	static int32_t special[] =
		{0, INT32_MAX, INT32_MIN, 0, 0} ;
	const float32 int32_max = 0x4F000000 ;
	extract_t s32 ;
	uint32_t type ;

	if ((type = Category(f32)) < 5) return special[type] ;

	Extract(f32, &s32) ;
	if (s32.exp < 0) return 0 ;			// fraction
	if (MAGNITUDE(f32) >= int32_max)	// overflow
		{
		return s32.msb? INT32_MIN : INT32_MAX ;
		}
	if (s32.exp > 23) s32.sig <<= s32.exp - 23 ;
	else 		  	  s32.sig >>= 23 - s32.exp ;

	return (int32_t) (s32.msb ? -s32.sig : +s32.sig) ;
	}

float32 Int32ToFloat(int32_t s32)
	{
	const float32 flpt_zero = 0x00000000 ;
	const float32 int32_min = 0xCF000000 ;
	extract_t f32 ;

	if (s32 == 0)			return flpt_zero ;
	if (s32 == INT32_MIN)	return int32_min ;

	f32.msb = s32 < 0 ;
	f32.exp = 23 + 3 ;		// +3 for rounding bits
	f32.sig = f32.msb ? -s32 : +s32 ;

	while ((int32_t) f32.sig > 0) ShiftLeft(&f32, 1) ;
	ShiftRight(&f32, 8 - 3) ;	// Keep 3 rounding bits
	return NearestEven(&f32) ;
	}

float32 MulFloats(float32 multiplicand, float32 multiplier)
	{
	static float32 zro = 0x00000000 ;
	static float32 inf = 0x7F800000 ;
	static float32 *tbl[6][6] =
		{
		{&qNaN, &qNaN, &qNaN, &qNaN, &qNaN, &qNaN},
		{&qNaN, &pINF, &nINF, &qNaN, &qNaN, &inf },
		{&qNaN, &nINF, &pINF, &qNaN, &qNaN, &inf },
		{&qNaN, &qNaN, &qNaN, &pZRO, &nZRO, &zro },
		{&qNaN, &qNaN, &qNaN, &nZRO, &pZRO, &zro },
		{&qNaN, &inf,  &inf,  &zro,  &zro,  NULL }
		} ;
	extract_t op1, op2, prd ;
	float32 *special ;
	uint32_t sign ;
	uint64_t uProd64 ;

	sign = IEEE(multiplicand)->msb ^ IEEE(multiplier)->msb ;
	IEEE(zro)->msb = IEEE(inf)->msb = sign ;
	special = SpecCase(tbl, multiplicand, multiplier) ;
	if (special != NULL) return *special ;

	Extract(multiplicand, &op1) ; Extract(multiplier, &op2) ;
	while (op1.sig < ONE_PT_ZRO) ShiftLeft(&op1, 1) ;
	while (op2.sig < ONE_PT_ZRO) ShiftLeft(&op2, 1) ;

	prd.msb = sign ; prd.exp = op1.exp + op2.exp ;
	uProd64 = (uint64_t) op1.sig * (uint64_t) op2.sig ;
	prd.sig = (uint32_t) (uProd64 >> (23 - 3)) ;
	if (((uint32_t) uProd64 & 0xFFFFF) != 0) prd.sig |= 1 ;

	Normalize(&prd) ;
	return NearestEven(&prd) ;
	}

float32 SubFloats(float32 minuend, float32 subtrahend)
	{
	IEEE(subtrahend)->msb = ~IEEE(subtrahend)->msb ;
	return AddFloats(minuend, subtrahend) ;
	}

/* Private Functions ------------------------------------- */
static int Category(float32 f32)
	{
	uint8_t u8 = IEEE(f32)->exp ;
	if (u8 == 0 && IEEE(f32)->sig != 0) u8 = 1 ;
	if ((uint8_t) (u8 - 1) < 254) return 5 ;	// Normal
	if (IsZRO(f32)) return 3 + IsNEG(f32) ;		// +/-0
	if (IsINF(f32)) return 1 + IsNEG(f32) ;		// +/-Inf
	return 0 ;									// NaN
	}

static void Extract(float32 f32, extract_t *p)
	{
	p->msb = IEEE(f32)->msb ;
	p->exp = IEEE(f32)->exp - 127 ;
	p->sig = IEEE(f32)->sig ;
	if (p->exp >= EXP_MIN) p->sig |= HIDDEN_BIT ;
	else if (p->sig != 0) p->exp = EXP_MIN ;
	}

static float32 Float32(uint32_t msb, int32_t exp, uint32_t sig)
	{
	ieee32_t s = {sig, exp + 127, msb} ;
	return FLOAT32(s) ;
	}

static float32 NearestEven(extract_t *p)
	{
	if ((p->sig & 0b111) > 0b100) p->sig += 0b100 ;
	else if ((p->sig & 0b1111) == 0b1100) p->sig += 0b100 ;
	if (p->sig >= (TWO_PT_ZRO << 3)) ShiftRight(p, 1) ;
	return Float32(p->msb, p->exp, p->sig >> 3) ;
	}

static void Normalize(extract_t *p)
	{
	if (p->sig >= (TWO_PT_ZRO << 3)) ShiftRight(p, 1) ;
	else while (p->sig <  (ONE_PT_ZRO << 3) && p->exp > EXP_UND) ShiftLeft(p, 1) ;
	if (p->exp < EXP_MIN) ShiftRight(p, EXP_MIN - p->exp), p->exp = EXP_UND ;
	else if (p->exp > EXP_MAX) p->exp = EXP_OVR, p->sig = 0 ;
	}

static void ShiftLeft(extract_t *p, uint32_t bits)
	{
	p->sig = (bits <= 31) ? p->sig << bits : 0 ;
	p->exp -= bits ;
	}

static void ShiftRight(extract_t *p, uint32_t bits)
	{
	if (bits <= 31)
		{
		uint32_t lost = (1 << bits) - 1 ;
		p->sig = (p->sig >> bits) | ((p->sig & lost) != 0) ;
		}
	else p->sig = 0 ;
	p->exp += bits ;
	}

static float32 *SpecCase(float32 *tbl[6][6], float32 op1, float32 op2)
	{
	int row = Category(op1) ;
	int col = Category(op2) ;
	return tbl[row][col] ;
	}
