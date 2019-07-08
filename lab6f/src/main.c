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

#define	LENGTH(a)	(sizeof(a)/sizeof(a[0]))

// Functions to be written in ARM assembly
extern int GetBit(void *message, uint32_t bitnum) ;
extern void DecodeMessage(void *message, char array[]) ;

// Private local functions
char *CreateTree(uint32_t [], uint32_t) ;
void Error(char *msg) ;
void SanityCheck(void) ;

int main(void)
	{
	static uint32_t code[] =
		{
		0x00000472, 0x00080678, 0x00280744, 0x00680745, 0x00180573, 0x00040365, 0x0002046E, 0x000A0461, 
		0x0006052A, 0x00160662, 0x0036066D, 0x000E0579, 0x001E0675, 0x003E0664, 0x00010469, 0x0009046F, 
		0x00050320, 0x00030541, 0x00130574, 0x000B0653, 0x002B0776, 0x006B0871, 0x00EB0877, 0x001B050A, 
		0x0007064C, 0x00270946, 0x01270A2D, 0x03270A4F, 0x00A70927, 0x01A70A6B, 0x03A70B21, 0x07A70B24, 
		0x0067072E, 0x0017076A, 0x00570743, 0x0037066C, 0x000F0668, 0x002F0747, 0x006F0763, 0x001F0667, 
		0x003F0766, 0x007F0848, 0x00FF094D, 0x01FF0A29, 0x03FF0C54, 0x0BFF0D28, 0x1BFF0D2C, 0x07FF0B70
 		} ;
	static uint32_t msg[] = 
		{
		0x6318C6BB, 0xF43FE98C, 0xBA178CB2, 0x64F5F428, 0xBA751A14, 0x318C631A, 0xD35EEF66, 0x521AF90F, 
		0x75B698C5, 0xFBEA4EBB, 0x6F4BAE21, 0xC87E9DF7, 0x23F7C9D7, 0x7711E6B1, 0xD2EB3A3E, 0x77DC7AEB, 
		0x422B8577, 0xE3482A6F, 0x535E9757, 0xFA77D76E, 0xF6AEC86C, 0x07A7BE2F, 0xA87BBC57, 0xEC7E9E7C, 
		0x8E3BFAFF, 0xBBDDCEFF, 0xD6DA6303, 0xF71EBAED, 0x57FF225D, 0x83D2EBF1, 0xFFFE21DA, 0xAD452EEE, 
		0x4BABE43B, 0x06FB33AF, 0xCE2D64EA, 0x3BEEE849, 0x427B9E7D, 0x9DF424F1, 0xA83DCF3E, 0x7D3BE842, 
		0xDFEEF79E, 0xB02AE071, 0x81BE84BF, 0x3EBE433A, 0xDD273DF1, 0xAEEEF15C, 0xCC425FDE, 0xA2E7C6C7, 
		0x5BD70457, 0x63F4F3E5, 0x3BBB5703, 0x47CEEF33, 0xCA8F2967, 0x3D2EB89B, 0xC7715AA8, 0x5C55DD74, 
		0x3F21FFED, 0x5426DB50, 0x0F4BAFC5, 0x31DC56AA, 0x1667775D, 0xFF62BDE7, 0xB4DD690F, 0xDA4BB9F1, 
		0xAD21FFEC, 0xEEEE369B, 0x6FB3C3C5, 0x16B35395, 0x7E9F4EFB, 0x3BC5FBF9, 0xDD76F328, 0xEFF6A90D, 
		0x9FFDCE38, 0xDC29E747, 0x975287F5, 0x77F4855E, 0xC934F667, 0xDEB93EF5, 0xCFA77D99, 0xA6315486, 
		0xBCEEDD6D, 0x00000F4F
		} ;	
	char *tree ;

	InitializeHardware(HEADER, "Lab6f: Huffman Compression") ;
	SanityCheck() ;
	tree = CreateTree(code, LENGTH(code)) ;
	DecodeMessage(msg, tree) ;
	return 0 ;
	}

char *CreateTree(uint32_t code[], uint32_t length)
	{
	typedef struct
		{
		uint32_t	symbol	:8 ;
		uint32_t	length	:8 ;
		uint32_t	bits	:16 ;
		} CODEBOOK ;
	static char tree[16379] = {0} ;
	uint32_t j, k, index, word ;
	CODEBOOK *cb ;

	cb = (CODEBOOK *) code ;
	for (j = 0; j < length; j++, cb++)
		{
		index = 0 ;
		word = cb->bits ;
		for (k = 0; k < cb->length; k++)
			{
			index = 2*index + 1 ;
			if (word & 1) index++ ;
			word >>= 1 ;
			}
		tree[index] = cb->symbol ;
		}

	return tree ;
	}

void Error(char *msg)
	{
	printf("\n%s\n", msg) ;
	exit(255) ;
	}

void SanityCheck(void)
	{
	static uint64_t words ;
	uint64_t bitpos, trial ;

	for (trial = 0; trial < 10; trial++)
		{
		bitpos = GetRandomNumber() % 64 ;

		words = 0LL ;
		words |= 1LL << bitpos ;
		if (GetBit(&words, bitpos) != 1) Error("GetBit: wrong value (0)") ;

		words = ~0LL ;
		words &= ~(1LL << bitpos) ;
		if (GetBit(&words, bitpos) != 0) Error("GetBit: wrong value (1)") ;
		}
	}

