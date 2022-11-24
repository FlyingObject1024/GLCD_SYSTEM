/*
 * timer.cpp
 *
 * Created: 2018/12/03 23:49:37
 *  Author: kodai
 */ 
 #include <avr/io.h>//基本（必須）
 #include <stdlib.h>//乱数用
 #include <avr/interrupt.h>//割り込み
 #include <avr/pgmspace.h>//これで、定数データをプログラムメモリに保存できる
 //PROGMEMで配置
 //pgm_read_byte(&変数);で呼び出し
 #include "defines.h"//defineを集めたもの
 #include "Glcd_function.h"//グラフィックLCD用関数群
 #include "sound.h"//音を出すための関数群
 #include <util/delay.h>//delay等

 #define READY 0
 #define COUNT 1
 #define PAUSE 2
 #define END   3

 typedef struct {
	 volatile uint64_t	nowtime;
	 volatile uint64_t	second_conpere;
	 volatile uint64_t	first_time;
	 volatile uint8_t	flag;
 } TIMER;//敵の弾の構造体

 static TIMER Timer;
 static TIMER StopWatch;

 void timer_set(){
	if(button_jadge_one() == A){
		if(Timer.flag == READY) Timer.flag = COUNT;
		else if(Timer.flag == COUNT) Timer.flag = PAUSE;
		else if(Timer.flag == PAUSE) Timer.flag = COUNT;
		else if(Timer.flag == END) Timer.flag = READY;
	}
	else if(button_jadge_one() == UP){
		if(Timer.flag == READY){
			Timer.nowtime += 100;
			Timer.nowtime -= Timer.nowtime%100;
		}
	}
	else if(button_jadge_one() == LEFT){
		if(Timer.flag == READY){
			Timer.nowtime += 600;
			Timer.nowtime -= Timer.nowtime%100;
		}
	}
	else if(button_jadge_one() == DOWN){
		if(Timer.flag == READY && Timer.nowtime > 0){
			Timer.nowtime -= 100;
			Timer.nowtime -= Timer.nowtime%100;
		}
	}
	
	if(button_jadge_one() == B){
		soundPlay(SOUND_TYPE_KEY);
		if(StopWatch.flag == READY) StopWatch.flag = COUNT;
		else if(StopWatch.flag == PAUSE){
			StopWatch.flag = END;
			StopWatch.nowtime = 0;
		}
		else if(StopWatch.flag == END) StopWatch.flag = READY;
	}
	else if(button_jadge_one() == SELECT){
		if(StopWatch.flag == COUNT) StopWatch.flag = PAUSE;
	}

	if (button_jadge_one() == START) start_button();
 }

 void timer_move(){
	if(Timer.flag == COUNT && Timer.nowtime > 0) Timer.nowtime = Timer.first_time - (system_clock - Timer.second_conpere);
	if(Timer.flag == COUNT && Timer.nowtime == 0) Timer.flag = END;
	if (Timer.flag == READY || Timer.flag == PAUSE){
		Timer.second_conpere = system_clock;
		Timer.first_time = Timer.nowtime;
	}

	if(StopWatch.flag == COUNT) StopWatch.nowtime = StopWatch.first_time + (system_clock - StopWatch.second_conpere);
	if (StopWatch.flag == READY || StopWatch.flag == PAUSE){
		StopWatch.second_conpere = system_clock;
		StopWatch.first_time = StopWatch.nowtime;
	}
 }

 void timer_draw(){
	Glcd_str(6,8,"TIMER");
	if(Timer.flag != END || (Timer.flag == END && system_clock%10 < 5)){
		Glcd_font(8,16,'0'+Timer.nowtime/10/60/60%10);
		Glcd_font(14,16,':');
		Glcd_font(20,16,'0'+Timer.nowtime/10/60/10%6);
		Glcd_font(26,16,'0'+Timer.nowtime/10/60%10);
		Glcd_font(32,16,':');
		Glcd_font(38,16,'0'+Timer.nowtime/10/10%6);
		Glcd_font(44,16,'0'+Timer.nowtime/10%10);
	}
	if(Timer.flag == END){
		if(system_clock % 2 == 0 && system_clock%10 <= 8) soundPlay(SOUND_TYPE_KEY);
	}
	
	Glcd_str(6,32,"STOPWATCH");
	Glcd_font(8,40,'0'+StopWatch.nowtime/10/60/60%10);
	Glcd_font(14,40,':');
	Glcd_font(20,40,'0'+StopWatch.nowtime/10/60/10%10);
	Glcd_font(26,40,'0'+StopWatch.nowtime/10/60%10);
	Glcd_font(32,40,':');
	Glcd_font(38,40,'0'+StopWatch.nowtime/10/10%6);
	Glcd_font(44,40,'0'+StopWatch.nowtime/10%10);
	Glcd_font(50,40,'.');
	Glcd_font(56,40,'0'+StopWatch.nowtime%10);

	Glcd_font(123,56,'0'+system_clock%10);
	Glcd_font(117,56,'0'+system_clock/10%10);
	Glcd_font(111,56,'0'+system_clock/100%10);
	Glcd_font(105,56,'0'+system_clock/1000%10);
	Glcd_font(99,56,'0'+system_clock/10000%10);
	Glcd_font(93,56,'0'+system_clock/100000%10);
	Glcd_font(87,56,'0'+system_clock/1000000%10);
	Glcd_font(81,56,'0'+system_clock/10000000%10);
 }

 void timer_main(){
	Timer.flag = READY;
	StopWatch.flag = READY;
	Timer.nowtime = 0;
	Timer.first_time = 0;
	StopWatch.nowtime = 0;
	StopWatch.first_time = 0;
	while (app_number == TIMER_STOPWATCH){
		Clear_Screen();
		timer_set();
		timer_move();
		timer_draw();
		Glcd_Print_map();
	}
 }