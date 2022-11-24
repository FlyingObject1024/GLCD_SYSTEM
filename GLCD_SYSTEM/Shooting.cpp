/*
 * Shooting.cpp
 *
 * Created: 2018/01/18 21:57:01
 *  Author: kodai
 */ 
 #include <avr/io.h>
 #include <stdlib.h>//乱数用
 #include <avr/interrupt.h>
 #include <avr/pgmspace.h>
 //PROGMEMで配置
 //pgm_read_byte(&変数);で呼び出し
 #include "Glcd_function.h"
 #include "defines.h"
 #include "sound.h"
 #include <util/delay.h>


#define		SCREEN_WIDTH	126
#define		SCREEN_HEIGHT	64
#define		BULLET_MAX		25
#define		ENEMY_MAX		12
#define		EXPLOSION_MAX	10

static uint8_t SHOT_MAX = 4;

typedef struct {
	int16_t	x;
	int16_t	y;
	int8_t	vx;
	int8_t	vy;
	uint8_t	isAlive;
} BULLET;

typedef struct {
	int8_t	x;
	int8_t	y;
	int8_t	type;
	uint8_t	state;
	uint8_t	isAlive;
} ENEMY;

typedef struct {
	int8_t	x;
	int8_t	y;
	uint8_t	isAlive;
} SHOT;

typedef struct {
	int8_t	x;
	int8_t	y;
	uint8_t	state;
} EXPLOSION;

// My Ship Status
static uint8_t ShotIntervalTimer = 0;
static int8_t Shield = 0;
static int8_t Mx = 0;
static int8_t My = 0;

// Boss Status
static int8_t Bx = 0;
static int8_t By = 0;
static int8_t Bs = 0;
static int8_t BHp= 6;

// Char Status
static BULLET Bullet[BULLET_MAX];
static ENEMY Enemy[ENEMY_MAX];
static SHOT Laser[10];
static EXPLOSION Explosion[EXPLOSION_MAX];

// Stage Status
static uint8_t PtnIndex = 0;
static uint8_t PtnTimer = 50;

static uint64_t shooting_score = 0;

static const int8_t EnemyPtn[][3] PROGMEM = {
    // StepTime, Y-Axis, Type
    {  8, 10, 2 },
    {  8, 10, 2 },
    {  8, 10, 2 },
    { 60, 10, 2 },
    {  8, 38, 2 },
    {  8, 38, 2 },
    {  8, 38, 2 },
    { 60, 38, 2 },
    { 42,  8, 1 },
    { 42, 16, 1 },
    { 42, 32, 1 },
    { 42, 24, 1 },
    { 20, 10, 2 },
    { 20, 38, 2 },
    { 20, 20, 2 },
    { 20, 28, 2 },
    { 20, 30, 2 },
    { 58, 18, 2 },
    {  8, 10, 2 },
    {  8, 10, 2 },
    {  8, 10, 2 },
    { 30, 10, 2 },
    { 60, 20, 1 },
    {  8, 38, 2 },
    {  8, 38, 2 },
    {  8, 38, 2 },
    { 30, 38, 2 },
    { 60, 20, 1 },
    { 80, 24, 3 },
    { -1,  0, 0 }
};

