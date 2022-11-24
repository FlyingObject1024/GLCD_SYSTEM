/*
 * setumei.cpp
 *
 * Created: 2018/09/17 1:26:17
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

  //「ｾﾂﾒｲ」のメイン関数
 void setumei_main(){
	Clear_Screen();
	Glcd_str_wait(0,0,"ｺﾉ ｻｸﾋﾝﾊ",10);
	Glcd_str_wait(0,8,"GLCD SYSTEM ﾄ ｲｲﾏｽ｡",10);
	_delay_ms(2000);
	Glcd_str_wait(0,16,"ｲﾜﾕﾙ ﾐﾆｹﾞｰﾑ ｼｭｳ ﾃﾞｽ｡",10);
	_delay_ms(2000);
	Glcd_str_wait(0,24,"----------------",10);
	_delay_ms(4000);
	Clear_Screen();
	
	Glcd_str_wait(0,0,"ﾎﾞﾀﾝﾉ ｾﾂﾒｲﾃﾞｽ｡",10);
	_delay_ms(3000);
	Clear_Screen();

	 while(app_number == SETUMEI){

		 Glcd_remocon_graph();
		 Glcd_str(0,56,"ﾎﾟｰｽﾞ ﾃﾞ ﾓﾄﾞﾙ");
		 Glcd_Print_map();
		 if(button_jadge(START)) start_button();
		 else if(button_jadge(B)) app_number = MENU;
	 }
 }
 //////////////////////////////////////////////////////////////////////////