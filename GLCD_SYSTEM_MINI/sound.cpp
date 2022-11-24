/******************************************************************************
	AVR ATMega328
	Beep Sound Module
	Using Timer0 and OC0B for PWM output
******************************************************************************/
#include	<avr/io.h>
#include	<avr/pgmspace.h>
#include	<avr/interrupt.h>

#include	"sound.h"
#include	"defines.h"

/* Overflow 8bit Counter @ 12MHz/256
#define NOTE_C3		((uint8_t)(COUNTER_FREQ / 130.8128))
#define NOTE_CS3	((uint8_t)(COUNTER_FREQ / 138.5913))
#define NOTE_D3		((uint8_t)(COUNTER_FREQ / 146.8324))
#define NOTE_DS3	((uint8_t)(COUNTER_FREQ / 155.5635))
#define NOTE_E3		((uint8_t)(COUNTER_FREQ / 164.8138))
#define NOTE_F3		((uint8_t)(COUNTER_FREQ / 174.6141))
#define NOTE_FS3	((uint8_t)(COUNTER_FREQ / 184.9972))
*/

#define SNDBUFF_SIZE	20

static volatile uint8_t SNDBUFF[SNDBUFF_SIZE];
static volatile uint8_t PlayIndex = 0;
static volatile uint8_t PlayCounter = 0;

const static uint8_t SOUND_DATA[][SNDBUFF_SIZE] PROGMEM = {
	// 0. SOUND_TYPE_KEY
	{ NOTE_C6, 2, NOTE_END },
	// 1. SOUND_TYPE_ENTER
	{ NOTE_G5, 1, NOTE_C6, 1, NOTE_G6, 1, NOTE_END },				
	// 2. SOUND_TYPE_CANCAL
	{ NOTE_G6, 1, NOTE_C6, 1, NOTE_G5, 1, NOTE_END },
	// 3. SOUND_TYPE_ERROR

	{ NOTE_G3, 1, NOTE_OFF, 1, NOTE_G3, 2, NOTE_END },
	// 4. SOUND_TYPE_FIX
	{ NOTE_G5, 1, NOTE_G4, 1, NOTE_G3, 1, NOTE_END },
	// 5. SOUND_TYPE_CLEAR
	{ NOTE_B4, 1, NOTE_G4, 1, NOTE_B5, 1, NOTE_G5, 1, NOTE_B6, 1, NOTE_G6, 1, NOTE_END },
	// 6. SOUND_TYPE_SHOT
	{ NOTE_F6, 1, NOTE_E6, 1, NOTE_END },
	// 7. SOUND_TYPE_HIT
	{ NOTE_G6, 1, NOTE_FS6, 1, NOTE_E6, 1, NOTE_G6, 1, NOTE_END },
	// 8. SOUND_TYPE_BOMB
	{ NOTE_C4, 1, NOTE_G3, 1, NOTE_G5, 1, NOTE_C5, 1, NOTE_G4, 1, NOTE_C4, 1, NOTE_END },
	// 9. SOUND_TYPE_JUMP
	{ NOTE_G4, 1, NOTE_GS4, 1, NOTE_A4, 1, NOTE_AS4, 1, NOTE_B4, 1, NOTE_C5, 1, NOTE_END },
	// 10. SOUND_TYPE_COIN
	{ NOTE_B5, 1, NOTE_E6, 5, NOTE_END },
	// 11. SOUND_TYPE_DAMAGE
	{ NOTE_C4, 1, NOTE_B3, 1, NOTE_G3, 1, NOTE_END },
	// 12. SOUND_TYPE_1UP
	{ NOTE_C4, 3, NOTE_E4, 3, NOTE_C5, 3, NOTE_A5, 3, NOTE_B5, 3, NOTE_E5, 3, NOTE_END },
	// 13. SOUND_TYPE_START
	{ NOTE_A4, 10, NOTE_A4, 10, NOTE_A4, 10, NOTE_A5, 10, NOTE_END },
};


void soundInit(void)
{
	uint8_t	i;

	// Timer0 Register
	//
	// TCCR0A ... COM0A1 COM0A0 COM0B1 COM0B0 -      -      WGM01  WGM00
	// TCCR0B ... FC0A   FOC0B  -      -      WGM02  CS02   CS01   CS00
	//
	// WGM0[2:0]  = 111	// Fast PWM Mode with Counter Top OCR0A
	// COM0A[1:0] = 00	// Normal Operation
	// COM0B[1:0] = 10	// Set at Bottom & Clear at OCR0B
	// CS0[2:0]   = 100 // Clock Select = Clock/256
	// FOC0A      = 0	// Force Output Compare A
	// FOC0B      = 0	// Force Output Compare B

	TCCR0A = 0b00100011;
	TCCR0B = 0b00001100;
	TIMSK0 = 0b00000000;		// Mask All Interrupt
	OCR0A  = 0;
	OCR0B  = 0;

	// Clear Sound Buffer
	for(i=0; i<SNDBUFF_SIZE; i++){
		SNDBUFF[i] = NOTE_END;
	}
}

void sound(uint8_t tone)
{
	if(tone == NOTE_OFF){
//		TCCR0A &= ~0b00110000;
		OCR0A  = 0;
		OCR0B  = 0;
	}
	else{
//		TCCR0A |=  0b00100000;
		OCR0A  = tone;
		OCR0B  = tone>>1;
	}
}

void soundPlay(uint8_t sound_type)
{
	uint8_t i = 0;

	cli();	// Disable Interrupt

	if(sound_switch){
		while(i<SNDBUFF_SIZE-1){
			uint8_t data = pgm_read_byte(&SOUND_DATA[sound_type][i]);
			if(data == 0) break;
			SNDBUFF[i] = data;
			i++;
		}
	}

	SNDBUFF[i] = NOTE_END;
	PlayIndex = 0;
	PlayCounter = 0;

	sei();	// Enable Interrupt
}

void simpleSound(uint8_t tone,uint8_t length){

	cli();	// Disable Interrupt
	
	if(sound_switch){
		SNDBUFF[0] = tone;
		SNDBUFF[1] = length;
		SNDBUFF[2] = NOTE_END;
		PlayIndex = 0;
		PlayCounter = 0;
	}
	sei();	// Enable Interrupt
}

void soundEngine(void)
{
	if(PlayCounter > 0){
		PlayCounter--;
	}
	else{
		if(SNDBUFF[PlayIndex] != NOTE_END){
			sound( SNDBUFF[PlayIndex++] );
			PlayCounter = SNDBUFF[PlayIndex++];
		}
		else{
			sound( NOTE_OFF );
		}
	}
}
