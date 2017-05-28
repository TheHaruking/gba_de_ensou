#include <gba.h>
#include <mappy.h>
#include "sound_play.h"

// 音程のデータ。(0000 : 最低音のド, 7オクターブ)
const unsigned short freq_tbl[84] = {
	0x0000, 0x0073, 0x00DF, 0x0146, 
	0x01A7, 0x0202, 0x0258, 0x02A9, 
	0x02F6, 0x033E, 0x0383, 0x03C3, 
	0x0400, 0x0439, 0x0470, 0x04A3, 
	0x04D3, 0x0501, 0x052C, 0x0555, 
	0x057B, 0x059F, 0x05C1, 0x05E2, 
	0x0600, 0x061D, 0x0638, 0x0651, 
	0x066A, 0x0680, 0x0696, 0x06AA, 
	0x06BD, 0x06D0, 0x06E1, 0x06F1, 
	0x0700, 0x070E, 0x071C, 0x0729, 
	0x0735, 0x0740, 0x074B, 0x0755, 
	0x075F, 0x0768, 0x0770, 0x0778, 
	0x0780, 0x0787, 0x078E, 0x0794, 
	0x079A, 0x07A0, 0x07A5, 0x07AB, 
	0x07AF, 0x07B4, 0x07B8, 0x07BC, 
	0x07C0, 0x07C4, 0x07C7, 0x07CA, 
	0x07CD, 0x07D0, 0x07D3, 0x07D5, 
	0x07D8, 0x07DA, 0x07DC, 0x07DE, 
	0x07E0, 0x07E2, 0x07E3, 0x07E5, 
	0x07E7, 0x07E8, 0x07E9, 0x07EB, 
	0x07EC, 0x07ED, 0x07EE, 0x07EF, 
};

void InitSoundPlay(SOUND_PLAY* d) {
    d->count 		= 0;
	d->note	    	= 0;
	d->offset		= 22;
	d->key 		    = 4;
	d->ab 			= 0;
	d->ab_sounding  = 0;
	d->high_flag	= 0;
	d->snd_duty	    = 2;
	d->snd_time 	= 2;	// 余韻の長さ
	d->snd_amp		= 0;
	d->snd_vol 	    = 15;	// Attack volume
};

void SoundPlay(SOUND_PLAY* d, BUTTON_INFO* btn) {
	// Ctr + key
	switch (halKeyCtr4(btn)) {
		case 1: d->snd_duty = (d->snd_duty + 1) & 0x03; break;
		case 3: d->snd_duty = (d->snd_duty - 1) & 0x03; break;
		case 0: d->offset += 12; break;
		case 2: d->offset -= 12; break; 			
	}

	// キー を 12通りに変更 (左回り)
	if (halKeyCtr12(btn) >= 0) {
		d->key = 11 - halKeyCtr12(btn);
	}

	// ↖の音程を臨機応変に
	switch (halKey8(btn)) {
		case 0: case 1: case 2:
			d->high_flag = 1;
			break;
		case 4: case 5: case 6:
			d->high_flag = 0;
			break;
	}

	// メロディ！
	// 1. AかBを押した瞬間
	// 2. AかBを押しつつ、十字キーを動かした瞬間
	// 3. AとBの片方離したが、一方が残っている場合
	if (   (halIsAB(btn)      && halIsKey_hold(btn))
		|| (halIsAB_hold(btn) && halIsKey(btn)) 
		|| (halIsAB_diff(btn) && halIsAB_hold(btn)) ) 
	{
		d->note = 16 - (halKey8(btn) * 2);
		
		// どのボタンで鳴らしているかを記憶しておく。
		if (halIsAB(btn)) {
			d->ab_sounding = (halIsAB(btn) & BTN_A) ? 1 : 0;
		}
		if (halIsAB_rrse(btn)) {
			d->ab_sounding = (halIsAB_rrse(btn) & BTN_B) ? 1 : 0;
		}
		if(halKey8(btn) == 7) {
			d->note += (d->high_flag) ? 16 : 0;
		}
		d->note += d->offset + d->key + d->ab_sounding;	
	}

	// 入力
	if (   (halIsAB(btn)      && halIsKey_hold(btn))
		|| (halIsAB_hold(btn) && halIsKey(btn)) 
		|| (halIsAB_diff(btn) && halIsAB_hold(btn)) ) 
	{
		int swp_total;
		int note_total;
		int snd_total;

		int snd_duty_fix = (d->snd_duty & 0x03) <<  6;
		int snd_time_fix = 0;		// (d->snd_time << 7)  & 0x07;
		int snd_amp_fix  = (d->snd_amp  & 0x01) << 11;
		int snd_vol_fix  = (d->snd_vol  & 0x0f) << 12;

		// 
		note_total = 0x8000 | freq_tbl[d->note];
		snd_total  = snd_duty_fix | snd_amp_fix | snd_time_fix | snd_vol_fix;
		// Sweep は 使わない
		swp_total  = 0x0000;

		// Beep!
		REG_SOUND1CNT_L = swp_total;
		REG_SOUND1CNT_H = snd_total;
		REG_SOUND1CNT_X = note_total;
		d->count++;
	}

	// キーオフ
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
};