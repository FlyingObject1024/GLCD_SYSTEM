#include <avr/io.h>
#include <stdlib.h>//乱数用
#include <avr/interrupt.h>

#include "Glcd_function.h"
#include "defines.h"
#include "sound.h"
#include <util/delay.h>
//「TETRIS」「VSTETRIS」のファイル　改造して分かりにくくなっているのでファイルに同封してあるはずのＣ言語版テトリスを読み解いた方がよい
//マスは16x10
#define BLOCK_HEIGHT 4
#define BLOCK_WIDTH 4
#define FIELD_HEIGHT 18
#define FIELD_WIDTH 21
#define SEARCH_START_Y 14
#define SEARCH_START_X 3
#define EM 0//空
#define BB 1//壁ブロック
#define WB 2//ブロック
#define IB 3//落ちた先ブロック
volatile static uint8_t print_cursorx=0;
volatile static uint8_t print_cursory=0;
volatile static uint8_t first_block[2][BLOCK_HEIGHT][BLOCK_WIDTH];
volatile static uint8_t block[2][BLOCK_HEIGHT][BLOCK_WIDTH];
volatile static uint8_t stage[2][FIELD_HEIGHT][FIELD_WIDTH];
volatile static uint8_t field[2][FIELD_HEIGHT][FIELD_WIDTH];

volatile static int8_t charge = 0;