static const int8_t DIR[256][2] PROGMEM = {
    {  0, 64}, {  2, 64}, {  3, 64}, {  5, 64}, {  6, 64}, {  8, 64}, {  9, 63}, { 11, 63},
    { 12, 63}, { 14, 62}, { 16, 62}, { 17, 62}, { 19, 61}, { 20, 61}, { 22, 60}, { 23, 60},
    { 24, 59}, { 26, 59}, { 27, 58}, { 29, 57}, { 30, 56}, { 32, 56}, { 33, 55}, { 34, 54},
    { 36, 53}, { 37, 52}, { 38, 51}, { 39, 50}, { 41, 49}, { 42, 48}, { 43, 47}, { 44, 46},
    { 45, 45}, { 46, 44}, { 47, 43}, { 48, 42}, { 49, 41}, { 50, 39}, { 51, 38}, { 52, 37},
    { 53, 36}, { 54, 34}, { 55, 33}, { 56, 32}, { 56, 30}, { 57, 29}, { 58, 27}, { 59, 26},
    { 59, 24}, { 60, 23}, { 60, 22}, { 61, 20}, { 61, 19}, { 62, 17}, { 62, 16}, { 62, 14},
    { 63, 12}, { 63, 11}, { 63,  9}, { 64,  8}, { 64,  6}, { 64,  5}, { 64,  3}, { 64,  2},
    { 64,  0}, { 64, -2}, { 64, -3}, { 64, -5}, { 64, -6}, { 64, -8}, { 63, -9}, { 63,-11},
    { 63,-12}, { 62,-14}, { 62,-16}, { 62,-17}, { 61,-19}, { 61,-20}, { 60,-22}, { 60,-23},
    { 59,-24}, { 59,-26}, { 58,-27}, { 57,-29}, { 56,-30}, { 56,-32}, { 55,-33}, { 54,-34},
    { 53,-36}, { 52,-37}, { 51,-38}, { 50,-39}, { 49,-41}, { 48,-42}, { 47,-43}, { 46,-44},
    { 45,-45}, { 44,-46}, { 43,-47}, { 42,-48}, { 41,-49}, { 39,-50}, { 38,-51}, { 37,-52},
    { 36,-53}, { 34,-54}, { 33,-55}, { 32,-56}, { 30,-56}, { 29,-57}, { 27,-58}, { 26,-59},
    { 24,-59}, { 23,-60}, { 22,-60}, { 20,-61}, { 19,-61}, { 17,-62}, { 16,-62}, { 14,-62},
    { 12,-63}, { 11,-63}, {  9,-63}, {  8,-64}, {  6,-64}, { 5, -64}, {  3,-64}, {  2,-64},
    {  0,-64}, { -2,-64}, { -3,-64}, { -5,-64}, { -6,-64}, { -8,-64}, { -9,-63}, {-11,-63},
    {-12,-63}, {-14,-62}, {-16,-62}, {-17,-62}, {-19,-61}, {-20,-61}, {-22,-60}, {-23,-60},
    {-24,-59}, {-26,-59}, {-27,-58}, {-29,-57}, {-30,-56}, {-32,-56}, {-33,-55}, {-34,-54},
    {-36,-53}, {-37,-52}, {-38,-51}, {-39,-50}, {-41,-49}, {-42,-48}, {-43,-47}, {-44,-46},
    {-45,-45}, {-46,-44}, {-47,-43}, {-48,-42}, {-49,-41}, {-50,-39}, {-51,-38}, {-52,-37},
    {-53,-36}, {-54,-34}, {-55,-33}, {-56,-32}, {-56,-30}, {-57,-29}, {-58,-27}, {-59,-26},
    {-59,-24}, {-60,-23}, {-60,-22}, {-61,-20}, {-61,-19}, {-62,-17}, {-62,-16}, {-62,-14},
    {-63,-12}, {-63,-11}, {-63, -9}, {-64, -8}, {-64, -6}, {-64, -5}, {-64, -3}, {-64, -2},
    {-64,  0}, {-64,  2}, {-64,  3}, {-64,  5}, {-64,  6}, {-64,  8}, {-63,  9}, {-63, 11},
    {-63, 12}, {-62, 14}, {-62, 16}, {-62, 17}, {-61, 19}, {-61, 20}, {-60, 22}, {-60, 23},
    {-59, 24}, {-59, 26}, {-58, 27}, {-57, 29}, {-56, 30}, {-56, 32}, {-55, 33}, {-54, 34},
    {-53, 36}, {-52, 37}, {-51, 38}, {-50, 39}, {-49, 41}, {-48, 42}, {-47, 43}, {-46, 44},
    {-45, 45}, {-44, 46}, {-43, 47}, {-42, 48}, {-41, 49}, {-39, 50}, {-38, 51}, {-37, 52},
    {-36, 53}, {-34, 54}, {-33, 55}, {-32, 56}, {-30, 56}, {-29, 57}, {-27, 58}, {-26, 59},
    {-24, 59}, {-23, 60}, {-22, 60}, {-20, 61}, {-19, 61}, {-17, 62}, {-16, 62}, {-14, 62},
    {-12, 63}, {-11, 63}, { -9, 63}, { -8, 64}, { -6, 64}, { -5, 64}, { -3, 64}, { -2, 64}
};

