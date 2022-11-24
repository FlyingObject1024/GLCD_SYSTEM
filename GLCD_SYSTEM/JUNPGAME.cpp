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
 #define PLAYER 0
 #define LOWWALL 1
 #define WALL 2
 #define HIGHWALL 3
 #define BIRD 4
 #define GOLD 5
 #define HEART 6
static uint64_t score=0;
static uint8_t HP=1;
#define NO 0
#define EXIST 1
class JUMP_OBJECT
 {
	public:
	 uint8_t objectname;
	 char object_x;
	 char object_y;
	 char being;
	 void setnum(uint8_t name,char x,char y,char b);
	 void draw_jump_object();
 };
void JUMP_OBJECT::setnum(uint8_t name,char x,char y,char b){
	 objectname=name;
	 object_x=x;
	 object_y=y;
	 being=b;
 }
void JUMP_OBJECT::draw_jump_object(){
	if(objectname==PLAYER)Glcd_graphic2(object_x,object_y,0xe0+being,0,0);
	else if(objectname==BIRD)Glcd_graphic(object_x,object_y,0x6e,0,0);
	else if(objectname==GOLD)Glcd_graphic(object_x,object_y,0x6f,0,0);
	else if(objectname==HEART)Glcd_graphic(object_x,object_y,0x6b,0,0);
	else{
		if(object_x+objectname*8>=128) DrawRectangle(object_x,object_y,127,object_y+objectname*8);
		else if(object_x<0) DrawRectangle(0,object_y,object_x+objectname+4,object_y+objectname*8);
		else DrawRectangle(object_x,object_y,object_x+objectname+4,object_y+objectname*8);
	}
}

void crawd_draw(char crawd_x){
	Glcd_graphic(crawd_x,20,0x49,PUT,0);
	Glcd_graphic(crawd_x+8,20,0x4a,PUT,0);
	Glcd_graphic(crawd_x,28,0x4b,PUT,0);
	Glcd_graphic(crawd_x+8,28,0x4c,PUT,0);
}

#define CASE break; case