volatile static uint8_t collision_field[2][FIELD_HEIGHT][FIELD_WIDTH];
volatile static const uint8_t blocks[28][16] = {
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0},
	{0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

	{0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
	{0,1,1,0,0,0,1,0,0,1,0,0,0,1,1,1},
	{0,0,1,0,1,1,1,0,0,1,1,0,0,1,0,0},
	{0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},

	{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
	{0,1,1,0,1,1,1,0,0,0,1,0,0,1,0,0},
	{0,1,0,0,0,0,1,0,0,1,1,0,0,1,1,1},
	{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,1,0,1,1,0,0,0,0,1,0,1,1,0,0},
	{0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0},
	{0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0},

	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,1,0,0,0,0,1,1,0,1,0,0,0,0,1,1},
	{0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0},
	{0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0},

	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0},
	{0,1,1,0,1,1,1,0,1,1,0,0,1,1,1,0},
	{0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0},

	{0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
	{0,1,0,0,1,1,1,1,0,1,0,0,1,1,1,1},
	{0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
	{0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0}
};

volatile static uint8_t fall;
volatile static uint8_t side;
volatile static uint8_t gameover_flag;
volatile static uint8_t block_flag;
volatile static uint8_t block_flag2;
volatile static uint8_t freeze_flag;
volatile static uint8_t ran;
volatile static uint8_t ran2;
volatile uint8_t lsig;
volatile static int turn_point;
volatile static uint32_t score;
volatile static uint16_t lines;
volatile static uint8_t lv;
volatile static uint8_t alart=0;
void getch(void);
void printtm(char font);
void printotm(char font);
void tetris_main();
void my_init_var(void);
void my_init_var2(void);
void my_make_block(void);
void my_gameover(void);
void my_get_key(void);
void my_make_field(void);
void my_make_collision_field(void);
void my_init_field(void);
void my_serch_fall_state();
void my_save_field(void);
void my_freeze_block(void);
void my_clear_freeze(void);
void my_search_line(void);
void my_draw_field(void);
void my_turn_right(void);
void my_turn_left(void);
void my_clear_field(void);

static uint8_t o_fall=0;
static uint8_t o_side=0;
static uint8_t o_gameover_flag=0;
static uint8_t o_block_flag=0;
static uint8_t o_block_flag2=0;
static uint8_t o_freeze_flag=0;
static uint8_t o_ran=0;
static uint8_t o_ran2=0;
static uint8_t o_lsig=0;
static int o_turn_point=0;
static uint32_t o_score=0;
static uint16_t o_lines=0;
static uint8_t o_lv=0;

static uint8_t o_alart=0;

static uint8_t tetris_delay=0;
static uint8_t o_tetris_delay=0;

void o_my_init_var(void);
void o_my_init_var2(void);
void o_my_make_block(void);
void o_my_gameover(void);
void o_my_get_key(void);
void o_my_serch_fall_state();
void o_my_make_field(void);
void o_my_make_collision_field(void);
void o_my_init_field(void);
void o_my_save_field(void);
void o_my_freeze_block(void);
void o_my_clear_freeze(void);
void o_my_search_line(void);
void o_my_turn_right(void);
void o_my_turn_left(void);
void o_my_clear_field(void);

void opponents_draw_field();
void getch(void){
	Glcd_str(37,28,"GAME OVER");
	Glcd_Print_map();
	_delay_ms(1000);
	while(sig==0){
		Glcd_str(8,36," Press any button");
		Glcd_Print_map();
	};
	app_number=MENU;
	return;
}

void vs_gameover(){
	Clear_Screen();
	Glcd_str(0,0,"SCORE");
	Glcd_Print_map();
	_delay_ms(1000);
	Glcd_font(0,7,'0'+score/10000000%10);
	Glcd_font(6,7,'0'+score/1000000%10);
	Glcd_font(12,7,'0'+score/100000%10);
	Glcd_font(18,7,'0'+score/10000%10);
	Glcd_font(24,7,'0'+score/1000%10);
	Glcd_font(30,7,'0'+score/100%10);
	Glcd_font(36,7,'0'+score/10%10);
	Glcd_font(42,7,'0'+score%10);

	Glcd_font(64,7,'0'+o_score/10000000%10);
	Glcd_font(70,7,'0'+o_score/1000000%10);
	Glcd_font(76,7,'0'+o_score/100000%10);
	Glcd_font(82,7,'0'+o_score/10000%10);
	Glcd_font(88,7,'0'+o_score/1000%10);
	Glcd_font(94,7,'0'+o_score/100%10);
	Glcd_font(100,7,'0'+o_score/10%10);
	Glcd_font(106,7,'0'+o_score%10);

	Glcd_Print_map();
	_delay_ms(1500);
	if(score > o_score) Glcd_str(0,15,"WINNER");
	else if(score < o_score) Glcd_str(64,15,"WINNER");
	else if(score = o_score) Glcd_str(50,15,"DRAW");
	Glcd_Print_map();
	_delay_ms(1500);
	Glcd_str(8,23," Press any button");
	Glcd_Print_map();
	while(sig == 0 && opponents_sig == 0);
	app_number = MENU;
}

void tetris_main(){
	uint8_t max;
	uint16_t alart_t=0;
	lines=0;
	lv=0;
	Clear_Screen();
	my_init_var();
	my_init_field();
	my_draw_field();
	
	while(sig==0){
		Glcd_str(8,36," Press any button");
		Glcd_Print_map();
	};

	while(app_number == TETRIS){
		if(gameover_flag == 0){
			Clear_Screen();
			if(alart>5){
				alart-=5;
				alart_t=15;
			}
			if(alart==0)Glcd_str(65,48,"        ");
			else if(alart==1){
				Glcd_str(65,48," SINGLE ");
			}
			else if(alart==2){
				Glcd_str(65,48," DOUBLE ");
			}
			else if(alart==3){
				Glcd_str(65,48," TRIPLE ");
			}
			else if(alart==4){
				Glcd_str(65,48," TETRIS ");
			}
			else if(alart==5){
				Glcd_str(65,48,"LEVEL UP");
			}

			if(alart_t>0) alart_t--;
			else alart=0;

			my_make_block();
			my_gameover();
			if(tetris_delay==max){
				my_clear_freeze();
				fall++;
				if(freeze_flag==1){
					freeze_flag=2;
					fall--;
				}
				tetris_delay=0;
			}
			my_get_key();
			my_serch_fall_state();
			my_make_field();
			my_init_field();
			my_freeze_block();
			my_draw_field();
			my_clear_field();
		}
		else{
			getch();
			break;
		}

		if(lv != (lines / 10) + 1){
			lv++;
			alart=10;
		}
		if(lv==1) max=15;
		else if(lv==2) max=10;
		else if(lv==3) max=9;
		else if(lv==4) max=8;
		else if(lv==5) max=7;
		else if(lv==6) max=5;
		else if(lv==7) max=4;
		else if(lv==8) max=3;
		else if(lv==9) max=2;
		else max=1;
		tetris_delay++;
	}
	return;
}

void vs_tetris_main(){
	uint8_t max = 15;
	uint16_t alart_t=0;
	uint8_t o_max = 15;
	uint16_t o_alart_t=0;
	charge = 0;
	lines=0;
	lv=0;
	o_lines=0;
	o_lv=0;
	Clear_Screen();
	my_init_var();
	my_init_field();
	o_my_init_var();
	o_my_init_field();
	opponents_draw_field();

	while(connection_flag == false){
		Glcd_str(8,36," Connect remocon ");
		Glcd_Print_map();
		if(button_jadge_one() == START) start_button();
		if(app_number != VSTETRIS) return;
	}
	while(sig == 0 && opponents_sig == 0){
		Glcd_str(8,36," Press any button");
		Glcd_Print_map();
	}


	while(app_number == VSTETRIS && connection_flag == true){
		if(gameover_flag == 0 || o_gameover_flag == 0){
			Clear_Screen();
			if(alart>5){
				alart-=5;
				alart_t=15;
			}
			
			if(alart==0)Glcd_str(0,57,"        ");
			else if(alart==1){
				Glcd_str(0,57," SINGLE ");
				charge += 1;
			}
			else if(alart==2){
				Glcd_str(0,57," DOUBLE ");
				charge += 2;
			}
			else if(alart==3){
				Glcd_str(0,57," TRIPLE ");
				charge += 3;
			}
			else if(alart==4){
				Glcd_str(0,57," TETRIS ");
				charge += 4;
			}
			else if(alart==5){
				Glcd_str(0,57,"LEVEL UP");
			}
			if(alart_t>0) alart_t--;

			else alart=0;

			if(o_alart>5){
				o_alart-=5;
				o_alart_t=15;
			}

			if(o_alart==0)Glcd_str(64,57,"        ");
			else if(o_alart==1){
				Glcd_str(64,57," SINGLE ");
				charge -= 1;
			}
			else if(o_alart==2){
				Glcd_str(64,57," DOUBLE ");
				charge -= 2;
			}
			else if(o_alart==3){
				Glcd_str(64,57," TRIPLE ");
				charge -= 3;
			}
			else if(o_alart==4){
				Glcd_str(64,57," TETRIS ");
				charge -= 4;
			}
			else if(o_alart==5){
				Glcd_str(64,57,"LEVEL UP");
			}
			if(o_alart_t>0) o_alart_t--;
			
			if(o_alart_t>0) o_alart_t--;
			else o_alart=0;

			my_make_block();
			o_my_make_block();
			my_gameover();
			o_my_gameover();
			if(tetris_delay == max){
				my_clear_freeze();
				fall++;
				if(freeze_flag==1){
					freeze_flag=2;
					fall--;
				}
				tetris_delay=0;
			}

			if(o_tetris_delay == o_max){
				o_my_clear_freeze();
				o_fall++;
				if(o_freeze_flag==1){
					o_freeze_flag=2;
					o_fall--;
				}
				o_tetris_delay=0;
			}
			if(gameover_flag == false)my_get_key();
			if(o_gameover_flag == false)o_my_get_key();
			/*if(gameover_flag == false)my_serch_fall_state();
			if(o_gameover_flag == false)o_my_serch_fall_state();*/
			if(gameover_flag == false)my_make_field();
			if(o_gameover_flag == false)o_my_make_field();
			if(gameover_flag == false)my_init_field();
			if(o_gameover_flag == false)o_my_init_field();
			if(gameover_flag == false)my_freeze_block();
			if(o_gameover_flag == false)o_my_freeze_block();
			opponents_draw_field();
			my_clear_field();
		}
		else{
			vs_gameover();
			break;
		}

		if(lv != (lines / 10) + 1){
			lv++;
			alart=10;
		}
		if(lv==1) max=15;
		else if(lv==2) max=10;
		else if(lv==3) max=9;
		else if(lv==4) max=8;
		else if(lv==5) max=7;
		else if(lv==6) max=5;
		else if(lv==7) max=4;
		else if(lv==8) max=3;
		else if(lv==9) max=2;
		else max=1;
		tetris_delay++;

		if(o_lv != (o_lines / 10) + 1){
			o_lv++;
			o_alart=10;
		}
		if(o_lv==1) o_max=15;
		else if(o_lv==2) o_max=10;
		else if(o_lv==3) o_max=9;
		else if(o_lv==4) o_max=8;
		else if(o_lv==5) o_max=7;
		else if(o_lv==6) o_max=5;
		else if(o_lv==7) o_max=4;
		else if(o_lv==8) o_max=3;
		else if(o_lv==9) o_max=2;
		else o_max=1;
		o_tetris_delay++;

	}
	return;
}

void my_init_var(){
	uint8_t i,j;
	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++){
			stage[0][i][j]=0;
			field[0][i][j]=0;
			collision_field[0][i][j]=0;
		}
	}
	fall = 0;
	side = 6;
	gameover_flag = 0;
	block_flag = 0;
	block_flag2 = 0;
	freeze_flag = 0;
	ran = 0;
	ran2 = 0;
	turn_point = 400;
	score = 0;
	lv = 1;
}
void my_init_var2(){
	fall = 0;
	side = 6;
	block_flag2 = 0;
	freeze_flag = 0;
	turn_point = 400;
}
void my_make_block(){
	uint8_t x,y;
	
	if(block_flag == 0){
		ran = (rand()%7);
		for(y=0;y<BLOCK_HEIGHT;y++){
			for(x=0;x<BLOCK_WIDTH;x++){
				first_block[0][y][x] = blocks[ran*4 + y][x];
			}
		}
		block_flag = 1;
	}
	
	if(block_flag2 == 0){
		ran2 = ran;
		ran = (rand()%7);
		for(y=0;y<BLOCK_HEIGHT;y++){
			for(x=0;x<BLOCK_WIDTH;x++){
				block[0][y][x] = first_block[0][y][x];
				first_block[0][y][x] = blocks[ran*4 + y][x];
			}
		}
		block_flag2 = 1;
	}
}
void my_gameover(){
	uint8_t x,y;
	my_make_collision_field();

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(block[0][y][x] != 0){
				if(collision_field[0][fall + y][side + x] != 0){
					gameover_flag++;
				}
			}
		}
	}
}
void my_get_key(){
	uint8_t x,y;
	uint8_t side_flag = 0;
	uint8_t fall_flag = 0;

	my_make_collision_field();

	if(button_jadge_one()!=NO){
		switch(button_jadge_one()){
			case START:
			start_button();
			break;
			case LEFT:
			soundPlay(SOUND_TYPE_KEY);
			for(y=0;y<BLOCK_HEIGHT;y++){
				for(x=0;x<BLOCK_WIDTH;x++){
					if(block[0][y][x] != 0){
						if(collision_field[0][fall + y][side + (x - 1)] != 0){
							side_flag++;
						}
					}
				}
			}
			if(side_flag == 0){
				side--;
			}
			my_clear_freeze();
			break;
			case RIGHT:
			soundPlay(SOUND_TYPE_KEY);
			for(y=0;y<BLOCK_HEIGHT;y++){
				for(x=0;x<BLOCK_WIDTH;x++){
					if(block[0][y][x] != 0){
						if(collision_field[0][fall + y][side + (x + 1)] != 0){
							side_flag++;
						}
					}
				}
			}
			if(side_flag == 0){
				side++;
			}
			my_clear_freeze();
			break;
			case UP:
			soundPlay(SOUND_TYPE_ENTER);
			if(lsig == UP) break;
			while(fall_flag == 0){
				for(y=0;y<BLOCK_HEIGHT;y++){
					for(x=0;x<BLOCK_WIDTH;x++){
						if(block[0][y][x] != 0){
							if(collision_field[0][fall + (y + 1)][side + x] != 0){
								fall_flag++;
							}
						}
					}
				}
				if(fall_flag == 0){
					tetris_delay=0;
					fall++;
					score++;
				}
			}
			freeze_flag=2;
			lsig=UP;
			break;
			case DOWN:
			soundPlay(SOUND_TYPE_KEY);
			my_clear_freeze();
			fall++;
			if(freeze_flag==1){
				freeze_flag=2;
				fall--;
			}
			tetris_delay=0;
			break;
			case A:
			soundPlay(SOUND_TYPE_SHOT);
			my_turn_right();
			my_clear_freeze();
			break;
			case B:
			soundPlay(SOUND_TYPE_SHOT);
			my_turn_left();
			my_clear_freeze();
			break;
		}
		lsig=button_jadge_one();
	}
	else lsig=0;
}
void my_make_field(){
	uint8_t i,j,x,y;

	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++) field[0][i][j] = field[0][i][j] + stage[0][i][j];
	}
	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(field[0][y+fall][x+side]==0)field[0][y+fall][x+side] = block[0][y][x];
		}
	}
}
void my_serch_fall_state(){
	uint8_t x,y,ff,f;
	my_make_collision_field();
	if(freeze_flag == 0){
	while(ff == 0){
		for(y=0;y<BLOCK_HEIGHT;y++){
			for(x=0;x<BLOCK_WIDTH;x++){
				if(block[0][y][x] != 0){
					if(collision_field[0][fall + f + (y + 1)][side + x] != 0 && collision_field[0][fall + f + (y + 1)][side + x] != 20){
						ff++;
					}
				}
			}
		}
		if(ff==0) f++;
	}

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(block[0][y][x] != 0){
				field[0][fall + f + y][side + x] = 20;
			}
		}
	}
	}
}
void my_make_collision_field(){
	uint8_t i,j;
	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++){
			collision_field[0][i][j] = stage[0][i][j];
			collision_field[0][i][0] = 9;
			collision_field[0][i][1] = 9;
			collision_field[0][i][2] = 9;
			collision_field[0][FIELD_HEIGHT-3][j] = 9;
			collision_field[0][FIELD_HEIGHT-2][j] = 9;
			collision_field[0][FIELD_HEIGHT-1][j] = 9;
			collision_field[0][i][13] = 9;
			collision_field[0][i][14] = 9;
			collision_field[0][i][15] = 9;
			collision_field[0][i][16] = 9;
			collision_field[0][i][17] = 9;
			collision_field[0][i][18] = 9;
			collision_field[0][i][19] = 9;
			collision_field[0][i][20] = 9;
		}
	}
}
void my_init_field(){
	uint8_t i,j,x,y;
	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++){
			field[0][i][0] = 9;
			field[0][i][1] = 9;
			field[0][i][2] = 9;
			field[0][FIELD_HEIGHT-3][j] = 9;
			field[0][FIELD_HEIGHT-2][j] = 9;
			field[0][FIELD_HEIGHT-1][j] = 9;
			field[0][i][13] = 9;
			field[0][i][14] = 9;
			field[0][i][15] = 9;
		}
	}

	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=16;j<FIELD_WIDTH;j++) field[0][i][j] = 19;
	}
	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++) field[0][y+2][x+16] = first_block[0][y][x] + 10;
	}
}
void my_freeze_block(){
	uint8_t x,y;

	my_make_collision_field();

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(block[0][y][x] != 0 && freeze_flag == 1){
				if(collision_field[0][fall + y][side + x] != 0) freeze_flag=2;
			}
		}
	}
	if(freeze_flag == 2){
		//fall-=2;
		my_search_line();
        my_save_field();
		my_init_var2();
		freeze_flag = 0;
	}
	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(block[0][y][x] != 0 && freeze_flag != 1){
				if(collision_field[0][fall + y + 1][side + x] != 0) freeze_flag++;
			}
		}
	}
	if(freeze_flag>0) freeze_flag=1;
	else freeze_flag=0;
}

