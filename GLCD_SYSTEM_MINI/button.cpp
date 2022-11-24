/*
 * button.cpp
 *
 * Created: 2017/12/29 2:24:55
 *  Author: kodai
 */ 
 /*
 * button.h
 *
 * Created: 2017/12/29 2:14:23
 *  Author: kodai
 */
 //ここはボタン管理用の関数群
#include <avr/io.h>
#include "Glcd_function.h"
#include "defines.h"
#include "sound.h"
#include <util/delay.h>

uint8_t button_keep(uint8_t s){
	if(last_button>>s & 0b00000001 == sig>>s & 0b00000001) return 1;
	else return 0;
}

uint8_t button_jadge(uint8_t s){
	last_button = sig;
	if(sig>>s & 0b00000001 == 1) return 1;
	else return 0;
}
//button_jadgeとbutton_jadge_oneでは性質が違う。
//button_jadgeは引数にボタンを設定し、戻り値に真偽を返し、
//button_jadge_oneは戻り値にボタン名が返る。
uint8_t button_jadge_one(){
	last_button = sig;
	if(sig>>LEFT & 0b00000001 ==1) return LEFT;
	else if(sig>>UP & 0b00000001 ==1) return UP;
	else if(sig>>DOWN & 0b00000001 ==1) return DOWN;
	else if(sig>>RIGHT & 0b00000001 ==1) return RIGHT;
	else if(sig>>SELECT & 0b00000001 ==1) return SELECT;
	else if(sig>>START & 0b00000001 ==1) return START;
	else if(sig>>B & 0b00000001 ==1) return B;
	else if(sig>>A & 0b00000001 ==1) return A;
	else return NO;
}
//opponentsとつくのは2p用ボタン
uint8_t opponents_button_jadge(uint8_t s){
	if(opponents_sig>>s & 0b00000001 == 1) return 1;
	else return 0;
}

uint8_t opponents_button_jadge_one(){
	if(opponents_sig>>LEFT & 0b00000001 ==1) return LEFT;
	else if(opponents_sig>>UP & 0b00000001 ==1) return UP;
	else if(opponents_sig>>DOWN & 0b00000001 ==1) return DOWN;
	else if(opponents_sig>>RIGHT & 0b00000001 ==1) return RIGHT;
	else if(opponents_sig>>SELECT & 0b00000001 ==1) return SELECT;
	else if(opponents_sig>>START & 0b00000001 ==1) return START;
	else if(opponents_sig>>B & 0b00000001 ==1) return B;
	else if(opponents_sig>>A & 0b00000001 ==1) return A;
	else return NO;
}
//スタート(ボーズ)ボタンを使った時の挙動
void start_button(){
	char start_cursor=0;
	while(1){
		Glcd_map[39] &= 0xfffffff0008fffff;//これ分からん
		DrawRectangle(33,23,90,40);//四角の枠を描く
		Glcd_str(34,24," CONTINUE\n\r END     ");//選択肢の文字
		if(start_cursor){
			Glcd_font(34,32,'}'+1);
			Glcd_font(34,24,' ');
		}
		else{
			Glcd_font(34,24,'}'+1);
			Glcd_font(34,32,' ');
		}//カーソル表示
		Glcd_Print_map();//画面へ表示
		if(button_jadge_one()==UP && start_cursor==1){
			soundPlay(SOUND_TYPE_KEY);
			start_cursor=0;
		}
		else if(button_jadge_one()==DOWN && start_cursor==0){
			soundPlay(SOUND_TYPE_KEY);
			start_cursor=1;
		}//カーソル上下
		else if(button_jadge_one()==A){
			soundPlay(SOUND_TYPE_ENTER);
			break;
		}
		else if(button_jadge_one()==B){
			soundPlay(SOUND_TYPE_CANCAL);
			start_cursor=0;
			break;
		}//決定または戻る
		
		
		
	}
	if(start_cursor){
		app_number=MENU;
		_delay_ms(150);
	}//カーソルがENDに重なっていたらアプリ実行変数をメニュー画面番号に書き換え
	if(app_number==0) _delay_ms(150);
}
