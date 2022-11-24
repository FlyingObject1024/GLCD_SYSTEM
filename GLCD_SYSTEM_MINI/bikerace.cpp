/*
 * bikerace.cpp
 *
 * Created: 2018/03/14 22:47:05
 *  Author: kodai
 */ 
 //ファイル名がbikeraceなのは、最初はレーサーを自転車にしようとしていたから。
 //うまく8x8のドット絵に動きが表せなかったため、スケボー的な形になった
 #include <avr/io.h>
 #include <stdlib.h>//乱数用
 #include <avr/interrupt.h>
 #include <avr/pgmspace.h>

 #include "Glcd_function.h"
 #include "defines.h"
 #include "sound.h"
 #include <util/delay.h>

 #define RACER_MAX 16
 #define HERDLE_MAX 100
 #define X_MAX 2048
 #define Y_MAX 24
 #define FAST_MAX 20
 #define LAP_MAX 3

 #define mad 0x30
 #define mad_tr 0x2f
 #define low_jumper 0x20
 #define high_jumper 0x21
 #define wall 0x22
 #define cooler 0x4f
 #define itembox 0x52
 #define sliper 0x32
 #define ball 0x33
 #define star 0x6c
 #define thunder 0x34//障害物等の番号
// #define S_LINE 0x99


 #define SLIP 1
 #define HEAT 2
 #define HIGH_JUMP 3
 #define LOW_JUMP 4
 #define STAR 10//レーサーの状態

 uint8_t race_stage = 0;
enum{
	NORMAL,JUMP_1,JUMP_2,JUMP_1_RE,JUMP_2_RE
};//レーサーの表示状態

