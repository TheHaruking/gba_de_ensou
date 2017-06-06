//---------------------------------------------------------------------------------
#ifndef _sound_play_h_
#define _sound_play_h_
//---------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------------
#include "hal_eightinput.h"

typedef struct _SOUND_PLAY_ {
    int count;
	int vector;
	int note;
	int note18;
	int offset;
	int ab;
	int key;
	int ab_sounding;
	int high_flag;
	int snd_duty;		// 0062_XX67 - duty 
	int snd_time;		// 0062_89AX - snd_time
	int snd_amp;		// 0062_XXXB - muki
	int snd_vol;		// 0062_CDEF - init volume
	BUTTON_INFO b;
} SOUND_PLAY, *PSOUND_PLAY;

void InitSoundPlay(SOUND_PLAY* );
void SoundPlay(SOUND_PLAY*, BUTTON_INFO*);

//---------------------------------------------------------------------------------
#ifdef __cplusplus
	}	   // extern "C"
#endif
//---------------------------------------------------------------------------------
#endif