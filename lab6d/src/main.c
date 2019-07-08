/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

// Adapted from code written by Jacek Wieczorek

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "library.h"
#include "graphics.h"
#include "touch.h"

// Functions to be implemented in assembly
extern uint32_t	GetNibble(void *nibbles, uint32_t which) ;
extern void		PutNibble(void *nibbles, uint32_t which, uint32_t value) ;

#define	CPU_CLOCK_HZ		168E6			// Clock cycle count speed
#define	ZERO_RATE_SAMPLES	300				// Number of gyro samples in running average
#define	BALL_CYCLES			3000000			// how fast to move the ball (larger is slower)
#define	SENSITIVITY			0.07			// Converts integer samples to degrees/sec
#define TILT_DEGREES		10.0			// Minimum tilt to cause ball to roll

#define	DIR_UP				(1 << 0)
#define	DIR_DOWN			(1 << 1)
#define	DIR_LEFT			(1 << 2)
#define	DIR_RIGHT			(1 << 3)
#define	DIR_ALL4			(DIR_UP | DIR_DOWN | DIR_LEFT | DIR_RIGHT)
#define	MAZE_WALL			(1 << 0)

#define	WIN_XMIN			0	// Pixel coordinate of left edge of white display area
#define	WIN_YMIN			48	// Pixel coordinate of top edge of white display area

#define	WIN_XMAX			239	// Pixel coordinate of right edge of white display area
#define	WIN_YMAX			303	// Pixel coordinate of bottom edge of white display area

#define	WIN_WIDTH			(WIN_XMAX - WIN_XMIN + 1)
#define	WIN_HEIGHT			(WIN_YMAX - WIN_YMIN + 1)

#define CELL_COLS			45	// Number of physical cell columns
#define CELL_ROWS			45	// Number of physical cell rows

#define	CELL_PXLS			5	// Number of vertical or horizontal pixels per physical cell
#define	CELL_INCR			4	// Number of physical cells per logical col or row

#define	GOAL_PIXELS			((CELL_INCR - 1)*CELL_PXLS)
#define BALL_RADIUS			(GOAL_PIXELS/2)

#define	MAZE_WIDTH			(CELL_COLS * CELL_PXLS)
#define	MAZE_HEIGHT			(CELL_ROWS * CELL_PXLS)

#define	MAZE_XMIN			(WIN_XMIN + (WIN_WIDTH  - MAZE_WIDTH )/2)
#define	MAZE_YMIN			(WIN_YMIN + (WIN_HEIGHT - MAZE_HEIGHT)/2 - 8)

#define	MAZE_XMAX			(MAZE_XMIN + MAZE_WIDTH  - 1)
#define	MAZE_YMAX			(MAZE_YMIN + MAZE_HEIGHT - 1)

#define	BALL_XOFF			(MAZE_XMIN + (CELL_INCR/2)*CELL_PXLS + CELL_PXLS/2)
#define	BALL_YOFF			(MAZE_YMIN + (CELL_INCR/2)*CELL_PXLS + CELL_PXLS/2)

typedef uint32_t			ROWCOL ;
typedef uint8_t				FLAGS ;
typedef	int					BOOL ;
#define	FALSE				0
#define	TRUE				1

#define	ROW(rowcol)			((uint16_t *) &rowcol)[0]
#define	COL(rowcol)			((uint16_t *) &rowcol)[1]

ROWCOL parent[CELL_ROWS][CELL_COLS] ;
FLAGS  flags[(CELL_ROWS*CELL_COLS+1)/2] ;
#define	WHICH(row,col)		((col) + CELL_COLS*(row))

// Library function prototypes for Gyro sensor (not included in library.h)
extern void GYRO_IO_Init(void) ;
extern void GYRO_IO_Write(uint8_t* data, uint8_t port, uint16_t bytes) ;
extern void GYRO_IO_Read(uint8_t* data, uint8_t port, uint16_t bytes) ;

const int GYRO_CTRL_REG1 = 0x20 ;
#define	GYRO_DR1_FLAG	(1 << 7)
#define	GYRO_DR0_FLAG	(1 << 6)
#define	GYRO_BW1_FLAG	(1 << 5)
#define	GYRO_BW0_FLAG	(1 << 4)
#define	GYRO_PD_FLAG	(1 << 3)
#define	GYRO_ZEN_FLAG	(1 << 2)
#define	GYRO_YEN_FLAG	(1 << 1)
#define	GYRO_XEN_FLAG	(1 << 0)

const int GYRO_CTRL_REG4 = 0x23 ;
#define	GYRO_BDU_FLAG	(1 << 7)
#define	GYRO_FS1_FLAG	(1 << 5)
#define	GYRO_FS0_FLAG	(1 << 4)