void jump_main(){
	score=0;
	char crawd_x=120;
	char crawd_exist=0;
	char jumpingl_y=0;
	char jumpingf_y=0;
	bool jump_flag=0;
	bool gameover_flag=0;
	bool drop_flag=0;
	HP=1;
	Clear_Screen();
	 JUMP_OBJECT player;
	 JUMP_OBJECT lowwall;
	 JUMP_OBJECT lowwall2;
	 JUMP_OBJECT wall;
	 JUMP_OBJECT wall2;
	 JUMP_OBJECT highwall;
	 JUMP_OBJECT highwall2;
	 JUMP_OBJECT bird;
	 JUMP_OBJECT bird2;
	 JUMP_OBJECT gold;
	 JUMP_OBJECT heart;
	 player.setnum(PLAYER,0,40,EXIST);
	 wall.setnum(WALL,0,32,NO);
	 wall2.setnum(WALL,0,32,NO);
	 lowwall.setnum(LOWWALL,0,40,NO);
	 lowwall2.setnum(LOWWALL,0,40,NO);
	 highwall.setnum(HIGHWALL,0,24,NO);
	 highwall2.setnum(HIGHWALL,0,24,NO);
	 bird.setnum(BIRD,0,0,NO);
	 bird2.setnum(BIRD,0,0,NO);
	 gold.setnum(GOLD,0,0,NO);
	 heart.setnum(HEART,0,0,NO);
	 char exist=0;
	while (app_number == JUMP){
		if(jump_flag){
			if(drop_flag==1)player.object_y+=10;
			else{
				jumpingl_y=player.object_y;
				player.object_y+=(player.object_y-jumpingf_y)+1;
				jumpingf_y=jumpingl_y;
			}
			if(player.object_y>=40){
				jump_flag=0;
				drop_flag=0;
				player.object_y=40;	
			}
		}
		Clear_Screen();
		draw_stage();
		player.draw_jump_object();
		if(wall.being==EXIST){
			wall.object_x-=4;
			wall.draw_jump_object();
			if(wall.object_x<12 && player.object_y>=32 && player.object_y<48) HP--;
		}
		if(wall2.being==EXIST){
			wall2.object_x-=4;
			wall2.draw_jump_object();
			if(wall2.object_x<12 && player.object_y>=32 && player.object_y<48) HP--;
		}
		if(lowwall.being==EXIST){
			lowwall.object_x-=4;
			lowwall.draw_jump_object();
			if(lowwall.object_x<12 && player.object_y>=40 && player.object_y<48) HP--;
		}
		if(lowwall2.being==EXIST){
			lowwall2.object_x-=4;
			lowwall2.draw_jump_object();
			if(lowwall2.object_x<12 && player.object_y>=40 && player.object_y<48) HP--;
		}
		if(highwall.being==EXIST){
			highwall.object_x-=4;
			highwall.draw_jump_object();
			if(highwall.object_x<12 && player.object_y>=24 && player.object_y<48) HP--;
		}
		if(highwall2.being==EXIST){
			highwall2.object_x-=4;
			highwall2.draw_jump_object();
			if(highwall2.object_x<12 && player.object_y>=24 && player.object_y<48) HP--;
		}
		if(bird.being==EXIST){
			bird.object_x-=4;
			bird.draw_jump_object();
			if(bird.object_x<12 && player.object_y>=bird.object_y && player.object_y<bird.object_y+8) HP--;
		}
		if(bird2.being==EXIST){
			bird2.object_x-=4;
			bird2.draw_jump_object();
			if(bird2.object_x<12 && player.object_y>=bird2.object_y && player.object_y<bird2.object_y+8) HP--;
		}
		if(gold.being>=EXIST){
			gold.object_x-=4;
			if(gold.being==EXIST)gold.object_y+=4;
			else gold.object_y-=4;
			if(gold.object_y<4) gold.being=1;
			else if(gold.object_y>=40) gold.being=2;
			gold.draw_jump_object();
			if(gold.object_x<12 && player.object_y>=gold.object_y && player.object_y<gold.object_y+8) HP--;
		}
		if(heart.being==EXIST){
			heart.object_x-=4;
			heart.draw_jump_object();
			if(heart.object_x<12 && player.object_y>=heart.object_y && player.object_y<heart.object_y+8){
				HP++;
				heart.being=NO;
			}
		}
		if(HP>4)HP=4;
		if(HP==0)gameover_flag=1;
		if(wall.object_x==0)wall.being=NO;
		if(wall2.object_x==0)wall2.being=NO;
		if(lowwall.object_x==0)lowwall.being=NO;
		if(lowwall2.object_x==0)lowwall2.being=NO;
		if(highwall.object_x==0)highwall.being=NO;
		if(highwall2.object_x==0)highwall2.being=NO;
		if(bird.object_x==0)bird.being=NO;
		if(bird2.object_x==0)bird2.being=NO;
		if(gold.object_x==0)gold.being=NO;
		if(heart.object_x==0)heart.being=NO;
		Glcd_Print_map();
		if(gameover_flag) getch();
		if(button_jadge(START)) start_button();
		else if(button_jadge(DOWN) && drop_flag==0){
			soundPlay(SOUND_TYPE_SHOT);
			drop_flag=1;
		}
		else if(button_jadge(A) && jump_flag==0){
			soundPlay(SOUND_TYPE_JUMP);
			jump_flag=1;
			player.being=3;
			jumpingf_y=player.object_y;
			player.object_y-=8;
		}
		if(jump_flag==0){
			if(score%6<2)player.being=0;
			else if(score%6>=2 && score%6<4) player.being=1;
			else player.being=2;
		}
//////////////////////////////////////////////////////////////////////////
		if(score%30==0 || (score>500 && score<2000 && score%15==0 && rand()%3==0) || (score>2500 && score%15==0) || (score%10==0 && score>3000)){
			while(1){
				if(score<1000)exist=rand()%8;
				else exist=rand()%8+1;
				if(exist==0 && wall.being==NO){
					wall.being=EXIST;
					wall.object_x=124;
					break;
				}
				else if(exist==1 && wall.being==NO){
					wall2.being=EXIST;
					wall2.object_x=124;
					break;
				}
				else if(exist==2 && lowwall.being==NO){
					lowwall.being=EXIST;
					lowwall.object_x=124;
					break;
				}
				else if(exist==3 && lowwall2.being==NO){
					lowwall2.being=EXIST;
					lowwall2.object_x=124;
					break;
				}
				else if(exist==4 && highwall.being==NO){
					highwall.being=EXIST;
					highwall.object_x=124;
					break;
				}
				else if(exist==5 && highwall2.being==NO){
					highwall2.being=EXIST;
					highwall2.object_x=124;
					break;
				}
				else if(exist==6 && bird.being==NO){
					bird.being=EXIST;
					bird.object_x=120;
					bird.object_y=rand()%30+10;
					break;
				}
				else if(exist==7 && bird2.being==NO){
					bird2.being=EXIST;
					bird2.object_x=120;
					bird2.object_y=rand()%30+10;
					break;
				}
				else if(exist==8 && gold.being==NO){
					gold.being=EXIST;
					gold.object_x=120;
					gold.object_y=rand()%30+10;
					break;
				}		
			}
		}
			/*if(score%500==0 && score>=2000){
				heart.being=EXIST;
				heart.object_x=120;
				heart.object_y=rand()%30+10;
				break;
			}*/
//////////////////////////////////////////////////////////////////////////
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
	/*for(uint8_t i=1;i<4;i++){
		if(i<HP)Glcd_graphic((i-2)*8,56,0x6b,PUT,0);
		else Glcd_graphic((i-2)*8,56,0x20,PUT,0);
	}*/
 }