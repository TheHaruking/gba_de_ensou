#include <math.h>
#include <gba.h>
#include <mappy.h>
#include "sound_play.h"

// 音程のデータ。(0020 : 最低音のド, 7オクターブ)
const unsigned short freq_tbl[84] = {
	0x0020, 0x0091, 0x00FC, 0x0161, 
	0x01C0, 0x021A, 0x026E, 0x02BE, 
	0x030A, 0x0351, 0x0395, 0x03D4, 
	0x0410, 0x0449, 0x047E, 0x04B0, 
	0x04E0, 0x050D, 0x0537, 0x055F, 
	0x0585, 0x05A9, 0x05CA, 0x05EA, 
	0x0608, 0x0624, 0x063F, 0x0658, 
	0x0670, 0x0686, 0x069C, 0x06B0, 
	0x06C2, 0x06D4, 0x06E5, 0x06F5, 
	0x0704, 0x0712, 0x071F, 0x072C, 
	0x0738, 0x0743, 0x074E, 0x0758, 
	0x0761, 0x076A, 0x0773, 0x077B, 
	0x0782, 0x0789, 0x0790, 0x0796, 
	0x079C, 0x07A2, 0x07A7, 0x07AC, 
	0x07B1, 0x07B5, 0x07B9, 0x07BD, 
	0x07C1, 0x07C5, 0x07C8, 0x07CB, 
	0x07CE, 0x07D1, 0x07D3, 0x07D6, 
	0x07D8, 0x07DB, 0x07DD, 0x07DF, 
	0x07E0, 0x07E2, 0x07E4, 0x07E6, 
	0x07E7, 0x07E8, 0x07EA, 0x07EB, 
	0x07EC, 0x07ED, 0x07EE, 0x07EF, 
};

// 使えるか未確認
void CalcFreqTbl(unsigned short* freq_tbl, int tune){
	const unsigned int MAX_FREQ  = 0x0800;
	unsigned int TUNE_FREQ = MAX_FREQ - tune;
	int freq;

	for(int i = 0; i < 84; i++){
		freq = round(TUNE_FREQ * pow(2, (-i / 12.0)));
		freq_tbl[i] = MAX_FREQ - freq;
	}
}

void InitSoundPlay(SOUND_PLAY* d) {
	d->note	    	= 0;
	d->octave		= 2;
	d->key 		    = 4;
	d->ab 			= 0;
	d->ab_sounding  = 0;
	d->high_flag	= 0;
	d->snd_duty	    = 2;
	d->snd_time 	= 2;	// 余韻の長さ
	d->snd_amp		= 0;
	d->snd_vol 	    = 15;	// Attack volume
	d->tune			= 32;	// ピッチ調整のため使用予定
};

// 先に START押しながら 十字キーで、
// 音種類、オクターブ を 変更
static void ChangeSnd(SOUND_PLAY* d, BUTTON_INFO* btn) {
	switch (halKeyCtr4(btn)) {
		// 左右 で 音種類
		case 1: d->snd_duty++; break;
		case 3: d->snd_duty--; break;
		// 上下 で オクターブ
		case 0: d->octave++; break;
		case 2: d->octave--; break; 			
	}

	// 音種類 補正
	switch (d->snd_duty) {
		case 0x03:
			d->snd_duty  = 0x00; break;
		case -1:
			d->snd_duty  = 0x02; break;
	}

	// オクターブ 補正
	switch (d->octave) {
		case  7:
			d->octave  = 6; break;
		case -2:
			d->octave  = -1; break;
	} 
}

static void ChangeKey(SOUND_PLAY* d, BUTTON_INFO* btn) {
	// 先に十字キー押しながら START で、
	// キー を 12通りに変更 (左回り)
	if (halKeyCtr12(btn) >= 0) {
		d->key = 11 - halKeyCtr12(btn);
	}
}

