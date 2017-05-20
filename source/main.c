
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>

#include "hal_eightinput.h"




//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	int count;
	BUTTON_INFO b;

	// Init
	irqInit();
	irqEnable(IRQ_VBLANK);
	consoleDemoInit();

	// Init private
	count = 0;
	halSetKeysSort(&b, 0, 1, 8, 3, 4, 5, 6, 7);

	while (1) {
		scanKeys();
		halSetKeys(&b, keysHeld());
		// ansi escape sequence to set print co-ordinates
		// /x1b[line;columnH
		inc(&count); 
		iprintf("/x1b[3;3btn = %04X \n", halKeyToNum(&b));
		VBlankIntrWait();
	}
}


