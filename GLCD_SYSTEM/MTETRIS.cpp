/*
 * MTETRIS.cpp
 *
 * Created: 2018/07/30 16:25:16
 *  Author: kodai
 */ 
 /******************************************************************************
	AVR ATMega328
	Game 1 (TETRIS)
******************************************************************************//*
#include	<avr/io.h>
#include	<avr/pgmspace.h>

#include	"sound.h"
#include	"graph.h"
#include	"button.h"

#define MAP_SIZE_X	10
#define MAP_SIZE_Y	20

static uint16_t Map[MAP_SIZE_Y];
static uint16_t Score = 0;
static int8_t Mx = 5;
static int8_t My = 2;
static int8_t Mt = 3;
static int8_t Mr = 0;
static int8_t Lv = 0;
static int8_t Lines = 0;
static int8_t GameSpeed = 16;
static int8_t FallTimer = 16;
static int8_t FixTimer = 10;
static int8_t GameState = 0;
static uint8_t Rand = 0;  

static const int8_t BLOCK_DATA[7][4][4][2] PROGMEM = {
{ // O TYPE
	{ {0,0}, {0,1}, {1,0}, {1,1} },
	{ {0,0}, {0,1}, {1,0}, {1,1} },
	{ {0,0}, {0,1}, {1,0}, {1,1} },
	{ {0,0}, {0,1}, {1,0}, {1,1} }
},
{ // T TYPE
	{ {0,0}, {-1,0}, {1,0}, {0,-1} },
	{ {0,0}, {0,-1}, {0,1}, {1,0} },
	{ {0,0}, {-1,0}, {1,0}, {0,1} },
	{ {0,0}, {0,-1}, {0,1}, {-1,0} }
},
{ // L TYPE
	{ {0,0}, {-1,0}, {1,0}, {1,-1} },
	{ {0,0}, {0,-1}, {0,1}, {1,1} },
	{ {0,0}, {-1,0}, {1,0}, {-1,1} },
	{ {0,0}, {0,-1}, {0,1}, {-1,-1} }
},
{ // L' TYPE
	{ {0,0}, {-1,0}, {1,0}, {-1,-1} },
	{ {0,0}, {0,-1}, {0,1}, {1,-1} },
	{ {0,0}, {-1,0}, {1,0}, {1,1} },
	{ {0,0}, {0,-1}, {0,1}, {-1,1} }
},
{ // Z TYPE
	{ {0,0}, {1,0}, {0,-1}, {-1,-1} },
	{ {0,0}, {1,0}, {0,1}, {1,-1} },
	{ {0,0}, {1,0}, {0,-1}, {-1,-1} },
	{ {0,0}, {1,0}, {0,1}, {1,-1} }
},
{ // S TYPE
	{ {0,0}, {-1,0}, {0,-1}, {1,-1} },
	{ {0,0}, {1,0}, {1,1}, {0,-1} },
	{ {0,0}, {-1,0}, {0,-1}, {1,-1} },
	{ {0,0}, {1,0}, {1,1}, {0,-1} }
},
{ // I TYPE
	{ {0,0}, {-2,0}, {-1,0}, {1,0} },
	{ {0,0}, {0,-1}, {0,-2}, {0,1} },
	{ {0,0}, {-2,0}, {-1,0}, {1,0} },
	{ {0,0}, {0,-1}, {0,-2}, {0,1} }
}
};

////////////////////////////////////////////////////////////////////////////////
uint8_t checkSpace(int8_t x, int8_t y, int8_t typ, int8_t r)
{
	uint8_t ret = 0;
	uint16_t bit;
	int8_t i, xx, yy;

	for(i=0; i<4; i++){
		xx = x + pgm_read_byte(&BLOCK_DATA[typ][r][i][0]);
		yy = y + pgm_read_byte(&BLOCK_DATA[typ][r][i][1]);
		bit = 0x0001 << xx;
		if(xx < 0 || yy < 0 || xx >= MAP_SIZE_X || yy >= MAP_SIZE_Y || ((Map[yy]&bit) != 0) ){
			ret = 1;
			break;
		}
	}
	return ret;
}

void fixBlocks(int8_t x, int8_t y, int8_t typ, int8_t r)
{
	uint16_t bit;
	int8_t i, xx, yy;

	for(i=0; i<4; i++){
		xx = x + pgm_read_byte(&BLOCK_DATA[typ][r][i][0]);
		yy = y + pgm_read_byte(&BLOCK_DATA[typ][r][i][1]);
		bit = 0x0001 << xx;
		if(xx < 0 || yy < 0 || xx >= MAP_SIZE_X || yy >= MAP_SIZE_Y){}
		else{
			Map[yy] |= bit;
			fbFillRect(80-yy*4, 4+xx*4, 3, 3, 1);
		}
	}
}

void drawBlocks(int8_t x, int8_t y, int8_t typ, int8_t r, int8_t color)
{
	uint16_t bit;
	int8_t i, xx, yy;

	for(i=0; i<4; i++){
		xx = x + pgm_read_byte(&BLOCK_DATA[typ][r][i][0]);
		yy = y + pgm_read_byte(&BLOCK_DATA[typ][r][i][1]);
		bit = 0x0001 << xx;
		if(xx < 0 || yy < 0 || xx >= MAP_SIZE_X || yy >= MAP_SIZE_Y || ((Map[yy]&bit) != 0) ){
		}
		else {
			fbDrawRect(80-yy*4, 4+xx*4, 3, 3, color);
		}
	}
}

uint8_t checkLines(void)
{
	uint8_t y, count;

	count = 0;
	for(y=0; y<MAP_SIZE_Y; y++){
		if( (Map[y] & 0x03FF) == 0x03FF ) count++;
	}

	return count;
}

uint8_t clearLines(void){
	int8_t	srcLine = MAP_SIZE_Y-1;
	int8_t	dstLine = MAP_SIZE_Y-1;
	int8_t	lineCount = 0;
	uint8_t	x, y;

	while(srcLine >= 0){
		if( (Map[srcLine] & 0x03FF) == 0x03FF ){
			lineCount++;
		}
		else{
			Map[dstLine] = Map[srcLine];
			dstLine--;
		}
		srcLine--;
	}
	while(dstLine>=0){
		Map[dstLine--] = 0;
	}

	for(y=0; y<MAP_SIZE_Y; y++){
		for(x=0; x<MAP_SIZE_X; x++){
			if( (Map[y] & (0x0001 << x)) != 0 ){
				fbFillRect(80-y*4, 4+x*4, 3, 3, 1);
			}
			else{
				fbFillRect(80-y*4, 4+x*4, 3, 3, 0);
			}
		}
	}

	return lineCount;
}

uint8_t blinkLines(void)
{
	static uint8_t i = 0;
	uint8_t y;

	i++;
	if(i<16){
		for(y=0; y<MAP_SIZE_Y; y++){
			if( (Map[y] & 0x03FF) == 0x03FF ){
				fbFillRect( 80-y*4, 4+0, 3, 40, 0 );
			}
		}
	}
	else{
		i = 0;
	}
	
	return (i==0) ? 1 : 0;
}

void printScore()
{
	uint8_t i;
	uint8_t lv;
	uint16_t sc;

	fbDrawStr(88, 0, "SCORE", DRAW_MODE_PUT);

	sc = Score;
	for(i=0; i<5; i++){
		fbDrawChar( 120-i*8, 8, '0' + (sc % 10), DRAW_MODE_PUT );
		sc = sc / 10;
	}	

	fbDrawStr(88, 24, "LEVEL", DRAW_MODE_PUT);

	lv = Lv;
	for(i=0; i<2; i++){
		fbDrawChar( 120-i*8, 32, '0' + (lv % 10), DRAW_MODE_PUT );
		lv = lv / 10;
	}
}

////////////////////////////////////////////////////////////////////////////////
uint8_t tetrisTitle(void)
{
	uint8_t i;

	fbDrawStr( 5*8, 16, "TETRIS", DRAW_MODE_PUT );

	if( btnOnFlag & BTN_OK ){
		for(i=0; i<MAP_SIZE_Y; i++){
			Map[i] = 0x0000;
		}
		Lines = 0;
		Score = 0;
		Lv = 0;
		GameSpeed = 16;
		FallTimer = 16;
		FixTimer = 10;

		fbClear();
		fbDrawRect(1,1,84,46,1);
		printScore();

		return 1;
	}
	return 0;
}

uint8_t tetrisMain(void)
{
	uint8_t nextBlockFlag = false;
	uint8_t gameOverFlag = false;

	// move & fall block
	if(GameState == 0){
		// delete current block
		drawBlocks(Mx, My, Mt, Mr, 0);

		// key input
		int8_t nx = Mx, nr = Mr;
		if((btnOnFlag & BTN_RIGHT) || (btnOnFlag & BTN_OK)) nr = (nr + 1) & 0x03;
		if( btnRepeatL& BTN_DOWN ) nx++;
		if( btnRepeatL& BTN_UP   ) nx--;
		if( btnStatus & BTN_LEFT ) FallTimer = 0;

		if( checkSpace(nx, My, Mt, nr) == 0 ){
			Mx = nx;
			Mr = nr;
		}

		// fall
		if(FallTimer <= 0){
			if( checkSpace(Mx, My+1, Mt, Mr) == 0 ){
				My++;
				FallTimer = GameSpeed;
				FixTimer = 10;
			}
			else{
				FixTimer--;
				if(FixTimer <= 0){
					soundPlay(SOUND_TYPE_FIX);
					fixBlocks(Mx, My, Mt, Mr);
					nextBlockFlag = true;
				}
			}
		}
		else{
			FallTimer--;
		}

		// redraw block at new position
		drawBlocks(Mx, My, Mt, Mr, 1);

		if(nextBlockFlag == 1){
			if( checkLines() != 0 ){
				soundPlay(SOUND_TYPE_CLEAR);
				GameState = 1;
			}
			else{
				GameState = 2;
			}
		}
	}

	// clear lines with animation
	if( GameState == 1 ){
		if( blinkLines() != 0 ){
			uint8_t n = clearLines();
			Lines += n;
			Score += ( 2 << (n-1) );
			if( Lines >= 5 ){
				Lines -= 5;
				if(Lv < 15) Lv++;
			}
			GameSpeed = 16 - Lv;
			GameState = 2;
		}
		printScore();
	}

	// next block or game over
	if(GameState == 2 ){
		Rand = Rand * 5 + 1;
		Mx = 5;
		My = 2;
		Mt = Rand % 7;
		Mr = 0;
		FallTimer = GameSpeed;
		drawBlocks(Mx, My, Mt, Mr, 1);
		if( checkSpace(Mx, My, Mt, Mr) != 0 ){
			gameOverFlag = true;
		}
		else{
			GameState = 0;
		}
	}

	return gameOverFlag;
}

uint8_t tetrisOver(void)
{
	fbFillRect( 5, 16, 9*8+6, 14, 0 );
	fbDrawStr( 8, 16, "GAME OVER", DRAW_MODE_PUT );
	if( btnOnFlag & BTN_OK ){
		fbClear();
		return 1;
	}
	return 0;
}