static void ChangeNote(SOUND_PLAY* d, BUTTON_INFO* btn) {
	// ↖の音程を臨機応変に 1
	switch (halKey8(btn)) {
		// ↑ ~ → なら、高く
		case 0: case 1: case 2: 
			d->high_flag = 1; break;
		// ↓ ~ ← なら、低く
		case 4: case 5: case 6: 
			d->high_flag = 0; break;
	}

	// vector
	if (halKey8(btn) >= 0){
		d->vector = 7 - halKey8(btn);
		// ↖の音程を臨機応変に 2
		if (d->vector == 0) {
			d->vector += (d->high_flag) ? 8 : 0;			
		}
	}

	// 音程
	if (   (halIsAB(btn)      && halIsKey_hold(btn))
		|| (halIsAB_hold(btn) && halIsKey(btn)) 
		|| (halIsAB_diff(btn) && halIsAB_hold(btn) && halIsKey_hold(btn)) ) 
	{
		// どのボタンで鳴らしているかを記憶しておく。
		if (halIsAB(btn)) {
			d->ab_sounding = (halIsAB(btn) & BTN_A) ? 1 : 0;
		}
		if (halIsAB_rrse(btn)) {
			d->ab_sounding = (halIsAB_rrse(btn) & BTN_B) ? 1 : 0;
		}

		// 音程計算
		d->note18 = (d->vector * 2) + d->ab_sounding;
		d->note   = (d->octave * 12) + d->key + d->note18;	
	}
}

// メロディ！
static void MelodyStart(SOUND_PLAY* d, BUTTON_INFO* btn){
	// 1. 十字キーを押しつつ、AかBを押した瞬間
	// 2. AかBを押しつつ、十字キーを動かした瞬間
	// 3. AとBの片方離したが、一方が残っている場合 (十字キーを押しつつ)
	if (   (halIsAB(btn)      && halIsKey_hold(btn))
		|| (halIsAB_hold(btn) && halIsKey(btn)) 
		|| (halIsAB_rrse(btn) && halIsAB_hold(btn) && halIsKey_hold(btn)) ) 
	{
		int swp_total, note_total, snd_total;
		int snd_duty_fix, snd_time_fix, snd_amp_fix, snd_vol_fix;

		// 範囲超えていたら 逃げる
		if ((d->note < 0) || (d->note >= 84))
			return;
		
		// 補正していく
		snd_duty_fix = (d->snd_duty & 0x03) <<  6;
		snd_time_fix = 0;		// (d->snd_time & 0x07) << 7;
		snd_amp_fix  = (d->snd_amp  & 0x01) << 11;
		snd_vol_fix  = (d->snd_vol  & 0x0f) << 12;

		// セットしていく
		note_total = 0x8000 | freq_tbl[d->note];
		snd_total  = snd_duty_fix | snd_amp_fix | snd_time_fix | snd_vol_fix;
		swp_total  = 0x0000;			// Sweep は 使わない。(いまのところ)

		// Beep!
		REG_SOUND1CNT_L = swp_total;
		REG_SOUND1CNT_H = snd_total;
		REG_SOUND1CNT_X = note_total;
	}
}

// キーオフ
static void MelodyEnd(SOUND_PLAY* d, BUTTON_INFO* btn){
	if (!halIsAB_hold(btn)){
		int snd_total;
		
		int snd_duty_fix = (d->snd_duty & 0x03) << 6;
		int snd_time_fix = (d->snd_time & 0x07) << 8;
		int snd_amp_fix  = (d->snd_amp  & 0x01) << 11;
		int snd_vol_fix  = (d->snd_vol  & 0x0f) << 12;
		snd_total = snd_duty_fix | snd_time_fix | snd_amp_fix | snd_vol_fix;

		// 減衰
		REG_SOUND1CNT_H = snd_total;
	}
}

// 
void SoundPlay(SOUND_PLAY* d, BUTTON_INFO* btn) {
	// 音程、音種類 など変更
	ChangeSnd(d, btn);
	ChangeKey(d, btn);
	ChangeNote(d, btn);

	// 鳴らす(鳴り終わらせる)
	MelodyStart(d, btn);
	MelodyEnd(d, btn);
};