void my_clear_freeze(){
	uint8_t x,y;
	my_make_collision_field();
	freeze_flag=0;
	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(block[0][y][x] != 0){
				if(collision_field[0][fall + y + 1][side + x] != 0) freeze_flag++;
			}
		}
	}
	if(freeze_flag>0) freeze_flag=1;
	else freeze_flag=0;
}
void my_search_line(){
	int i,j;
	uint8_t clear_lines = 0;
	uint8_t zero_count = 0;
	uint8_t clear_flag = 0;
	uint8_t clear_lines_point[4] = {0};
	uint8_t clear_line_index = 0;
	uint8_t remain_lines_point[20] = {0};
	uint8_t remain_line_index = 0;

	for(i=SEARCH_START_Y;i>=0;i--){
		for(j=SEARCH_START_X;j<SEARCH_START_X+10;j++){
			if(field[0][i][j] == 0){
				zero_count++;
			}
		}
		if(zero_count == 0){
			clear_flag++;
			clear_lines_point[clear_line_index] = i;
			clear_line_index++;
			clear_lines++;
			lines++;
		}
		else{
			remain_lines_point[remain_line_index] = i;
			remain_line_index++;
			zero_count = 0;
		}
	}

	if(clear_flag != 0){
		for(clear_line_index=0;clear_line_index<4;clear_line_index++){
			if(clear_lines_point[clear_line_index] != 0){
				for(j=SEARCH_START_X;j<SEARCH_START_X+10;j++){
					field[0][clear_lines_point[clear_line_index]][j] = 0;
				}
			}
		}
		if(app_number == TETRIS)my_draw_field();
		else if(app_number == VSTETRIS) opponents_draw_field();

		remain_line_index = 0;
		for(i=SEARCH_START_Y;i>=SEARCH_START_Y-14;i--){
			for(j=SEARCH_START_X;j<SEARCH_START_X+10;j++){
				field[0][i][j] = field[0][remain_lines_point[remain_line_index]][j];
			}
			remain_line_index++;
		}
	}
	if(clear_lines==1){
		soundPlay(SOUND_TYPE_COIN);
		score+=40;
	}
	else if(clear_lines==2){
		soundPlay(SOUND_TYPE_COIN);
		score+=100;
	}
	else if(clear_lines==3){
		soundPlay(SOUND_TYPE_CLEAR);
		score+=300;
	}
	else if(clear_lines==4){
		soundPlay(SOUND_TYPE_1UP);
		score+=1200;
	}
	if(clear_lines>0) alart= clear_lines+5;
	/*
	if(charge <= -2){
		for(i=4;i<FIELD_HEIGHT;i++){
			for(j=0;j<FIELD_WIDTH;j++){
				field[0][i-1][j] = field[0][i][j];
			}
		}
		for(j=0;j<FIELD_WIDTH;j++){
			field[0][FIELD_HEIGHT][j] = 1;
		}
		field[0][FIELD_HEIGHT][j+rand()%FIELD_WIDTH] = 0;
		charge += 2;
		if(charge > 0)charge = 0;
	}*/
}
void my_save_field(){
	uint8_t i,j;
	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++) stage[0][i][j] = field[0][i][j];
	}
}
void my_turn_right(){
	uint8_t x,y;
	uint8_t turn_flag = 0;
	uint8_t turn_block[4][4] = { 0 };

	turn_point++;

	my_make_collision_field();

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			turn_block[y][x] = blocks[ran2*4 + y][(turn_point%4)*4 + x];
		}
	}

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(turn_block[y][x] != 0){
				if(collision_field[0][fall + y][side + x] != 0){
					turn_flag++;
				}
			}
		}
	}

	if(turn_flag == 0){
		for(y=0;y<BLOCK_HEIGHT;y++){
			for(x=0;x<BLOCK_WIDTH;x++){
				block[0][y][x] = turn_block[y][x];
			}
		}
	}
	else{
		turn_point--;
	}
}
void my_turn_left(){
	uint8_t x,y;
	uint8_t turn_flag = 0;
	uint8_t turn_block[4][4] = { 0 };

	turn_point--;

	my_make_collision_field();

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			turn_block[y][x] = blocks[ran2*4 + y][(turn_point % 4)*4 + x];
		}
	}

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(turn_block[y][x] != 0){
				if(collision_field[0][fall + y][side + x] != 0){
					turn_flag++;
				}
			}
		}
	}

	if(turn_flag == 0){
		for(y=0;y<BLOCK_HEIGHT;y++){
			for(x=0;x<BLOCK_WIDTH;x++){
				block[0][y][x] = turn_block[y][x];
			}
		}
	}
	else turn_point++;
}
void my_clear_field(){
	uint8_t i,j;
	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++){
			if(gameover_flag ==false)field[0][i][j] = 0;
			if(o_gameover_flag ==false)field[1][i][j] = 0;
		}
	}
}


