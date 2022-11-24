#include <avr/io.h>//��{�i�K�{�j
#include <stdlib.h>//�����p
#include <avr/interrupt.h>//���荞��
#include <avr/pgmspace.h>//����ŁA�萔�f�[�^���v���O�����������ɕۑ��ł���
//PROGMEM�Ŕz�u
//pgm_read_byte(&�ϐ�);�ŌĂяo��
#include "defines.h"//define���W�߂�����
#include "Glcd_function.h"//�O���t�B�b�NLCD�p�֐��Q
#include "sound.h"//�����o�����߂̊֐��Q
#include <util/delay.h>//delay��

volatile uint8_t sound_switch=0;//���̃I���I�t
volatile uint8_t sig=0;//1p�p�����R���̐M���ۑ��ϐ�
volatile uint8_t last_button=0;//�Ō�ɉ������{�^���A����������̂��߂ɍ�������g���ĂȂ��͂�
volatile uint8_t app_number=MENU;//���s����A�v���̕ϐ�
//volatile �͊��荞�݂����������Ƃ��ϐ��̒l������ɒ�������Ȃ��悤�ɂ������
volatile uint8_t connection_flag = false;//2p�����R�����q�����Ă邩�Ȃ����ĂȂ������肵�悤�Ƃ������́B���ǃm�C�Y�̂����ňӖ����Ȃ��ĂȂ��B
//���̃t���O�������Ă���ƃ��j���[��ʉE��ɗ����\��
volatile uint8_t opponents_sig = 0;//2p�p�����R���̐M���ۑ��ϐ�

volatile uint8_t c_emagency = 0;//���ꕪ�����

volatile uint64_t system_clock=0;

void menu_main();//����֐��̃v���g�^�C�v�錾�B��ɐ錾���邱�ƂŁA�Ăяo���ȍ~�ɐ錾����Ă���Ƃ����G���[��h�~����B
void setumei_main();


extern void tetris_main();//���ꂼ��̃A�v���̃��C���֐�������main�t�@�C���ł��ǂݍ��߂�悤�ɂ��Ă���B
extern void jump_main();//��������̂ɂ͋�J����
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
//���̓�̓����R���̐M���̎�M�BISR_BLOCK�Ŋ��荞�ݒ��ɂق��̊��荞�݂��������Ȃ��悤�ɂ��Ă���
typedef void (*FUNCPTR)(void);
//��������u�\���́v���g���������ƂȂ�B���������ɓ���B���̂������̍\���̂̓|�C���^���g���Ă���̂ŗ]�v����ɔ��Ԃ��������Ă���
typedef struct {
	char *name;
	FUNCPTR main;
}APP_FUNC;
//�\���̂����֐��A�ϐ���錾���Ă���B���̍\���̂͂��ꂼ��̃A�v���̃��C���֐��A�A�v����(�\���������́A�܂蕶���R�[�h)�������B
//�����ł́u�\���́v��錾�����̂ł����āu�A�v���v��錾�����̂ł͂Ȃ�
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
//���ꂼ��̃A�v���̐錾���s���Ă���BNULL�Ƃ����̂͏I�[��\���Ă���B�����ł͓񎟌��z��̌`���Ƃ��Ă���(�����悭�킩���Ă��Ȃ�)
void menu_main(void)//�A�v���ԍ��Q�O�@���j���[��ʂ̃��C���֐�
{
	uint8_t i = 0;//���[�v�p�ϐ��Auint8_t�@�Ƃ����̂́@unsigned char �Ɠ����B 8�Ƃ��������̓r�b�g����\��
	volatile static int8_t menu_cursor=0;//�J�[�\��������ꏊ��\���ϐ��B�Ⴆ�΁@0�@���ƃA�v���@��Ҳ�@�ɃJ�[�\���������Ă���B
	volatile static int8_t page_cursor=0;//�y�[�W���X�N���[�����邽�߂̕ϐ�
	while(app_number==MENU){//�A�v���p�ϐ��Ǝ��s���A�v���͂��̂悤�ɏƍ����Ă���B
		i=0;//���[�v�p�ϐ���0�ɂ��Ă���
		Clear_Screen();//��ʂ𔒂ɂ���
		while( AppFunc[i].main != NULL ){//�I�[�������܂ō\���̂𒲂׏グ��
			if(i<8)Glcd_str( 8, i*8, AppFunc[i+page_cursor].name);//��ʂɃA�v������\�����Ă���B��ʂ�8�����\���ł��Ȃ��̂�i�Ŕ��肵�Ă���B
			i++;
		}
	Glcd_font(0,(menu_cursor - page_cursor)*8,'}'+1);//�J�[�\���̕\��
	if(page_cursor>0) Glcd_font(64,48,'�'+1);
	else Glcd_font(64,48,' ');
	if(page_cursor+7<i && page_cursor+8 < i)Glcd_font(64,56,'�'+2);//�J�[�\�������킹�Ă���A�v�����X�ڈȍ~�Ȃ�J�[�\������ԉ��ɕ\������悤�ɂ��Ă���B
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

	Glcd_Print_map();//�f�[�^����ʂɕ\��

	if(menu_cursor > 0 && button_jadge_one() == UP){//��{�^�����������ꍇ�J�[�\������Ɉړ�

		menu_cursor--;
		soundPlay(SOUND_TYPE_KEY);//���ʉ�

	}
	else if(menu_cursor < i-1 && button_jadge_one() == DOWN){//���{�^��

		menu_cursor++;
		soundPlay(SOUND_TYPE_KEY);

	}
	else if(button_jadge_one() == A && last_button != A){//A�{�^���@�A�v���I������A�������Ă�����܂��A�v���ɓ���̂ł��̖h�~�����Ă���

		app_number = menu_cursor;//�J�[�\���̈ʒu�����s�A�v���ϐ��ɑ���B
		soundPlay(SOUND_TYPE_ENTER);

	}
	else if(button_jadge_one() == SELECT){//SELECT�{�^��

		if(sound_switch) sound_switch = 0;
		else sound_switch = 1;//���ʉ��̃I���I�t

	}

	if(menu_cursor >= 8) page_cursor = menu_cursor - 7;//�J�[�\���ʒu����X�N���[�����鐔���Z�o
	else if(menu_cursor - page_cursor < 0) page_cursor--;
	_delay_ms(100);
}
}