const int GYRO_CTRL_REG5 = 0x24 ;
#define	GYRO_OSEL1_FLAG	(1 << 1)
#define	GYRO_HPEN_FLAG	(1 << 4)

const int GYRO_STAT_REG	= 0x27 ;
#define	GYRO_ZYXDA_FLAG	(1 << 3)

const int GYRO_DATA_REG	= 0x28 ;

static int		Blocked(int x, int y, int dx, int dy) ;
static ROWCOL	CreatePath(ROWCOL rowcol) ;
static void		DrawMaze(void) ;
static float	FloatAbsVal(float x) ;
static void		GetVelocity(float velocity[]) ;
static void		InitializeGyroscope(void) ;
static void		InitializeMaze(void) ;
static void		MoveBall(float roll, float pitch) ;
static int		PlayGame(void) ;
static int		SanityChecksOK(void) ;

int main(void)
	{
	InitializeHardware(HEADER, "Lab 6d: Mazes & Gyroscopes") ;
	InitializeGyroscope() ;
	SanityChecksOK() ;
	InitializeMaze() ;
	DrawMaze() ;
	PlayGame() ;
	}

// Setup crucial cells
static void InitializeMaze(void)
	{
	ROWCOL frst, last ;
	int row, col ;

	memset(parent, 0, sizeof(parent)) ;
	memset(flags,  0, sizeof(flags)) ;

	for (row = 0; row < CELL_ROWS; row += CELL_INCR)
		{
		for (col = 0; col < CELL_COLS; col++)
			{
			PutNibble(flags, WHICH(row, col), MAZE_WALL) ;
			}
		}

	for (col = 0; col < CELL_COLS; col += CELL_INCR)
		{
		for (row = 0; row < CELL_ROWS; row++)
			{
			PutNibble(flags, WHICH(row, col), MAZE_WALL) ;
			}
		}

	for (row = 2; row < CELL_ROWS; row += CELL_INCR)
		{
		for (col = 2; col < CELL_COLS; col += CELL_INCR)
			{
			PutNibble(flags, WHICH(row, col), DIR_ALL4) ;
			}
		}

	// Setup starting cell
	ROW(frst) = COL(frst) = CELL_INCR/2 ;
	parent[CELL_INCR/2][CELL_INCR/2] = frst ;
	
	// Connect cells until start cell is reached and can't be left
	last = frst ;
	do
		{
		last = CreatePath(last) ;
		} while (last != frst) ;
	}

// Connects cell to random neighbor (if possible) and returns
// address of next cell that should be visited
static ROWCOL CreatePath(ROWCOL rowcol)
	{
	int which = WHICH(ROW(rowcol), COL(rowcol)) ;
	uint32_t nibble ;

	// While there are directions still unexplored
	while (((nibble = GetNibble(flags, which)) & DIR_ALL4) != 0)
		{
		int next_row, next_col, dir ;

		// Randomly pick one direction
		dir = 1 << (GetRandomNumber() % 4) ;
		
		// If it has already been explored - try again
		if ((nibble & dir) == 0) continue ;
		
		// Mark direction as explored
		PutNibble(flags, which, nibble & ~dir) ;
		
		// Depending on chosen direction
		next_row = ROW(rowcol) ;
		next_col = COL(rowcol) ;
		switch (dir)
			{
			// Check if it's possible to go right
			case DIR_RIGHT:
				if (COL(rowcol) + CELL_INCR < CELL_COLS) next_col = COL(rowcol) + CELL_INCR ;
				else continue ;
				break ;
			
			// Check if it's possible to go down
			case DIR_DOWN:
				if (ROW(rowcol) + CELL_INCR < CELL_ROWS) next_row = ROW(rowcol) + CELL_INCR ;
				else continue ;
				break ;
			
			// Check if it's possible to go left	
			case DIR_LEFT:
				if (COL(rowcol) >= CELL_INCR) next_col = COL(rowcol) - CELL_INCR ;
				else continue ;
				break ;
			
			// Check if it's possible to go up
			case DIR_UP:
				if (ROW(rowcol) >= CELL_INCR) next_row = ROW(rowcol) - CELL_INCR ;
				else continue ;
				break ;
			}
		
		// If destination is a linked cell already - abort
		if (parent[next_row][next_col] != 0) continue ;
			
		// Otherwise, adopt cell
		parent[next_row][next_col] = rowcol ;
			
		// Remove wall between cells
		if (next_row != ROW(rowcol))
			{
			int row, col, k ;

			row = ROW(rowcol) + (next_row - ROW(rowcol)) / 2 ;
			col = COL(rowcol) - (CELL_INCR/2 - 1) ;
			for (k = 0; k < CELL_INCR - 1; k++, col++)
				{
				PutNibble(flags, WHICH(row, col), 0) ;
				}
			}

		if (next_col != COL(rowcol))
			{
			int row, col, k ;

			row = ROW(rowcol) - (CELL_INCR/2 - 1) ;
			col = COL(rowcol) + (next_col - COL(rowcol)) / 2 ;
			for (k = 0; k < CELL_INCR - 1; k++, row++)
				{
				which = col + CELL_COLS*row ;
				PutNibble(flags, WHICH(row, col), 0) ;
				}
			}
			
		// Return address of the child cell
		ROW(rowcol) = next_row ;
		COL(rowcol) = next_col ;
		return rowcol ;
		}
	
	// If nothing more can be done here - return parent's address
	return parent[ROW(rowcol)][COL(rowcol)] ;
	}

