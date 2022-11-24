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

volatile uint8_t sound_switch=0;//音のオンオフ
volatile uint8_t sig=0;//1p用リモコンの信号保存変数
volatile uint8_t last_button=0;//最後に押したボタン、長押し判定のために作ったが使ってないはず
volatile uint8_t app_number=MENU;//実行するアプリの変数
//volatile は割り込みが発生したとき変数の値が勝手に訂正されないようにするもの
volatile uint8_t connection_flag = false;//2pリモコンが繋がってるかつながってないか判定しようとしたもの。結局ノイズのせいで意味をなしてない。
//このフラグが立っているとメニュー画面右上に雷が表示
volatile uint8_t opponents_sig = 0;//2p用リモコンの信号保存変数

volatile uint8_t c_emagency = 0;//これ分からん

volatile uint64_t system_clock=0;

void menu_main();//自作関数のプロトタイプ宣言。先に宣言することで、呼び出し以降に宣言されているというエラーを防止する。
void setumei_main();


extern void tetris_main();//それぞれのアプリのメイン関数をこのmainファイルでも読み込めるようにしてある。
extern void jump_main();//理解するのには苦労した
extern void move_main();
extern void shooting_main(void);
extern void boardrace_main(void);
extern void snake_main(void);

extern void timer_main();


ISR( USART0_RX_vect , ISR_BLOCK){
	sig=UDR0;
}

ISR( USART1_RX_vect , ISR_BLOCK){
	connection_flag = true;
	opponents_sig = UDR1;
}
//この二つはリモコンの信号の受信。ISR_BLOCKで割り込み中にほかの割り込みが発生しないようにしている
typedef void (*FUNCPTR)(void);
//ここから「構造体」を使った部分となる。理解が非常に難しい。そのうえこの構造体はポインタを使っているので余計難しさに拍車がかかっている
typedef struct {
	char *name;
	FUNCPTR main;
}APP_FUNC;
//構造体が持つ関数、変数を宣言している。この構造体はそれぞれのアプリのメイン関数、アプリ名(表示されるもの、つまり文字コード)を扱う。
//ここでは「構造体」を宣言したのであって「アプリ」を宣言したのではない
APP_FUNC	AppFunc[] = {
	{ "TIMER",timer_main},
	{ "TETRIS", tetris_main},
	{ "JUMP", jump_main},
	{ "MOVE", move_main},
	{ "SHOOTING" , shooting_main},
	{ "BOARDRACE" , boardrace_main},
	{ "SNAKE" , snake_main},
	{ NULL, NULL }
};
//それぞれのアプリの宣言を行っている。NULLというのは終端を表している。ここでは二次元配列の形をとっている(私もよくわかっていない)
void menu_main(void)//アプリ番号２０　メニュー画面のメイン関数
{
	uint8_t i = 0;//ループ用変数、uint8_t　というのは　unsigned char と同じ。 8という数字はビット数を表す
	volatile static int8_t menu_cursor=0;//カーソルがある場所を表す変数。例えば　0　だとアプリ　ｾﾂﾒｲ　にカーソルがあっている。
	volatile static int8_t page_cursor=0;//ページをスクロールするための変数
	while(app_number==MENU){//アプリ用変数と実行中アプリはこのように照合している。
		i=0;//ループ用変数を0にしている
		Clear_Screen();//画面を白にする
		while( AppFunc[i].main != NULL ){//終端が現れるまで構造体を調べ上げる
			if(i<8)Glcd_str( 8, i*8, AppFunc[i+page_cursor].name);//画面にアプリ名を表示している。画面に8個しか表示できないのでiで判定している。
			i++;
		}
	Glcd_font(0,(menu_cursor - page_cursor)*8,'}'+1);//カーソルの表示
	if(page_cursor>0) Glcd_font(64,48,'ﾟ'+1);
	else Glcd_font(64,48,' ');
	if(page_cursor+7<i && page_cursor+8 < i)Glcd_font(64,56,'ﾟ'+2);//カーソルを合わせているアプリが９個目以降ならカーソルを一番下に表示するようにしている。
	else Glcd_font(64,56,' ');
	Glcd_str(72,8,"GLCD\n\r  SYSTEM");
	Glcd_str(69,24,"mini");
	/*Glcd_str(72,48,"SOUND");
	if (sound_switch) Glcd_str(102,48," ON");
	else Glcd_str(102,48," OFF");*/
	DrawLine(80,50,80,52,1);
	DrawLine(81,50,81,52,1);
	DrawLine(82,50,82,52,1);
	DrawLine(83,49,83,53,1);
	DrawLine(84,48,84,54,1);
	if(sound_switch){
		DrawDot(86,50);DrawDot(87,51);DrawDot(86,52);
		DrawDot(88,49);DrawLine(89,50,89,52,1);DrawDot(88,53);
		DrawDot(90,48);DrawLine(91,49,91,53,1);DrawDot(90,54);
	}
	else{
		DrawLine(86,48,92,54,1);
		DrawLine(86,54,92,48,1);
	}

	Glcd_Print_map();//データを画面に表示

	if(menu_cursor > 0 && button_jadge_one() == UP){//上ボタンを押した場合カーソルを上に移動

		menu_cursor--;
		soundPlay(SOUND_TYPE_KEY);//効果音

	}
	else if(menu_cursor < i-1 && button_jadge_one() == DOWN){//下ボタン

		menu_cursor++;
		soundPlay(SOUND_TYPE_KEY);

	}
	else if(button_jadge_one() == A && last_button != A){//Aボタン　アプリ終了時にAを押しているをまたアプリに入るのでその防止をしている

		app_number = menu_cursor;//カーソルの位置を実行アプリ変数に代入。
		soundPlay(SOUND_TYPE_ENTER);

	}
	else if(button_jadge_one() == SELECT){//SELECTボタン

		if(sound_switch) sound_switch = 0;
		else sound_switch = 1;//効果音のオンオフ

	}

	if(menu_cursor >= 8) page_cursor = menu_cursor - 7;//カーソル位置からスクロールする数を算出
	else if(menu_cursor - page_cursor < 0) page_cursor--;
	_delay_ms(100);
}
}