void o_my_init_var(){
	uint8_t i,j;
	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++){
			stage[1][i][j]=0;
			field[1][i][j]=0;
			collision_field[1][i][j]=0;
		}
	}
	o_fall = 0;
	o_side = 6;
	o_gameover_flag = 0;
	o_block_flag = 0;
	o_block_flag2 = 0;
	o_freeze_flag = 0;
	o_ran = 0;
	o_ran2 = 0;
	o_turn_point = 400;
	o_score = 0;
	o_lv = 1;
}
void o_my_init_var2(){
	o_fall = 0;
	o_side = 6;
	o_block_flag2 = 0;
	o_freeze_flag = 0;
	o_turn_point = 400;
}
void o_my_make_block(){
	uint8_t x,y;
	
	if(o_block_flag == 0){
		o_ran = (rand()%7);
		for(y=0;y<BLOCK_HEIGHT;y++){
			for(x=0;x<BLOCK_WIDTH;x++){
				first_block[1][y][x] = blocks[o_ran*4 + y][x];
			}
		}
		o_block_flag = 1;
	}
	
	if(o_block_flag2 == 0){
		o_ran2 = o_ran;
		o_ran = (rand()%7);
		for(y=0;y<BLOCK_HEIGHT;y++){
			for(x=0;x<BLOCK_WIDTH;x++){
				block[1][y][x] = first_block[1][y][x];
				first_block[1][y][x] = blocks[o_ran*4 + y][x];
			}
		}
		o_block_flag2 = 1;
	}
}
void o_my_gameover(){
	uint8_t x,y;
	o_my_make_collision_field();

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(block[1][y][x] != 0){
				if(collision_field[1][o_fall + y][o_side + x] != 0){
					o_gameover_flag++;
				}
			}
		}
	}
}
void o_my_get_key(){
	uint8_t x,y;
	uint8_t o_side_flag = 0;
	uint8_t o_fall_flag = 0;

	o_my_make_collision_field();

	if(opponents_button_jadge_one()!=NO){
		switch(opponents_button_jadge_one()){
			case LEFT:
			soundPlay(SOUND_TYPE_KEY);
			for(y=0;y<BLOCK_HEIGHT;y++){
				for(x=0;x<BLOCK_WIDTH;x++){
					if(block[1][y][x] != 0){
						if(collision_field[1][o_fall + y][o_side + (x - 1)] != 0){
							o_side_flag++;
						}
					}
				}
			}
			if(o_side_flag == 0){
				o_side--;
			}
			o_my_clear_freeze();
			break;
			case RIGHT:
			soundPlay(SOUND_TYPE_KEY);
			for(y=0;y<BLOCK_HEIGHT;y++){
				for(x=0;x<BLOCK_WIDTH;x++){
					if(block[1][y][x] != 0){
						if(collision_field[1][o_fall + y][o_side + (x + 1)] != 0){
							o_side_flag++;
						}
					}
				}
			}
			if(o_side_flag == 0){
				o_side++;
			}
			o_my_clear_freeze();
			break;
			case UP:
			soundPlay(SOUND_TYPE_ENTER);
			if(o_lsig==UP) break;
			while(o_fall_flag == 0){
				for(y=0;y<BLOCK_HEIGHT;y++){
					for(x=0;x<BLOCK_WIDTH;x++){
						if(block[1][y][x] != 0){
							if(collision_field[1][o_fall + (y + 1)][o_side + x] != 0){
								o_fall_flag++;
							}
						}
					}
				}
				if(o_fall_flag == 0){
					o_tetris_delay=0;
					o_fall++;
					o_score++;
				}
			}
			o_freeze_flag=2;
			o_lsig=UP;
			break;
			case DOWN:
			soundPlay(SOUND_TYPE_KEY);
			o_my_clear_freeze();
			o_fall++;
			if(o_freeze_flag==1){
				o_freeze_flag=2;
				o_fall--;
			}
			o_tetris_delay=0;
			break;
			case A:
			soundPlay(SOUND_TYPE_SHOT);
			o_my_turn_right();
			o_my_clear_freeze();
			break;
			case B:
			soundPlay(SOUND_TYPE_SHOT);
			o_my_turn_left();
			o_my_clear_freeze();
			break;
		}
		o_lsig=opponents_button_jadge_one();
	}
	else o_lsig=0;
}
void o_my_make_field(){
	uint8_t i,j,x,y;

	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++) field[1][i][j] = field[1][i][j] + stage[1][i][j];
	}
	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(field[1][y+o_fall][x+o_side]==0)field[1][y+o_fall][x+o_side] = block[1][y][x];
		}
	}


}
void o_my_serch_fall_state(){
	uint8_t x,y,ff,f;
	if(o_freeze_flag == 0){
		while(ff == 0){
			for(y=0;y<BLOCK_HEIGHT;y++){
				for(x=0;x<BLOCK_WIDTH;x++){
					if(block[1][y][x] != 0){
						if(collision_field[1][o_fall + f + (y + 1)][o_side + x] != 0){
							ff++;
						}
					}
				}
			}
			if(ff==0) f++;
		}

		for(y=0;y<BLOCK_HEIGHT;y++){
			for(x=0;x<BLOCK_WIDTH;x++){
				if(block[1][y][x] != 0){
					field[1][o_fall + f + y][o_side + x] = 20;
				}
			}
		}
	}
}
void o_my_make_collision_field(){
	uint8_t i,j;
	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++){
			collision_field[1][i][j] = stage[1][i][j];
			collision_field[1][i][0] = 9;
			collision_field[1][i][1] = 9;
			collision_field[1][i][2] = 9;
			collision_field[1][FIELD_HEIGHT-3][j] = 9;
			collision_field[1][FIELD_HEIGHT-2][j] = 9;
			collision_field[1][FIELD_HEIGHT-1][j] = 9;
			collision_field[1][i][13] = 9;
			collision_field[1][i][14] = 9;
			collision_field[1][i][15] = 9;
			collision_field[1][i][16] = 9;
			collision_field[1][i][17] = 9;
			collision_field[1][i][18] = 9;
			collision_field[1][i][19] = 9;
			collision_field[1][i][20] = 9;
		}
	}
}
void o_my_init_field(){
	uint8_t i,j,x,y;

	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++){
			field[1][i][0] = 9;
			field[1][i][1] = 9;
			field[1][i][2] = 9;
			field[1][FIELD_HEIGHT-3][j] = 9;
			field[1][FIELD_HEIGHT-2][j] = 9;
			field[1][FIELD_HEIGHT-1][j] = 9;
			field[1][i][13] = 9;
			field[1][i][14] = 9;
			field[1][i][15] = 9;
		}
	}

	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=16;j<FIELD_WIDTH;j++) field[1][i][j] = 19;
	}
	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++) field[1][y+2][x+16] = first_block[1][y][x] + 10;
	}
}
void o_my_freeze_block(){
	uint8_t x,y;

	o_my_make_collision_field();

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(block[1][y][x] != 0 && o_freeze_flag == 1){
				if(collision_field[1][o_fall + y][o_side + x] != 0) o_freeze_flag=2;
			}
		}
	}
	if(o_freeze_flag == 2){
		//fall-=2;
		o_my_search_line();
		o_my_save_field();
		o_my_init_var2();
		o_freeze_flag = 0;
	}
	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(block[1][y][x] != 0 && o_freeze_flag != 1){
				if(collision_field[1][o_fall + y + 1][o_side + x] != 0) o_freeze_flag++;
			}
		}
	}
	if(o_freeze_flag>0) o_freeze_flag=1;
	else o_freeze_flag=0;
}

