/*
 * BATTLE.cpp
 *
 * Created: 2018/06/27 16:25:32
 *  Author: kodai
 */ 

 #include <avr/io.h>
 #include <stdlib.h>//乱数用
 #include <avr/interrupt.h>
 #include <avr/pgmspace.h>
 //PROGMEMで配置
 //pgm_read_byte(&変数);で呼び出し
 #include "defines.h"
 #include "Glcd_function.h"
 #include "sound.h"
 #include <util/delay.h>

 #define HITOBJECTS_MAX 20//当たり判定の限界数

 
 #define SWORD 0
 #define GUN 1
 #define SPEAR 2
 #define BRAW 3

 #define STOP 0
 #define JUMPING 1
 #define JUMPING_TWICE 2

 #define WALK1        1
 #define WALK2        2
 #define WALK3        3
 
 #define SHOOT_ZERO   4
 #define SHOOT_BEAM   5
 #define SHOOT_BOMB   6

 #define RAISE_SWING1 7
 #define RAISE_SWING2 8
 #define RAISE_SWING3 9
 #define RAISE_SWING4 10
 #define LOWER_SWING1 11
 #define LOWER_SWING2 12
 #define LOWER_SWING3 13
 #define LOWER_SWING4 14
 #define SLIDING1	  15
 #define SLIDING2	  16

 #define NORMAL_SPEAR1 17
 #define NORMAL_SPEAR2 18
 #define NORMAL_SPEAR3 19
 #define NORMAL_SPEAR4 20
 #define LONG_SPEAR1   21
 #define LONG_SPEAR2   22
 #define LONG_SPEAR3   23
 #define LONG_SPEAR4   24
 #define LONG_SPEAR5   25
 #define LONG_SPEAR6   26
 #define LONG_SPEAR7   27
 #define LONG_SPEAR8   28
 #define LONG_SPEAR9   29
 #define LONG_SPEAR10  30
 #define THROW_SPEAR1  31
 #define THROW_SPEAR2  32
 #define THROW_SPEAR3  33
 #define THROW_SPEAR4  34
 #define THROW_SPEAR5  35
 #define THROW_SPEAR6  36
 #define THROW_SPEAR7  37
 #define THROW_SPEAR8  38
 #define THROW_SPEAR9  39

 #define PUNCH		   40
 #define KICK		   41
 #define UPPER         42
 #define COUNTER	   43
 #define COUNTER_ATK   44//この辺りはモーション用

 #define NO_FLAGS 0
 #define DEAD_FLAG 1
 #define GAMEOVER_FLAG 2

 #define TYPE_SHOT 0
 #define TYPE_BOMB 1
 #define TYPE_EXPLOSIONS 2
 #define TYPE_SWORD_SWING 3
 #define TYPE_SHORT_SPEAR 4
 #define TYPE_LONG_SPEAR  5
 #define TYPE_THROW_SPEAR 6
 #define TYPE_PUNCH		  7
 #define TYPE_KICK		  8
 #define TYPE_COUNTER	  9
 #define TYPE_UPPER		  10
 #define TYPE_WAVE		  11//当たり判定の種類
 
 #define NOT_CP 0
 #define EASY   1
 #define NORMAL 2
 #define HARD   3
 #define IMMPOSIBBLE 4

 typedef struct {
	 volatile int16_t	x;
	 volatile int16_t	y;
	 volatile uint8_t	reach_x;
	 volatile uint8_t	reach_y;
	 volatile int8_t	vx;
	 volatile int8_t	vy;
	 volatile uint8_t	type;
	 volatile uint8_t	counter;
	 volatile uint8_t	isAlive;
 } HITOBJECTS;//当たり判定の構造体
 
 static HITOBJECTS Hitobjects[HITOBJECTS_MAX];

 void init_objects();
 void set_objects(int16_t x,int16_t y,int8_t reach_x,int8_t reach_y,int8_t vx,int8_t vy,uint8_t type,uint8_t isAlive,uint8_t counter);
 void move_objects();
 void draw_objects();

 class PLAYER
 {
	public:
		PLAYER();
		void set(uint8_t p_type,uint8_t dir,int16_t sx,uint8_t head);
		void action(uint8_t button);
		void move();
		void hit();
		void graphic();
		uint8_t playertype(){
			return player_type;
		}
		volatile int16_t hp;
		volatile uint8_t flags;
		volatile uint8_t life;
		volatile int16_t x;
		volatile int8_t y;
		volatile int8_t vx;
		volatile int8_t vy;
	private:
		volatile uint8_t direction;
		volatile uint8_t action_type;
		volatile int8_t action_time;
		volatile uint8_t player_type;
		volatile uint8_t dropt;
		volatile uint8_t jump_f;
		volatile uint8_t keep_jump;
		volatile uint8_t player_head;
		volatile uint8_t player_rand;
 };

 PLAYER::PLAYER(){
	hp = 0;
	action_time = 0;
	action_type = 0;
	jump_f = STOP;
	direction = RIGHT;
	life = 3;
	flags = NO_FLAGS;
	vx = 0;
	vy = 0;
 }
 //コンストラクタ
 void PLAYER::set(uint8_t p_type,uint8_t dir,int16_t sx,uint8_t head){
	hp = 0;
	action_time = 0;
	action_type = 0;
	jump_f = STOP;
	direction = RIGHT;
	life = 3;
	flags = NO_FLAGS;
	vx = 0;
	vy = 0;
	 player_type = p_type;
	 direction = dir;
	 x = sx;
	 player_head = head;
	 player_rand = 16 * (rand() % 4);
 }

 void PLAYER::action(uint8_t button){
	uint8_t right_left = 0;
	if((button>>LEFT & 0b00000001 == 1 || button>>RIGHT & 0b00000001 == 0) && action_time == 0){
		direction = LEFT;
		vx -= 2;
		if     (action_type == STOP ) action_type = WALK1;
		else if(action_type == WALK2) action_type = WALK3;
		else if(action_type == WALK3) action_type = WALK1;
		else if(action_type == WALK1) action_type = WALK2;
	}
	else if((button>>LEFT & 0b00000001 == 0 || button>>RIGHT & 0b00000001 == 1) && action_time == 0){
		direction = RIGHT;
		vx += 2;
		if     (action_type == STOP ) action_type = WALK1;
		else if(action_type == WALK2) action_type = WALK3;
		else if(action_type == WALK3) action_type = WALK1;
		else if(action_type == WALK1) action_type = WALK2;
	}
	if(action_type <= WALK3  && jump_f == STOP && button>>LEFT & 0b00000001 == 0 && button>>RIGHT & 0b00000001 == 0) action_type = STOP;

	if(player_type == SPEAR) vx /= 2;
	else if(player_type == BRAW) vx *= 1.5;

	if(direction == RIGHT) right_left = 0;
	else if(direction == LEFT) right_left = 1;

	if(button>>A & 0b00000001 == 1 && action_type <= WALK3 && jump_f != JUMPING_TWICE){
		jump_f = JUMPING_TWICE;
		dropt = 0;
		vy = 0;
		soundPlay(SOUND_TYPE_JUMP);
	}

	if(button>>A & 0b00000001 == 1) keep_jump = 1;
	else if(button>>A & 0b00000001 == 0) keep_jump = 0;


	if(jump_f != STOP && dropt < 5){
		dropt += 1;
		vy -= 4;
		if(button>>DOWN & 0b00000001 == 1) vy += 1;
	}
	
	vy += 1;//重力
	if(player_type == SPEAR) vy += 1;
	move();

	if(player_type == SWORD){
		if      (action_type == RAISE_SWING1){
			action_type = RAISE_SWING2;
			set_objects(x+(-16)*(right_left)+8,y+8,7,7,0,0,TYPE_SWORD_SWING,1,0);
		}
		else if (action_type == RAISE_SWING2){
			action_type = RAISE_SWING3;
			set_objects(x+(-16)*(right_left)+8,y,7,7,0,0,TYPE_SWORD_SWING,1,0);
		}
		else if (action_type == RAISE_SWING3){
			action_type = RAISE_SWING4;
			set_objects(x+(-16)*(right_left)+8,y,7,7,0,0,TYPE_SWORD_SWING,1,0);
		}
		else if (action_type == RAISE_SWING4) action_type = STOP;

		if      (action_type == LOWER_SWING1){
			action_type = LOWER_SWING2;
			set_objects(x+(-16)*(right_left)+8,y+8,7,7,0,0,TYPE_SWORD_SWING,1,0);
		}
		else if (action_type == LOWER_SWING2){
			action_type = LOWER_SWING3;
			set_objects(x+(-16)*(right_left)+8,y+8,7,7,0,0,TYPE_SWORD_SWING,1,0);
		}
		else if (action_type == LOWER_SWING3){
			action_type = LOWER_SWING4;
			set_objects(x+(-16)*(right_left)+8,y+8,7,7,0,0,TYPE_SWORD_SWING,1,0);
		}
		else if (action_type == LOWER_SWING4){
			action_type = STOP;
		}

		if(action_type == SLIDING1){
			if(action_time == 0) action_type = SLIDING2;
			else vx = 4 + (-8)*right_left;
			set_objects(x+(-16)*(right_left)+8,y+8,7,7,0,0,TYPE_SWORD_SWING,1,0);
		}
		else if(action_type == SLIDING2){
			action_type = STOP;
		}

		if (button>>B & 0b00000001 == 1 && action_time == 0 && action_type <= WALK3 && jump_f != JUMPING_TWICE){
			if(button>>DOWN & 0b00000001 == 1){
				action_type = SLIDING1;
				action_time = 8;
				vx = 2 + (-4)*right_left;
			}
			else if(button>>UP & 0b00000001 == 1){
				action_type = RAISE_SWING1;
				jump_f = JUMPING_TWICE;
				set_objects(x+(-16)*(right_left)+8,y,7,7,0,0,TYPE_SWORD_SWING,1,0);
			}
			else if(jump_f != JUMPING_TWICE){
				action_type = LOWER_SWING1;
				set_objects(x+(-16)*(right_left)+8,y,7,7,0,0,TYPE_SWORD_SWING,1,0);
			}
		}

	}
	else if(player_type == GUN){
		if(action_type != 0) action_type = STOP;
		if (button>>B & 0b00000001 == 1 && action_time == 0 && action_type <= WALK3){
			if(button>>DOWN & 0b00000001 == 1){
				action_type = SHOOT_ZERO;
				soundPlay(SOUND_TYPE_BOMB);
				set_objects(x+(-16)*(right_left)+8,y+3,7,7,2+(-4)*right_left,0,TYPE_EXPLOSIONS,8,0);
				action_time = 4;
			}
			else if(button>>UP & 0b00000001 == 1){
				action_type = SHOOT_BOMB;
				set_objects(x+(-16)*(right_left)+8,y+3,7,7,(-4)*(right_left)+2,2,TYPE_BOMB,10,0);
				action_time = 8;
				soundPlay(SOUND_TYPE_SHOT);
			}
			else{
				action_type = SHOOT_BEAM;
				set_objects(x+(-16)*(right_left)+8,y+7,2,2,(-6)*(right_left)+3,0,TYPE_SHOT,16,0);
				action_time = 3;
				soundPlay(SOUND_TYPE_SHOT);
			}
		}

	}
	else if(player_type == SPEAR){
		if      (action_type == NORMAL_SPEAR1){
			action_type = NORMAL_SPEAR2;
		}
		else if (action_type == NORMAL_SPEAR2){
			action_type = NORMAL_SPEAR3;
			set_objects(x+(-16)*(right_left)+8,y+8,7,4,0,0,TYPE_SHORT_SPEAR,1,0);
			set_objects(x+(-32)*(right_left)+16,y+8,7,4,0,0,TYPE_SHORT_SPEAR,1,0);
		}
		else if (action_type == NORMAL_SPEAR3) action_type = STOP;

		if      (action_type == LONG_SPEAR1){
			action_type = LONG_SPEAR2;
		}
		else if      (action_type == LONG_SPEAR2){
			action_type = LONG_SPEAR3;
		}
		else if      (action_type == LONG_SPEAR3){
			action_type = LONG_SPEAR4;
		}
		else if (action_type == LONG_SPEAR4){
			action_type = LONG_SPEAR5;
			set_objects(x+(-16)*(right_left)+8,y+8,7,4,0,0,TYPE_LONG_SPEAR,1,0);
			set_objects(x+(-32)*(right_left)+16,y+8,7,4,0,0,TYPE_LONG_SPEAR,1,0);
		}
		else if (action_type == LONG_SPEAR5){
			action_type = LONG_SPEAR6;
			set_objects(x+(-16)*(right_left)+8,y+6,7,4,0,0,TYPE_LONG_SPEAR,1,0);
			set_objects(x+(-32)*(right_left)+16,y+6,7,4,0,0,TYPE_LONG_SPEAR,1,0);
		}
		else if (action_type == LONG_SPEAR6){
			action_type = LONG_SPEAR7;
			set_objects(x+(-16)*(right_left)+8,y+6,7,4,0,0,TYPE_LONG_SPEAR,1,0);
			set_objects(x+(-32)*(right_left)+16,y+6,7,4,0,0,TYPE_LONG_SPEAR,1,0);
		}
		else if (action_type == LONG_SPEAR7){
			action_type = LONG_SPEAR8;
			set_objects(x+(-32)*(right_left)+16,y+6,7,4,0,0,TYPE_LONG_SPEAR,1,0);
			set_objects(x+(-42)*(right_left)+21,y+6,7,4,0,0,TYPE_LONG_SPEAR,1,0);
		}
		else if (action_type == LONG_SPEAR8) action_type = LONG_SPEAR9;
		else if (action_type == LONG_SPEAR9) action_type = LONG_SPEAR10;
		else if (action_type == LONG_SPEAR10) action_type = STOP;
		
		if(action_type == THROW_SPEAR1) action_type = THROW_SPEAR2;
		else if(action_type == THROW_SPEAR2) action_type = THROW_SPEAR3;
		else if(action_type == THROW_SPEAR3) action_type = THROW_SPEAR4;
		else if(action_type == THROW_SPEAR4) action_type = THROW_SPEAR5;
		else if(action_type == THROW_SPEAR5) action_type = THROW_SPEAR6;
		else if(action_type == THROW_SPEAR6){
			set_objects(x+(-16)*(right_left)+8,y+8,7,4,0,0,TYPE_SHORT_SPEAR,1,0);
			set_objects(x+(-32)*(right_left)+16,y+8,7,4,0,0,TYPE_SHORT_SPEAR,1,0);
			action_type = THROW_SPEAR7;
		}
		else if(action_type == THROW_SPEAR7){
			set_objects(x+(-32)*(right_left)+16,y+6,15,7,4+(-8)*(right_left),(-6),TYPE_THROW_SPEAR,16,player_rand);
			action_type = THROW_SPEAR8;
		}
		else if(action_type == THROW_SPEAR8) action_type = THROW_SPEAR9;
		else if(action_type == THROW_SPEAR9) action_type = STOP;

		if (button>>B & 0b00000001 == 1 && action_time == 0 && action_type <= WALK3){
			if(button>>DOWN & 0b00000001 == 1){
				action_type = THROW_SPEAR1;
				action_time = 10;
			}
			else if(button>>UP & 0b00000001 == 1){
				action_type = LONG_SPEAR1;
				action_time = 14;
			}
			else if(jump_f != JUMPING_TWICE){
				action_type = NORMAL_SPEAR1;
				action_time = 6;
			}
		}

	}
	else if(player_type == BRAW){
		if(action_type == COUNTER_ATK && action_time == 7){
			if(button>>LEFT & 0b00000001 == 1 || button>>RIGHT & 0b00000001 == 0){
				direction = LEFT;
			}
			else if(button>>LEFT & 0b00000001 == 0 || button>>RIGHT & 0b00000001 == 1){
				direction = RIGHT;
			}
		}
		if(action_type == KICK){
			vx = 4 + (-8)*right_left;
			set_objects(x+(-16)*(right_left)+8,y+3,7,7,0,0,TYPE_KICK,1,0);
		}
		else if(action_type == COUNTER_ATK){
			vx = action_time + (-action_time)*right_left;
			set_objects(x+(-16)*(right_left)+8,y+3,7,7,0,0,TYPE_COUNTER,1,0);
		}
		if(action_type == UPPER){
			set_objects(x+(-4)*(right_left)+4,y-4,3,3,0,0,TYPE_UPPER,1,0);
		}
		if(action_type == PUNCH){
			set_objects(x+(-16)*(right_left)+12,y+3,3,3,3+(-6)*(right_left),(-3)+rand()%6,TYPE_WAVE,3,0);
		}

		if (button>>B & 0b00000001 == 1 && action_time == 0 && action_type <= WALK3){
			if(button>>DOWN & 0b00000001 == 1){
				action_type = KICK;
				set_objects(x+(-16)*(right_left)+8,y+3,7,7,0,0,TYPE_KICK,1,0);
				action_time = 6;
				soundPlay(SOUND_TYPE_BOMB);
				vy = 0;
				set_objects(x+(-16)*(right_left)+8,y+3,7,7,0,0,TYPE_KICK,1,0);
			}
			else if(button>>UP & 0b00000001 == 1 && jump_f != JUMPING_TWICE){
				action_type = UPPER;
				jump_f = JUMPING_TWICE;
				action_time = 8;
				set_objects(x+(-4)*(right_left)+4,y-4,3,3,0,0,TYPE_UPPER,1,0);
				soundPlay(SOUND_TYPE_1UP);
			}
			else{
				action_type = PUNCH;
				set_objects(x+(-16)*(right_left)+8,y+5,7,7,0,0,TYPE_PUNCH,1,0);
				action_time = 2;
				soundPlay(SOUND_TYPE_SHOT);
			}
		}

	}
	if(action_time > 0){
		action_time--;
		if(action_time == 0) action_type = STOP;
	}
 }

 void PLAYER::graphic(){
	uint8_t right_left = 0;
	if(direction == RIGHT) right_left = 0;
	else if(direction == LEFT) right_left = 1;

	 if(player_type == GUN){
		 Glcd_graphic(x,y,player_head,OR,right_left);
		 if     (action_type == STOP ) Glcd_graphic(x  ,y+8,0x01,OR,right_left);
		 else if(action_type == WALK1) Glcd_graphic(x  ,y+8,0x02,OR,right_left);
		 else if(action_type == WALK2) Glcd_graphic(x  ,y+8,0x03,OR,right_left);
		 else if(action_type == WALK3) Glcd_graphic(x  ,y+8,0x04,OR,right_left);
		 else if((action_type == SHOOT_BEAM && action_time == 2) || (action_type == SHOOT_BOMB && action_time == 4) || (action_type == SHOOT_ZERO && action_time == 7)){
									   Glcd_graphic(x  ,y+8,0x05,OR,right_left);
		 }
		 else						   Glcd_graphic(x  ,y+8,0x01,OR,right_left);
	 }
	 else if (player_type == SWORD){
		 Glcd_graphic(x,y,player_head,OR,right_left);

		 if(action_type <= WALK3){

			if     (action_type == STOP ) Glcd_graphic(x  ,y+8,0xa8,OR,right_left);
			else if(action_type == WALK1) Glcd_graphic(x  ,y+8,0xa9,OR,right_left);
			else if(action_type == WALK2) Glcd_graphic(x  ,y+8,0xaa,OR,right_left);
			else if(action_type == WALK3) Glcd_graphic(x  ,y+8,0xab,OR,right_left);

			Glcd_graphic(x+8-16*right_left,y+8,0xa4,OR,right_left);
		 
		 }
		 else{

			if(action_type == RAISE_SWING1 || action_type == LOWER_SWING1){
				Glcd_graphic(x,y+8,0xa0,OR,right_left);
				
				Glcd_graphic(x+8-16*right_left,y  ,0x91,OR,right_left);
				Glcd_graphic(x+8-16*right_left,y+8,0xa1,OR,right_left);
				
				if(action_type == RAISE_SWING1){
					Glcd_graphic(x+8-16*right_left,y+6,0xa2,OR,right_left);
				}
				else if(action_type == LOWER_SWING1){
					Glcd_graphic(x+8-16*right_left,y+1,0x92,OR,right_left);
				}
			}
			else if(action_type == RAISE_SWING2 || action_type == RAISE_SWING3 || action_type == RAISE_SWING4){
				Glcd_graphic(x,y+8,0x93,OR,right_left);
				
				Glcd_graphic(x+8-16*right_left,y  ,0x94,OR,right_left);
				
				if(action_type == RAISE_SWING2){
					Glcd_graphic(x+8-16*right_left,y,0xa6,OR,right_left);
				}
				else if(action_type == RAISE_SWING3){
					Glcd_graphic(x+8-16*right_left,y,0xa7,OR,right_left);
				}
			}
			else if(action_type == LOWER_SWING2 || action_type == LOWER_SWING3 || action_type == LOWER_SWING4){
				Glcd_graphic(x,y+8,0xa3,OR,right_left);
				
				Glcd_graphic(x+8-16*right_left,y+8,0xa4,OR,right_left);
				
				if(action_type == LOWER_SWING2){
					Glcd_graphic(x+8-16*right_left,y+8,0x95,OR,right_left);
				}
				else if(action_type == LOWER_SWING3){
					Glcd_graphic(x+8-16*right_left,y+8,0xa5,OR,right_left);
				}
			}
			else if(action_type == SLIDING1 || action_type == SLIDING2){
				Glcd_graphic(x,y+8,0xac,OR,right_left);
				
				Glcd_graphic(x+8-16*right_left,y+8,0xad,OR,right_left);
				
				if(action_type == SLIDING1){
					Glcd_graphic(x+8-16*right_left,y+8,0xae,OR,right_left);
				}
			}
		 }

	 }
	 else if (player_type == SPEAR){
		 Glcd_graphic(x,y,player_head,OR,right_left);

		 if(action_type <= WALK3){

			 if     (action_type == STOP ) Glcd_graphic(x  ,y+8,0xb0,OR,right_left);
			 else if(action_type == WALK1) Glcd_graphic(x  ,y+8,0xb1,OR,right_left);
			 else if(action_type == WALK2) Glcd_graphic(x  ,y+8,0xb2,OR,right_left);
			 else if(action_type == WALK3) Glcd_graphic(x  ,y+8,0xb3,OR,right_left);

			 Glcd_graphic(x,y+8,0xaf,OR,right_left);
			 Glcd_graphic(x+8-16*right_left,y+8,0xbf+player_rand,OR,right_left);
			 
		 }
		 else{

			 if(action_type == NORMAL_SPEAR1 || action_type == LONG_SPEAR1 || action_type == NORMAL_SPEAR2 || action_type == LONG_SPEAR2 || action_type == THROW_SPEAR1 || action_type == THROW_SPEAR2 || action_type == THROW_SPEAR3){
				 Glcd_graphic(x,y+8,0xb4,OR,right_left);
				 
				 Glcd_graphic(x-4+8*right_left,y+8,0xaf,OR,right_left);
				 Glcd_graphic(x+4-8*right_left,y+8,0xbf+player_rand,OR,right_left);
			 }
			 else if(action_type == THROW_SPEAR4 || action_type == THROW_SPEAR5 || action_type == THROW_SPEAR6){
				Glcd_graphic(x,y+8,0xc7,OR,right_left);

				Glcd_graphic(x-8+16*right_left,y+8,0xaf,OR,right_left);
				Glcd_graphic(x,y+8,0xbf+player_rand,OR,right_left);
			 }
			 else if(action_type == THROW_SPEAR8 || action_type == THROW_SPEAR9){
				Glcd_graphic(x,y+8,0xb6,OR,right_left);
			 }
			 else if(action_type == NORMAL_SPEAR3 || action_type == LONG_SPEAR5 || action_type == THROW_SPEAR7){
				 Glcd_graphic(x,y+8,0xb5,OR,right_left);
				 
				 Glcd_graphic(x+4-8*right_left,y+8,0xaf,OR,right_left);
				 Glcd_graphic(x+8-16*right_left,y+8,0xbe,OR,right_left);
				 Glcd_graphic(x+12-24*right_left,y+8,0xbf+player_rand,OR,right_left);
			 }
			 else if(action_type == NORMAL_SPEAR4 || action_type == LONG_SPEAR10){
				 Glcd_graphic(x,y+8,0xb5,OR,right_left);
				 
				 Glcd_graphic(x+4-8*right_left,y+8,0xaf,OR,right_left);
				 Glcd_graphic(x+12-24*right_left,y+8,0xbf+player_rand,OR,right_left);
			 }
			 else if(action_type == LONG_SPEAR3 || action_type == LONG_SPEAR4){
				 Glcd_graphic(x,y+8,0xb7,OR,right_left);
				 
				 Glcd_graphic(x-7+14*right_left,y+8,0xaf,OR,right_left);
				 Glcd_graphic(x+1-2*right_left,y+8,0xbf+player_rand,OR,right_left);
			 }
			 else if(action_type == LONG_SPEAR6 || action_type == LONG_SPEAR7 || action_type == LONG_SPEAR8 || action_type == LONG_SPEAR9){
				 Glcd_graphic(x,y+8,0xb6,OR,right_left);
				 if(action_type == LONG_SPEAR6){
					Glcd_graphic(x+8-16*right_left,y+6,0xaf,OR,right_left);
					Glcd_graphic(x+16-32*right_left,y+6,0xbf+player_rand,OR,right_left);
					Glcd_graphic(x+12-24*right_left,y+6,0xbe,OR,right_left);
				 }
				 else{
					Glcd_graphic(x+8-16*right_left,y+6,0xce,OR,right_left);
					Glcd_graphic(x+16-32*right_left,y+6,0xaf,OR,right_left);
					Glcd_graphic(x+24-48*right_left,y+6,0xbf+player_rand,OR,right_left);
					if(action_type == LONG_SPEAR7){
						Glcd_graphic(x+19-38*right_left,y+6,0xbe,OR,right_left);
					}
				 }
			 }
			 
		 }

	 }
	 else if (player_type == BRAW){
		 
		 if(action_type <= WALK3){
			 Glcd_graphic(x,y,player_head,OR,right_left);
			 if     (action_type == STOP ) Glcd_graphic(x  ,y+8,0xb8,OR,right_left);
			 else if(action_type == WALK1) Glcd_graphic(x  ,y+8,0xb9,OR,right_left);
			 else if(action_type == WALK2) Glcd_graphic(x  ,y+8,0xba,OR,right_left);
			 else if(action_type == WALK3) Glcd_graphic(x  ,y+8,0xbb,OR,right_left);
			 
		 }
		 else{

			 if(action_type == PUNCH){
				if(action_time == 1){
					Glcd_graphic(x+3-6*right_left,y,player_head,OR,right_left);
					Glcd_graphic(x,y+8,0xca,OR,right_left);
					Glcd_graphic(x+8-16*right_left,y+8,0xcb,OR,right_left);
				}
				else{
					Glcd_graphic(x,y,player_head,OR,right_left);
					Glcd_graphic(x  ,y+8,0xb8,OR,right_left);
				}
			 }
			 else if(action_type == KICK || action_type == COUNTER_ATK){
				 Glcd_graphic(x,y,player_head,OR,right_left);

				 Glcd_graphic(x,y+8,0xcc,OR,right_left);
				 
				 Glcd_graphic(x+8-16*right_left,y,0xbd,OR,right_left);
				 Glcd_graphic(x+8-16*right_left,y+8,0xcd,OR,right_left);
				 
			 }
			 else if(action_type == COUNTER){
				Glcd_graphic(x,y,player_head,OR,right_left);
				Glcd_graphic(x,y+8,0xbc,OR,right_left);
			 }
			 else if(action_type == UPPER){
				 Glcd_graphic(x,y,player_head,OR,right_left);
				 Glcd_graphic(x,y,0xdc,OR,right_left);
				 Glcd_graphic(x,y+8,0xdd,OR,right_left);
			 }
		 }

	 }
 }

 void PLAYER::move(){
	 int8_t i,j;

	 if((y+16 < 56) || ( (y+16 >= 56) && (x+7 < 16 || x > 111) )){
	 	if(jump_f == STOP) jump_f = JUMPING;
	 	dropt = 5;
	 }

	 if(x < -15 || x >= 150 || y < -15 || y >= 80){
	 	jump_f = STOP;
	 	flags = DEAD_FLAG;
		life -= 1;
		if(life <= 0) flags = GAMEOVER_FLAG;
		x = 60;
		y = 0;
		vx = 0;
		vy = 0;
		hp = 0;
	 }
	 if(vy > 0){
		 for(j = 0;j < abs(vy); j++){
			if(y+j+16 == 56 && ((x+vx > 16 && x+vx < 111) || (x+vx+8 > 16 && x+vx+8 < 111))){
				jump_f = STOP;
				dropt = 0;
				vy = j;
				break;
		 	}
		}
	 }
	 y += vy;
	 x += vx;
	 vx /= 2;
	 return;
 }
 
 void PLAYER::hit(){
	uint8_t hit = 0;
	int16_t damage = 0;
	for (uint8_t i=0;i<HITOBJECTS_MAX;i++){
		if(Hitobjects[i].isAlive > 0){
			if((Hitobjects[i].x + Hitobjects[i].reach_x >= x && Hitobjects[i].x + Hitobjects[i].reach_x < x + 8) || (Hitobjects[i].x >= x && Hitobjects[i].x < x + 8)){
				if((Hitobjects[i].y + Hitobjects[i].reach_y >= y && Hitobjects[i].y + Hitobjects[i].reach_y < y + 16) || (Hitobjects[i].y >= y && Hitobjects[i].y < y + 16)){
					hit = 1;
					if(Hitobjects[i].type == TYPE_SHOT){
						damage = 4;
					}
					else if(Hitobjects[i].type == TYPE_SWORD_SWING){
						damage = 6;
					}
					else if(Hitobjects[i].type == TYPE_EXPLOSIONS){
						damage += 8;
					}
					else if(Hitobjects[i].type == TYPE_SHORT_SPEAR){
						damage += 6;
					}
					else if(Hitobjects[i].type == TYPE_LONG_SPEAR){
						damage += 6;
					}
					else if(Hitobjects[i].type == TYPE_THROW_SPEAR){
						damage += 6;
						Hitobjects[i].vy = 0;
					}
					else if(Hitobjects[i].type == TYPE_PUNCH){
						damage += 4;
					}
					else if(Hitobjects[i].type == TYPE_KICK){
						damage += 5;
					}
					else if(Hitobjects[i].type == TYPE_COUNTER){
						damage += 8;
					}
					else if(Hitobjects[i].type == TYPE_UPPER){
						if(hp > 0){
							damage += 10;
							vy = 0;
							vy -= 5;
						}
					}
					else if(Hitobjects[i].type == TYPE_WAVE){
						damage += 3;
					}

					if(action_type == COUNTER && action_time >= 3){
						action_type == COUNTER_ATK;
						action_time = 7;
						damage = 0;
					}
					else if(action_type == COUNTER_ATK){
						if(action_time < 3) hp += damage;
						damage = 0;
					}
					else{
						hp += damage;
						if(damage > 0)soundPlay(SOUND_TYPE_DAMAGE);
					}
					if(damage > 4){
						jump_f = JUMPING_TWICE;
						if(player_type == SPEAR) vy -= Hitobjects[i].vy + hp / 40;
						if(player_type == BRAW) vy -= hp / 20;
						else vy -= Hitobjects[i].vy + hp / 30;
					}

					if(Hitobjects[i].type != TYPE_BOMB)Hitobjects[i].isAlive = 0;
					if(hp > 999) hp = 999;
					
				}
			}
		}
	}
 }
  //プレイヤーのクラス
  PLAYER player1,player2;

 void graph_battlefield(){
	 DrawRectangle(16,56,111,63);
	 DrawRectangle(17,57,110,62);
 }
 

 void init_objects(){
	for (uint8_t i=0;i<HITOBJECTS_MAX;i++){
		Hitobjects[i].x = 0;
		Hitobjects[i].y = 0;
		Hitobjects[i].reach_x = 0;
		Hitobjects[i].reach_y = 0;
		Hitobjects[i].vx = 0;
		Hitobjects[i].vy = 0;
		Hitobjects[i].type = 0;
		Hitobjects[i].counter = 0;
		Hitobjects[i].isAlive = 0;
	}
 }

 void set_objects(int16_t x,int16_t y,int8_t reach_x,int8_t reach_y,int8_t vx,int8_t vy,uint8_t type,uint8_t isAlive,uint8_t counter){
	for (uint8_t i=0;i<HITOBJECTS_MAX;i++){
		if(Hitobjects[i].isAlive == 0){
			Hitobjects[i].x = x;
			Hitobjects[i].y = y;
			Hitobjects[i].reach_x = reach_x;
			Hitobjects[i].reach_y = reach_y;
			Hitobjects[i].vx = vx;
			Hitobjects[i].vy = vy;
			Hitobjects[i].type = type;
			Hitobjects[i].counter = counter;
			Hitobjects[i].isAlive = isAlive;
			break;
		}
	}
 }

 void move_objects(){
	uint8_t i,j;
	j=0;
	for (i=0;i<HITOBJECTS_MAX;i++){
		if(Hitobjects[i].isAlive > 0){
			j++;
			if(Hitobjects[i].x + Hitobjects[i].vx <= -8 || Hitobjects[i].x + Hitobjects[i].vx >= 136 || Hitobjects[i].y + Hitobjects[i].vy <= -8 || Hitobjects[i].y + Hitobjects[i].vy >= 72){
				Hitobjects[i].isAlive = 0;
			}
			else{
				Hitobjects[i].x += Hitobjects[i].vx;
				if(Hitobjects[i].type == TYPE_BOMB){
					if(Hitobjects[i].vy > 0){
						for(uint8_t j = 0;j < abs(Hitobjects[i].vy); j++){
							if(Hitobjects[i].y+j+8 == 56 && ((Hitobjects[i].x+Hitobjects[i].vx > 16 && Hitobjects[i].x+Hitobjects[i].vx < 111) || (Hitobjects[i].x+Hitobjects[i].vx+8 > 16 && Hitobjects[i].x+Hitobjects[i].vx+8 < 111))){
								Hitobjects[i].vy = j;
								Hitobjects[i].vx = 0;
								break;
							}
						}
					}
				}
				else if(Hitobjects[i].type == TYPE_THROW_SPEAR){
					Hitobjects[i].x += Hitobjects[i].vx;
					if(abs(Hitobjects[i].vx) >= 2)Hitobjects[i].vx /= 2;
					Hitobjects[i].y += Hitobjects[i].vy;
					Hitobjects[i].vy += 2;
				}
				else Hitobjects[i].y += Hitobjects[i].vy;
			}

			if(Hitobjects[i].x + Hitobjects[i].vx <= -8 || Hitobjects[i].x + Hitobjects[i].vx >= 136){
				Hitobjects[i].isAlive = 0;
			}
			else{
				Hitobjects[i].isAlive -= 1;
				if(Hitobjects[i].isAlive == 0 && Hitobjects[i].type == TYPE_BOMB){
					soundPlay(SOUND_TYPE_BOMB);
					Hitobjects[i].type = TYPE_EXPLOSIONS;
					Hitobjects[i].isAlive = 8;
				}
			}

		}
	}
	if(j==HITOBJECTS_MAX){
		init_objects();
	}
 }

 void draw_objects(){
	for (uint8_t i=0;i<HITOBJECTS_MAX;i++){
		if(Hitobjects[i].isAlive > 0){
			if(Hitobjects[i].type == TYPE_SHOT){
				if(Hitobjects[i].x >= 1 && Hitobjects[i].x < 127 && Hitobjects[i].y >= 1 && Hitobjects[i].y < 63){
					DrawDot(Hitobjects[i].x-1,Hitobjects[i].y);
					DrawDot(Hitobjects[i].x+1,Hitobjects[i].y);
					DrawDot(Hitobjects[i].x,Hitobjects[i].y+1);
					DrawDot(Hitobjects[i].x,Hitobjects[i].y-1);
				}
			}
			else if(Hitobjects[i].type == TYPE_BOMB){
				if(Hitobjects[i].x > 7 && Hitobjects[i].x < 128 && Hitobjects[i].y >= 0 && Hitobjects[i].y < 64){
					Glcd_graphic(Hitobjects[i].x,Hitobjects[i].y,0x6d,OR,0);
				}
				else{
					Glcd_graphic(0,Hitobjects[i].y,0x6d,OR,0);
				}
			}
			else if(Hitobjects[i].type == TYPE_EXPLOSIONS){
				if(Hitobjects[i].x > 7 && Hitobjects[i].x < 128 && Hitobjects[i].y >= 0 && Hitobjects[i].y < 64){
					Glcd_graphic(Hitobjects[i].x,Hitobjects[i].y,0x68 - Hitobjects[i].isAlive,OR,0);
				}
				else{
					Glcd_graphic(0,Hitobjects[i].y,0x60 - Hitobjects[i].isAlive,OR,0);
				}
			}
			else if(Hitobjects[i].type == TYPE_WAVE){
				if(Hitobjects[i].x > 7 && Hitobjects[i].x < 128 && Hitobjects[i].y >= 0 && Hitobjects[i].y < 64){
					if(Hitobjects[i].vx >= 0){
						Glcd_graphic(Hitobjects[i].x,Hitobjects[i].y,0xee,OR,0);
					}
					else Glcd_graphic(Hitobjects[i].x,Hitobjects[i].y,0xee,OR,1);
				}
				else{
					if(Hitobjects[i].vx >= 0){
						Glcd_graphic(0,Hitobjects[i].y,0xee,OR,0);
					}
					else Glcd_graphic(0,Hitobjects[i].y,0xee,OR,1);
				}
			}
			else if(Hitobjects[i].type == TYPE_THROW_SPEAR){
				if(Hitobjects[i].vx >= 0){
					Glcd_graphic(Hitobjects[i].x,Hitobjects[i].y,0xaf,OR,0);
					Glcd_graphic(Hitobjects[i].x+8,Hitobjects[i].y,0xbf+Hitobjects[i].counter,OR,0);
				}
				else{
					Glcd_graphic(Hitobjects[i].x+8,Hitobjects[i].y,0xaf,OR,1);
					Glcd_graphic(Hitobjects[i].x,Hitobjects[i].y,0xbf+Hitobjects[i].counter,OR,1);
				}
			}

		}
	}
 }

 uint8_t computer_action(char cplevel){
	uint8_t cp_button = 0;
	static uint8_t cp_action_time = 0;
	if(cplevel == EASY){
		if(cp_action_time > 0){
			if(player1.x+9 < player2.x){
				cp_button |= 1 << LEFT;
			}
			else if(player1.x-9 > player2.x){
				cp_button |= 1 << RIGHT;
			}
			if((player1.x-7 < player2.x && player2.x < player1.x) || (player1.x+7 < player2.x && player2.x < player1.x+15)){
				cp_button |= 1 << B;
				if (abs(player1.x-player2.x) <= 10){
					if(player2.playertype() == SWORD && rand()%20 == 0) cp_button |= 1 << UP;
					else if(player2.playertype() == GUN && rand()%8 == 0) cp_button |= 1 << DOWN;
				}
			}
			else{
				if(player2.playertype() == GUN && rand()%3 == 0) cp_button |= 1 << B;
				else if(player2.playertype() == SWORD && rand()%15 == 0) cp_button |= (1 << B) | (1 << DOWN);
			}
			cp_action_time--;
		}
		else if(rand()%20 == 0){
			cp_action_time = 20;
		}
	}
	else if(cplevel == NORMAL){
		if(cp_action_time > 0){
			if(player1.x+9 < player2.x && player2.x >= 17){
				if(player2.playertype() == GUN){
					if(player1.x+32 < player2.x)cp_button |= 1 << LEFT;
					else if(player1.x+32 >= player2.x)cp_button |= 1 << RIGHT;
				}
				else cp_button |= 1 << LEFT;
			}
			else if(player1.x-9 > player2.x && player2.x < 110){
				if(player2.playertype() == GUN){
					if(player1.x-24 > player2.x) cp_button |= 1 << RIGHT;
					else if(player1.x-24 <= player2.x) cp_button |= 1 << LEFT;
				}
				else cp_button |= 1 << RIGHT;
			}
			else if(player1.x > player2.x && player2.x >= 17){
				cp_button |= 1 << RIGHT;
			}
			else if(player1.x < player2.x && player2.x < 110){
				cp_button |= 1 << LEFT;
			}
			if((player1.x-8 < player2.x && player2.x < player1.x) || (player1.x+7 < player2.x && player2.x < player1.x+16)){
				cp_button |= 1 << B;
				if (abs(player1.x-player2.x) <= 10){
					if(player2.playertype() == SWORD || player2.playertype() == SPEAR) cp_button |= 1 << UP;
					else if(player2.playertype() == BRAW || player2.playertype() == GUN) cp_button |= 1 << DOWN;
				}
			}
			else{
				if(player2.playertype() == GUN && rand()%3 == 0){
					cp_button |= 1 << B;
					if(10 < abs(player1.x-player2.x) && abs(player1.x-player2.x) < 40) cp_button |= 1 << UP;
				}
				else if(player2.playertype() == SWORD && rand()%8 == 0) cp_button |= (1 << B) | (1 << DOWN);
			}
			cp_action_time--;
		}
		else if(rand()%5 == 0){
			cp_action_time = 30;
		}
	}
	else if(cplevel == HARD){

			if(player1.x+9 < player2.x && player2.x >= 17 && player2.x < 110){
				if(player2.playertype() == GUN){
					if(player1.x+32 < player2.x)cp_button |= 1 << LEFT;
					else if(player1.x+32 >= player2.x)cp_button |= 1 << RIGHT;
				}
				else cp_button |= 1 << LEFT;
			}
			else if(player1.x-9 > player2.x && player2.x >= 17 && player2.x < 110){
				if(player2.playertype() == GUN){
					if(player1.x-24 > player2.x) cp_button |= 1 << RIGHT;
					else if(player1.x-24 <= player2.x) cp_button |= 1 << LEFT;
				}
				else cp_button |= 1 << RIGHT;
			}
			else if(player1.x > player2.x && player2.x >= 17 && player2.x < 110){
				cp_button |= 1 << RIGHT;
			}
			else if(player1.x < player2.x && player2.x >= 17 && player2.x < 110){
				cp_button |= 1 << LEFT;
			}
			if((player1.x-8 < player2.x && player2.x < player1.x) || (player1.x+7 < player2.x && player2.x < player1.x+16)){
				cp_button |= 1 << B;
				if (abs(player1.x-player2.x) <= 10){
					if(player2.playertype() == SWORD || player2.playertype() == SPEAR) cp_button |= 1 << UP;
					else if(player2.playertype() == BRAW || player2.playertype() == GUN) cp_button |= 1 << DOWN;
				}
			}
			else{
				if(player2.playertype() == GUN && rand()%3 == 0){
					cp_button |= 1 << B;
					if(10 < abs(player1.x-player2.x) && abs(player1.x-player2.x) < 40) cp_button |= 1 << UP;
				}
				else if(player2.playertype() == SWORD && rand()%8 == 0) cp_button |= (1 << B) | (1 << DOWN);
			}
			cp_action_time--;
		
	}
	else if(cplevel == IMMPOSIBBLE){
			if(player1.x+9 < player2.x && player2.x >= 17 && player2.x < 110){
				cp_button |= 1 << LEFT;
			}
			else if(player1.x-9 > player2.x && player2.x >= 17  && player2.x < 110){
				cp_button |= 1 << RIGHT;
			}
			else if(player1.x > player2.x && player2.x >= 17 && player2.x < 110){
				cp_button |= 1 << RIGHT;
			}
			else if(player1.x < player2.x && player2.x >= 17  && player2.x < 110){
				cp_button |= 1 << LEFT;
			}
			
			if(player2.x < 16){
				cp_button = 1 << RIGHT;
			}
			else if(player2.x > 100){
				cp_button = 1 << LEFT;
			}

			if(16 < abs(player1.x - player2.x) && abs(player1.x - player2.x) <= 24){
				if(player1.x < player2.x && player2.x - 24 >= 17){
					cp_button |= 1 << LEFT;
					cp_button |= 1 << B;
					cp_button |= 1 << DOWN;
				}
				else if(player1.x >= player2.x && player2.x + 24 < 110){
					cp_button |= 1 << RIGHT;
					cp_button |= 1 << B;
					cp_button |= 1 << DOWN;
				}
			}
			else if(player1.x == player2.x && player1.y < player2.y){
				cp_button |= 1 << B;
				cp_button |= 1 << UPPER;
			}
			else if((player1.x-8 < player2.x && player2.x < player1.x) || (player1.x+7 < player2.x && player2.x < player1.x+16)){
				cp_button |= 1 << B;
			}
			
		}
	return cp_button;
 }

 void battle_main(){
	Clear_Screen();
	init_objects();
	volatile static char cursor1 = 0;
	volatile static char cursor2 = 0;
	volatile static char cplevel = 0;
	uint8_t i = 0;
	Glcd_str(0,0,"FIGHTER type");
	Glcd_Print_map();
	_delay_ms(2000);
	cursor1 = 0;
	cursor2 = 0;
	cplevel = 0;
	while(app_number == BATTLE){
		Clear_Screen();
		Glcd_str(18, 0,"1P");
		Glcd_str(82, 0,"2P");
		Glcd_str( 6,16," SWORD\n\r GUN\n\r SPEAR\n\r BRAW");
		Glcd_str(70,16," SWORD\n\r GUN\n\r SPEAR\n\r BRAW");
		
		Glcd_str(0,56,"PRESS SELECT:VS CP");

		if     (cursor1 < 10) Glcd_font(6,16+cursor1*8,'}'+1);
		else Glcd_font(6,16+(cursor1-10)*8,'*');

		if     (cursor2 < 10) Glcd_font(70,16+cursor2*8,'}'+1);
		else Glcd_font(70,16+(cursor2-10)*8,'*');

		if(button_jadge_one()==UP && cursor1 > 0 && cursor1 < 10){
			soundPlay(SOUND_TYPE_KEY);
			cursor1 -= 1;
		}
		else if(button_jadge_one()==DOWN && cursor1 < 3){
			soundPlay(SOUND_TYPE_KEY);
			cursor1 += 1;
		}
		else if(button_jadge_one()==A && cursor1 <= 3){
			soundPlay(SOUND_TYPE_ENTER);
			cursor1 += 10;
		}

		if(button_jadge_one()==SELECT){
			cursor2 = rand()%2;
			soundPlay(SOUND_TYPE_ENTER);
			cursor2 += 10;
			cplevel = EASY;
		}

		if(opponents_button_jadge_one()==UP && cursor2 > 0 && cursor2 < 10){
			soundPlay(SOUND_TYPE_KEY);
			cursor2 -= 1;
		}
		else if(opponents_button_jadge_one()==DOWN && cursor2 < 3){
			soundPlay(SOUND_TYPE_KEY);
			cursor2 += 1;
		}
		else if(opponents_button_jadge_one()==A && cursor2 <= 3){
			soundPlay(SOUND_TYPE_ENTER);
			cursor2 += 10;
		}
		if(cursor1 + cursor2 >= 20){
			player1.set(cursor1 - 10,RIGHT,60,0x00);
			player2.set(cursor2 - 10,LEFT, 60,0x90);
			_delay_ms(150);
			if(cplevel == EASY){
				Clear_Screen();
				Glcd_str(40,30,"VS EASY");
				Glcd_Print_map();
				_delay_ms(3000);
			}
			Clear_Screen();
			Glcd_str(40,30,"READY...");
			Glcd_Print_map();
			simpleSound(NOTE_A4,4);
			_delay_ms(1000);
			Clear_Screen();
			Glcd_str(52,30,"GO!");
			Glcd_Print_map();
			simpleSound(NOTE_A5,4);
			_delay_ms(500);
			break;
		}

		if(sig>>START & 0b00000001 == 1) start_button();
		Glcd_Print_map();
		_delay_ms(100);
	}
	

	 while(app_number == BATTLE){
		Clear_Screen();
		player1.hit();
		player2.hit();
		move_objects();
		player1.action(sig);
		
		if(cplevel > 0)player2.action(computer_action(cplevel));
		else player2.action(opponents_sig);

		if(sig>>START & 0b00000001 == 1) start_button();
		
		if(player1.y == player2.y){
			if(player1.x < player2.x && player2.x < player1.x+8){
				player1.vx -= 1;
				player2.vx += 1;
			}
			else if(player2.x < player1.x && player1.x < player2.x+8){
				player1.vx += 1;
				player2.vx -= 1;
			}
		}

		player1.graphic();
		player2.graphic();
		draw_objects();
		graph_battlefield();
		Glcd_str(0,0,"   ");
		Glcd_font(0,0,player1.hp/100%10+'0');
		Glcd_font(6,0,player1.hp/10%10+'0');
		Glcd_font(12,0,player1.hp%10+'0');
		Glcd_str(111,0,"   ");
		Glcd_font(111,0,player2.hp/100%10+'0');
		Glcd_font(117,0,player2.hp/10%10+'0');
		Glcd_font(123,0,player2.hp%10+'0');
		for (i=0;i<player1.life;i++){
			Glcd_graphic(54-i*8,0,0x6a,OR,0);
		}
		for (i=0;i<player2.life;i++){
			Glcd_graphic(66+i*8,0,0x6a,OR,0);
		}
		if(player1.flags == DEAD_FLAG){
			if(cplevel > 0) Glcd_str(0,0,"YOU DIED");
			else Glcd_str(0,0,"1P DIED");
		}
		if(player2.flags == DEAD_FLAG){
			if(cplevel > 0) Glcd_str(86,0,"CP DIED");
			else Glcd_str(86,0,"2P DIED");
		}
		if(player1.flags == GAMEOVER_FLAG && player2.flags == GAMEOVER_FLAG){
			Glcd_str(52,0,"DRAW");
		}
		else if(player1.flags == GAMEOVER_FLAG){
			if(cplevel > 0) Glcd_str(29,46,"YOU LOSE");
			else Glcd_str(29,46,"2P WIN");
		}
		else if(player2.flags == GAMEOVER_FLAG){
			if(cplevel > 0) Glcd_str(29,46,"YOU WIN");
			else Glcd_str(29,46,"1P WIN");
		}
		Glcd_Print_map();
		if(player1.flags == DEAD_FLAG || player2.flags == DEAD_FLAG){
			if(player1.flags == DEAD_FLAG) player1.flags = NO_FLAGS;
			if(player2.flags == DEAD_FLAG) player2.flags = NO_FLAGS;
			_delay_ms(1000);
		}
		if(player1.flags == GAMEOVER_FLAG || player2.flags == GAMEOVER_FLAG){
			_delay_ms(5000);
			if(cplevel > 0 && player2.flags == GAMEOVER_FLAG){
				cplevel++;
				if(cplevel > IMMPOSIBBLE){
					app_number = MENU;
				}
				else {
					init_objects();
					Clear_Screen();
					if(cplevel == NORMAL){
						player2.set(rand()%2,LEFT, 60,0x90);
						Glcd_str(40,30,"VS NORMAL");
					}
					else if(cplevel == HARD){
						player2.set(rand()%4,LEFT, 60,0x90);
						Glcd_str(40,30,"VS HARD");
					}
					else if(cplevel == IMMPOSIBBLE){
						player2.set(3,LEFT, 60,0x90);
						Glcd_str(20,30,"VS IMMPOSIBBLE");
					}
					player1.x = 60;
					player1.y = 0;
					Glcd_Print_map();
					_delay_ms(3000);
					Clear_Screen();
					Glcd_str(40,30,"READY...");
					Glcd_Print_map();
					simpleSound(NOTE_A4,4);
					_delay_ms(1000);
					Clear_Screen();
					Glcd_str(52,30,"GO!");
					Glcd_Print_map();
					simpleSound(NOTE_A5,4);
					_delay_ms(500);
				}
			}
			else{
				app_number = MENU;
				cplevel = 0;
			}
			player1.hp = 0;
			player2.hp = 0;
			player1.flags = 0;
			player2.flags = 0;
			player1.life = 3;
			player2.life = 3;
			player1.vx = 0;
			player2.vx = 0;
			player1.vy = 0;
			player2.vy = 0;
		}
	 }

 }