////////////////////////////////////////////////////////////////////////////////
void initLaser(void);
void shotLaser(int8_t x, int8_t y);
void moveLaser(void);
void drawLaser(void);

static void initBullet(void);
static void shotBullet(int8_t x, int8_t y, int8_t vx, int8_t vy);
static void moveBullet(void);
static void drawBullet(void);

void initEnemy(void);
void setEnemy(int8_t x, int8_t y, uint8_t type);
void moveEnemy(void);
void drawEnemy(void);
uint8_t hitEnemy(int8_t x, int8_t y);

void initExplosion(void);
void setExplosion(int8_t x, int8_t y);
void moveExplosion(void);
void drawExplosion(void);

void initBoss(void);
uint8_t moveBoss(void);
uint8_t hitBoss(int8_t x, int8_t y);
void drawBoss(void);

uint8_t shootingOver(void);

////////////////////////////////////////////////////////////////////////////////
void shootingTitle(void)
{
		ShotIntervalTimer = 0;
		Shield = 0;
		Mx = 8;
		My = 16;

		Bx = 0;
		By = 0;
		Bs = 0;
		BHp= 6;

		PtnTimer = 0;
		PtnIndex = 0;

		initEnemy();
		initBullet();
		initLaser();
		initExplosion();
}

void shooting_main(void)
{
	shooting_score = 0;
	shootingTitle();
	while( app_number==SHOOTING ){
		Clear_Screen();
		if( hitEnemy(Mx, My) || hitBoss(Mx, My) || Shield < 0){
			// Game Over
			setExplosion( Mx-4, My-4 );	
			setExplosion( Mx+4, My-4 );
			setExplosion( Mx-4, My+4 );
			setExplosion( Mx+4, My+4 );
			shootingOver();
		}	
	    // key input
	    else{ 
			if( (button_jadge(LEFT) ) && Mx >   4) Mx-=2;
			if( (button_jadge(RIGHT)) && Mx < 112) Mx+=2;
			if( (button_jadge(UP))    && My >   4) My-=2;
			if( (button_jadge(DOWN))  && My <  43) My+=2;
			if( (button_jadge(A)) && ShotIntervalTimer == 0) {
			    ShotIntervalTimer = 4;
			    shotLaser(Mx+4, My);
			    soundPlay(SOUND_TYPE_SHOT);
			}
			if(button_jadge(START)) start_button();
		}
	    if( ShotIntervalTimer > 0 ) ShotIntervalTimer--;

	    // draw my ship
	    Glcd_graphic( Mx-4, My-4, 0x68,0,PUT);

		// enemy
	    if( PtnTimer > 0 ) PtnTimer--;
	    if( PtnTimer == 0 ){
			int8_t step =  pgm_read_byte(&EnemyPtn[PtnIndex][0]);
	        if( step >= 0 ){
	            PtnTimer = step;
				int8_t ey =  pgm_read_byte(&EnemyPtn[PtnIndex][1]);
				int8_t et =  pgm_read_byte(&EnemyPtn[PtnIndex][2]);
	            if(et > 0){
	                setEnemy( 125, ey, et );
	            }
	            PtnIndex++;
	        }
	        else{
	            if( moveBoss() ) PtnIndex = 0;
	        }
	    }
		
		if(PtnTimer % 20 == 10 && shooting_score >= 60000 && rand() % 4 == 0) setEnemy( 125, rand() % 40, rand() % 3 + 1 );
	
	    moveLaser();
	    moveBullet();
	    moveEnemy();
	    moveExplosion();

	    drawLaser();
	    drawBullet();
	    drawEnemy();
	    drawBoss();
	    drawExplosion();
	    Glcd_str(0,56,"SCORE:");
	    Glcd_font(128-5,56,'0'+shooting_score%10);
	    if(shooting_score>=10)Glcd_font(128-11,56,'0'+shooting_score/10%10);
	    if(shooting_score>=100)Glcd_font(128-17,56,'0'+shooting_score/100%10);
	    if(shooting_score>=1000)Glcd_font(128-23,56,'0'+shooting_score/1000%10);
	    if(shooting_score>=10000)Glcd_font(128-29,56,'0'+shooting_score/10000%10);
	    if(shooting_score>=100000)Glcd_font(128-35,56,'0'+shooting_score/100000%10);
	    if(shooting_score>=1000000)Glcd_font(128-41,56,'0'+shooting_score/1000000%10);
	    if(shooting_score>=10000000)Glcd_font(128-47,56,'0'+shooting_score/10000000%10);
	    if(shooting_score>=99999999)Glcd_str(128-53,56,"99999999");
		Glcd_Print_map();
		if(F_CPU >= 16000000)_delay_ms(15);
	}
}