#define TIMER_FREQ	60	//[Hz]//この辺は「AVRMIniGame」からの引用。ＡＶＲ内蔵のタイマーとPWMをを使い、音を出している
void timer1Init(void)
{
	// TCCR1A ... COM1A1 COM1A0 COM1B1 COM1B0 -     -    WGM11 WGM10
	// TCCR1B ... ICNC1  ICES1  -      WGM13  WGM12 CS12 CS11  CS10
	// COM1A[1:0] = 00		// Normal Operation
	// COM1B[1:0] = 00		// Normal Operation
	// ICNC1      = 0		// Input Capture Noise Canceler
	// ICES1      = 0		// Input Capture Edge Select
	// WGM1[3:0]  = 0100	// CTC mode with top value OCR1A
	// CS0[2:0]   = 010		// Clock Select = Clock/8

	cli();	// Disable Interrupt

	TCCR1A = 0b00000000;
	TCCR1B = 0b00001010;
	TCCR1C = 0;

	OCR1AH = ( ((F_CPU/(TIMER_FREQ*8))-1)>>8 ) & 0x00FF;
	OCR1AL = ( ((F_CPU/(TIMER_FREQ*8))-1)    ) & 0x00FF;

	OCR1BH = 0;		// Not Used
	OCR1BL = 0;		// Not Used
	
	TIMSK1 = 0b00000010;	// Compare A Match Interrupt Enable

	sei();	// Enable Interrupt
}

ISR(TIMER1_COMPA_vect)
{
	volatile static uint8_t counter = 0;
	volatile static uint8_t second_counter = 0;
	soundEngine();
	counter++;
	if(counter == 2){
		counter = 0;
	}
	second_counter++;
	if(second_counter % 6 == 0){
		system_clock++;
		second_counter == 0;
		if(second_counter % 180 == 0){
			system_clock--;
			second_counter = 0;
		}
	}
	
}
//タイマーの値を参考にしたタイマ割り込み
int main(void)//大元のメイン関数
{
	//InitI2c();
	DDRA=0xff;//この辺は「テキスト集」の「AVR入門」で読んで下さい
	DDRB=0xff;//Arduino的に言うと「pinMode」です。
	DDRC=0xfc;
	PORTA=0;//ここで、最初に出力をオフにすることで、不具合対策をしている
	PORTB=0;//この文はArdiono的に言うと「digitalWrite」
	PORTC=0b00010000;
	//シリアル通信の設定。詳しいことは「開成物理部テキスト　マイコン編」もしくは動画「零からの電子工作(零電)」の「シリアル通信」でどうぞ。
	UCSR0A=0b00000000;
	UCSR0B=0b10010000;//受信・割り込み有効
	UCSR0C=0b00100110;//非同期通信・偶数パリティ8bit
	UCSR1A=0b00000000;
	UCSR1B=0b10010000;//受信有効・割り込みあり
	UCSR1C=0b00100110;//非同期通信・偶数パリティ8bit
	//奇数...odd//偶数...even
	if(F_CPU == 12000000){
		UBRR0=259;//1M 12/8M 103
		UBRR1=259;
	}
	else if(F_CPU == 8000000){
		UBRR0=103;//1M 12/8M 103
		UBRR1=103;
	}//マイコンのクロック違いで、ボーレートを調整している。
	//通常のAVRのクロックは書き込み時のfuses 項目の　CKDIV8　のチェックで８分の１になっているので、ここをいじるとクロック制限を解除できる。
	//このGLCD SYSTEMでは制限を解除しないととても重くて使い物にならない
	timer1Init();//Initというのは「初期化」を表す。
	soundInit();

	//EIMSK=0b01000000;
	//MCUCR=0b00000010;

	sei();//割り込みの許可
	Glcd_SelectChip(TRUE,TRUE);//この辺はグラフィックLCDの設定。詳しくは「開成物理部テキスト　マイコン編」まで。というか丸コピ
	Glcd_Disp(TRUE);
	Glcd_SelectChip(TRUE,TRUE);
	Glcd_StartLine(0);
	Glcd_Print_map();
	while(1){
		if(app_number==MENU){
			rand();
			menu_main();
		}
		else{
			AppFunc[ app_number ].main();//アプリメイン関数の管理
		}
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////