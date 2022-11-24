/*
 * defines.h
 *
 * Created: 2017/12/29 2:05:34
 *  Author: kodai
 */ 

#ifndef DEFINES_H_//���̕��� DEFINES_H_ ��define����Ă��Ȃ�������A�Ƃ����Ӗ�
#define DEFINES_H_//������define ����ł܂��������ǂݍ��܂ꂽ�ꍇ��̕��ł͂������

#define F_CPU 8000000UL//�}�C�R���̃N���b�N�������B���̐��ɉ�����delay�̓������ς��
extern volatile uint8_t sound_switch;
extern volatile uint8_t sig;
extern volatile uint8_t lsig;
extern volatile uint8_t last_button;
extern volatile uint8_t connection_flag;
extern volatile uint8_t opponents_sig;
extern volatile uint8_t opponents_last_button;
extern volatile uint8_t app_number;//�ǂ̃A�v���ł��g���ϐ��A�֐������̃t�@�C�����Ăяo�����Ƃňꊇ�ŌĂяo����B
extern volatile uint64_t system_clock;
extern uint8_t button_keep(uint8_t s);
extern uint8_t button_jadge(uint8_t s);
extern uint8_t button_jadge_one();
extern uint8_t opponents_button_jadge(uint8_t s);
extern uint8_t opponents_button_jadge_one();
extern void start_button();
extern uint64_t Glcd_map[128];//�}�b�v�f�[�^�B64�r�b�g��128��64x128�̉�ʃf�[�^��ۑ�
//extern void Printtime();

/*****�l�����ւ���}�N��*****/
#define swap(x,y) do{uint8_t temp=x;x=y;y=temp;}while(0)



#define Master 1
#define Slave 2//���ĒʐM�V�X�e������낤�Ƃ����Ƃ��̖��c�B�g���ĂȂ��͂�

#define A 7
#define B 6
#define UP 2
#define DOWN 1
#define RIGHT 3
#define LEFT 0
#define START 5
#define SELECT 4
#define NO 8//�Ƃ肠�����{�^�����@������������������₷��
#define MENU 20//���j���[�̃A�v���ԍ��@�Q�O�������B�O�@�Ɗe���ŏ������C������B
enum{//#define �̏ȗ��̂悤�Ȃ��̏ォ��0,1,2,�Ǝ����Ŋ���U��
	TIMER_STOPWATCH,
	TETRIS,
	JUMP,
	MOVE,
	SHOOTING,
	BOARDRACE,
	SNAKE,
};

#endif /* DEFINES_H_ *///�����ł�{}���Ȃ����ߑ���ɏI�[������ŕ\��
