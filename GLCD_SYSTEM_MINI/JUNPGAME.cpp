/*
 * JUNPGAME.cpp
 *
 * Created: 2017/12/29 11:29:32
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
 #include <util/delay.h>
 #include "sound.h"

 void jump_main();
 void draw_stage();
 void move_player();
 extern void getch(void);
 #define HEART 0
 #define LOWWALL 1
 #define WALL 2
 #define HIGHWALL 3
 #define BIRD 4
 #define GOLD 5
static uint64_t score=0;
static uint8_t HP=1;


typedef struct {
	volatile uint8_t type;
	volatile int16_t	x;
	volatile int8_t	y;
	volatile uint8_t	rx;
	volatile uint8_t	ry;
	volatile int8_t		vx;
	volatile int8_t		vy;
	volatile uint8_t	isAlive;
} JUMP_OBJECT;
#define JUMP_OBJECT_MAX 10
static JUMP_OBJECT Jump_object[JUMP_OBJECT_MAX];

void initJump_object(void)
{
	uint8_t i;
	for(i=0; i<JUMP_OBJECT_MAX; i++){
		Jump_object[i].isAlive = 0;
	}
}

void setJump_object(int16_t x,uint8_t type)
{
	uint8_t i;
	for(i=0; i<JUMP_OBJECT_MAX; i++){
		if(Jump_object[i].isAlive == 0){
			Jump_object[i].isAlive = 1;
			Jump_object[i].x = x;
			Jump_object[i].rx = 8;
			Jump_object[i].type = type;
			Jump_object[i].vx = (-1) * (score / 2000 + 4);
			Jump_object[i].vy = 0;
			if(type == LOWWALL){
				Jump_object[i].y = 40;
				Jump_object[i].ry = 8;
			}
			else if(type == WALL){
				Jump_object[i].y = 32;
				Jump_object[i].ry = 8;
			}
			else if(type == HIGHWALL){
				Jump_object[i].y = 24;
				Jump_object[i].ry = 8;
			}
			else if(type == BIRD){
				Jump_object[i].y = rand()%40;
				Jump_object[i].ry = 8;
			}
			else if(type == GOLD){
				Jump_object[i].y = rand()%40;
				Jump_object[i].vy = ((-1)+(2*(rand()%2)))*(1+rand()%6);
				Jump_object[i].ry = 8;
				
				Jump_object[i].vx = -6;
			}
			else if(type == HEART){
				Jump_object[i].y = rand()%40;
				Jump_object[i].ry = 8;
			}
			if(Jump_object[i].vx <= -8) Jump_object[i].vx = -8;
			break;
		}
	}
}

void moveJump_object(void)
{
	uint8_t i;
	for(i=0; i<JUMP_OBJECT_MAX; i++){
		if(Jump_object[i].isAlive != 0){
			
			Jump_object[i].x = Jump_object[i].x + Jump_object[i].vx;
			Jump_object[i].y = Jump_object[i].y + Jump_object[i].vy;
			if(Jump_object[i].y < 0){
				Jump_object[i].y *= (-1);
				Jump_object[i].vy *= (-1);
			}
			else if(Jump_object[i].y > 40){
				Jump_object[i].y = 80-Jump_object[i].y;
				Jump_object[i].vy *= (-1);
			}

			if(Jump_object[i].x < (-1)*Jump_object[i].rx) Jump_object[i].isAlive = 0;
		}
	}
}

void drawJump_object(void)
{
	uint8_t i;
	int8_t graphx;
	uint8_t cut;
	for(i=0; i<JUMP_OBJECT_MAX; i++){
		if(Jump_object[i].isAlive != 0){
			graphx = Jump_object[i].x;
			if(graphx < 0){
				graphx = 0;
				cut = (-1)*Jump_object[i].x;
			}
			
			if(Jump_object[i].type == LOWWALL){
				Glcd_graphmap(graphx, Jump_object[i].y   , 0x43 ,OR,0,cut,0);
			}
			else if(Jump_object[i].type == WALL){
				Glcd_graphmap(graphx, Jump_object[i].y   , 0x43 ,OR,0,cut,0);
				Glcd_graphmap(graphx, Jump_object[i].y+8 , 0x43 ,OR,0,cut,0);
			}
			else if(Jump_object[i].type == HIGHWALL){
				Glcd_graphmap(graphx, Jump_object[i].y   , 0x43 ,OR,0,cut,0);
				Glcd_graphmap(graphx, Jump_object[i].y+8 , 0x43 ,OR,0,cut,0);
				Glcd_graphmap(graphx, Jump_object[i].y+16, 0x43 ,OR,0,cut,0);
			}
			else if(Jump_object[i].type == BIRD){
				Glcd_graphmap(graphx, Jump_object[i].y	 , 0x6e ,OR,0,cut,0);
			}
			else if(Jump_object[i].type == GOLD){
				Glcd_graphmap(graphx, Jump_object[i].y	 , 0x6f ,OR,0,cut,0);
			}
			else if(Jump_object[i].type == HEART){
				Glcd_graphmap(graphx, Jump_object[i].y	 , 0x6a ,OR,0,cut,0);
			}
		}
	}
}

uint8_t hitJump_object(int8_t x, int8_t y)
{
	uint8_t i;
	for(i=0; i<JUMP_OBJECT_MAX; i++){
		if(Jump_object[i].isAlive != 0){
			if(
			    (
				   (Jump_object[i].x <= x   && x   < Jump_object[i].x + Jump_object[i].rx)
				   ||(Jump_object[i].x <= x+7 && x+7 < Jump_object[i].x + Jump_object[i].rx)
				   )
				&&(
				   (Jump_object[i].y <= y   && y   < Jump_object[i].y + Jump_object[i].ry)
				 ||(Jump_object[i].y <= y+7 && y+7 < Jump_object[i].y + Jump_object[i].ry)
				)
			){
				soundPlay(SOUND_TYPE_BOMB);
				return 1;
			}

		}
	}
	return 0;
}

void jump_main(){
	score=0;
	HP=1;
	int8_t player_y = 40;
	int8_t player_vy = 0;
	initJump_object();
	while (app_number == JUMP){
		Clear_Screen();
		if(button_jadge(A) && player_y >= 40){
			player_vy = (-11);
			soundPlay(SOUND_TYPE_JUMP);
		}
		if(button_jadge(DOWN)){
			player_vy = 12;
			//soundPlay(SOUND_TYPE_JUMP);
		}
		
		if(button_jadge(START)) start_button();

		player_y += player_vy;
		player_vy += 2;
		if(player_vy > 10) player_vy = 10;

		if(player_y >= 40){
			player_y = 40;
			player_vy = 0;
		}
		if(player_y < 40){
			Glcd_graphic(0,player_y,0x13,OR,0);
		}
		else Glcd_graphic(0,player_y,0x10 + score%3,OR,0);
		if(score < 500){
			if(score%30 == 0) setJump_object(128,rand()%4+1);
		}
		else if(score < 1000){
			if(score%30 == 0 || score%75 == 0) setJump_object(128,rand()%4+1);
		}
		else if(score < 2000){
			if(score%30 == 0 || score%75 == 0) setJump_object(128,rand()%5+1);
		}
		else if(score < 3000){
			if(score%15 == 0) setJump_object(128,rand()%5+1);
		}
		else if(score < 4000){
			if(score%15 == 0){
				if(rand()%6==0){
					setJump_object(128,rand()%3+1);
					setJump_object(136,rand()%3+1);
					setJump_object(144,rand()%3+1);
				}
				else setJump_object(128,rand()%5+1);
			}
		}
		else if(score < 5000){
			if(score%15 == 0){
				if(rand()%3==0){
					setJump_object(128,rand()%3+1);
					setJump_object(136,rand()%3+1);
					setJump_object(144,rand()%3+1);
				}
				else setJump_object(128,rand()%4+2);
			}
		}
		else{
			if(score%15 == 0){
				if(rand()%3==0){
					setJump_object(128,rand()%3+1);
					setJump_object(136,rand()%3+1);
					setJump_object(144,rand()%3+1);
				}
				else setJump_object(128,rand()%4+2);
			}
			if(score%10 == 0 && rand()%6==0) setJump_object(128,LOWWALL);
		}
		moveJump_object();
		drawJump_object();
		draw_stage();
		if(hitJump_object(0,player_y)){
			getch();
		}
		Glcd_Print_map();
		score++;
	}
}

 void draw_stage(){
	DrawLine_line(48,0,127);
	Glcd_str(16,0,"SCORE:");
	Glcd_font(128-5,0,'0'+score%10);
	if(score>=10)Glcd_font(128-11,0,'0'+score/10%10);
	if(score>=100)Glcd_font(128-17,0,'0'+score/100%10);
	if(score>=1000)Glcd_font(128-23,0,'0'+score/1000%10);
	if(score>=10000)Glcd_font(128-29,0,'0'+score/10000%10);
	if(score>=100000)Glcd_font(128-35,0,'0'+score/100000%10);
	if(score>=1000000)Glcd_font(128-41,0,'0'+score/1000000%10);
	if(score>=10000000)Glcd_font(128-47,0,'0'+score/10000000%10);
	if(score>=99999999)Glcd_str(128-53,0,"99999999");
	Glcd_str(0,56,"A:JUMP");
	Glcd_graphic(48,56,0x4f,PUT,0);
	Glcd_str(56,56,":DOWN");
 }