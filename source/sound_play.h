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
	// 音高
	int note;			// 最終音階
	int vector;			// 入力値
	int vector_keep;	// 入力値(十字キー無しで押したとき用)
	int octave;			// オクターブ
	int key;			// 調 (0 ～ 11)
	int ab_sounding;	// a なら 1
	int high_flag;		// ↖のとき、高くするか低くするか
	int tune;			// チューニング用
	// Visual Play で使う
	int note18;			// vector * 2 + ab
	int ofs;			// octave * 12 + key
	// 音質
	int snd_duty;		// 0062_XX67 - duty 
	int snd_time;		// 0062_89AX - snd_time
	int snd_amp;		// 0062_XXXB - muki
	int snd_vol;		// 0062_CDEF - init
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