void o_my_clear_freeze(){
	uint8_t x,y;
	o_my_make_collision_field();
	o_freeze_flag=0;
	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(block[1][y][x] != 0){
				if(collision_field[1][o_fall + y + 1][o_side + x] != 0) o_freeze_flag++;
			}
		}
	}
	if(o_freeze_flag>0) o_freeze_flag=1;
	else o_freeze_flag=0;
}
void o_my_search_line(){
	int i,j;
	uint8_t o_clear_lines = 0;
	uint8_t o_zero_count = 0;
	uint8_t o_clear_flag = 0;
	uint8_t o_clear_lines_point[4] = {0};
	uint8_t o_clear_line_index = 0;
	uint8_t o_remain_lines_point[20] = {0};
	uint8_t o_remain_line_index = 0;

	for(i=SEARCH_START_Y;i>=0;i--){
		for(j=SEARCH_START_X;j<SEARCH_START_X+10;j++){
			if(field[1][i][j] == 0){
				o_zero_count++;
			}
		}
		if(o_zero_count == 0){
			o_clear_flag++;
			o_clear_lines_point[o_clear_line_index] = i;
			o_clear_line_index++;
			o_clear_lines++;
			o_lines++;
		}
		else{
			o_remain_lines_point[o_remain_line_index] = i;
			o_remain_line_index++;
			o_zero_count = 0;
		}
	}

	if(o_clear_flag != 0){
		for(o_clear_line_index=0;o_clear_line_index<4;o_clear_line_index++){
			if(o_clear_lines_point[o_clear_line_index] != 0){
				for(j=SEARCH_START_X;j<SEARCH_START_X+10;j++){
					field[1][o_clear_lines_point[o_clear_line_index]][j] = 0;
				}
			}
		}
		opponents_draw_field();

		o_remain_line_index = 0;
		for(i=SEARCH_START_Y;i>=SEARCH_START_Y-14;i--){
			for(j=SEARCH_START_X;j<SEARCH_START_X+10;j++){
				field[1][i][j] = field[1][o_remain_lines_point[o_remain_line_index]][j];
			}
			o_remain_line_index++;
		}
	}
	if(o_clear_lines==1){
		soundPlay(SOUND_TYPE_COIN);
		o_score+=40;
	}
	else if(o_clear_lines==2){
		soundPlay(SOUND_TYPE_COIN);
		o_score+=100;
	}
	else if(o_clear_lines==3){
		soundPlay(SOUND_TYPE_CLEAR);
		o_score+=300;
	}
	else if(o_clear_lines==4){
		soundPlay(SOUND_TYPE_1UP);
		o_score+=1200;
	}
	if(o_clear_lines>0) o_alart = o_clear_lines+5;
	/*
	if(charge >= 2){
		for(i=4;i<FIELD_HEIGHT;i++){
			for(j=0;j<FIELD_WIDTH;j++){
				field[1][i-1][j] = field[1][i][j];
			}
		}
		for(j=3;j<FIELD_WIDTH;j++){
			field[1][FIELD_HEIGHT][j] = 1;
		}
		field[1][FIELD_HEIGHT][j+rand()%FIELD_WIDTH] = 0;
		charge -= 2;
		if(charge < 0)charge = 0;
	}*/
}
void o_my_save_field(){
	uint8_t i,j;
	for(i=0;i<FIELD_HEIGHT;i++){
		for(j=0;j<FIELD_WIDTH;j++) stage[1][i][j] = field[1][i][j];
	}
}
void o_my_turn_right(){
	uint8_t x,y;
	uint8_t o_turn_flag = 0;
	uint8_t o_turn_block[4][4] = { 0 };

	o_turn_point++;

	o_my_make_collision_field();

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			o_turn_block[y][x] = blocks[o_ran2*4 + y][(o_turn_point%4)*4 + x];
		}
	}

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(o_turn_block[y][x] != 0){
				if(collision_field[1][o_fall + y][o_side + x] != 0){
					o_turn_flag++;
				}
			}
		}
	}

	if(o_turn_flag == 0){
		for(y=0;y<BLOCK_HEIGHT;y++){
			for(x=0;x<BLOCK_WIDTH;x++){
				block[1][y][x] = o_turn_block[y][x];
			}
		}
	}
	else{
		o_turn_point--;
	}
}
void o_my_turn_left(){
	uint8_t x,y;
	uint8_t o_turn_flag = 0;
	uint8_t o_turn_block[4][4] = { 0 };

	o_turn_point--;

	o_my_make_collision_field();

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			o_turn_block[y][x] = blocks[o_ran2*4 + y][(o_turn_point % 4)*4 + x];
		}
	}

	for(y=0;y<BLOCK_HEIGHT;y++){
		for(x=0;x<BLOCK_WIDTH;x++){
			if(o_turn_block[y][x] != 0){
				if(collision_field[1][o_fall + y][o_side + x] != 0){
					o_turn_flag++;
				}
			}
		}
	}

	if(o_turn_flag == 0){
		for(y=0;y<BLOCK_HEIGHT;y++){
			for(x=0;x<BLOCK_WIDTH;x++){
				block[1][y][x] = o_turn_block[y][x];
			}
		}
	}
	else o_turn_point++;
}