const static int16_t hx[3][HERDLE_MAX] ={//座標
	{112,120,144,152,168,176,184,184,200,264,280,512,512,512,512,800,1024,1024,1024,1024,1016,1040,1600,1600,1990,1998,1985,1980,2008,2010,2020,2030,800,1700,1700,1800,1800},
	{128,128,128,128,160,160,160,160,400,400,400,400,460,460,460,460,540,540,540,540,620,620,620,620,700,700,700,700,800,800,824,824,880,880,880,904,904,928,928,928,952,1300,1300,1300,1300,2000,2000,2000,2000,1500,1500,1500,1500,},
	{112,112,136,136,136,136,200,200,200,240,240,280,280,280,300,300,340,340,340,400,400,400,400,800,800,800,800,830,830,838,838,850,850,850,1000,1000,1000,1000,1200,1230,1230,1230,1230,1240,1600,1608,1608,},
};
const static int8_t hy[3][HERDLE_MAX] = {
	{0,0,24,24,8,8,0,16,24,8,8,0,8,16,24,0,0,8,16,24,24,16,0,8,0,8,16,24,0,8,16,24,24,0,24,8,16},
	{0,8,16,24,0,8,16,24,0,8,16,24,0,8,16,24,0,8,16,24,0,8,16,24,0,8,16,0,8,16,0,8,24,0,24,0,16,24,8,0,8,26,24,0,8,16,24,},
	{0,16,0,8,16,24,0,8,24,0,16,8,16,24,0,24,0,8,16,24,0,8,16,24,0,8,16,24,8,24,0,16,0,8,24,0,8,16,24,8,0,8,16,24,8,8,8,16,0,8,16,24},
};
const static uint8_t htype[3][HERDLE_MAX] = {//種類
	{mad,mad,mad,mad,mad,mad,low_jumper,high_jumper,cooler,itembox,wall,itembox,itembox,itembox,itembox,cooler,mad,mad,mad,mad,low_jumper,itembox,wall,wall,mad,mad,mad,mad,mad,mad,mad,mad,itembox,high_jumper,high_jumper,low_jumper,low_jumper},
	{itembox,itembox,itembox,itembox,high_jumper,high_jumper,low_jumper,low_jumper,mad_tr,mad_tr,mad_tr,mad_tr,mad_tr,mad_tr,mad_tr,mad_tr,mad_tr,mad_tr,mad_tr,mad_tr,mad_tr,mad_tr,high_jumper,mad_tr,mad_tr,mad_tr,cooler,mad_tr,mad_tr,mad_tr,mad_tr,cooler,itembox,itembox,cooler,},
	{high_jumper,itembox,mad,mad,mad,mad,cooler,wall,wall,mad,mad,mad,itembox,mad,low_jumper,high_jumper,mad,mad,mad,mad,itembox,itembox,itembox,itembox,high_jumper,high_jumper,high_jumper,high_jumper,wall,wall,mad,mad,cooler,mad,mad,wall,itembox,wall,itembox,high_jumper,mad,mad,mad,mad,wall,low_jumper,wall,wall},
};

 typedef struct {
	 int16_t	x;
	 int8_t		y;
	 int8_t		z;
	 uint8_t	fast;
	 uint8_t	heat;
	 uint8_t	wait;
	 uint8_t	flags;
	 uint8_t	move;
	 uint8_t	item;
	 uint8_t	up_down;
	 uint8_t	graph;
	 uint8_t    lap;
	 int    	jump;
	 uint8_t	state;
	 uint8_t	rank;
 } RACER; //レーサー構造体
 
 typedef struct {
	 int16_t	x;
	 int8_t		y;
	 uint8_t	type;
 } HERDLE;//障害物構造体　綴りが違う気がする・・・

 static RACER racer[RACER_MAX];
 static HERDLE herdle[HERDLE_MAX];

 void init_racer();
 void move_racer();
 void draw_racer();
 void com_racer();
 void init_herdle();
 void draw_herdle();
 void hit();
 void use_item(uint8_t i);

 void init_racer(){
	uint8_t i;
	for (i=0;i<RACER_MAX;i++){
		racer[i].x = i*4;
		racer[i].y = i*8 % (Y_MAX+8);
		racer[i].z = 0;
		racer[i].fast = 0;
		racer[i].heat = 0;
		racer[i].wait = 0;
		racer[i].flags = 0;
		racer[i].item = 0;
		racer[i].move = 0;
		racer[i].up_down = 0;
		racer[i].graph = NORMAL;
		racer[i].lap = 0;
		racer[i].jump = 0;
		racer[i].state = rand()%100;
		racer[i].rank = 0;
	}
 }

 void move_racer(){
	uint8_t dusher[20]={0,1,1,2,2,3,3,4,4,4,4,5,5,5,6,6,6,7,7,8,};//レーサーの速度
	uint8_t low_jump[8]={5,8,9,10,9,8,5,0};//低空ジャンプの座標移動
	uint8_t high_jump[10]={7,12,15,16,17,16,15,12,7,0};//ハイジャンプの座標移動
	uint8_t i;
	for(i=0;i<RACER_MAX;i++){
		if(racer[i].up_down == UP){
			if(racer[i].y > 0) racer[i].y -= 2;
			if(racer[i].y%8 == 0) racer[i].up_down = 0;
			racer[i].graph = JUMP_1;
		}
		else if(racer[i].up_down == DOWN){
			if(racer[i].y < Y_MAX) racer[i].y += 2;
			if(racer[i].y%8 == 0) racer[i].up_down = 0;
			racer[i].graph = JUMP_1_RE;
		}

		if(racer[i].heat >= 30 && racer[i].flags == 0){
			racer[i].flags = HEAT;
		}

		if(racer[i].flags == SLIP){
			racer[i].move = 0;
			racer[i].wait -= 1;
			if(racer[i].wait <= 0) racer[i].flags =0;
			racer[i].graph = JUMP_2;
		}
		else if(racer[i].flags == HEAT){
			racer[i].move = 0;
			if(racer[i].heat <= 0){
				racer[i].flags = 0;
			}
			else{
				racer[i].heat -= 1;
				racer[i].graph = JUMP_2;
			}
		}
		else if(racer[i].flags == LOW_JUMP){
			racer[i].jump += 1;
			racer[i].z = low_jump[racer[i].jump];
			if(racer[i].jump == 7){
				racer[i].jump = 0;
				racer[i].flags = 0;
			}
		}
		else if(racer[i].flags == HIGH_JUMP){
			racer[i].jump += 1;
			racer[i].z = high_jump[racer[i].jump];
			if(racer[i].jump == 9){
				racer[i].jump = 0;
				racer[i].flags = 0;
			}
		}
		else if(racer[i].flags == STAR){
			racer[i].fast = FAST_MAX-1;
			racer[i].heat = 0;
			racer[i].wait -= 1;
			racer[i].graph = STAR;
			if(racer[i].wait <= 0){
				racer[i].flags = 0;
				racer[i].item = 0;
			}
		}
		else{
			if(racer[i].up_down == 0) racer[i].graph = NORMAL;
		}
		if(racer[i].move == B){
			if(i==0)simpleSound(NOTE_C4,1);
			racer[i].fast += 1;
			racer[i].heat += 1;
			if(racer[i].fast >= FAST_MAX-1 ) racer[i].fast = FAST_MAX-1;
			racer[i].x += (dusher[racer[i].fast]*1.5);
		}
		else if(racer[i].move == A){
			racer[i].fast += 1;
			if(racer[i].fast >= FAST_MAX-1 ) racer[i].fast = FAST_MAX-1;
			racer[i].x += dusher[racer[i].fast];
		}
		else{
			if(racer[i].fast > 0 ) racer[i].fast -= 1;
			racer[i].x += dusher[racer[i].fast];
		}
		if(racer[i].x >= X_MAX){
			racer[i].x = racer[i].x % X_MAX;
			racer[i].lap += 1;
		}
	}
 }

 void draw_racer(){
 uint8_t i;
 uint8_t first_l1,first_l2,first_l3;
	if(app_number == BOARDRACE){

		if(racer[0].graph == NORMAL)Glcd_graphic(0,(uint8_t)(racer[0].y-racer[0].z+27),0x0c,PUT,0);
		else if(racer[0].graph == JUMP_1)Glcd_graphic(0,(uint8_t)(racer[0].y-racer[0].z+27),0x0d,PUT,0);
		else if(racer[0].graph == JUMP_1_RE)Glcd_graphic(0,(uint8_t)(racer[0].y-racer[0].z+27),0x0d,PUT,1);
		else if(racer[0].graph == JUMP_2)Glcd_graphic(0,(uint8_t)(racer[0].y-racer[0].z+27),0x0e,PUT,0);
		else if(racer[0].graph == JUMP_2_RE)Glcd_graphic(0,(uint8_t)(racer[0].y-racer[0].z+27),0x0e,PUT,1);
		else if(racer[0].graph == STAR)Glcd_graphic(0,(uint8_t)(racer[0].y-racer[0].z+27),star,PUT,0);
		for (i=1;i<RACER_MAX;i++){
			if(racer[i].x>=racer[0].x && racer[i].x <= racer[0].x+128){
				if(racer[i].graph == NORMAL)Glcd_graphic(racer[i].x-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),0x0c,PUT,0);
				else if(racer[i].graph == JUMP_1)Glcd_graphic(racer[i].x-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),0x0d,PUT,0);
				else if(racer[i].graph == JUMP_1_RE)Glcd_graphic(racer[i].x-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),0x0d,PUT,1);
				else if(racer[i].graph == JUMP_2)Glcd_graphic(racer[i].x-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),0x0e,PUT,0);
				else if(racer[i].graph == JUMP_2_RE)Glcd_graphic(racer[i].x-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),0x0e,PUT,1);
				else if(racer[i].graph == STAR)Glcd_graphic(racer[i].x-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),star,PUT,0);
			}
			else if(racer[0].x >= X_MAX-128 && racer[i].x < 128){
				if(racer[i].graph == NORMAL)Glcd_graphic(racer[i].x+X_MAX-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),0x0c,PUT,0);
				else if(racer[i].graph == JUMP_1)Glcd_graphic(racer[i].x+X_MAX-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),0x0d,PUT,0);
				else if(racer[i].graph == JUMP_1_RE)Glcd_graphic(racer[i].x+X_MAX-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),0x0d,PUT,1);
				else if(racer[i].graph == JUMP_2)Glcd_graphic(racer[i].x+X_MAX-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),0x0e,PUT,0);
				else if(racer[i].graph == JUMP_2_RE)Glcd_graphic(racer[i].x+X_MAX-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),0x0e,PUT,1);
				else if(racer[i].graph == STAR)Glcd_graphic(racer[i].x+X_MAX-racer[0].x,(uint8_t)(racer[i].y-racer[i].z+27),star,PUT,0);
			}
		}
		//drawroad
		DrawLine(0,59,128,59,1);
		DrawLine(0,31,128,31,1);
		if(X_MAX-racer[0].x < 128 && X_MAX-racer[0].x >= 0){
			DrawLine(X_MAX-racer[0].x,31,X_MAX-racer[0].x+16,59,1);
			DrawLine(X_MAX-racer[0].x+3,31,X_MAX-racer[0].x+16+3,59,1);
		}
		
		first_l1 = 21 - racer[0].x % 22;
		if(first_l1<11) DrawLine(0,38,first_l1,38,1);
		else DrawLine(first_l1-10,38,first_l1,38,1);
		first_l2 = 21 - (racer[0].x-4) % 22;
		if(first_l2<11) DrawLine(0,46,first_l2,46,1);
		else DrawLine(first_l2-10,46,first_l2,46,1);
		first_l3 = 21 - (racer[0].x-8) % 22;
		if(first_l3<11) DrawLine(0,54,first_l3,54,1);
		else DrawLine(first_l3-10,54,first_l3,54,1);

		for(i=1;i<7;i++){
			DrawLine(i*22+first_l1-10,38,i*22+first_l1,38,1);
			DrawLine(i*22+first_l2-10,46,i*22+first_l2,46,1);
			DrawLine(i*22+first_l3-10,54,i*22+first_l3,54,1);
		}
		
	}
 }
 
 void com_racer(){//コンピュータレーサーの関数
	uint8_t i;
	uint8_t h;
	if(app_number == BOARDRACE) i=1;
	//if(app_number == VSRACE) i=2;
	for (i;i<RACER_MAX;i++){
		if(racer[i].heat < 29 && racer[i].fast >= 19) racer[i].move = B;
		else racer[i].move = A;

		if(racer[i].item == cooler){
			if(racer[i].heat >= 29)use_item(i);
		}
		else if(racer[i].item == sliper){
			if(racer[i].state == 30)use_item(i);
		}
		else if(racer[i].item == low_jumper || racer[i].item == high_jumper){
			for (h=0;h<HERDLE_MAX;h++){
				if((herdle[h].type == wall || herdle[h].type == mad) && (herdle[h].x > racer[i].x && herdle[h].x <= racer[i].x+16 && herdle[h].y/8 == racer[h].y/8)){
					use_item(i);
				}
			}
		}
		else if(racer[i].item != 0) use_item(i);

		racer[i].state += 1;
		if(racer[i].state >= 99){
			racer[i].state = 0;
		}

		if(rand()%15 == 0 || racer[i].fast == 1) racer[i].up_down = rand()%2+1;
	}
 }

 void rank_racer(){//レーサーの順位判別
	uint8_t i;
	uint8_t rank[RACER_MAX];
	uint8_t r;
	uint8_t s;
	uint32_t max=0;
	uint8_t max_racer=0;
	uint8_t mflag=0;
	for (r=0;r<RACER_MAX;r++){
		rank[r] = 100;
	}
	for (r=0;r<RACER_MAX;r++){
		max=0;
		for(i=0;i<RACER_MAX;i++){
			mflag=0;
			for (s=0;s<RACER_MAX;s++){
				if(rank[s] == i) mflag = 1;
			}

			if(racer[i].x + racer[i].lap * X_MAX > max && mflag == 0){
				max = racer[i].x + racer[i].lap * X_MAX;
				max_racer = i;
			}
		}
		rank[r] = max_racer;
	}
	for(r=0;r<RACER_MAX;r++) racer[rank[r]].rank = r + 1;
 }

 void init_herdle(){//障害物初期化
	
	uint8_t i;
	for(i=0;i<HERDLE_MAX;i++){
		herdle[i].type = 0;
	}
	for(i=0;i<HERDLE_MAX;i++){
		herdle[i].x = hx[race_stage][i];
		herdle[i].y = hy[race_stage][i];
		herdle[i].type = htype[race_stage][i];
	}
 }

 void draw_herdle(){//障害物表示
	if(app_number == BOARDRACE){
	 for (uint8_t i=0;i<HERDLE_MAX;i++){
			if(herdle[i].type == ball){
				herdle[i].x += 12;
				if(herdle[i].x >= X_MAX) herdle[i].x -= X_MAX;
			}
			 if(herdle[i].x>racer[0].x && herdle[i].x < racer[0].x+256 && herdle[i].type!=0 && herdle[i].type!=mad_tr){
				Glcd_graphic(herdle[i].x-racer[0].x,herdle[i].y+27,herdle[i].type,OR,0);
			 }
			 else if(racer[0].x>=X_MAX-128 && herdle[i].x<128 && herdle[i].type!=0 && herdle[i].type!=mad_tr){
				Glcd_graphic(herdle[i].x+X_MAX-racer[0].x,herdle[i].y+27,herdle[i].type,OR,0);
			 }
			 else if( herdle[i].type == mad_tr ){
				 if( herdle[i].x>racer[0].x && herdle[i].x < racer[0].x+256) Glcd_graphic(herdle[i].x-racer[0].x,herdle[i].y+27,mad,OR,0);
				 if( herdle[i].x+8>racer[0].x && herdle[i].x+8 < racer[0].x+256) Glcd_graphic(herdle[i].x+8-racer[0].x,herdle[i].y+27,mad,OR,0);
				 if( herdle[i].x+16>racer[0].x && herdle[i].x+16 < racer[0].x+256) Glcd_graphic(herdle[i].x+16-racer[0].x,herdle[i].y+27,mad,OR,0);
			 }	
		}
	 }

 }

 void hit(){//接触
	uint8_t i,j;
	for (i=0;i<RACER_MAX;i++){
		for (j=0;j<RACER_MAX;j++){
			if(i!=j && racer[i].z == racer[j].z && racer[i].y == racer[j].y && racer[i].x+8 > racer[j].x && racer[i].x+8 < racer[j].x+4 && racer[i].flags ==0){
				racer[i].flags = SLIP;
				if(i==0)soundPlay(SOUND_TYPE_CANCAL);
				racer[i].wait = 5;
			}
		}
		for (j=0;j<HERDLE_MAX;j++){
			if(herdle[j].type==mad_tr && racer[i].y == herdle[j].y && racer[i].z == 0 && racer[i].x+8 > herdle[j].x && racer[i].x+8 < herdle[j].x+28){
				racer[i].fast=1;
			}	
			if(herdle[i].type != 0 && racer[i].y == herdle[j].y && racer[i].z == 0 && racer[i].x+8 > herdle[j].x && racer[i].x+8 < herdle[j].x+12){
				if(herdle[j].type==mad){
					racer[i].fast=1;
				}
				else if(herdle[j].type==low_jumper && racer[i].flags != STAR){
					if(i==0)soundPlay(SOUND_TYPE_JUMP);
					racer[i].flags=LOW_JUMP;
					if(racer[i].fast+5 < FAST_MAX)racer[i].fast+=5;
					else racer[i].fast=FAST_MAX;
				}
				else if(herdle[j].type==high_jumper && racer[i].flags != STAR){
					if(i==0)soundPlay(SOUND_TYPE_JUMP);
					racer[i].flags=HIGH_JUMP;
					if(racer[i].fast+10 < FAST_MAX)racer[i].fast+=10;
					else racer[i].fast=FAST_MAX;
				}
				else if(herdle[j].type==cooler){
					racer[i].heat = 0;
				}
				else if(herdle[j].type==itembox && racer[i].item == 0){
					if(i==0)soundPlay(SOUND_TYPE_CLEAR);
					uint8_t items[8]={high_jumper,low_jumper,sliper,sliper,cooler, ball, star,thunder};
					if(racer[i].rank >= RACER_MAX/2)racer[i].item = items[rand()%6+2];
					else racer[i].item = items[rand()%6];
				}
				else if(herdle[j].type==sliper && racer[i].flags != STAR){
					if(i==0)soundPlay(SOUND_TYPE_CANCAL);
					herdle[j].type=0;
					racer[i].flags=SLIP;
					racer[i].wait = 10;
				}
				else if(herdle[j].type==wall && racer[i].flags != STAR){
					racer[i].fast=0;
					racer[i].x=herdle[j].x-8;
				}
				else if(herdle[j].type==ball && racer[i].flags != STAR){
					if(i==0)soundPlay(SOUND_TYPE_CANCAL);
					if(i == 0) Glcd_graphic(60,8,ball,PUT,0);
					herdle[j].type=0;
					racer[i].flags = SLIP;
					racer[i].wait = 10;
				}
			}
		}
	}
 }

 void use_item(uint8_t i){
	uint8_t j;
	if(racer[i].item != 0){
		if(racer[i].item == low_jumper) racer[i].flags = LOW_JUMP;
		else if(racer[i].item == high_jumper) racer[i].flags = HIGH_JUMP;
		else if(racer[i].item == cooler) racer[i].heat = 0;
		else if(racer[i].item == sliper){
			for (j=0;j<HERDLE_MAX;j++){
				if(herdle[j].type == 0){
					herdle[j].type = sliper;
					herdle[j].x = racer[i].x-8;
					herdle[j].y = racer[i].y/8*8;
				}
			}
		}
		else if(racer[i].item == ball){
			for (j=0;j<HERDLE_MAX;j++){
				if(herdle[j].type == 0 || herdle[j].type == sliper){
					herdle[j].type = ball;
					herdle[j].x = racer[i].x+8;
					herdle[j].y = (racer[i].y/8)*8;
				}
			}
		}
		else if(racer[i].item == star && racer[i].flags == 0){
			racer[i].heat = 0;
			racer[i].flags = STAR;
			racer[i].wait = 60;
		}
		else if(racer[i].item == thunder){
			soundPlay(SOUND_TYPE_BOMB);
			for (j=0;j<RACER_MAX;j++){
				if(racer[j].flags == 0 && racer[j].rank < racer[i].rank){
					racer[j].flags = SLIP;
					racer[j].wait = (RACER_MAX - racer[j].rank) * 2;
				}
			}
			Glcd_graphic(60,0,thunder,PUT,0);
			Glcd_graphic(60,8,thunder,PUT,0);
			Glcd_graphic(60,16,thunder,PUT,0);
			Glcd_graphic(60,24,thunder,PUT,0);
			Glcd_graphic(60,32,thunder,PUT,0);
			Glcd_graphic(60,40,thunder,PUT,0);
			Glcd_graphic(60,48,thunder,PUT,0);
			Glcd_graphic(60,56,thunder,PUT,0);
		}
		if(racer[i].item != star)racer[i].item = 0;
	}
	return;
 }
  
 void game_over(){
	Clear_Screen();
	if(app_number == BOARDRACE){
		if(racer[0].rank == 1) Glcd_str(64,29,"1st");
		else if(racer[0].rank == 2) Glcd_str(64,29,"2nd");
		else if(racer[0].rank == 3) Glcd_str(64,29,"3rd");
		else{
			if(racer[0].rank>=10)Glcd_font(58,29,racer[0].rank/10+'0');
			Glcd_font(64,29,racer[0].rank%10+'0');
			Glcd_str(70,29,"th");
			Glcd_str(37,37,"GAME OVER");
			app_number = MENU;
		}
		Glcd_Print_map();
		_delay_ms(5000);
		if(app_number == BOARDRACE){
			race_stage += 1;
			if(race_stage >= 3) app_number = MENU;
		}

	}

 }

 void boardrace_main(){
	init_racer();
	init_herdle();
	uint8_t start = 0;
	race_stage = 0;
	while(app_number==BOARDRACE){
		if(button_jadge_one()==UP && button_jadge_one()!=DOWN && racer[0].up_down==0){
			racer[0].up_down=UP;
		}
		else if(button_jadge_one()==DOWN && button_jadge_one()!=UP && racer[0].up_down==0){
			racer[0].up_down=DOWN;
		}

		if(button_jadge(B)){
			racer[0].move=B;
		}
		else if(button_jadge(A)){
			racer[0].move=A;
		}
		else racer[0].move=0;

		if(button_jadge(SELECT)){
			use_item(0);
		}
		else if(button_jadge(START)) start_button();
		
		Clear_Screen();
		com_racer();

		hit();
		move_racer();
		if(racer[0].lap >= LAP_MAX){
			game_over();
			init_racer();
			init_herdle();
			if(app_number == BOARDRACE) start = 0;
			Clear_Screen();
		}
		draw_racer();
		draw_herdle();
		Glcd_str(0,0," HEAT ");
		Glcd_str(52,0,"LAP:");
		Glcd_font(76,0,racer[0].lap+'1');
		Glcd_font(82,0,'/');
		Glcd_font(88,0,LAP_MAX+'0');
		DrawRectangle(0,8,31,11);
		for (uint8_t i=0;i<30;i++){
			if(racer[0].heat >= i)DrawLine(1+i,8,1+i,11,1);
		}
		DrawRectangle(0,13,11,24);
		if(racer[0].item != 0)Glcd_graphic(2,15,racer[0].item,PUT,0);

		rank_racer();

		if(racer[0].rank == 1) Glcd_str(64,8,"1st");
		else if(racer[0].rank == 2) Glcd_str(64,8,"2nd");
		else if(racer[0].rank == 3) Glcd_str(64,8,"3rd");
		else{
			if(racer[0].rank>=10)Glcd_font(58,8,racer[0].rank/10+'0');
			Glcd_font(64,8,racer[0].rank%10+'0');
			Glcd_str(70,8,"th");	
		}
		DrawRectangle(94,0,127,5);
		for (uint8_t i=0;i<RACER_MAX;i++){
			DrawDot(95+racer[i].x/(X_MAX/32),racer[i].y/8+1);
		}
		Glcd_Print_map();
		if(start == 0){
			Glcd_str(46,22,"STAGE");
			Glcd_font(76,22,'1' + race_stage);
			Glcd_font(64,30,'3');
			Glcd_Print_map();
			simpleSound(NOTE_A4,4);
			_delay_ms(1000);
			Glcd_font(64,30,'2');
			Glcd_Print_map();
			simpleSound(NOTE_A4,4);
			_delay_ms(1000);
			Glcd_font(64,30,'1');
			Glcd_Print_map();
			simpleSound(NOTE_A4,4);
			_delay_ms(1000);
			Glcd_str(64,30,"GO!");
			Glcd_Print_map();
			simpleSound(NOTE_A5,4);
			_delay_ms(500);
			Glcd_str(64,30,"   ");
			Glcd_Print_map();
			start = 1;
		}
	}
	race_stage = 0;
 }
