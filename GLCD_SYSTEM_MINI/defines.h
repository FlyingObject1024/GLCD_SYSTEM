/*
 * defines.h
 *
 * Created: 2017/12/29 2:05:34
 *  Author: kodai
 */ 

#ifndef DEFINES_H_//この文は DEFINES_H_ がdefineされていなかったら、という意味
#define DEFINES_H_//ここでdefine これでまたここが読み込まれた場合上の文ではじかれる

#define F_CPU 8000000UL//マイコンのクロックを書く。この数に応じてdelayの動きが変わる
extern volatile uint8_t sound_switch;
extern volatile uint8_t sig;
extern volatile uint8_t lsig;
extern volatile uint8_t last_button;
extern volatile uint8_t connection_flag;
extern volatile uint8_t opponents_sig;
extern volatile uint8_t opponents_last_button;
extern volatile uint8_t app_number;//どのアプリでも使う変数、関数をこのファイルを呼び出すことで一括で呼び出せる。
extern volatile uint64_t system_clock;
extern uint8_t button_keep(uint8_t s);
extern uint8_t button_jadge(uint8_t s);
extern uint8_t button_jadge_one();
extern uint8_t opponents_button_jadge(uint8_t s);
extern uint8_t opponents_button_jadge_one();
extern void start_button();
extern uint64_t Glcd_map[128];//マップデータ。64ビットが128で64x128の画面データを保存
//extern void Printtime();

/*****値を入れ替えるマクロ*****/
#define swap(x,y) do{uint8_t temp=x;x=y;y=temp;}while(0)



#define Master 1
#define Slave 2//かつて通信システムを作ろうとしたときの名残。使ってないはず

#define A 7
#define B 6
#define UP 2
#define DOWN 1
#define RIGHT 3
#define LEFT 0
#define START 5
#define SELECT 4
#define NO 8//とりあえずボタン名　こうした方が分かりやすい
#define MENU 20//メニューのアプリ番号　２０だった。０　と各所で書いた気がする。
enum{//#define の省略のようなもの上から0,1,2,と自動で割り振る
	TIMER_STOPWATCH,
	TETRIS,
	JUMP,
	MOVE,
	SHOOTING,
	BOARDRACE,
	SNAKE,
};

#endif /* DEFINES_H_ *///ここでは{}がないため代わりに終端をこれで表す
