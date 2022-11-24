/*
 * move.cpp
 *
 * Created: 2018/01/09 21:03:48
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
 
 uint8_t SHOT_MAX = 3;//打てる弾数
 #define SCREEN_WIDTH 128
 #define SCREEN_HEIGHT 64
 #define MAP_WIDTH 80
 #define MAP_HEIGHT 54
 #define ENEMY_MAX 15//敵の数。これ以上の数は出現しない
 #define BULLET_MAX 50//敵が打つ弾の限界

 #define FREE_BLOCK 0
 #define WALL_BLOCK 1
 #define WATER_BLOCK 2
 #define IRON_BLOCK 3//ブロックの系統を分けたもの

 #define EM 0x40//空//empty
 #define FL 0x41//床//floor
 #define BL 0x42//ブロック//block
 #define FB 0x43//fake break
 #define IB 0x44//鉄骨//ironborn
 #define JU 0x45//ジャンプ台//jump
 #define WA 0x46//水//water
 #define PO 0x47//毒水//poison
 #define BA 0x48//バリア
 #define NE 0x49//とげ//needle
 #define TL 0x3f//テレポートエリア
 #define IT 0x4b//アイテム
 #define KE 0x3a//ボス鍵
 #define KO 0x1f//孔明

 #define NORMAL 1 
 #define WALK1 2
 #define WALK2 3
 #define WALK3 4
 #define SHOOT 5
 #define LOWER 6
 #define LOWER_SHOOT 7
 #define RASE 9
 #define RASE_SHOOT 11//プレイヤーのモーション
 enum{
	TWICE_JUMP,MISILE,WAVE_SHOT,FAST_MOVE,FAST_MISILE,WIDE_SHOT,INFINITY_JUMP,MANY_SHOT,
 };//能力

 uint8_t player_ability = 0b00000000;//獲得した能力
 uint8_t boss_flags = 0b00000000;//倒したボス
 static uint8_t move_alart = 0;//左上に表示する文字に関する変数

 #define SLIME 0x35
 #define BAT 0x80
 #define SQUID 0x81
 #define FAKE_BLOCK 0x82
 #define FAKE_ITEM 0x83
 #define SHOT_BLOCK 0x84

 #define FIRST_BOSS 0x8e
 #define SECOND_BOSS 0x8a
 #define THIRD_BOSS 0x8c
 #define LAST_BOSS_F 0x86
 #define LAST_BOSS_S 0x88//敵番号
 //マップ
uint8_t mmap[MAP_HEIGHT][MAP_WIDTH]={
	{FL,FL,FL,FL,FL,FL,FL,FL,FL,FL, FL,FL,FL,FL,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,},
	{FL,EM,EM,EM,EM,EM,FL,FL,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,KE,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,FL,FL,},
	{FL,EM,EM,EM,FL,IB,IB,IB,FL,EM, EM,EM,EM,EM,IB,FL,EM,EM,EM,FL, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,},
	{FL,EM,EM,EM,FL,EM,EM,EM,FL,IB, EM,EM,EM,EM,EM,EM,EM,EM,EM,FL, FL,FL,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,FL,IB,EM,EM,EM,EM,EM,EM,IB,FL,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,},
	{FL,EM,IT,EM,FL,EM,EM,EM,FL,IB, EM,EM,EM,EM,EM,EM,EM,EM,EM,FL, FL,FL,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,FL,FL,EM,EM,EM,EM,EM,EM,EM,EM,FL,FL,FL,FL,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,IT,EM,FL,EM,EM,FL,},
	{FL,FL,FL,FL,FL,EM,IT,EM,FL,EM, EM,FL,EM,EM,EM,EM,EM,EM,EM,FL, FL,FL,NE,NE,NE,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,FL,FL,FL,EM,EM,IB,IB,IB,IB,EM,EM,FL,FL,FL,FL,NE,NE,NE,FL,EM,EM,EM,EM,EM,FL,FL,FL,FL,EM,EM,FL,},
	{FL,FL,FL,FL,FL,IB,IB,IB,FL,FL, FL,FL,FL,FL,FL,FL,FL,FL,FL,FL, FL,FL,FL,FL,FL,FL,FL,IB,IB,IB,IB,FL,FL,FL,FL,FL,FL,FL,FL,FL,EM,EM,EM,EM,EM,EM,EM,EM,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,EM,EM,FL,},
	{FL,EM,BA,BA,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,},
	{FL,EM,BA,BA,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,IB,IB,IB,IB,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,},
	{FL,EM,BA,BA,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,},
	{FL,EM,BA,BA,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,IB,EM,EM,EM,EM,FL,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,},
	{FL,EM,BA,BA,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,IB,EM,EM,EM,EM,EM, IB,EM,EM,EM,EM,EM,EM,IB,FL,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,BL,EM,EM,EM,IB,IB,EM,EM,EM,BL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,BL,EM,EM,FL,},
	{FL,EM,FL,FL,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,FL,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,PO,PO,PO,PO,FL,EM,EM,EM,EM,EM,EM,EM,KE,FL,},
	{FL,EM,FL,FL,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,FL,FL,EM,EM,EM,EM,EM,EM,FL,FL,FL,EM,BL,EM,EM,EM,EM,EM,EM,EM,EM,BL,EM,EM,FL,EM,FL,PO,PO,PO,PO,FL,EM,EM,EM,EM,EM,BL,EM,FL,FL,},
	{FL,EM,FL,FL,FL,FL,FL,FL,FL,FL, FL,FL,FL,FL,FL,FL,FL,FL,FL,FL, FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,JU,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,IB,IB,IB,IB,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,},
//18
	{FL,EM,FL,EM,EM,EM,EM,EM,FL,FL, FL,EM,EM,EM,EM,FL,EM,EM,EM,FL, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,FL,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,FL,EM,EM,EM,FL, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,IB,IB,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,FL,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,FL,EM,EM,EM,BL, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,FL,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,FL,EM,EM,EM,BL, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,FL,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,FL,EM,EM,EM,BL, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,IB,IB,IB,IB,IB,FL,FL,FL,FL,FB,FB,FL,FL,FL,FL,FL,FL,IB,IB,FL,FL,FL,FL,FL,FL,IB,IB,EM,FL,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,FL,IB,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,IB,FL,EM,EM,EM,FL, FL,IB,EM,EM,EM,IB,EM,EM,EM,EM,FL,FL,EM,EM,EM,EM,EM,FB,FB,FB,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,FL,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,BL,EM,EM,EM,FL, FL,PO,PO,PO,PO,PO,PO,PO,PO,PO,FL,FL,EM,EM,EM,EM,EM,FB,FB,FB,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,IB,FL,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,FL,EM,EM,EM,EM,EM,IB,IB, EM,EM,EM,EM,EM,BL,EM,EM,EM,FL, FL,FL,PO,PO,PO,PO,PO,PO,PO,PO,FL,FL,IT,EM,EM,EM,EM,FB,FB,FB,EM,IB,IB,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,FL,FL,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,FL,FL,JU,FL,FL, FL,FL,FL,FL,FL,FL,PO,FL,FL,FL,FL,FL,FL,FB,FB,FB,FL,FL,FL,FL,EM,EM,EM,EM,FL,EM,FL,FL,FL,IB,IB,FL,FL,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,EM,FL,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,EM,EM,EM,EM,WA, WA,WA,WA,WA,WA,WA,PO,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,FL,WA,WA,WA,WA,FL,EM,EM,EM,FL,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,IB,FL,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,EM,FL,IB,EM,EM,EM,EM,EM, EM,EM,EM,IB,FL,EM,EM,EM,EM,WA, WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,FL,WA,IB,IB,WA,FL,EM,EM,EM,FL,EM,EM,EM,FL,IB,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,EM,EM,EM,EM,WA, WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,FL,WA,WA,WA,WA,FL,EM,EM,EM,FL,EM,EM,KE,FL,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,EM,BL,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,EM,EM,EM,FL,WA, WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,FL,WA,WA,WA,WA,FL,EM,EM,EM,FL,EM,IB,FL,FL,EM,EM,EM,EM,EM,EM,IB,FL,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,EM,BL,IB,EM,EM,EM,EM,EM, EM,EM,EM,IB,FL,EM,IB,EM,FL,WA, WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,BL,WA,IB,IB,WA,FL,EM,EM,EM,EM,EM,EM,FL,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,FL,},
	{FL,EM,EM,FL,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,EM,EM,EM,FL,WA, WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,BL,WA,WA,WA,WA,FL,EM,EM,EM,EM,EM,EM,FL,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,FL,},
	{FL,PO,PO,FL,NE,NE,NE,FL,BA,BA, FL,NE,NE,NE,FL,EM,IT,EM,FL,WA, WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,BL,WA,WA,WA,WA,FL,NE,NE,NE,FL,NE,NE,FL,FL,PO,PO,PO,PO,PO,PO,FL,FL,FL,FL,FL,EM,EM,EM,FL,},
	{FL,FL,FL,FL,FL,FL,FL,FL,FB,FB, FL,FL,FL,FL,FL,FL,FL,FL,FL,JU, FL,FL,JU,FL,FL,FL,JU,FL,FL,FL,JU,FL,FL,FL,FL,JU,FL,FL,FL,FL,FL,IB,IB,IB,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,EM,EM,EM,FL,},
//36 
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,PO,PO,PO,PO,PO, PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,FL,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,KE,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,FL,FL,FL,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,FL,FL,FL,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,EM,IB,EM,EM,EM, FL,EM,EM,EM,EM,EM,EM,IB,EM,EM,EM,EM,IB,EM,EM,EM,EM,EM,EM,EM,EM,EM,BL,BL,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,EM,EM,EM,EM,EM, FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,IB,EM,EM,FL,FL,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,FL,FL,FL,FL,FB,FB,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,EM,EM,EM,EM,EM, FL,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,EM,EM,EM,EM,FL,EM,FB,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,NE,EM,EM,EM,EM, FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,IB,EM,FL,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,FL,EM,EM,EM,IB, FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FB,EM,EM,EM,EM,EM, FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FB,EM,EM,EM,EM,NE, FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FB,EM,IT,EM,NE,FL, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,EM,EM,IB,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,FL,FL,FL,FL,FL,BL, EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,PO,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,BL,BL,BL,BL,BL, BL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,PO,EM,EM,EM,EM,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,EM,BL,EM,BL,EM, BL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,PO,EM,EM,EM,IT,EM,FL,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,EM,FL,},
	{FL,EM,EM,EM,EM,EM,EM,EM,EM,EM, EM,EM,EM,EM,EM,BL,EM,BL,EM,IT, BL,PO,PO,PO,PO,FL,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,WA,PO,WA,FL,FL,FL,FL,FL,PO,PO,PO,PO,PO,PO,PO,PO,IB,IB,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,PO,FL,},
	{FL,NE,NE,NE,NE,NE,NE,NE,FL,FL, NE,NE,NE,NE,FL,FL,FL,FL,FL,FL, FL,NE,NE,NE,NE,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,FL,},

};	

uint8_t enemy_state[25][2]={
	{27,8},{30,8},{60,8},{30,16},{62,16},
	
	{22,18},{28,18},{35,18},{16,18},{40,18},{35,31},{30,31},{38,30},
	
	{22,38},{28,43},{35,40},{16,49},{24,48},{30,48},

	{27,8},{62,16},{28,18},{35,18},{16,18},{48,18},
};//敵の位置

uint8_t enemy_type[25]={
						SLIME,SLIME,SLIME,SLIME,SLIME,
						BAT,BAT,BAT,BAT,BAT,SLIME,SLIME,SLIME,
						SQUID,SQUID,SQUID,FAKE_BLOCK,FAKE_BLOCK,FAKE_BLOCK,
						FAKE_ITEM,FAKE_ITEM,SHOT_BLOCK,SHOT_BLOCK,SHOT_BLOCK,SHOT_BLOCK,
						};//敵の種類


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
//傾き表　敵の弾の軌道に使用
typedef struct {
	volatile int16_t	x;
	volatile int16_t	y;//座標
	volatile uint8_t direction;//方向
	volatile uint8_t isAlive;
} SHOT;//弾の構造体

typedef struct {
	volatile int16_t	x;
	volatile int16_t	y;
	volatile uint8_t type;//敵の種類
	uint16_t hp;//体力
	volatile uint8_t isAlive;
	uint8_t move_flag;//移動に使う
	uint8_t jump_f;//使ってないはず
	uint8_t dropt;//使ってないはず
	uint8_t direction;//移動に使う
} ENEMY;//敵の構造体

typedef struct {
	volatile int16_t	x;
	volatile int16_t	y;
	volatile int8_t	vx;
	volatile int8_t	vy;
	volatile uint8_t	isAlive;
} BULLET;//敵の弾の構造体

static SHOT Shot[50];
static SHOT Misile;
static SHOT Recover;//使えなかった
static ENEMY Enemy[ENEMY_MAX];
static BULLET Bullet[BULLET_MAX];//構造体からオブジェクト生成
int16_t player_HP=99;
int16_t player_HP_MAX=99;
volatile int16_t player_x=104;
volatile uint16_t scroll_x=0;//スクロール
volatile int16_t player_y=48;
volatile int16_t scroll_y=0;
volatile static int16_t reset_point_x=104;
volatile static int16_t reset_point_y=48;
uint8_t player_jump_f=0;//ジャンプの判定
uint8_t player_dropt=4;//上下の移動に使用(ジャンプ・落下)
int8_t Misile_Shot_Able = 0;//ミサイル発射が可能かどうか

volatile uint32_t flame_count = 0;
volatile uint32_t shot_count = 0;
volatile uint32_t damage_count = 0;//総合結果用


void move_initItem(void);
void move_setitem(int16_t x, int16_t y, uint8_t type);
void move_moveitem(void);
void move_drawitem(void);
uint8_t move_hititem(int16_t x, int16_t y);
void move_initEnemy(void);
void move_setEnemy(int16_t x, int16_t y, uint8_t type);
void move_moveEnemy(void);
void move_drawEnemy(void);
uint8_t move_hitEnemy(int16_t x, int16_t y,uint8_t damage);
uint8_t map_block(int16_t x,int16_t y);
uint8_t move_player_move(int16_t x,int16_t y);
void end_game();//プロトタイプ宣言群

void initBullet(void);
void shotBullet(int16_t x, int16_t y, int8_t vx, int8_t vy, uint8_t type);
void moveBullet(void);
void drawBullet(void);
extern uint8_t ATAN(int8_t x, int8_t y);

void move_select_button(){
	Clear_Screen();
	while(sig != 0){
		Glcd_str(0 ,0 ,"ｹﾞｰﾑ ﾉ ﾋﾝﾄ");
		Glcd_graphic(0 ,8 ,IT,PUT,0);
		Glcd_str(8 ,8 ,"ﾉｳﾘｮｸ ｷｮｳｶ");
		Glcd_graphic(0 ,16,KE,PUT,0);
		Glcd_str(8 ,16,"ﾎﾞｽ ﾉ ｶｷﾞ");
		Glcd_graphic(0 ,24,BL,PUT,0);
		Glcd_str(8 ,24,"ﾓﾛｲ ｶﾍﾞ");
		Glcd_str(0 ,32,"A:ｼﾞｬﾝﾌﾟ B:ｼｮｯﾄ");
		Glcd_str(0 ,40,"ｶﾍﾞﾃﾞﾅｲ ｶﾍﾞﾆﾁｭｳｲ");
		Glcd_Print_map();
	}
}

uint8_t ability(uint8_t able){
	if((player_ability >> able & 0b00000001) == 1) return 1;
	else return 0;
}

int8_t drop(uint16_t x,uint16_t y,uint8_t *dropt,uint8_t *jump_f,uint16_t *hp);//プロトタイプ宣言

uint8_t map_block(int16_t x,int16_t y){//ブロックの種類判定
	if(mmap[y/8][x/8] == EM || mmap[y/8][x/8] == FB || mmap[y/8][x/8] == KE || mmap[y/8][x/8] == IT || mmap[y/8][x/8] == 0){
		return FREE_BLOCK;
	}
	else if(mmap[y/8][x/8] == FL || mmap[y/8][x/8] == BA || mmap[y/8][x/8] == BL){
		return WALL_BLOCK;
	}
	else if(mmap[y/8][x/8] == WA || mmap[y/8][x/8] == PO){
		return WATER_BLOCK;
	}
	else if(mmap[y/8][x/8] == IB){
		return IRON_BLOCK;
	}
	else return WALL_BLOCK;
	return 0;
}


uint8_t move_player_move(int16_t x ,int16_t y , int8_t vx , int8_t vy){//プレイヤーの動きだが、たぶん自分でも解明できないほどわかりにくい
	int8_t i=0;
	uint8_t block_flag = 0;
	if(vx >= 0){
		for (i=0;i<=vx;i++){
			if(map_block(x+7 + i,y) == WALL_BLOCK || map_block(x+7 + i,y+15) == WALL_BLOCK || map_block(x+7 + i,y+7) == WALL_BLOCK){
				player_x += (i - 1);
				return 0;
			}
			else if(map_block(x+7 + i,y) == WATER_BLOCK || map_block(x+7 + i,y+15) == WATER_BLOCK || map_block(x+7 + i,y+7) == WATER_BLOCK){
				block_flag = WATER_BLOCK;
			}
		}
		if(block_flag == WATER_BLOCK){
			player_x += (i/2);
		}
		else player_x += (i - 1);
	}
	else if(vx < 0){
		for (i=0;i<=(-vx);i++){
			if(map_block(x - i,y) == WALL_BLOCK || map_block(x - i,y+15) == WALL_BLOCK || map_block(x - i,y+7) == WALL_BLOCK){
				player_x -= (i - 1);
				return 0;
			}
			else if(map_block(x - i,y) == WATER_BLOCK || map_block(x - i,y+15) == WATER_BLOCK || map_block(x - i,y+7) == WATER_BLOCK){
				block_flag = WATER_BLOCK;
			}
		}
		if(block_flag == WATER_BLOCK){
			player_x -= (i/2);
		}
		else player_x -= (i - 1);
	}	
	return 0;
}

void gameover(){
	uint8_t i;
	for(i=0;i<8;i++){
		for(uint8_t j=0;j<16;j++) Glcd_graphic(j*8,i*8,FL,0,0);
		Glcd_Print_map();
		_delay_ms(100);
	}
	Glcd_str(37,28,"GAME OVER");
	Glcd_Print_map();
	_delay_ms(1000);

	player_HP=player_HP_MAX;
	player_x=reset_point_x;
	scroll_x=0;
	player_y=reset_point_y;
	scroll_y=0;
	player_jump_f=0;
	player_dropt=4;

	while(sig == 0){
		Glcd_str(14,36,"Press A:CONTINUE");
		Glcd_str(14,44,"Press B:GAME END");
		Glcd_Print_map();
		i = button_jadge_one();
		if(i == A) break;
		else if(i == B){
			app_number=MENU;
			break;
		}
	}
	return;
}

void move_enemy1(uint8_t i)//敵それぞれの動き　これはスライム
{
	if(Enemy[i].direction==RIGHT && (mmap[Enemy[i].y/8][Enemy[i].x/8+1]==EM || mmap[Enemy[i].y/8][Enemy[i].x/8+1]==IB || mmap[Enemy[i].y/8][Enemy[i].x/8+1]==WA)==false) Enemy[i].direction=LEFT;
	if(Enemy[i].direction==LEFT && (mmap[Enemy[i].y/8][(Enemy[i].x-1)/8]==EM || mmap[Enemy[i].y/8][(Enemy[i].x+1)/8]==IB || mmap[Enemy[i].y/8][(Enemy[i].x+1)/8]==WA)==false) Enemy[i].direction=RIGHT;

	if(Enemy[i].direction==RIGHT) Enemy[i].x += 1;
	else if(Enemy[i].direction==LEFT) Enemy[i].x -= 1;

}

void move_first_boss(uint8_t i)
{
	if(Enemy[i].direction==RIGHT && ((mmap[Enemy[i].y/8][Enemy[i].x/8+2]==EM || mmap[Enemy[i].y/8][Enemy[i].x/8+2]==IB)==false || (mmap[Enemy[i].y/8+1][Enemy[i].x/8+2]==EM || mmap[Enemy[i].y/8+1][Enemy[i].x/8+2]==IB)==false)) Enemy[i].direction=LEFT;
	if(Enemy[i].direction==LEFT && ((mmap[Enemy[i].y/8][(Enemy[i].x-1)/8]==EM || mmap[Enemy[i].y/8][(Enemy[i].x+1)/8]==IB)==false ||(mmap[Enemy[i].y/8][(Enemy[i].x-1)/8]==EM || mmap[Enemy[i].y/8+1][(Enemy[i].x+1)/8]==IB)==false)) Enemy[i].direction=RIGHT;

	if(Enemy[i].direction==RIGHT) Enemy[i].x += 1;
	else if(Enemy[i].direction==LEFT) Enemy[i].x -= 1;

	
	if(Enemy[i].move_flag < 30) Enemy[i].move_flag += 1;
	if(Enemy[i].move_flag == 30 && Enemy[i].x >= player_x - 64 && Enemy[i].x <= player_x + 64 && Enemy[i].y >= player_y - 16){
		Enemy[i].move_flag += 1;
	}
	else if(Enemy[i].move_flag > 30){
		Enemy[i].move_flag += 1;
		for(uint8_t t=0;t<4;t++){
			if(Enemy[i].direction==RIGHT && ((mmap[Enemy[i].y/8][Enemy[i].x/8+2]==EM || mmap[Enemy[i].y/8][Enemy[i].x/8+2]==IB)==false || (mmap[Enemy[i].y/8+1][Enemy[i].x/8+2]==EM || mmap[Enemy[i].y/8+1][Enemy[i].x/8+2]==IB)==false)) Enemy[i].direction=LEFT;
			if(Enemy[i].direction==LEFT && ((mmap[Enemy[i].y/8][(Enemy[i].x-1)/8]==EM || mmap[Enemy[i].y/8][(Enemy[i].x+1)/8]==IB)==false ||(mmap[Enemy[i].y/8][(Enemy[i].x-1)/8]==EM || mmap[Enemy[i].y/8+1][(Enemy[i].x+1)/8]==IB)==false)) Enemy[i].direction=RIGHT;
		
			if(Enemy[i].direction==RIGHT) Enemy[i].x += 1;
			else if(Enemy[i].direction==LEFT) Enemy[i].x -= 1;
		}
		if(Enemy[i].move_flag >= 40) Enemy[i].move_flag = 0;
	}
}

void move_enemy2(uint8_t i)//コウモリ
{
	int8_t bat_move[18]={4,4,6,6,6,8,4,2,1,-1,-2,-4,-8,-6,-6,-6,-4,-4};

	if(Enemy[i].x >= player_x - 16 && Enemy[i].x <= player_x + 16 && Enemy[i].move_flag == 0){
		Enemy[i].direction = 0;
		Enemy[i].move_flag = 1;
	}

	if(Enemy[i].move_flag > 0){
		Enemy[i].y += (int16_t)bat_move[Enemy[i].direction];
		Enemy[i].direction += 1;
		if(Enemy[i].direction > 18) Enemy[i].move_flag = 0;
	}
}

void move_second_boss(uint8_t i)
{
	Enemy[i].move_flag += 1;
	if(Enemy[i].move_flag % 20 == 0){
		uint8_t d = ATAN( player_x-Enemy[i].x, player_y-Enemy[i].y );
		int8_t vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 10;
		int8_t vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 10;
		if(d >= 0) shotBullet( Enemy[i].x-2 + rand() % 8 , Enemy[i].y + rand() % 8, vx, vy , 1);
	}
}

void move_enemy3(uint8_t i)//イカ
{
	int8_t squid_move[18]={1,1,1,1,1,1,2,2,2,2,4,4,4,-8,-8,-4,-4,-1};
	
	Enemy[i].y += (int16_t)squid_move[Enemy[i].move_flag];
	
	Enemy[i].move_flag += 1;

	if(Enemy[i].move_flag >= 18){
		Enemy[i].move_flag = 0 ;
	}

	if(Enemy[i].direction==RIGHT && map_block(Enemy[i].x+8,Enemy[i].y) == WALL_BLOCK) Enemy[i].direction=LEFT;
	if(Enemy[i].direction==LEFT  && map_block(Enemy[i].x-8,Enemy[i].y) == WALL_BLOCK) Enemy[i].direction=RIGHT;

	if(Enemy[i].direction==RIGHT) Enemy[i].x += 1;
	else if(Enemy[i].direction==LEFT) Enemy[i].x -= 1;

}

void move_third_boss(uint8_t i)
{
	if(Enemy[i].dropt != UP || Enemy[i].dropt != DOWN) Enemy[i].dropt = DOWN;
	if(Enemy[i].move_flag < 30){
		
		if(Enemy[i].direction==RIGHT && (mmap[Enemy[i].y/8][Enemy[i].x/8+1]==EM || mmap[Enemy[i].y/8][Enemy[i].x/8+1]==IB || mmap[Enemy[i].y/8][Enemy[i].x/8+1]==WA)==false) Enemy[i].direction=LEFT;
		if(Enemy[i].direction==LEFT && (mmap[Enemy[i].y/8][(Enemy[i].x-1)/8]==EM || mmap[Enemy[i].y/8][(Enemy[i].x+1)/8]==IB || mmap[Enemy[i].y/8][(Enemy[i].x+1)/8]==WA)==false) Enemy[i].direction=RIGHT;
		
		if(Enemy[i].dropt == UP && map_block(Enemy[i].x , Enemy[i].y - 8) != FREE_BLOCK && map_block(Enemy[i].x + 8, Enemy[i].y - 8) != FREE_BLOCK) Enemy[i].dropt = DOWN;
		if(Enemy[i].dropt == DOWN && map_block(Enemy[i].x , Enemy[i].y + 16) != FREE_BLOCK && map_block(Enemy[i].x + 8, Enemy[i].y + 16) != FREE_BLOCK) Enemy[i].dropt = UP;

		if(Enemy[i].direction==RIGHT) Enemy[i].x += 2;
		else if(Enemy[i].direction==LEFT) Enemy[i].x -= 2;
		
		if(Enemy[i].dropt == DOWN) Enemy[i].y += 2;
		else if(Enemy[i].dropt == UP) Enemy[i].y -= 2;
		
		Enemy[i].move_flag += 1;
	}
	else{
		Enemy[i].move_flag += 1;
		if(map_block(Enemy[i].x , Enemy[i].y - 4) != FREE_BLOCK || map_block(Enemy[i].x + 8, Enemy[i].y - 4) != FREE_BLOCK){
			Enemy[i].move_flag = 0;
			Enemy[i].dropt = DOWN;
		}
		if(Enemy[i].move_flag <  40) Enemy[i].y -= 6;
		if(Enemy[i].move_flag >= 40 && Enemy[i].move_flag < 50) Enemy[i].move_flag = 0;
		if(Enemy[i].move_flag == 50){
			Enemy[i].y -= 6;
			if(Enemy[i].y <= MAP_HEIGHT * 8 - 24) Enemy[i].move_flag = 0;
		}
		if(Enemy[i].move_flag == 60){
			Enemy[i].x -= 6;
			if(Enemy[i].x >= 24) Enemy[i].move_flag = 0;
		}
	}

	if(Enemy[i].y + 16 >= MAP_HEIGHT * 8){
		Enemy[i].move_flag = 50;
		Enemy[i].dropt = UP;
	}
	if(Enemy[i].x - 16 <= 0){
		Enemy[i].move_flag = 60;
		Enemy[i].direction = RIGHT;
	}
	Enemy[i].jump_f += 1;
	if(Enemy[i].jump_f % 10 == 0){
		uint8_t d = ATAN( player_x-Enemy[i].x, player_y-Enemy[i].y );
		int8_t vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 16;
		int8_t vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 16;
		if(d >= 0) shotBullet( Enemy[i].x-2, Enemy[i].y, vx, vy ,1);
	}
	
}

void move_enemy4(uint8_t i)//フェイク敵
{
	int8_t fake_move[16]={-4,-4,-6,-6,-6,-8,-4,-2,2,4,8,6,6,6,4,4};

	if(Enemy[i].x >= player_x - 8 && Enemy[i].x <= player_x + 7 && Enemy[i].move_flag == 0){
		Enemy[i].direction = 0;
		Enemy[i].move_flag = 1;
	}

	if(Enemy[i].move_flag > 0){
		Enemy[i].y += (int16_t)fake_move[Enemy[i].direction];
		Enemy[i].direction += 1;
		if(Enemy[i].direction > 16) Enemy[i].move_flag = 0;
	}
}

void move_enemy5(uint8_t i)//砲台
{
	Enemy[i].move_flag += 1;
	if(Enemy[i].move_flag % 20 == 0){
		uint8_t d = ATAN( player_x-Enemy[i].x, player_y-Enemy[i].y );
		int8_t vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 20;
		int8_t vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 20;
		if(d >= 0) shotBullet( Enemy[i].x-2, Enemy[i].y, vx, vy , 1);
	}
	if(Enemy[i].move_flag % 100 == 0){
		uint8_t d = ATAN( player_x-Enemy[i].x, player_y-Enemy[i].y );
		int8_t vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 8;
		int8_t vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 8;
		if(d >= 0) shotBullet( Enemy[i].x-2, Enemy[i].y, vx, vy , 1);
	}
}

void move_last_boss_f(uint8_t i)
{
	uint8_t d;
	int8_t vx;
	int8_t vy;

	Enemy[i].move_flag += 1;

	if(Enemy[i].move_flag % 10 == 0){
		uint8_t d = ATAN( player_x-Enemy[i].x, player_y-Enemy[i].y );
		int8_t vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 8;
		int8_t vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 8;
		if(d >= 0) shotBullet( Enemy[i].x-2, Enemy[i].y, vx, vy , 2);
		d = rand() % 5;
		if(rand() % 3 == 0){
			switch(d){
				case 0:
				Enemy[i].x = 50*8;
				Enemy[i].y = 40*8;
				break;
				case 1:
				Enemy[i].x = 54*8;
				Enemy[i].y = 40*8;
				break;
				case 2:
				Enemy[i].x = 46*8;
				Enemy[i].y = 40*8;
				break;
				case 3:
				Enemy[i].x = 50*8;
				Enemy[i].y = 44*8;
				break;
				case 4:
				Enemy[i].x = 50*8;
				Enemy[i].y = 36*8;
				break;
			}
		}
	}
	if(Enemy[i].move_flag % 100 == 0){
		d = ATAN( player_x-Enemy[i].x, player_y-Enemy[i].y );
		vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 8;
		vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 8;
		if(d >= 0) shotBullet( Enemy[i].x-2, Enemy[i].y, vx, vy , 2);
	}
}

void move_last_boss_s(uint8_t i)
{
	uint8_t d;
	int8_t vx;
	int8_t vy;

	Enemy[i].move_flag += 1;

	if(Enemy[i].move_flag % 100 < 50 && Enemy[i].move_flag % 100 % 2 == 0){
		uint8_t d = ATAN( player_x-Enemy[i].x, player_y-Enemy[i].y );
		int8_t vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 32;
		int8_t vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 32;
		if(d >= 0) shotBullet( Enemy[i].x-2 + rand() % 4, Enemy[i].y + rand() % 4, vx, vy , 2);
	}
	if(Enemy[i].move_flag % 100 == 0){
		d = ATAN( player_x-Enemy[i].x, player_y-Enemy[i].y );
		vx = (int8_t)pgm_read_byte(&DIR[d][0]) / 8;
		vy = (int8_t)pgm_read_byte(&DIR[d][1]) / 8;
		if(d >= 0) shotBullet( Enemy[i].x-2, Enemy[i].y, vx, vy , 2);
	}
}

void move_initEnemy(void)//敵の構造体初期化
{
	uint8_t i;
	for(i=0; i<ENEMY_MAX; i++){
		Enemy[i].isAlive = 0;
		if(Enemy[i].type == FIRST_BOSS){
			Enemy[i].isAlive = 1;
			Enemy[i].hp = 100;
		}
		else if(Enemy[i].type == SECOND_BOSS){
			Enemy[i].isAlive = 1;
			Enemy[i].hp = 200;
		}
		else if(Enemy[i].type == THIRD_BOSS){
			Enemy[i].isAlive = 1;
			Enemy[i].hp = 300;
		}
		else if(Enemy[i].type == LAST_BOSS_F){
			Enemy[i].isAlive = 1;
			Enemy[i].hp = 1000;
		}
		else if(Enemy[i].type == LAST_BOSS_S){
			Enemy[i].isAlive = 1;
			Enemy[i].hp = 3000;
		}
	}
}

void move_setEnemy(int16_t x, int16_t y, uint8_t type)//敵の配置
{
	uint8_t i;
	for(i=0; i<ENEMY_MAX; i++){
		if(Enemy[i].isAlive == 0 || type == FIRST_BOSS || type == SECOND_BOSS|| type == THIRD_BOSS || type == LAST_BOSS_F || type == LAST_BOSS_S){
			if(type == SLIME){
				Enemy[i].hp = 10;
			}
			else if(type == BAT){
				Enemy[i].hp = 15;
			}
			else if(type == SQUID){
				Enemy[i].hp = 25;
			}
			else if(type == FAKE_BLOCK || type == FAKE_ITEM){
				Enemy[i].hp = 50;
			}
			else if(type == SHOT_BLOCK){
				Enemy[i].hp = 80;
			}
			else if(type == FIRST_BOSS){
				Enemy[i].hp = 100;
				move_alart = 2;
			}
			else if(type == SECOND_BOSS){
				Enemy[i].hp = 200;
				move_alart = 2;
			}
			else if(type == THIRD_BOSS){
				Enemy[i].hp = 300;
				move_alart = 2;
			}
			else if(type == LAST_BOSS_F){
				Enemy[i].hp = 1000;
				move_alart = 2;
			}
			else if(type == LAST_BOSS_S){
				Enemy[i].hp = 1500;
			}
			else continue;
			Enemy[i].direction = RIGHT;
			Enemy[i].x = x;
			Enemy[i].y = y;
			Enemy[i].type = type;
			Enemy[i].jump_f = 0;
			Enemy[i].move_flag = 0;
			Enemy[i].dropt = 5;
			Enemy[i].isAlive = 1;
			break;
		}
	}
}

void move_moveEnemy(void)//敵の動き。敵の種類で振り分け
{
	uint8_t i;
	for(i=0; i<ENEMY_MAX; i++){
		if(Enemy[i].isAlive != 0){
			int16_t nx = Enemy[i].x;
			int16_t ny = Enemy[i].y;
			if( nx < 0 || nx >= MAP_WIDTH*8 || ny < 0 || ny > MAP_HEIGHT*8 ){
				Enemy[i].isAlive = 0;
			}
			switch(Enemy[i].type){
				case SLIME:		  move_enemy1(i); break;
				case FIRST_BOSS:  move_first_boss(i); break;
				case BAT:		  move_enemy2(i); break;
				case SQUID:		  move_enemy3(i); break;
				case SECOND_BOSS: move_second_boss(i); break;
				case THIRD_BOSS:  move_third_boss(i); break;
				case FAKE_BLOCK:  move_enemy4(i); break;
				case FAKE_ITEM:   move_enemy4(i); break;
				case SHOT_BLOCK:  move_enemy5(i); break;
				case LAST_BOSS_F: move_last_boss_f(i); break;
				case LAST_BOSS_S: move_last_boss_s(i); break;
			}
		}
	}
}

void move_drawEnemy(void)//敵の描画
{
	uint8_t i;
	uint8_t n=0;
	for(i=0; i<ENEMY_MAX; i++){
		if(Enemy[i].isAlive != 0){
			n++;
			if(Enemy[i].x-scroll_x>0 && Enemy[i].x-scroll_x<128 && Enemy[i].y-scroll_y-16>0 && Enemy[i].y-scroll_y-16<64){
				if(Enemy[i].type == FIRST_BOSS || Enemy[i].type == SECOND_BOSS || Enemy[i].type == THIRD_BOSS || Enemy[i].type == LAST_BOSS_F || Enemy[i].type == LAST_BOSS_S){
					EraseRectangle(127,0,105,4);
					DrawRectangle(127,0,106,3);
					EraseRectangle(126,1,107,2);
					switch(Enemy[i].type){

						case FIRST_BOSS: DrawRectangle((uint8_t)(106+(Enemy[i].hp/5)),1,106,2); 
						break;

						case SECOND_BOSS: DrawRectangle((uint8_t)(106+(Enemy[i].hp/10)),1,106,2);
						break;

						case THIRD_BOSS: DrawRectangle((uint8_t)(106+(Enemy[i].hp/15)),1,106,2);
						break;

						case LAST_BOSS_F: DrawRectangle((uint8_t)(106+(Enemy[i].hp/50)),1,106,2);
						break;

						case LAST_BOSS_S: DrawRectangle((uint8_t)(106+(Enemy[i].hp/75)),1,106,2);
						break;
					}
					if(Enemy[i].direction == RIGHT){
						Glcd_graphic( Enemy[i].x - scroll_x - 4 , Enemy[i].y - scroll_y - 16     , Enemy[i].type      ,OR,0);
						Glcd_graphic( Enemy[i].x - scroll_x + 4 , Enemy[i].y - scroll_y - 16     , Enemy[i].type + 1  ,OR,0);
						Glcd_graphic( Enemy[i].x - scroll_x - 4 , Enemy[i].y - scroll_y - 16 + 8 , Enemy[i].type + 16 ,OR,0);
						Glcd_graphic( Enemy[i].x - scroll_x + 4 , Enemy[i].y - scroll_y - 16 + 8 , Enemy[i].type + 17 ,OR,0);
					}
					if(Enemy[i].direction == LEFT){
						Glcd_graphic( Enemy[i].x - scroll_x - 4 , Enemy[i].y - scroll_y - 16     , Enemy[i].type + 1  ,OR,1);
						Glcd_graphic( Enemy[i].x - scroll_x + 4 , Enemy[i].y - scroll_y - 16     , Enemy[i].type      ,OR,1);
						Glcd_graphic( Enemy[i].x - scroll_x - 4 , Enemy[i].y - scroll_y - 16 + 8 , Enemy[i].type + 17 ,OR,1);
						Glcd_graphic( Enemy[i].x - scroll_x + 4 , Enemy[i].y - scroll_y - 16 + 8 , Enemy[i].type + 16 ,OR,1);
					}
				}
				else{
					Glcd_graphic(Enemy[i].x-scroll_x+4, Enemy[i].y-scroll_y-16,Enemy[i].type,OR,0);
				}
			}
		}
	}
}

uint8_t move_hitEnemy(int16_t x, int16_t y, uint8_t damege)//敵との接触
{
	uint8_t i,j;
	for(i=0; i<ENEMY_MAX; i++){
		if(Enemy[i].isAlive != 0){
			if( Enemy[i].x >= x-8 && Enemy[i].x < x && y >= Enemy[i].y && y < Enemy[i].y+8 && Enemy[i].type != FIRST_BOSS  && Enemy[i].type != SECOND_BOSS  && Enemy[i].type != THIRD_BOSS  && Enemy[i].type != LAST_BOSS_F  && Enemy[i].type != LAST_BOSS_S){
				if(Enemy[i].hp >= damege){
					soundPlay(SOUND_TYPE_DAMAGE);
					Enemy[i].hp -= damege;
				}
				else {
					soundPlay(SOUND_TYPE_DAMAGE);
					Enemy[i].hp = 0;
				}

				

				if(Enemy[i].hp <= 0){
					Enemy[i].hp = 0;
					Enemy[i].isAlive = 0;
				}
				return 1;
			}
			if( Enemy[i].x >= x-8 && Enemy[i].x < x+8 && y >= Enemy[i].y && y < Enemy[i].y+16 && (Enemy[i].type == FIRST_BOSS || Enemy[i].type == SECOND_BOSS || Enemy[i].type == THIRD_BOSS || Enemy[i].type == LAST_BOSS_F || Enemy[i].type == LAST_BOSS_S) ){
				if(Enemy[i].hp >= damege){
					soundPlay(SOUND_TYPE_DAMAGE);
					Enemy[i].hp -= damege;
				}
				else {
					soundPlay(SOUND_TYPE_DAMAGE);
					Enemy[i].hp = 0;
				}

				

				if(Enemy[i].hp <= 0){
					Enemy[i].isAlive = 0;
					if(Enemy[i].type == FIRST_BOSS){
						for(i=0;i<MAP_HEIGHT;i++){
							if(mmap[i][2] == BA)mmap[i][2] = EM;
							if(mmap[i][3] == BA)mmap[i][3] = EM;
						}
						if(boss_flags >= 8){
							move_setEnemy( 9*8  , 19*8   , SECOND_BOSS );
						}
						else boss_flags = 1;
						for(j=0;j<8;j++){
							move_setEnemy((uint16_t)(enemy_state[j+5][0]*8),(uint16_t)(enemy_state[j+5][1]*8),enemy_type[j+5]);
						}
						reset_point_x = 13*8;
						reset_point_y = 18*8;
						Recover.x = Enemy[i].x + 8;
						Recover.y = Enemy[i].y + 8;
						Recover.isAlive = 1;
						Recover.direction = 0;
						soundPlay(SOUND_TYPE_1UP);
						player_HP_MAX += 50;
						player_HP = player_HP_MAX;
					}
					else if(Enemy[i].type == SECOND_BOSS){
						for(i=0;i<MAP_HEIGHT;i++){
							if(mmap[i][8] == BA)mmap[i][8] = EM;
							if(mmap[i][9] == BA)mmap[i][9] = EM;
						}
						if(boss_flags >= 8){
							move_setEnemy( 12*8 , 40*8   , THIRD_BOSS  );
						}
						else boss_flags = 2;
						for(j=0;j<6;j++){
							move_setEnemy((uint16_t)(enemy_state[j+13][0]*8),(uint16_t)(enemy_state[j+13][1]*8),enemy_type[j+13]);
						}
						reset_point_x = 8*8;
						reset_point_y = 30*8;
						Recover.x = Enemy[i].x + 8;
						Recover.y = Enemy[i].y + 8;
						Recover.isAlive = 1;
						Recover.direction = 0;
						soundPlay(SOUND_TYPE_1UP);
						player_HP_MAX += 50;
						player_HP = player_HP_MAX;
					}
					else if(Enemy[i].type == THIRD_BOSS){
						for(i=0;i<MAP_WIDTH;i++){
							if(mmap[i][38] == BA)mmap[i][38] = EM;
							if(mmap[i][39] == BA)mmap[i][39] = EM;
						}
						if(boss_flags >= 8){
							move_setEnemy( 50*8 , 40*8   , LAST_BOSS_F );
						}
						else boss_flags = 3;
						for(j=0;j<6;j++){
							move_setEnemy((uint16_t)(enemy_state[j+19][0]*8),(uint16_t)(enemy_state[j+19][1]*8),enemy_type[j+19]);
						}
						reset_point_x = 8*8;
						reset_point_y = 30*8;
						Recover.x = Enemy[i].x + 8;
						Recover.y = Enemy[i].y + 8;
						Recover.isAlive = 1;
						Recover.direction = 0;
						soundPlay(SOUND_TYPE_1UP);
						player_HP_MAX += 50;
						player_HP = player_HP_MAX;
					}
					else if(Enemy[i].type == LAST_BOSS_F){
						if(boss_flags >= 8){
							
						}
						else boss_flags = 4;
						_delay_ms(1000);
						for(j=0;j<64;j++){
							DrawLine(j,0,j,63,0);
							DrawLine(63-j,0,63-j,63,0);
							Glcd_Print_map();
							soundPlay(SOUND_TYPE_CANCAL);
						}
						for(i=0;i<8;i++){
							for(j=0;j<16;j++) Glcd_graphic(j*8,i*8,FL,0,0);
						}
						Glcd_Print_map();
						soundPlay(SOUND_TYPE_BOMB);
						_delay_ms(100);
						for(i=0;i<8;i++){
							for(j=0;j<16;j++) Glcd_graphic(j*8,i*8,EM,0,0);
						}
						Glcd_Print_map();
						_delay_ms(1000);
						move_setEnemy( 50*8 , 40*8   , LAST_BOSS_S );
					}
					else if(Enemy[i].type == LAST_BOSS_S){
						if(boss_flags >= 8){
							
						}
						else boss_flags = 5;
						soundPlay(SOUND_TYPE_1UP);
						_delay_ms(1000);
						
						for(j=0;j<64;j++){
							DrawLine(63-j,0,63-j,63,1);
							DrawLine(64+j,0,64+j,63,1);
							Glcd_Print_map();
							soundPlay(SOUND_TYPE_BOMB);
						}
						for(j=0;j<64;j++){
							DrawLine(63-j,0,63-j,63,0);
							DrawLine(64+j,0,64+j,63,0);
							Glcd_Print_map();
							soundPlay(SOUND_TYPE_BOMB);
						}
						_delay_ms(3000);
						move_setEnemy( 10*8 , 15*8+2 , FIRST_BOSS  );
						end_game();
					}
					Enemy[i].x = 3000;
					Enemy[i].y = 3000;
					soundPlay(SOUND_TYPE_1UP);
				}
				return 1;
			}
		}
	}
	return 0;
}

void end_game(){//エンディング
	uint8_t i,j;
	Glcd_str_wait(37,24,"Thank you",10);
	Glcd_str_wait(16,32,"For your playing!",10);
	_delay_ms(5000);
	for(j=0;j<64;j++){
		DrawLine(j+64,0,j+64,63,0);
		DrawLine(63-j,0,63-j,63,0);
		Glcd_Print_map();
	}
	Clear_Screen();
	_delay_ms(1000);
	Glcd_str(0,8,"flame count");
	Glcd_Print_map();
	_delay_ms(1500);
	soundPlay(SOUND_TYPE_KEY);
	Glcd_font(128-5,16,'0'+flame_count%10);
	if(flame_count>=10)Glcd_font(128-11,16,'0'+flame_count/10%10);
	if(flame_count>=100)Glcd_font(128-17,16,'0'+flame_count/100%10);
	if(flame_count>=1000)Glcd_font(128-23,16,'0'+flame_count/1000%10);
	if(flame_count>=10000)Glcd_font(128-29,16,'0'+flame_count/10000%10);
	if(flame_count>=100000)Glcd_font(128-35,16,'0'+flame_count/100000%10);
	if(flame_count>=1000000)Glcd_font(128-41,16,'0'+flame_count/1000000%10);
	if(flame_count>=10000000)Glcd_font(128-47,16,'0'+flame_count/10000000%10);
	Glcd_Print_map();
	soundPlay(SOUND_TYPE_1UP);
	_delay_ms(1500);
	Glcd_str(0,24,"your damage");
	Glcd_Print_map();
	soundPlay(SOUND_TYPE_KEY);
	_delay_ms(1500);
	Glcd_font(128-5,32,'0'+damage_count%10);
	if(damage_count>=10)Glcd_font(128-11,32,'0'+damage_count/10%10);
	if(damage_count>=100)Glcd_font(128-17,32,'0'+damage_count/100%10);
	if(damage_count>=1000)Glcd_font(128-23,32,'0'+damage_count/1000%10);
	if(damage_count>=10000)Glcd_font(128-29,32,'0'+damage_count/10000%10);
	Glcd_Print_map();
	soundPlay(SOUND_TYPE_1UP);
	_delay_ms(1500);
	Glcd_str(0,40,"shot number");
	Glcd_Print_map();
	soundPlay(SOUND_TYPE_KEY);
	_delay_ms(1500);
	Glcd_font(128-5,48,'0'+shot_count/10%10);
	if(shot_count>=10)Glcd_font(128-11,48,'0'+shot_count/10%10);
	if(shot_count>=100)Glcd_font(128-17,48,'0'+shot_count/100%10);
	if(shot_count>=1000)Glcd_font(128-23,48,'0'+shot_count/1000%10);
	if(shot_count>=10000)Glcd_font(128-29,48,'0'+shot_count/10000%10);
	if(shot_count>=100000)Glcd_font(128-35,48,'0'+shot_count/100000%10);
	if(shot_count>=1000000)Glcd_font(128-41,48,'0'+shot_count/1000000%10);
	if(shot_count>=10000000)Glcd_font(128-47,48,'0'+shot_count/10000000%10);
	Glcd_Print_map();
	soundPlay(SOUND_TYPE_1UP);
	_delay_ms(10000);
	for(j=0;j<64;j++){
		DrawLine(j+64,0,j+64,63,0);
		DrawLine(63-j,0,63-j,63,0);
		Glcd_Print_map();
	}
	if(boss_flags == 8){
		Glcd_str_wait(0,0,"ﾊﾞｸﾞ ﾋﾄﾞｲﾃﾞｼｮｳ?",10);
		_delay_ms(1000);
		Glcd_str_wait(0,8,"ｾｯｼｮｸ ﾌﾘｮｳ ﾓ",10);
		Glcd_str_wait(0,16,"ﾋﾄﾞｲｶﾗ ｱｿﾋﾞ",10);
		Glcd_str_wait(0,24,"ﾆｸｶｯﾀﾗ ｽｲﾏｾﾝ",10);
		_delay_ms(1000);
		Glcd_str_wait(0,32,"ｺﾒﾝﾄ ﾀﾞｲ2ﾀﾞﾝﾃﾞｼﾀ",10);
		_delay_ms(8000);
		boss_flags = 9;
	}
	else if(boss_flags == 9){
		Glcd_str_wait(0,0,"ｺﾉ ｻｸﾋﾝ ﾊ ｼﾞﾂﾊ",10);
		_delay_ms(1000);
		Glcd_str_wait(0,8,"ｹﾞﾝｻｸ ｶﾞ ｱﾘﾏｽ",10);
		_delay_ms(1000);
		Glcd_str_wait(0,16,"AVR_LCD_ｹﾞｰﾑ ﾄﾃﾞﾓ",10);
		Glcd_str_wait(0,24,"ﾈｯﾄﾃﾞ ｼﾗﾍﾞﾙﾄ",10);
		Glcd_str_wait(0,32,"ﾃﾞﾃｷﾏｽ｡",10);
		_delay_ms(1000);
		Glcd_str_wait(0,40,"ｼﾞﾌﾞﾝﾓ ﾋﾄﾂ ﾂｸﾘﾏｼﾀ",10);
		_delay_ms(8000);
		boss_flags = 10;
	}
	else if(boss_flags >= 10){
		Glcd_str_wait(0,0,"ｺﾒﾝﾄ ﾉ ﾃﾞｰﾀ ｼｮｳﾋｶﾞ",10);
		Glcd_str_wait(0,8,"ｴｸﾞｶｯﾀ ﾉﾃﾞ ｺﾝｶｲﾃﾞ",10);
		Glcd_str_wait(0,16,"ｺﾒﾝﾄﾊ ｵﾜﾘﾃﾞｽ",10);
		_delay_ms(1000);
		Glcd_str_wait(0,24,"ﾎﾝﾄﾆ ｱｿﾝﾃﾞｸﾚﾃ ｱﾘｶﾞﾄｳ",10);
		Glcd_str_wait(0,32,"--------------",10);
		_delay_ms(3000);
	}
	else{
		Glcd_str_wait(0,0,"ﾋﾄｺﾄ ｲｳﾄ･･･",10);
		_delay_ms(1000);
		Glcd_str_wait(0,8,"ｺｺﾏﾃﾞ ｱｿﾝﾃﾞｸﾚﾃ ｱﾘｶﾞﾄｳ",10);
		_delay_ms(1000);
		Glcd_str_wait(0,16,"ｱﾅﾀﾊ ｹｯｺｳ ",10);
		_delay_ms(1000);
		Glcd_str_wait(0,24,"ｺﾚﾃﾞ ｱｿﾋﾞ ﾏｼﾀﾈ?",10);
		_delay_ms(1000);
		Glcd_str_wait(0,32,"ﾀﾌﾞﾝ､ｻｷﾞｮｳ ｶﾞ ｱﾙﾊｽﾞ｡",10);
		_delay_ms(3000);
		boss_flags = 8;
	}
	for(j=0;j<64;j++){
		DrawLine(j+64,0,j+64,63,0);
		DrawLine(63-j,0,63-j,63,0);
		Glcd_Print_map();
	}
	Glcd_str_wait(52,20,"2018",10);
	Glcd_str_wait(9,28,"Graphic LCD SYSTEM",10);
	_delay_ms(1000);
	Glcd_str_wait(27,44,"presented by",10);
	Glcd_str_wait(28,52,"-----------",10);
	_delay_ms(3000);
	app_number = MENU;
}

////////////////////////////////////////////////////////////////////////////////
static const uint8_t ATAN_TABLE[33] PROGMEM = {
	0,  1,  3,  4,  5,  6,  8,  9, 10, 11, 12, 13, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 25, 26, 27, 28, 29, 29, 30, 31, 31, 32
};
////////////////////////////////////////////////////////////////////////////////
void initBullet(void)
{
	uint8_t i;
	for(i=0; i<BULLET_MAX; i++){
		Bullet[i].isAlive = 0;
	}
}

void shotBullet(int16_t x, int16_t y, int8_t vx, int8_t vy,uint8_t type)
{
	uint8_t i;
	for(i=0; i<BULLET_MAX; i++){
		if(Bullet[i].isAlive == 0){
			Bullet[i].x = x;
			Bullet[i].y = y;
			Bullet[i].vx = vx;
			Bullet[i].vy = vy;
			Bullet[i].isAlive = type;
			break;
		}
	}
}

void moveBullet(void)
{
	uint8_t i;
	for(i=0; i<BULLET_MAX; i++){
		if(Bullet[i].isAlive != 0){
			int16_t nnx = Bullet[i].x + Bullet[i].vx;
			int16_t nny = Bullet[i].y + Bullet[i].vy;
			if( nnx < 2 || nnx >= MAP_WIDTH*8 || nny < 2 || nny > MAP_HEIGHT*8 ){
				Bullet[i].isAlive = 0;
			}
			if( Bullet[i].x > player_x && Bullet[i].x < player_x+8 && Bullet[i].y > player_y && Bullet[i].y < player_y+16){
				soundPlay(SOUND_TYPE_DAMAGE);
				player_HP -= 5;
				damage_count += 5;
				Bullet[i].isAlive = 0;
			}
			if( map_block(Bullet[i].x , Bullet[i].y ) !=  FREE_BLOCK && Bullet[i].isAlive == 1){
				Bullet[i].isAlive = 0;
			}
			Bullet[i].x = nnx;
			Bullet[i].y = nny;
		}
	}

	if( Recover.x > player_x && Recover.x < player_x+8 && Recover.y > player_y && Recover.y < player_y+16 && Recover.isAlive != 0){
		soundPlay(SOUND_TYPE_1UP);
		player_HP_MAX += 50;
		player_HP = player_HP_MAX;
		Recover.isAlive = 0;
	}
}

void drawBullet(void)
{
	uint8_t i;
	int8_t graph_shot_y;
	
	for(i=0; i<BULLET_MAX; i++){
		if(Bullet[i].isAlive != 0){
			graph_shot_y=(Bullet[i].y-scroll_y-16);
			if(Bullet[i].x - scroll_x > 3 && Bullet[i].x - scroll_x < 125 && graph_shot_y > 3 && graph_shot_y <= 60 && Bullet[i].isAlive == 1) DrawFillCircle( Bullet[i].x - scroll_x, graph_shot_y, 2);
			if(Bullet[i].x - scroll_x > 3 && Bullet[i].x - scroll_x < 125 && graph_shot_y > 3 && graph_shot_y <= 60 && Bullet[i].isAlive == 2) DrawCircle( Bullet[i].x - scroll_x, graph_shot_y, 2);
		}
	}
	if(Recover.isAlive != 0){
		if(Recover.x - scroll_x > 3 && Recover.x - scroll_x < 125 && Recover.y-scroll_y-16 > 3 && Recover.y-scroll_y-16 <= 60) DrawCircle( Recover.x - scroll_x, Recover.y-scroll_y-16, Recover.direction + 1);
		Recover.direction += 1;
		if(Recover.direction >= 3) Recover.direction = 0;
	}
}

void initShot(void)
{
	uint8_t i;
	for(i=0; i<SHOT_MAX; i++){
		Shot[i].isAlive = 0;
	}
}

void shotShot(int16_t x, int16_t y, uint8_t direction)
{
	uint8_t i;
	if(ability(MISILE) && Misile_Shot_Able == 0 && Misile.isAlive == 0){
		soundPlay(SOUND_TYPE_BOMB);
		Misile_Shot_Able = 24;
		Misile.isAlive = 1;
		if(direction>>1%2==1 && direction%2==0){
			Misile.x = x+3;
			Misile.y = y-4;
			Misile.direction = UP;
		}
		else if(direction>>1%2==1 && direction%2==1){
			Misile.x = x-2;
			Misile.y = y-4;
			Misile.direction = UP;
		}
		else if(direction%2==0 && x<120){
			Misile.x = x+8;
			if(button_jadge(DOWN)) Misile.y = y+10;
			else Misile.y = y+8;
			Misile.direction = RIGHT;
		}
		else if(direction%2==1 && x>=2){
			Misile.x = x-2;
			if(button_jadge(DOWN)) Misile.y = y+10;
			else Misile.y = y+8;
			Misile.direction = LEFT;
		}
		shot_count++;
	}
	else{
		for(i=0;i<SHOT_MAX; i++){
			if(Shot[i].isAlive == 0){
				soundPlay(SOUND_TYPE_SHOT);
				if(direction>>1%2==1 && direction%2==0){
					Shot[i].x = x+4;
					Shot[i].y = y-2;
					Shot[i].direction = UP;
				}
				else if(direction>>1%2==1 && direction%2==1){
					Shot[i].x = x+2;
					Shot[i].y = y-2;
					Shot[i].direction = UP;
				}
				else if(direction%2==0 && x<120){
					Shot[i].x = x+8;
					if(button_jadge(DOWN)) Shot[i].y = y+10;
					else Shot[i].y = y+8;
					Shot[i].direction = RIGHT;
				}
				else if(direction%2==1 && x>=2){
					Shot[i].x = x-2;
					if(button_jadge(DOWN)) Shot[i].y = y+10;
					else Shot[i].y = y+8;
					Shot[i].direction = LEFT;
				}
				else break;
				shot_count++;
				Shot[i].isAlive = 1;
				break;
			}
		}
	}
}

void moveShot(void)
{
	uint8_t i;
	int16_t nx;
	int16_t ny;
	
	if(Misile.isAlive != 0){
		if(Misile.direction==RIGHT){
			nx = Misile.x + 4;
			ny = Misile.y;
		}
		else if(Misile.direction==LEFT){
			nx = Misile.x - 4;
			ny = Misile.y;
		}
		else if(Misile.direction==UP){
			nx = Misile.x;
			ny = Misile.y - 4;
		}
		else Misile.isAlive = 0;
		
		if( nx < 0 || nx >= SCREEN_WIDTH-2 || ny < 0 || ny > MAP_HEIGHT*8){
			Misile.isAlive = 0;
		}
		else if (mmap[Misile.y/8][(Misile.x+scroll_x)/8]==BL)
		{
			mmap[Misile.y/8][(Misile.x+scroll_x)/8] = EM;
			Misile.isAlive = 0;
		}
		else if (mmap[Misile.y/8][(Misile.x+scroll_x)/8]!=EM && ability(WAVE_SHOT) == 0){
			Misile.isAlive = 0;
		}

		if( move_hitEnemy(Misile.x + scroll_x, Misile.y,10) ){
			soundPlay(SOUND_TYPE_DAMAGE);
			Misile.isAlive = 0;
		}

		Misile.x = nx;
		Misile.y = ny;
	}
	else{
		Misile.x=0;
		Misile.y=0;
	}
	for(i=0; i<SHOT_MAX; i++){
		if(Shot[i].isAlive != 0){
			if(Shot[i].direction==RIGHT){
				nx = Shot[i].x + 4;
				ny = Shot[i].y;
			}
			else if(Shot[i].direction==LEFT){
				nx = Shot[i].x - 4;
				ny = Shot[i].y;
			}
			else if(Shot[i].direction==UP){
				nx = Shot[i].x;
				ny = Shot[i].y - 4;
			}
			else Shot[i].isAlive = 0;
			
			if( nx < 0 || nx >= SCREEN_WIDTH-2 || ny < 0 || ny > MAP_HEIGHT*8){
				Shot[i].isAlive = 0;
			}
			else if (mmap[Shot[i].y/8][(Shot[i].x+scroll_x)/8]!=EM && ability(WAVE_SHOT) == 0)
			{
				Shot[i].isAlive = 0;
			}

			if( move_hitEnemy(Shot[i].x + scroll_x, Shot[i].y,1)){
				Shot[i].isAlive = 0;
			}

			Shot[i].x = nx;
			Shot[i].y = ny;
		}
	}
}

void drawShot(void)
{
	uint8_t i;
	//int8_t graph_shot_x;
	int8_t graph_shot_y;
	if(Misile.isAlive != 0){
		graph_shot_y=Misile.y-scroll_y-16-3;
		if(Misile.direction == RIGHT)Glcd_graphic(Misile.x,graph_shot_y,0x33,OR,0);
		else if(Misile.direction == LEFT)Glcd_graphic(Misile.x,graph_shot_y,0x33,OR,1);
		else if(Misile.direction == UP)Glcd_graphic(Misile.x,graph_shot_y,0x36,OR,0);
	}
	for(i=0; i<SHOT_MAX; i++){
		if(Shot[i].isAlive != 0){
				//graph_shot_x=(Shot[i].x-scroll_x)%256;
				graph_shot_y=(Shot[i].y-scroll_y-16)%256;
				if(ability(WAVE_SHOT)){
					if(Shot[i].x-1 >= 0 && graph_shot_y-1 >= 0)DrawRectangle(Shot[i].x-1,graph_shot_y-1,Shot[i].x+1,graph_shot_y+1);
				}
				else {
					//DrawRectangle(Shot[i].x,graph_shot_y,Shot[i].x+1,graph_shot_y+1);
					DrawDot(Shot[i].x, graph_shot_y);
					DrawDot(Shot[i].x, graph_shot_y+1);
					DrawDot(Shot[i].x+1, graph_shot_y);
					DrawDot(Shot[i].x+1, graph_shot_y+1);
				}
		}
	}
}

void graph_player(uint8_t x,uint8_t y,uint8_t direction,uint8_t mothion){
	uint8_t headmotion=0;
	if(mothion==RASE) headmotion=RASE-1;
	else if(mothion==RASE_SHOOT) headmotion=RASE_SHOOT-1;

	if(direction%2==0){
		if((direction & 0b00000100)>>2 == 0b00000001) Glcd_graphic(x,y+2,headmotion,OR,0);
		else Glcd_graphic(x,y,headmotion,OR,0);
		Glcd_graphic(x,y+8,mothion,OR,0);
	}
	else{
		if((direction & 0b00000100)>>2 == 0b00000001) Glcd_graphic(x,y+2,headmotion,OR,1);
		else Glcd_graphic(x,y,headmotion,OR,1);
		Glcd_graphic(x,y+8,mothion,OR,1);
	}
}

void graph_map(){
	for(uint8_t i=0;i<17;i++){
		for (uint8_t j=0;j<9;j++){
			if(i==0 && j==0) Glcd_graphmap(0,0,mmap[j+scroll_y/8+2][i+scroll_x/8],PUT,0,scroll_x%8,scroll_y%8);
			else if(i!=0 && j==0) Glcd_graphmap(i*8-scroll_x%8,0,mmap[j+scroll_y/8+2][i+scroll_x/8],PUT,0,0,scroll_y%8);
			else if(i==0 && j!=0) Glcd_graphmap(0,j*8-scroll_y%8,mmap[j+scroll_y/8+2][i+scroll_x/8],PUT,0,scroll_x%8,0);
			else Glcd_graphmap(i*8-scroll_x%8,j*8-scroll_y%8,mmap[j+scroll_y/8+2][i+scroll_x/8],PUT,0,0,0);
			if(i+scroll_x/8 < 0 || i+scroll_x/8 >= MAP_WIDTH) Glcd_graphmap(i*8,j*8,mmap[j+2][i],PUT,0,0,0);
		}
	}
}

int8_t drop(uint16_t x,uint16_t y,uint8_t *dropt,uint8_t *jump_f,int16_t *hp){//プレイヤーの上下移動関数
	int8_t droplog[11]={-16,-10,-8,-4,-2,0,2,4,8,10,10};
	int8_t i; 
	if(*jump_f==0 && (mmap[y/8+2][x/8]==EM || mmap[y/8+2][x/8]==FB || mmap[y/8+2][x/8]==WA || mmap[y/8+2][x/8]==PO || mmap[y/8+2][x/8]==IT) && (mmap[y/8+2][(x+7)/8]==EM || mmap[y/8+2][(x+7)/8]==FB || mmap[y/8+2][(x+7)/8]==WA || mmap[y/8+2][(x+7)/8]==PO || mmap[y/8+2][(x+7)/8]==IT)){
		*jump_f = 1;
		*dropt = 5;
	}
	else if(*jump_f==0 && (mmap[y/8+2][x/8]==NE && mmap[y/8+2][(x+7)/8]==NE)){
		*jump_f = 1;
		*dropt = 2;
	}
	else if(*jump_f==0 && (mmap[y/8+2][x/8]==JU && mmap[y/8+2][(x+7)/8]==JU)){
		*jump_f = 1;
		*dropt = 0;
	}
	if(*jump_f>0){
		if(droplog[(*dropt)+1]>0){
		for(i=0;i<droplog[(*dropt)+1];i++){
			if(mmap[(y+i+16)/8][x/8]==NE || mmap[(y+16+i)/8][(x+7)/8]==NE){
				*jump_f = 1;
				*dropt = 2;
				*hp -= 3;
				damage_count += 3;
				return i;
			}
			else if(mmap[(y+i+16)/8][x/8]==JU || mmap[(y+16+i)/8][(x+7)/8]==JU){
				*jump_f = 1;
				*dropt = 0;
				return i;
			}
			else if(map_block(x,y+i+16) == WALL_BLOCK || map_block(x,y+i+16) == IRON_BLOCK || map_block(x+7,y+i+16) == WALL_BLOCK || map_block(x+7,y+i+16) == IRON_BLOCK){
				*jump_f = 0;
				*dropt = 5;
				return i;
			}
		}
		}
		else if(droplog[(*dropt)+1]<0){
			for(i=0;i<-droplog[(*dropt)+1];i++){
				if((mmap[(y-i)/8][x/8]==EM || mmap[(y-i)/8][x/8]==FB || mmap[(y-i)/8][x/8]==IB || mmap[(y-i)/8][x/8]==WA || mmap[(y-i)/8][x/8]==PO || mmap[(y-i)/8][x/8]==IT)==false || (mmap[(y-i)/8][(x+7)/8]==EM || mmap[(y-i)/8][(x+7)/8]==FB || mmap[(y-i)/8][(x+7)/8]==IB || mmap[(y-i)/8][(x+7)/8]==WA || mmap[(y-i)/8][(player_x+7)/8]==PO || mmap[(y-i)/8][(player_x+7)/8]==IT)==false){
					*dropt = 5;
					return -i;
				}
			}
		}
		*dropt += 1;
	}
	if(*dropt >= 9) *dropt = 9;
	return droplog[*dropt];
}

void move_main(){
	uint8_t i,j;
	uint8_t alart_timer = 0;
	initShot();
	initBullet();
	move_initEnemy();
	player_x = reset_point_x;
	player_y = reset_point_y;
	if(boss_flags == 0){
		for(i=0;i<5;i++){
			move_setEnemy((uint16_t)(enemy_state[i][0]*8),(uint16_t)(enemy_state[i][1]*8),enemy_type[i]);
		}
	}
	else if(boss_flags == 1){
		for(i=0;i<8;i++){
			move_setEnemy((uint16_t)(enemy_state[i+5][0]*8),(uint16_t)(enemy_state[i+5][1]*8),enemy_type[i+5]);
		}
	}
	else if(boss_flags == 2){
		for(i=0;i<6;i++){
			move_setEnemy((uint16_t)(enemy_state[i+13][0]*8),(uint16_t)(enemy_state[i+13][1]*8),enemy_type[i+13]);
		}
	}
	else if(boss_flags == 3){
		for(i=0;i<6;i++){
			move_setEnemy((uint16_t)(enemy_state[i+19][0]*8),(uint16_t)(enemy_state[i+19][1]*8),enemy_type[i+19]);
		}
	}
	else if(boss_flags >= 8){
		for(i=0;i<10;i++){
			move_setEnemy((uint16_t)(enemy_state[i*2][0]*8),(uint16_t)(enemy_state[i*2][1]*8),enemy_type[i*2]);
		}
		move_setEnemy( 10*8 , 15*8+2 , FIRST_BOSS  );
	}
	uint8_t walks=0;
	uint8_t mothion=0;
	uint8_t graph_player_x=0;
	uint8_t graph_player_y=0;
	uint8_t player_direction=0;
	while(app_number == MOVE){
		Clear_Screen();

		if(mmap[player_y/8+1][player_x/8] == IT || mmap[player_y/8+1][(player_x+1)/8+1] == IT){
			move_alart = 1;
			for (i=0;i<8;i++){
				if((player_ability>>i & 0b00000001) == 0){
					player_ability |= (0b00000001 << i);
					break;
				}
			}
			if( ability(WAVE_SHOT) ) SHOT_MAX = 4;
			if( ability(WIDE_SHOT) ) SHOT_MAX = 12;
			if( ability(MANY_SHOT) ) SHOT_MAX = 50;


			if     ( mmap[ player_y/8 + 1 ][ player_x/8        ] == IT) mmap[ player_y/8 + 1 ][ player_x/8         ] = EM;
			else if( mmap[ player_y/8 + 1 ][ (player_x+1)/8 + 1] == IT) mmap[ player_y/8 + 1 ][ (player_x+1)/8 + 1 ] = EM;
		}
		else if( mmap[ player_y/8 + 1 ][ player_x/8 ] == KE || mmap[ player_y/8 + 1][ (player_x+1)/8 + 1 ] == KE){

			if     ( mmap[ player_y/8 + 1 ][ player_x/8         ] == KE) mmap[ player_y/8 + 1 ][ player_x/8         ] = EM;
			else if( mmap[ player_y/8 + 1 ][ (player_x+1)/8 + 1 ] == KE) mmap[ player_y/8 + 1 ][ (player_x+1)/8 + 1 ] = EM;

					if     (boss_flags == 0) move_setEnemy( 10*8 , 15*8+2 , FIRST_BOSS  );
					else if(boss_flags == 1) move_setEnemy( 9*8  , 19*8   , SECOND_BOSS );
					else if(boss_flags == 2) move_setEnemy( 12*8 , 40*8   , THIRD_BOSS  );
					else if(boss_flags == 3) move_setEnemy( 50*8 , 40*8   , LAST_BOSS_F );
		}

		//RIGHT
		else if( (mothion==LOWER || mothion==RASE || mothion==LOWER_SHOOT || mothion==RASE_SHOOT) == false && button_jadge(RIGHT) ){
			
			if( ability(FAST_MOVE) ) move_player_move( player_x , player_y , 4 , 0 );
			else					 move_player_move( player_x , player_y , 2 , 0 );
			
			player_direction &= 0b11111110;
			mothion = walks + WALK1;
			walks++;
			if(walks >= 3) walks = 0;
		}
		//LEFT
		else if( (mothion==LOWER || mothion==RASE || mothion==LOWER_SHOOT || mothion==RASE_SHOOT) == false && button_jadge(LEFT)){
			
			if( ability(FAST_MOVE) ) move_player_move( player_x , player_y , -4 , 0 );
			else					 move_player_move( player_x , player_y , -2 , 0 );
			
			player_direction |= 0b00000001;
			mothion = walks + WALK1;
			walks++;
			if(walks >= 3) walks = 0;
		}
		else{
			mothion = NORMAL;
			walks = 0;
		}
		if(button_jadge(A) && (player_jump_f==0 || (player_dropt>=6 && ((player_jump_f==1 && ability(TWICE_JUMP)) || ability(INFINITY_JUMP))))){
			soundPlay(SOUND_TYPE_JUMP);
			player_jump_f += 1;
			if(ability(INFINITY_JUMP)) player_jump_f = 1;
			player_dropt = 1;
			mothion = WALK1;
		}

		if(button_jadge(UP)){
			player_direction |= 0b00000010;
			mothion = RASE;
		}
		else player_direction &= 0b11111101;

		if(player_dropt == 5 && button_jadge(DOWN)){
			player_direction |= 0b00000100;
			mothion = LOWER;
		}
		else player_direction &= 0b11111011;
		
		if(button_jadge(B)){
			
			if      ( mothion <= WALK3 ) mothion = SHOOT;
			else if ( mothion == LOWER ) mothion = LOWER_SHOOT;
			else if ( mothion == RASE  ) mothion = RASE_SHOOT;

			shotShot( graph_player_x , player_y , player_direction );

			if(ability(WIDE_SHOT)){
				if(mothion == RASE){
					shotShot( graph_player_x + 2 , player_y     , player_direction );
					shotShot( graph_player_x - 2 , player_y	    , player_direction );
				}
				else{
					shotShot( graph_player_x	 , player_y + 2 , player_direction );
					shotShot( graph_player_x	 , player_y - 2 , player_direction );
				}
			}
		}

		if     ( button_jadge(START)  ) start_button();
		else if( button_jadge(SELECT) ) move_select_button();

		if( move_hitEnemy( player_x , player_y , 0 ) || move_hitEnemy( player_x , player_y + 8 , 0 )){
			player_jump_f = 1;
			player_dropt = 2;
			player_y -= 8;
			player_HP -= 10;
			damage_count += 10;
		}
		else player_y += drop( player_x , player_y , &player_dropt , &player_jump_f , &player_HP );
		
		graph_player_x = ( player_x - scroll_x      ) % 256;
		graph_player_y = ( player_y - scroll_y - 16 ) % 256;
		if(player_x >= 64 && player_x < MAP_WIDTH*8 - 157){
			scroll_x = player_x - 64;
		}
		if(graph_player_x < 56 && scroll_x > 0 && player_x < MAP_WIDTH*8-157){
			scroll_x = player_x - 64;
		}
		
		if(scroll_x < 0 || scroll_x > MAP_WIDTH*8 ) scroll_x = 0;

		if(graph_player_y > 40 && player_y < MAP_HEIGHT*8){
			if(scroll_y < player_y - ( 40 + 16 )){
				if     ((player_y - ( 40 + 16 )) - scroll_y >= 16 ) scroll_y += 10;
				else if((player_y - ( 40 + 16 )) - scroll_y >= 4  ) scroll_y += 4;
				else												scroll_y += 2;
			}
		}
		else if((graph_player_y<16 || player_y-16 <= scroll_y) && scroll_y>0 && player_y<MAP_HEIGHT*8){
			if(scroll_y > (player_y - ( 40 + 16 ))){
				scroll_y -= 4;
			}
			else if(player_y - 16 -scroll_y < 0){
				scroll_y -= 2;
			}
		}
		if(scroll_y<0 || scroll_y > MAP_HEIGHT * 8) scroll_y=0;
		if(mmap[player_y/8][player_x/8]==PO || mmap[player_y/8+1][player_x/8]==PO || mmap[player_y/8][player_x/8+1]==PO || mmap[player_y/8+1][player_x/8+1]==PO){
			player_HP--;
			damage_count++;
		}
		
		moveShot();
		move_moveEnemy();
		moveBullet();

		graph_map();

		drawShot();
		drawBullet();
		move_drawEnemy();

		graph_player(graph_player_x,graph_player_y,player_direction,mothion);
		if(player_HP<0) player_HP=0;
		if(player_HP<=0) gameover();
		if(player_HP >= 100){
			Glcd_str(0,0,"   ");
			Glcd_font(0,0,player_HP/100%10+'0');
			Glcd_font(6,0,player_HP/10%10+'0');
			Glcd_font(12,0,player_HP%10+'0');
			if(ability(MISILE)){
				Glcd_graphmap(18,0,EM,PUT,0,0,0);
				Glcd_graphmap(18,0,0x33,PUT,0,Misile_Shot_Able/4,0);
			}
		}
		else{
			Glcd_str(0,0,"  ");
			Glcd_font(0,0,player_HP/10%10+'0');
			Glcd_font(6,0,player_HP%10+'0');
			if(ability(MISILE)){
				Glcd_graphmap(12,0,EM,PUT,0,0,0);
				Glcd_graphmap(12,0,0x33,PUT,0,Misile_Shot_Able/4,0);
			}
		}
		if(move_alart > 0 && alart_timer == 0) alart_timer = 50;
		else if(move_alart > 0 && alart_timer > 0) alart_timer--;

		if(alart_timer <= 0){
			move_alart = 0;
			alart_timer = 0;
		}
		else if(alart_timer > 0 && move_alart == 1){
			for (i=0;i<8;i++){
				if((player_ability>>i & 0b00000001) == 0){
					Glcd_str(0,0,"                ");
					if     (i == 1) Glcd_str(0,0,"GOT TWICEJUMP"   );
					else if(i == 2) Glcd_str(0,0,"GOT MISILE"      );
					else if(i == 3) Glcd_str(0,0,"GOT WAVESHOT"    );
					else if(i == 4) Glcd_str(0,0,"GOT FASTMOVE"    );
					else if(i == 5) Glcd_str(0,0,"GOT EARLYMISILE" );
					else if(i == 6) Glcd_str(0,0,"GOT WIDESHOT"    );
					else if(i == 7) Glcd_str(0,0,"GOT INFINITYJUMP");
					if     (i == 8) Glcd_str(0,0,"GOT MANYSHOT"    );
					break;
				}
				if(ability(MANY_SHOT)) Glcd_str(0,0,"GOT MANYSHOT");
			}
		}
		else if(alart_timer > 0 && move_alart == 2){
					Glcd_str(0,0,"          ");
					if     ( boss_flags == 0) Glcd_str(0,0,"FIRSTBOSS ");
					else if( boss_flags == 1) Glcd_str(0,0,"SECONDBOSS");
					else if( boss_flags == 2) Glcd_str(0,0,"THIRDBOSS ");
					else if( boss_flags == 3) Glcd_str(0,0,"LASTBOSS  ");
					else if( boss_flags >= 8) Glcd_str(0,0,"BOSS RUSH!");
		}

		Glcd_Print_map();
		if(Misile_Shot_Able > 0){
			if(ability(FAST_MISILE)) Misile_Shot_Able -= 3;
			else Misile_Shot_Able--;
			
			if(Misile_Shot_Able <= 0) Misile_Shot_Able = 0;
		}
		flame_count++;
	}
}