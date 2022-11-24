#ifndef SOUND_H_
#define SOUND_H_

#define SOUND_TYPE_KEY		0
#define SOUND_TYPE_ENTER	1
#define SOUND_TYPE_CANCAL	2
#define SOUND_TYPE_ERROR	3
#define SOUND_TYPE_FIX		4
#define SOUND_TYPE_CLEAR	5
#define SOUND_TYPE_SHOT		6
#define SOUND_TYPE_HIT		7
#define SOUND_TYPE_BOMB		8
#define SOUND_TYPE_JUMP		9
#define SOUND_TYPE_COIN		10
#define SOUND_TYPE_DAMAGE	11
#define SOUND_TYPE_1UP		12

#define F_CPU 8000000UL

#define COUNTER_FREQ	(F_CPU/256)

#define NOTE_G3		((uint8_t)(COUNTER_FREQ / 195.9977))
#define NOTE_GS3	((uint8_t)(COUNTER_FREQ / 207.6523))
#define NOTE_A3		((uint8_t)(COUNTER_FREQ / 220.0000))
#define NOTE_AS3	((uint8_t)(COUNTER_FREQ / 233.0819))
#define NOTE_B3		((uint8_t)(COUNTER_FREQ / 246.9417))
#define NOTE_C4		((uint8_t)(COUNTER_FREQ / 261.6256))
#define NOTE_CS4	((uint8_t)(COUNTER_FREQ / 277.1826))
#define NOTE_D4		((uint8_t)(COUNTER_FREQ / 293.6648))
#define NOTE_DS4	((uint8_t)(COUNTER_FREQ / 311.1270))
#define NOTE_E4		((uint8_t)(COUNTER_FREQ / 329.6276))
#define NOTE_F4		((uint8_t)(COUNTER_FREQ / 349.2282))
#define NOTE_FS4	((uint8_t)(COUNTER_FREQ / 369.9944))
#define NOTE_G4		((uint8_t)(COUNTER_FREQ / 391.9954))
#define NOTE_GS4	((uint8_t)(COUNTER_FREQ / 415.3047))
#define NOTE_A4		((uint8_t)(COUNTER_FREQ / 440.0000))
#define NOTE_AS4	((uint8_t)(COUNTER_FREQ / 466.1638))
#define NOTE_B4		((uint8_t)(COUNTER_FREQ / 493.8833))
#define NOTE_C5		((uint8_t)(COUNTER_FREQ / 523.2511))
#define NOTE_CS5	((uint8_t)(COUNTER_FREQ / 554.3653))
#define NOTE_D5		((uint8_t)(COUNTER_FREQ / 587.3295))
#define NOTE_DS5	((uint8_t)(COUNTER_FREQ / 622.2540))
#define NOTE_E5		((uint8_t)(COUNTER_FREQ / 659.2551))
#define NOTE_F5		((uint8_t)(COUNTER_FREQ / 698.4565))
#define NOTE_FS5	((uint8_t)(COUNTER_FREQ / 739.9888))
#define NOTE_G5		((uint8_t)(COUNTER_FREQ / 783.9909))
#define NOTE_GS5	((uint8_t)(COUNTER_FREQ / 830.6094))
#define NOTE_A5		((uint8_t)(COUNTER_FREQ / 880.0000))
#define NOTE_AS5	((uint8_t)(COUNTER_FREQ / 932.3275))
#define NOTE_B5		((uint8_t)(COUNTER_FREQ / 987.7666))
#define NOTE_C6		((uint8_t)(COUNTER_FREQ / 1046.5023))
#define NOTE_CS6	((uint8_t)(COUNTER_FREQ / 1108.7305))
#define NOTE_D6		((uint8_t)(COUNTER_FREQ / 1174.6591))
#define NOTE_DS6	((uint8_t)(COUNTER_FREQ / 1244.5079))
#define NOTE_E6		((uint8_t)(COUNTER_FREQ / 1318.5102))
#define NOTE_F6		((uint8_t)(COUNTER_FREQ / 1396.9129))
#define NOTE_FS6	((uint8_t)(COUNTER_FREQ / 1479.9777))
#define NOTE_G6		((uint8_t)(COUNTER_FREQ / 1567.9817))
#define NOTE_GS6	((uint8_t)(COUNTER_FREQ / 1661.2188))
#define NOTE_A6		((uint8_t)(COUNTER_FREQ / 1760.0000))
#define NOTE_AS6	((uint8_t)(COUNTER_FREQ / 1864.6550))
#define NOTE_B6		((uint8_t)(COUNTER_FREQ / 1975.5332))
#define NOTE_OFF	0
#define NOTE_END	255

extern void soundInit(void);
extern void soundPlay(uint8_t sound_type);
extern void soundEngine(void);
extern void simpleSound(uint8_t tone,uint8_t length);

#endif /* SOUND_H_ */