void my_draw_field(){
	uint8_t i,j;
	print_cursorx=0;
	print_cursory=0;
	for(i=0;i<FIELD_HEIGHT-2;i++){
		for(j=2;j<14;j++){
			if(field[0][i][j] == 9) printtm((char)BB);
			else if(field[0][i][j] == 1) printtm((char)WB);
			else if(field[0][i][j] == 2) printtm((char)BB);
			else if(field[0][i][j] == 0) printtm((char)EM);
			else if(field[0][i][j] == 20) printtm((char)IB);
			else printtm((char)WB);
		}
		for(j=16;j<20;j++){
			if(field[0][i][j] == 0) printtm((char)BB);
			else if(field[0][i][j] == 19) printtm((char)BB);
			else if(field[0][i][j] == 11) printtm((char)WB);
			else if(field[0][i][j] == 10) printtm((char)EM);
			else if(field[0][i][j] == 20) printtm((char)IB);
			else printtm((char)BB);
		}
		printtm('n');
	}
	Glcd_str(65,0,"SCORE");
	Glcd_font(81,8,'0'+score/10000000%10);
	Glcd_font(87,8,'0'+score/1000000%10);
	Glcd_font(93,8,'0'+score/100000%10);
	Glcd_font(99,8,'0'+score/10000%10);
	Glcd_font(105,8,'0'+score/1000%10);
	Glcd_font(111,8,'0'+score/100%10);
	Glcd_font(117,8,'0'+score/10%10);
	Glcd_font(123,8,'0'+score%10);
	Glcd_str(65,24,"LEVEL");
	Glcd_font(99,32,'0'+lv);
	Glcd_Print_map();
	if(lsig=='s') _delay_ms(100);
}

