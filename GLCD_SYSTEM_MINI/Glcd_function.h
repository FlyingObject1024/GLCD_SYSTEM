//グラフィックLCD用の置き換えや関数読み込み用ヘッダファイル
#define SYSTEM_MODE 1
#define BOARD 1
#define BREAD 0//ここら辺はブレッドボードと基盤の配線が違ってた時に使っていた
//この辺は「開成物理部テキスト　マイコン編」でどうぞ
#ifndef GLCD_FUNCTION_H_
#define GLCD_FUNCTION_H_
#define DATA_PORT PORTA
#define FUNC_PORT PORTC
#define PIN_DI 4
#define PIN_RW 3
#define PIN_E 2
#define GLCD_CS1 7
#define GLCD_CS2 6


#define TRUE 1
#define FALSE 0
#define PUT 0
#define AND 1
#define OR 2
#define wait() _delay_loop_1(20)
/*****分身の宣言*****/
extern uint64_t Glcd_map[128];
/*****IC1 と IC2 の選択*****/
extern void Glcd_SelectChip(uint8_t IC1,uint8_t IC2);
extern void Glcd_Disp(uint8_t state);
/*****Set Start Address*****/
extern void Glcd_StartLine(uint8_t line);
/*****Set Page Address*****/
extern void Glcd_SetPage(uint8_t page);
/*****Set Column Address*****/
extern void Glcd_SetColumn(uint8_t column);
/*****Write Data*****/
extern void Glcd_Write(uint8_t data);
/*****座標 (X,Y) に点を打つ*****/
extern void DrawDot(uint8_t x,uint8_t y);
/*****(x1,y) から (x2,y) まで横線を引く*****/
extern void DrawLine_line(uint8_t y,uint8_t x1,uint8_t x2);
/*****(x,y1) から (x,y2) まで縦線を引く*****/
extern void DrawLine_column(uint8_t x,uint8_t y1,uint8_t y2);
/*****(x1,y1) から (x2,y2) まで線を引く*****/
extern void DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t color);
/*****四角形を描画する*****/
extern void DrawRectangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);
/*****円を描画する*****/
extern void DrawCircle(uint8_t x0, uint8_t y0, uint8_t r);
/*****塗りつぶし円を描画する*****/
extern void DrawFillCircle(uint8_t x0, uint8_t y0, uint8_t r);
/*****Glcd_map[128] を液晶本体に転送*****/
extern void Glcd_Print_map();
/*****Glcd_map[128] の全てのデータを消す*****/
extern void Clear_Screen();
/***** (X,Y,"文字列") 文字列表示*****/
extern void Glcd_font(uint8_t start_x,uint8_t start_y,char font_num);
/*****小さい文字　未完成*****/
//void Glcd_minifont(uint8_t start_x,uint8_t start_y,uint8_t font_num);
/***** (X,Y,"文字列",次の文字表示までの時間) １文字ずつ文字列表示*****/
extern void Glcd_str(uint8_t start_x,uint8_t start_y,char *str);
/***** (X,Y,画像番号,重ね設定,左右反転) graphicsに入った8x8の画像表示*****/
extern void Glcd_graphic(uint8_t start_x,uint8_t start_y,char graph_num,uint8_t modes,uint8_t re);
/***** (X,Y,画像番号,分割番号,重ね設定,左右反転) graphicsに入った4x4の画像表示*****/
extern void Glcd_graphic_mini(uint8_t start_x,uint8_t start_y,char graph_num,char mini_num,uint8_t modes,uint8_t re);
/***** (X,Y,画像番号,重ね設定,左右反転) graphics2に入った8x8の画像表示*****/
//extern void Glcd_graphic2(uint8_t start_x,uint8_t start_y,char graph_num,uint8_t modes,uint8_t re);
/***** (X,Y,画像番号,重ね設定,左右反転,縦削り,横削り) graphicsに入った8x8の画像表示(マップ表示用)*****/
extern void Glcd_graphmap(uint8_t start_x,uint8_t start_y,char graph_num,uint8_t modes,uint8_t re,uint8_t del,uint8_t dely);
/*****　リモコンの説明画像表示　*****/
extern void Glcd_remocon_graph();

extern void EraseRectangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);
extern void Glcd_Erase_Line_column(uint8_t x,uint8_t y1,uint8_t y2);
extern void Glcd_Erase_Line_line(uint8_t y,uint8_t x1,uint8_t x2);
/***** (X,Y,"文字列",次の文字表示までの時間) １文字ずつ文字列表示*****/
extern void Glcd_str_wait(uint8_t start_x,uint8_t start_y,char *str,uint8_t lag);
#endif