static void DrawMaze(void)
	{
	int row, col, x, y ;

	SetColor(COLOR_BLACK) ;
	y = MAZE_YMIN ;
	for (row = 0; row < CELL_ROWS; row++)
		{
		x = MAZE_XMIN ;
		for (col = 0; col < CELL_COLS; col++)
			{
			if ((GetNibble(flags, WHICH(row, col)) & MAZE_WALL) != 0)
				{
				FillRect(x, y, CELL_PXLS, CELL_PXLS) ;
				}
			x += CELL_PXLS ;
			}
		y += CELL_PXLS ;
		}

	// Mark the start ...
	SetColor(COLOR_RED) ;
	FillCircle(BALL_XOFF, BALL_YOFF, BALL_RADIUS) ;

	// and destination
	SetColor(COLOR_GREEN) ;
	x = MAZE_XMIN + CELL_PXLS*CELL_COLS - CELL_INCR*CELL_PXLS ;
	y = MAZE_YMIN + CELL_PXLS*CELL_ROWS - CELL_INCR*CELL_PXLS ;
	FillRect(x, y, GOAL_PIXELS, GOAL_PIXELS) ;
	}

static int PlayGame(void)
	{
	float degr_roll, degr_ptch, prev_ptch_dps, prev_roll_dps, deltaT, dps[3] ;
	uint32_t ball_timeout, curr_cycles, prev_cycles ;

	prev_ptch_dps = prev_roll_dps = 0.0 ;
	degr_roll = degr_ptch = 0.0 ;
	curr_cycles = GetClockCycleCount() ;
	ball_timeout = curr_cycles + BALL_CYCLES ;
	while (1)
		{
		char text[100] ;
		uint8_t sts ;

		// Check to see if new gyro data is available
		GYRO_IO_Read(&sts, GYRO_STAT_REG, sizeof(sts)) ;
		if ((sts & GYRO_ZYXDA_FLAG) != 0)
			{
			prev_cycles = curr_cycles ;
			curr_cycles = GetClockCycleCount() ;
			deltaT = (curr_cycles - prev_cycles) / CPU_CLOCK_HZ ;
			GetVelocity(dps) ;

			// Perform trapezoidal integration to get position
			degr_ptch += deltaT * (dps[0] + prev_ptch_dps) / 2 ;
			degr_roll += deltaT * (dps[1] + prev_roll_dps) / 2 ;
			prev_ptch_dps = dps[0] ;
			prev_roll_dps = dps[1] ;

			SetColor(COLOR_BLACK) ;
			sprintf(text, "Pitch: %4d", (int) degr_ptch) ;
			DisplayStringAt(MAZE_XMIN, WIN_YMAX - 16, text) ;
			sprintf(text, "Roll: %4d", (int) degr_roll) ;
			DisplayStringAt(MAZE_XMAX - 7*strlen(text), WIN_YMAX - 16, text) ;
			}

		// Reset roll and pitch if user presses blue pushbutton
		if (PushButtonPressed()) degr_ptch = degr_roll = 0.0 ;

		if ((int) (ball_timeout - GetClockCycleCount()) <= 0)
			{
			MoveBall(degr_roll, degr_ptch) ;
			ball_timeout += BALL_CYCLES ;
			}
		}

	return 0 ;
	}

static void MoveBall(float roll, float pitch)
	{
	static int x = 0 ;
	static int y = 0 ;
	int oldx = x ;
	int oldy = y ;

	if (FloatAbsVal(roll) > TILT_DEGREES && (y % CELL_INCR) == 0)
		{
		int newx = x + roll/TILT_DEGREES ;

		while (x < newx)
			{
			if (Blocked(x, y, +CELL_INCR/2, 0)) break ; 
			if (x + 1 > newx) break ;
			x++ ;
			}

		while (x > newx)
			{
			if (Blocked(x, y, -CELL_INCR/2, 0)) break ;
			if (x - 1 < newx) break ;
			x-- ;
			}
		}

	if (FloatAbsVal(pitch) > TILT_DEGREES && (x % CELL_INCR) == 0)
		{
		int newy = y + pitch/TILT_DEGREES ;

		while (y < newy)
			{
			if (Blocked(x, y, 0, +CELL_INCR/2)) break ;
			if (y + 1 > newy) break ;
			y++ ;
			}

		while (y > newy)
			{
			if (Blocked(x, y, 0, -CELL_INCR/2)) break ;
			if (y - 1 < newy) break ;
			y-- ;
			}
		}

	if (x == oldx && y == oldy) return ;

	// Erase old ball position
	SetColor(COLOR_WHITE) ;
	FillCircle(CELL_PXLS*oldx + BALL_XOFF, CELL_PXLS*oldy + BALL_YOFF, BALL_RADIUS) ;

	// Paint ball at new position
	SetColor(COLOR_RED) ;
	FillCircle(CELL_PXLS*x + BALL_XOFF, CELL_PXLS*y + BALL_YOFF, BALL_RADIUS) ;
	}

