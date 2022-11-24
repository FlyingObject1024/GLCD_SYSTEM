//�O���t�B�b�NLCD�p�̒u��������֐��ǂݍ��ݗp�w�b�_�t�@�C��
#define SYSTEM_MODE 1
#define BOARD 1
#define BREAD 0//������ӂ̓u���b�h�{�[�h�Ɗ�Ղ̔z��������Ă����Ɏg���Ă���
//���̕ӂ́u�J���������e�L�X�g�@�}�C�R���ҁv�łǂ���
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
/*****���g�̐錾*****/
extern uint64_t Glcd_map[128];
/*****IC1 �� IC2 �̑I��*****/
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
/*****���W (X,Y) �ɓ_��ł�*****/
extern void DrawDot(uint8_t x,uint8_t y);
/*****(x1,y) ���� (x2,y) �܂ŉ���������*****/
extern void DrawLine_line(uint8_t y,uint8_t x1,uint8_t x2);
/*****(x,y1) ���� (x,y2) �܂ŏc��������*****/
extern void DrawLine_column(uint8_t x,uint8_t y1,uint8_t y2);
/*****(x1,y1) ���� (x2,y2) �܂Ő�������*****/
extern void DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t color);
/*****�l�p�`��`�悷��*****/
extern void DrawRectangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);
/*****�~��`�悷��*****/
extern void DrawCircle(uint8_t x0, uint8_t y0, uint8_t r);
/*****�h��Ԃ��~��`�悷��*****/
extern void DrawFillCircle(uint8_t x0, uint8_t y0, uint8_t r);
/*****Glcd_map[128] ���t���{�̂ɓ]��*****/
extern void Glcd_Print_map();
/*****Glcd_map[128] �̑S�Ẵf�[�^������*****/
extern void Clear_Screen();
/***** (X,Y,"������") ������\��*****/
extern void Glcd_font(uint8_t start_x,uint8_t start_y,char font_num);
/*****�����������@������*****/
//void Glcd_minifont(uint8_t start_x,uint8_t start_y,uint8_t font_num);
/***** (X,Y,"������",���̕����\���܂ł̎���) �P������������\��*****/
extern void Glcd_str(uint8_t start_x,uint8_t start_y,char *str);
/***** (X,Y,�摜�ԍ�,�d�ːݒ�,���E���]) graphics�ɓ�����8x8�̉摜�\��*****/
extern void Glcd_graphic(uint8_t start_x,uint8_t start_y,char graph_num,uint8_t modes,uint8_t re);
/***** (X,Y,�摜�ԍ�,�����ԍ�,�d�ːݒ�,���E���]) graphics�ɓ�����4x4�̉摜�\��*****/
extern void Glcd_graphic_mini(uint8_t start_x,uint8_t start_y,char graph_num,char mini_num,uint8_t modes,uint8_t re);
/***** (X,Y,�摜�ԍ�,�d�ːݒ�,���E���]) graphics2�ɓ�����8x8�̉摜�\��*****/
//extern void Glcd_graphic2(uint8_t start_x,uint8_t start_y,char graph_num,uint8_t modes,uint8_t re);
/***** (X,Y,�摜�ԍ�,�d�ːݒ�,���E���],�c���,�����) graphics�ɓ�����8x8�̉摜�\��(�}�b�v�\���p)*****/
extern void Glcd_graphmap(uint8_t start_x,uint8_t start_y,char graph_num,uint8_t modes,uint8_t re,uint8_t del,uint8_t dely);
/*****�@�����R���̐����摜�\���@*****/
extern void Glcd_remocon_graph();

extern void EraseRectangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);
extern void Glcd_Erase_Line_column(uint8_t x,uint8_t y1,uint8_t y2);
extern void Glcd_Erase_Line_line(uint8_t y,uint8_t x1,uint8_t x2);
/***** (X,Y,"������",���̕����\���܂ł̎���) �P������������\��*****/
extern void Glcd_str_wait(uint8_t start_x,uint8_t start_y,char *str,uint8_t lag);
#endif