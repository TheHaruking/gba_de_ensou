
#include <gba.h>
#include <stdio.h>
#include <stdlib.h>

#include "hal_eightinput.h"
#include "sound_play.h"

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	SOUND_PLAY  sp_data;
	BUTTON_INFO b;

	// Init
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();

	// Init private
	InitSoundPlay(&sp_data);
	halSetKeysSort(&b, 4, 5, 6, 7, 0, 1, 3, 2);

	// Sound Init
	REG_SOUNDCNT_X = 0x80;		// turn on sound circuit
	REG_SOUNDCNT_L = 0x1177;	// full volume, enable sound 1 to left and right
	REG_SOUNDCNT_H = 2;			// Overall output ratio - Full

	while (1) {
		scanKeys();
		halSetKeys(&b, keysHeld());

		// 音処理
		SoundPlay(&sp_data, &b);

		// 表示
		if (halIsAB(&b)) {
			iprintf("[%04X]\n", sp_data.count);
		}

		VBlankIntrWait();
	}
}


