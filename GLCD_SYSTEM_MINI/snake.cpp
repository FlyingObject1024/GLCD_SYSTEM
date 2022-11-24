/*
 * snake.cpp
 *
 * Created: 2018/03/21 17:56:50
 *  Author: kodai
 */ 
 

 #include <avr/io.h>
 #include <stdlib.h>//óêêîóp
 #include <avr/interrupt.h>

 #include "Glcd_function.h"
 #include "defines.h"
 #include "sound.h"
 #include <util/delay.h>

void snake_game_over(uint8_t l){
	soundPlay(SOUND_TYPE_DAMAGE);
	Glcd_str(0,0,"GAMEOVER");
	Glcd_Print_map();
	_delay_ms(1000);
	Glcd_str(0,8,"SCORE:");
	Glcd_Print_map();
	_delay_ms(1000);
	Glcd_font(36,8,l/100%10+'0');
	Glcd_font(42,8,l/10%10+'0');
	Glcd_font(48,8,l%10+'0');
	Glcd_Print_map();
	_delay_ms(1000);
	while(button_jadge(B) == 0){
		Glcd_str(0,16,"Press B button");
		Glcd_Print_map();
	}
	app_number = MENU;
}

 void snake_main(){
	uint8_t snake_field[16][32];
	int8_t snake_x = 0;
	int8_t snake_y = 0;
	int8_t food_x = 0;
	int8_t food_y = 0;
	int8_t snake_direction = RIGHT;
	uint8_t length = 1;
	uint8_t lag = 1;
	uint8_t i,j;
	for (i=0;i<16;i++){
		for(j=0;j<32;j++){
			snake_field[i][j] = 0;
		}
	}
	food_x = rand() % 32;
	food_y = rand() % 16;
	while(app_number == SNAKE){

		Clear_Screen();

		if(button_jadge_one() == UP		    && snake_direction != DOWN){
			snake_direction = UP;
		}
		else if(button_jadge_one() == DOWN  && snake_direction != UP){
			snake_direction = DOWN;
		}
		else if(button_jadge_one() == RIGHT && snake_direction != LEFT){
			snake_direction = RIGHT;
		}
		else if(button_jadge_one() == LEFT  && snake_direction != RIGHT){
			snake_direction = LEFT;
		}

		if(button_jadge(START)) start_button();
		
		if(button_jadge(B)) lag = 0;
		else lag--;

		if(lag == 0){
			lag = 2;
			if(F_CPU >= 16000000) lag = 4;
			if(snake_direction == UP) snake_y -= 1;
			else if(snake_direction == DOWN) snake_y += 1;
			else if(snake_direction == LEFT) snake_x -= 1;
			else if(snake_direction == RIGHT) snake_x += 1;
			for (i=0;i<16;i++){
				for(j=0;j<32;j++){
					if(snake_field[i][j] > 0 && j == snake_x && i == snake_y) snake_game_over(length);
				}
			}
			if(food_x == snake_x && food_y == snake_y){
				soundPlay(SOUND_TYPE_CLEAR);
				length++;
				for (i=0;i<16;i++){
					for(j=0;j<32;j++){
						if(snake_field[i][j] > 0) snake_field[i][j] += 1;
					}
				}
				do {
					food_x = rand() % 32;
					food_y = rand() % 16;
				} while (snake_field[food_y][food_x] != 0);

			}
			if(snake_x < 0 || snake_x >= 32 || snake_y < 0 || snake_y >= 16) snake_game_over(length);
			snake_field[snake_y][snake_x] = length;
			for (i=0;i<16;i++){
				for(j=0;j<32;j++){
					if(snake_field[i][j] > 0) snake_field[i][j]--;
				}
			}

		}
		snake_field[snake_y][snake_x] = length;
		for (i=0;i<16;i++){
			for(j=0;j<32;j++){
				if(snake_field[i][j] > 0){
					DrawRectangle(j*4,i*4,j*4+3,i*4+3);
					DrawRectangle(j*4+1,i*4+1,j*4+2,i*4+2);
				}
				if(i == food_y && j == food_x){
					DrawRectangle(j*4,i*4,j*4+3,i*4+3);
				}
			}
		}
		Glcd_Print_map();
	}
 } 
 