void opponents_draw_field(){
	uint8_t i,j;
	print_cursorx=0;
	print_cursory=0;
	for(i=0;i<FIELD_HEIGHT-2;i++){
		for(j=2;j<14;j++){
			if(field[0][i][j] == 9)printotm((char)BB);
			else if(field[0][i][j] == 1)printotm((char)WB);
			else if(field[0][i][j] == 2)printotm((char)BB);
			else if(field[0][i][j] == 0)printotm((char)EM);
			else if(field[0][i][j] == 20)printotm((char)IB);
			else printotm((char)WB);
		}
		for(j=16;j<20;j++){
			if(field[0][i][j] == 0) printotm((char)BB);
			else if(field[0][i][j] == 19)printotm((char)BB);
			else if(field[0][i][j] == 11)printotm((char)WB);
			else if(field[0][i][j] == 10)printotm((char)EM);
			else if(field[0][i][j] == 20)printotm((char)IB);
			else printotm((char)BB);
		}
		printotm('n');
	}
	print_cursorx = 20;
	print_cursory = 0;
	for(i=0;i<16;i++){
		for(j=2;j<14;j++){
			if(field[1][i][j] == 9)printotm((char)BB);
			else if(field[1][i][j] == 1)printotm((char)WB);
			else if(field[1][i][j] == 2)printotm((char)BB);
			else if(field[1][i][j] == 0)printotm((char)EM);
			else if(field[1][i][j] == 20)printotm((char)IB);
			else printotm((char)WB);
		}
		for(j=16;j<20;j++){
			if(field[1][i][j] == 0) printotm((char)BB);
			else if(field[1][i][j] == 19)printotm((char)BB);
			else if(field[1][i][j] == 11)printotm((char)WB);
			else if(field[1][i][j] == 10)printotm((char)EM);
			else if(field[1][i][j] == 20)printotm((char)IB);
			else printotm((char)BB);
		}
		printotm('n');
	}
	Glcd_font(0,49,'0'+score/10000000%10);
	Glcd_font(6,49,'0'+score/1000000%10);
	Glcd_font(12,49,'0'+score/100000%10);
	Glcd_font(18,49,'0'+score/10000%10);
	Glcd_font(24,49,'0'+score/1000%10);
	Glcd_font(30,49,'0'+score/100%10);
	Glcd_font(36,49,'0'+score/10%10);
	Glcd_font(42,49,'0'+score%10);

	Glcd_font(64,49,'0'+o_score/10000000%10);
	Glcd_font(70,49,'0'+o_score/1000000%10);
	Glcd_font(76,49,'0'+o_score/100000%10);
	Glcd_font(82,49,'0'+o_score/10000%10);
	Glcd_font(88,49,'0'+o_score/1000%10);
	Glcd_font(94,49,'0'+o_score/100%10);
	Glcd_font(100,49,'0'+o_score/10%10);
	Glcd_font(106,49,'0'+o_score%10);
	Glcd_str(49,0,"LV");
	Glcd_str(113,0,"LV");
	Glcd_font(55,8,'0'+lv);
	Glcd_font(119,8,'0'+o_lv);
	Glcd_Print_map();
}