static int Blocked(int x, int y, int dx, int dy)
	{
	int row = y + CELL_INCR/2 ;
	int col = x + CELL_INCR/2 ;
 	return (GetNibble(flags, WHICH(row+dy, col+dx)) & MAZE_WALL) != 0 ;
	}

static int SanityChecksOK(void)
	{
#	define	WORDS(a)	(sizeof(a)/sizeof(a[0]))
#	define	NIBBLES(a)	(4*WORDS(a))
	uint32_t storage[100], index, word, left , bugs ;

	for (int i = 0; i < WORDS(storage); i++) storage[i] = 0 ;

	bugs = 0 ;

	do index = GetRandomNumber() % NIBBLES(storage) ; while (index < 8) ;
	PutNibble(storage, index, 0xF) ;
	word = index / 8 ;
	left  = index % 8 ;
	if (storage[word] != (0xF << 4*left)) bugs |= 0x1 ;
	storage[word] = 0 ;

	do index = GetRandomNumber() % NIBBLES(storage) ; while (index < 8) ;
	word = index / 8 ;
	left  = index % 8 ;
	storage[word] = 0xF << 4*left ;
	if (GetNibble(storage, index) != 0xF) bugs |= 0x2 ;
	storage[word] = 0 ;

	if (!bugs) return 1 ;

	SetForeground(COLOR_WHITE) ;
	SetBackground(COLOR_RED) ;
	if (bugs & 0x1) DisplayStringAt(5, 50, (uint8_t *) " Bad Function PutNibble\n") ;
	if (bugs & 0x2) DisplayStringAt(5, 70, (uint8_t *) " Bad Function GetNibble\n") ;
	return 0 ;
	}

static void InitializeGyroscope(void)
	{
	uint8_t cmd ;

	GYRO_IO_Init() ;

	// Enable Block Data Update and full scale = 2000 dps
	cmd = GYRO_BDU_FLAG | GYRO_FS1_FLAG ;
	GYRO_IO_Write(&cmd, GYRO_CTRL_REG4, sizeof(cmd)) ;

	// Use output from high-pass filter
	cmd = GYRO_OSEL1_FLAG ;
	GYRO_IO_Write(&cmd, GYRO_CTRL_REG5, sizeof(cmd)) ;

	// Enable X, Y and Z channels; 200 Hz ODR; 50 Hz BW; Normal Mode
	cmd = GYRO_DR0_FLAG|GYRO_BW1_FLAG|GYRO_PD_FLAG|GYRO_XEN_FLAG|GYRO_YEN_FLAG|GYRO_ZEN_FLAG ;
	GYRO_IO_Write(&cmd, GYRO_CTRL_REG1, sizeof(cmd)) ;
	}

static void GetVelocity(float degrees_per_sec[])
	{
	static float min[3] = {INT16_MAX, INT16_MAX, INT16_MAX} ;
	static float max[3] = {INT16_MIN, INT16_MIN, INT16_MIN} ;
	static float total[3] = {0} ;
	static int32_t count = 0 ;
	int16_t samples[3] ;
	float value ;
	int channel ;

	GYRO_IO_Read((uint8_t *) samples, GYRO_DATA_REG, sizeof(samples)) ;

	for (channel = 0; channel < 3; channel++)
		{
		float sample = (float) samples[channel] ;
		if (count < ZERO_RATE_SAMPLES)
			{
			total[channel] += sample ;
			if (sample > max[channel]) max[channel] = sample + 1 ;
			if (sample < min[channel]) min[channel] = sample - 1 ;
			count++ ;
			}

		// subtract zero-rate-level and compute angular velocity
		if (min[channel] <= sample && sample <= max[channel]) value = 0 ;
		else value = sample - total[channel] / count ;
		degrees_per_sec[channel] = SENSITIVITY * value ;
		}
	}

static float FloatAbsVal(float x)
	{
	asm
		(
		"VABS.F32	%[fp_reg],%[fp_reg]"
		: [fp_reg]	"+w"	(x)
		) ;

	return x ;
	}