#define TIMER_FREQ	60	//[Hz]//���̕ӂ́uAVRMIniGame�v����̈��p�B�`�u�q�����̃^�C�}�[��PWM�����g���A�����o���Ă���
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
//�^�C�}�[�̒l���Q�l�ɂ����^�C�}���荞��
int main(void)//�匳�̃��C���֐�
{
	//InitI2c();
	DDRA=0xff;//���̕ӂ́u�e�L�X�g�W�v�́uAVR����v�œǂ�ŉ�����
	DDRB=0xff;//Arduino�I�Ɍ����ƁupinMode�v�ł��B
	DDRC=0xfc;
	PORTA=0;//�����ŁA�ŏ��ɏo�͂��I�t�ɂ��邱�ƂŁA�s��΍�����Ă���
	PORTB=0;//���̕���Ardiono�I�Ɍ����ƁudigitalWrite�v
	PORTC=0b00010000;
	//�V���A���ʐM�̐ݒ�B�ڂ������Ƃ́u�J���������e�L�X�g�@�}�C�R���ҁv�������͓���u�납��̓d�q�H��(��d)�v�́u�V���A���ʐM�v�łǂ����B
	UCSR0A=0b00000000;
	UCSR0B=0b10010000;//��M�E���荞�ݗL��
	UCSR0C=0b00100110;//�񓯊��ʐM�E�����p���e�B8bit
	UCSR1A=0b00000000;
	UCSR1B=0b10010000;//��M�L���E���荞�݂���
	UCSR1C=0b00100110;//�񓯊��ʐM�E�����p���e�B8bit
	//�...odd//����...even
	if(F_CPU == 12000000){
		UBRR0=259;//1M 12/8M 103
		UBRR1=259;
	}
	else if(F_CPU == 8000000){
		UBRR0=103;//1M 12/8M 103
		UBRR1=103;
	}//�}�C�R���̃N���b�N�Ⴂ�ŁA�{�[���[�g�𒲐����Ă���B
	//�ʏ��AVR�̃N���b�N�͏������ݎ���fuses ���ڂ́@CKDIV8�@�̃`�F�b�N�łW���̂P�ɂȂ��Ă���̂ŁA������������ƃN���b�N�����������ł���B
	//����GLCD SYSTEM�ł͐������������Ȃ��ƂƂĂ��d���Ďg�����ɂȂ�Ȃ�
	timer1Init();//Init�Ƃ����̂́u�������v��\���B
	soundInit();

	//EIMSK=0b01000000;
	//MCUCR=0b00000010;

	sei();//���荞�݂̋���
	Glcd_SelectChip(TRUE,TRUE);//���̕ӂ̓O���t�B�b�NLCD�̐ݒ�B�ڂ����́u�J���������e�L�X�g�@�}�C�R���ҁv�܂ŁB�Ƃ������ۃR�s
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
			AppFunc[ app_number ].main();//�A�v�����C���֐��̊Ǘ�
		}
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////