void printtm(char font){
	uint8_t i=0;
	if(font=='n'){
		print_cursorx=0;
		print_cursory++;
	}
	else if(font==EM){
		for(i = print_cursorx*4;i <= print_cursorx*4+3;i++) Glcd_map[i] &= ((uint64_t)0xfffffffffffffff0 << print_cursory*4 | (uint64_t)0x0fffffffffffffff >> (60-print_cursory*4));
		print_cursorx++;
	}
	else if(font==BB){
		DrawRectangle(print_cursorx*4,print_cursory*4,print_cursorx*4+3,print_cursory*4+3);
		DrawRectangle(print_cursorx*4+1,print_cursory*4+1,print_cursorx*4+2,print_cursory*4+2);
		print_cursorx++;
	}
	else if(font==WB){
		DrawRectangle(print_cursorx*4,print_cursory*4,print_cursorx*4+3,print_cursory*4+3);
		print_cursorx++;
	}
	else if(font==IB){
		DrawDot(print_cursorx*4,print_cursory*4);
		DrawDot(print_cursorx*4+3,print_cursory*4+3);
		DrawDot(print_cursorx*4+3,print_cursory*4);
		DrawDot(print_cursorx*4,print_cursory*4+3);
		print_cursorx++;
	}
}

void printotm(char font){
	uint8_t i=0;
	if(font=='n'){
		if(print_cursorx >= 20) print_cursorx = 20;
		else print_cursorx=0;
		print_cursory++;
	}
	else if(font==EM){
		print_cursorx++;
	}
	else if(font==BB){
		if(print_cursorx >= 20){
			DrawRectangle((print_cursorx-20)*3+64,print_cursory*3,(print_cursorx-20)*3+2+64,print_cursory*3+2);
			DrawDot((print_cursorx-20)*3+1+64,print_cursory*3+1);
		}
		else{
			DrawRectangle(print_cursorx*3,print_cursory*3,print_cursorx*3+2,print_cursory*3+2);
			DrawDot(print_cursorx*3+1,print_cursory*3+1);
		}
		print_cursorx++;
	}
	else if(font==WB){
		if(print_cursorx >= 20) DrawRectangle((print_cursorx-20)*3+64,print_cursory*3,(print_cursorx-20)*3+2+64,print_cursory*3+2);
		else DrawRectangle(print_cursorx*3,print_cursory*3,print_cursorx*3+2,print_cursory*3+2);
		print_cursorx++;
	}
	else if(font==IB){
		if(print_cursorx >= 20){
			DrawDot((print_cursorx-20)*3+64,print_cursory*3);
			DrawDot((print_cursorx-20)*3+64+2,print_cursory*3);
			DrawDot((print_cursorx-20)*3+64,print_cursory*3+2);
			DrawDot((print_cursorx-20)*3+64+2,print_cursory*3+2);
		}
		else{
			DrawDot(print_cursorx*3,print_cursory*3);
			DrawDot(print_cursorx*3,print_cursory*3+2);
			DrawDot(print_cursorx*3+2,print_cursory*3);
			DrawDot(print_cursorx*3+2,print_cursory*3+2);
		}
		print_cursorx++;
	}
}



