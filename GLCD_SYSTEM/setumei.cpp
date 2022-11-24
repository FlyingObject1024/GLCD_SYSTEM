/*
 * setumei.cpp
 *
 * Created: 2018/09/17 1:26:17
 *  Author: kodai
 */ 
 
  #include <avr/io.h>
  #include <stdlib.h>//�����p
  #include <avr/interrupt.h>
  #include <avr/pgmspace.h>
  //PROGMEM�Ŕz�u
  //pgm_read_byte(&�ϐ�);�ŌĂяo��

  #include "Glcd_function.h"
  #include "defines.h"
  #include "sound.h"
  #include <util/delay.h>

  //�u��Ҳ�v�̃��C���֐�
 void setumei_main(){
	Clear_Screen();
	Glcd_str_wait(0,0,"�� �����",10);
	Glcd_str_wait(0,8,"GLCD SYSTEM � ��Ͻ�",10);
	_delay_ms(2000);
	Glcd_str_wait(0,16,"���� �ƹް� ��� �޽�",10);
	_delay_ms(2000);
	Glcd_str_wait(0,24,"----------------",10);
	_delay_ms(4000);
	Clear_Screen();
	
	Glcd_str_wait(0,0,"����� ��Ҳ�޽�",10);
	_delay_ms(3000);
	Clear_Screen();

	 while(app_number == SETUMEI){

		 Glcd_remocon_graph();
		 Glcd_str(0,56,"�߰�� �� ����");
		 Glcd_Print_map();
		 if(button_jadge(START)) start_button();
		 else if(button_jadge(B)) app_number = MENU;
	 }
 }
 //////////////////////////////////////////////////////////////////////////