uint8_t shootingOver(void)
{
	static uint8_t state = 0;
	//Clear_Screen();
	state++;
	if(state == 0xFF) state = 0x40;
	if((state & 0x20) == 0){
		Glcd_str( 28, 16, "GAME OVER");
		Glcd_str( 28, 24," Press B button");
	}

	int8_t step = pgm_read_byte(&EnemyPtn[PtnIndex][0]);
	if( step < 0 ){
		moveBoss();
	}

    moveLaser();
    moveBullet();
    moveEnemy();
    moveExplosion();

    drawLaser();
    drawBullet();
    drawEnemy();
    drawBoss();
    drawExplosion();
	if(button_jadge_one()==B) app_number=MENU;
}

////////////////////////////////////////////////////////////////////////////////
static const uint8_t ATAN_TABLE[33] PROGMEM = {
	 0,  1,  3,  4,  5,  6,  8,  9, 10, 11, 12, 13, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 25, 26, 27, 28, 29, 29, 30, 31, 31, 32
};

uint8_t ATAN(int8_t x, int8_t y)
{
    if(x==0 && y==0) return -1;

    if (x >= 0){
        if (y >= 0){
            if (y >= x){    // 0-45
                uint8_t r = (uint8_t)( 32 * x / y );
                uint8_t d = pgm_read_byte( &ATAN_TABLE[r] );
                return (uint8_t)d;
            }
            else{           // 45-90
                uint8_t r = (uint8_t)( 32 * y / x );
                uint8_t d = pgm_read_byte( &ATAN_TABLE[r] );
                return (uint8_t)64 - d;
            }
        }
        else{
            if (x >= -y){   // 90-135
                uint8_t r = (uint8_t)( -32 * y / x);
                uint8_t d = pgm_read_byte( &ATAN_TABLE[r] );
                return (uint8_t)64 + d;
            }
            else{           // 135-180
                uint8_t r = (uint8_t)( 32 * x / -y );
                uint8_t d = pgm_read_byte( &ATAN_TABLE[r] );
                return (uint8_t)128 - d;
            }
        }
    }
    else{
        if (y < 0){
            if (-y >= -x){  // 180-225
                uint8_t r = (uint8_t)( -32 * x / -y );
                uint8_t d = pgm_read_byte( &ATAN_TABLE[r] );
                return (uint8_t)128 + d;
            }
            else{           // 225-270
                uint8_t r = (uint8_t)( -32 * y / -x );
                uint8_t d = pgm_read_byte( &ATAN_TABLE[r] );
                return (uint8_t)192 - d;
                }
        }
        else{
            if (-x >= y){   // 270-315
                uint8_t r = (uint8_t)( 32 * y / -x );
                uint8_t d = pgm_read_byte( &ATAN_TABLE[r] );
                return (uint8_t)192 + d;
            }
            else{           // 315-360
                uint8_t r = (uint8_t)( -32 * x / y );
                uint8_t d = pgm_read_byte( &ATAN_TABLE[r] );
                return (uint8_t)256 - d;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
static void initBullet(void)
{
    uint8_t i;
    for(i=0; i<BULLET_MAX; i++){
        Bullet[i].isAlive = 0;
    }    
}

static void shotBullet(int8_t x, int8_t y, int8_t vx, int8_t vy)
{
    uint8_t i;
    for(i=0; i<BULLET_MAX; i++){
        if(Bullet[i].isAlive == 0){
            Bullet[i].x = x*8;
            Bullet[i].y = y*8;
            Bullet[i].vx = vx;
            Bullet[i].vy = vy;
            Bullet[i].isAlive = 1;
            break;
        }
    }        
}

static void moveBullet(void)
{
    uint8_t i;
    for(i=0; i<BULLET_MAX; i++){
        if(Bullet[i].isAlive != 0){
            int16_t nnx = Bullet[i].x + Bullet[i].vx;
            int16_t nny = Bullet[i].y + Bullet[i].vy;
            int8_t nx = (int8_t)(nnx / 8);
            int8_t ny = (int8_t)(nny / 8);
            if( nx < 2 || nx >= SCREEN_WIDTH || ny < 2 || ny > SCREEN_HEIGHT ){
                Bullet[i].isAlive = 0;
            }
            if( nx > Mx-3 && nx < Mx+3 && ny > My-3 && ny < My+3){
                Bullet[i].isAlive = 0;
                Shield--;
            }
            Bullet[i].x = nnx;
            Bullet[i].y = nny;
        }
    }
}

static void drawBullet(void)
{
    uint8_t i;
    for(i=0; i<BULLET_MAX; i++){
        if(Bullet[i].isAlive != 0){
			if(Bullet[i].y>4 || Bullet[i].y<128){
				DrawCircle( Bullet[i].x/8, Bullet[i].y/8, 2);
				DrawFillCircle( Bullet[i].x/8, Bullet[i].y/8, 2);
			}
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void initLaser(void)
{
	uint8_t i;
	for(i=0; i<SHOT_MAX; i++){
		Laser[i].isAlive = 0;
	}
}

void shotLaser(int8_t x, int8_t y)
{
	uint8_t i;
	for(i=0; i<SHOT_MAX; i++){
		if(Laser[i].isAlive == 0){
			Laser[i].x = x;
			Laser[i].y = y;
			Laser[i].isAlive = 1;
			break;
		}
	}
}

void moveLaser(void)
{
	uint8_t i;
	for(i=0; i<SHOT_MAX; i++){
		if(Laser[i].isAlive != 0){
			int8_t nx = Laser[i].x + 4;
			int8_t ny = Laser[i].y;
			if( nx < 0 || nx >= SCREEN_WIDTH-8 || ny < 0 || ny > SCREEN_HEIGHT ){
				Laser[i].isAlive = 0;
			}
			if( hitEnemy(nx, ny) || hitBoss(nx, ny) ){
				Laser[i].isAlive = 0;
			}
			Laser[i].x = nx;
		}
	}
}

void drawLaser(void)
{
	uint8_t i;
	for(i=0; i<SHOT_MAX; i++){
		if(Laser[i].isAlive != 0){
			Glcd_graphic(Laser[i].x-4, Laser[i].y-4, 0x69,OR,0);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void enemy1(int8_t i)
{
	Enemy[i].x -= 1;
	Enemy[i].y += (int8_t)pgm_read_byte(&DIR[Enemy[i].state][0]) / 32;
	Enemy[i].state = Enemy[i].state + 8;
	if(Enemy[i].state == 64){
		shotBullet(Enemy[i].x, Enemy[i].y, -24, 0 );
	}
	else if(shooting_score >= 10000 && Enemy[i].state % 32 == 0){
		shotBullet(Enemy[i].x, Enemy[i].y, -24, 0 );
	}
}

void enemy2(int8_t i)
{
	static uint8_t shotTiming = 0;
	
	shotTiming = (shotTiming + 1) % 64;
	if(shotTiming == 0){
		uint8_t d = ATAN( Mx-Enemy[i].x, My-Enemy[i].y );
		int8_t vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 4;
		int8_t vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 4;
		if(d >= 0) shotBullet( Enemy[i].x-2, Enemy[i].y, vx, vy );
	}
	else if(shooting_score >= 30000 && shotTiming % 16 == 0){
		uint8_t d = ATAN( Mx-Enemy[i].x, My-Enemy[i].y );
		int8_t vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 4;
		int8_t vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 4;
		if(d >= 0) shotBullet( Enemy[i].x-2, Enemy[i].y, vx, vy );
	}

	switch( Enemy[i].state ){
		case 0:
			Enemy[i].x -= 2;
			if(Enemy[i].x < 80 ) Enemy[i].state = 1;
			break;
		case 1:
			if( Enemy[i].y < 24){
				Enemy[i].state = 2;
			}
			else{
				Enemy[i].state = 3;
			}
			break;
		case 2:
			Enemy[i].x += 1;
			Enemy[i].y += 1;
			if( Enemy[i].y > 36 ) Enemy[i].state = 4;
			break;
		case 3:
			Enemy[i].x += 1;
			Enemy[i].y -= 1;
			if( Enemy[i].y < 12 ) Enemy[i].state = 4;
			break;
		case 4:
			Enemy[i].x -= 2;
			break;
	}
}

void enemy3(int8_t i)
{
	if( Enemy[i].state == 0 ){
		Enemy[i].x -= 2;
		if( Enemy[i].x < 80 ) Enemy[i].state++;
	}
	else if( Enemy[i].state < 0x30 ){
		if( (Enemy[i].state & 0x07) == 0 ){
			uint8_t d = Enemy[i].state + 0xA8;
			int8_t vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 4;
			int8_t vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 4;
			shotBullet( Enemy[i].x-3, Enemy[i].y, vx, vy );
			shotBullet( Enemy[i].x-3, Enemy[i].y, vx,-vy );
		}
		Enemy[i].state++;
	}
	else{
		Enemy[i].x += 2;
	}
}

void initEnemy(void)
{
	uint8_t i;
    for(i=0; i<ENEMY_MAX; i++){
        Enemy[i].isAlive = 0;
    }
}

void setEnemy(int8_t x, int8_t y, uint8_t type)
{
    uint8_t i;
    for(i=0; i<ENEMY_MAX; i++){
        if(Enemy[i].isAlive == 0){
            Enemy[i].x = x;
            Enemy[i].y = y;
            Enemy[i].type = type;
            Enemy[i].state = 0;
            Enemy[i].isAlive = 1;
            break;
        }
    }
}

void moveEnemy(void)
{
    uint8_t i;
    for(i=0; i<ENEMY_MAX; i++){
        if(Enemy[i].isAlive != 0){
            int8_t nx = Enemy[i].x;
            int8_t ny = Enemy[i].y;
            if( nx < 0 || nx >= SCREEN_WIDTH || ny < 0 || ny > SCREEN_HEIGHT ){
                Enemy[i].isAlive = 0;
            }
            switch(Enemy[i].type){
            case 1: enemy1(i); break;
            case 2: enemy2(i); break;
            case 3: enemy3(i); break;
            }
        }
    }
}

void drawEnemy(void)
{
    uint8_t i;
    for(i=0; i<ENEMY_MAX; i++){
        if(Enemy[i].isAlive != 0){
            Glcd_graphic(Enemy[i].x-4, Enemy[i].y-4, 0x5d + Enemy[i].type - 1,OR,0);
        }
    }
}

uint8_t hitEnemy(int8_t x, int8_t y)
{
    uint8_t i;
    for(i=0; i<ENEMY_MAX; i++){
        if(Enemy[i].isAlive != 0){
            if( Enemy[i].x > x-4 && Enemy[i].x < x+4 && Enemy[i].y > y-4 && Enemy[i].y < y+4 ){
                Enemy[i].isAlive = 0;
                soundPlay(SOUND_TYPE_BOMB);
                setExplosion(Enemy[i].x, Enemy[i].y);
				if(Enemy[i].type==1) shooting_score+=100;
				else if(Enemy[i].type==2) shooting_score+=250;
				else if(Enemy[i].type==3) shooting_score+=500;
				if(shooting_score >= 40000) SHOT_MAX = 5;
                return 1;
            }
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
void initExplosion(void)
{
    uint8_t i;
    for(i=0; i<EXPLOSION_MAX; i++){
        Explosion[i].state = 0;
    }
}

void setExplosion(int8_t x, int8_t y)
{
    uint8_t i;
    for(i=0; i<EXPLOSION_MAX; i++){
        if(Explosion[i].state == 0){
            Explosion[i].x = x;
            Explosion[i].y = y;
            Explosion[i].state = 16;
            break;
        }
    }
}

void moveExplosion(void)
{
    uint8_t i;
    for(i=0; i<EXPLOSION_MAX; i++){
        if(Explosion[i].state > 0){
            Explosion[i].state--;
        }
    }
}

void drawExplosion(void)
{
    uint8_t i;
    for(i=0; i<EXPLOSION_MAX; i++){
		uint8_t state = Explosion[i].state/2;
        if(state > 0){
			Glcd_graphic( Explosion[i].x-4, Explosion[i].y-4, 0x5f+state,OR,0);
			Glcd_graphic( Explosion[i].x-4, Explosion[i].y-4, 0x60+state,OR,0);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void initBoss(void)
{
}

uint8_t moveBoss(void)
{
	static uint8_t shotTiming = 0;
    uint8_t rc = 0;

    shotTiming = (shotTiming + 1) % 64;
    if((((Bs == 2 || Bs == 3) && shooting_score < 10000) || shooting_score >= 10000) ){
        if( ((shotTiming == 0 || shotTiming == 20) && shooting_score < 60000) || (shotTiming % 15 == 0 && shooting_score >= 60000)){
            shotBullet( Bx-8, By-9, -14, 0 );
            shotBullet( Bx-12, By-5, -14, 0 );
            shotBullet( Bx-12, By+5, -14, 0 );
            shotBullet( Bx-8, By+9, -14, 0 );
			if( shooting_score >= 30000 ) shotBullet( Bx-12, By, -14, 0 );
        }

		if( shooting_score >= 50000 &&  shotTiming % 16 == 0){
			uint8_t d = ATAN( Mx - Bx, By );
			int8_t vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 4;
			int8_t vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 4;
			if(d >= 0) shotBullet( Bx-2, By, vx, vy );
		}
    }

    if(Bs < 6) {
        switch( Bs ){
            case 0:
                Bx = 127;
                By = 24;
                if(shooting_score >= 50000) BHp = 24;
                else if(shooting_score >= 30000) BHp = 18;
				else if(shooting_score >= 10000) BHp = 12;
                else BHp = 6;
                Bs = 1;
                break;
            case 1:
                Bx--;
                if(Bx < 96) Bs = 2;
                break;
            case 2:
                By -= 1;
                if(By < 10) Bs = 3;
                break;
            case 3:
                By += 1;
                if(By > 37) Bs = 2;
                break;
            case 4:
                Bx -= 8;
                if(Bx < 16) Bs = 5;
                break;
            case 5:
                Bx += 8;
                if(Bx > 90) Bs = 2;
                break;
        }
        if( BHp == 0 ) Bs = 6;
		if(shooting_score >= 40000 && shotTiming == 63) Bs = 4; 
    }
    else{
        Bs++;
        if(Bs == 6){
            setExplosion( Bx, By );
        }
        else if(Bs == 18){
            setExplosion( Bx-6, By   );
            setExplosion( Bx+6, By   );
            setExplosion( Bx  , By-6 );
            setExplosion( Bx  , By+6 );
        }
        else if(Bs == 30){
            setExplosion( Bx-8, By-8 );
            setExplosion( Bx+8, By-8 );
            setExplosion( Bx-8, By+8 );
            setExplosion( Bx+8, By+8 );
        }
        else if(Bs > 42){
            Bs = 0;
            rc = 1;
			shooting_score+=1000;
        }
    }

    return rc;
}

uint8_t hitBoss(int8_t x, int8_t y)
{
    if(Bs > 1){
        if( x > Bx-8 && x < Bx && y > By-3 && y < By+3 ){
            BHp--;
            soundPlay(SOUND_TYPE_BOMB);
            setExplosion( Bx-5, By );
			shooting_score+=100;
            return 1;
        }
        if( x > Bx-4 && x < Bx+8 && y > By-8 && y < By+8 ){
			soundPlay(SOUND_TYPE_CANCAL);
            return 1;
        }
    }
    return 0; 
}

void drawBoss(void)
{
    if( Bs > 0 ){
		// draw body
		Glcd_graphic(Bx-8, By-8, 0x7c,PUT,0);
		Glcd_graphic(Bx  , By-8, 0x7d,PUT,0);
		Glcd_graphic(Bx-8, By  , 0x7e,PUT,0);
		Glcd_graphic(Bx  , By  , 0x7f,PUT,0);
		
		// draw HP
		if(BHp > 0){
			DrawLine( 127 , 0      , 123 , 0      ,1);
		}
		for (uint8_t i=1;i <= BHp;i++){
			DrawDot ( 123 , 2*i - 1);
			DrawDot ( 127 , 2*i - 1);
			DrawLine( 127 , 2*i    , 123 , 2*i    ,1);
		}

		// draw core
		//Glcd_graphic(Bx-8, By-4, 0x7B - BHp/2,PUT,OR);
    }
}

////////////////////////////////////////////////////////////////////////////////
