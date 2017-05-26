
#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal_eightinput.h"
#include "sound_play.h"
const unsigned short zero[128] = { 0 };
// 映像用データ
typedef struct _VP_LINE_ {
	int		id;
	int		x, y;
	int		len;
	int		col;
	int		frame;
} VP_LINE, *PVP_LINE;

typedef struct _VISUAL_PLAY_ {
	unsigned int		frame;
	unsigned short mem[256][128];
	unsigned short vram[256][512];
} VISUAL_PLAY, *PVISUAL_PLAY;

void InitVisualPlay(VISUAL_PLAY* vp_data){
	vp_data->frame = 0;
//	memcpy(vp_data->mem,  0, 256 * 128 * 2);
//	memcpy(vp_data->vram, 0, 256 * 512 * 2);
}

void MoveLine(VISUAL_PLAY* vp_data, int id) {
	vp_data->frame++;
}

// 方法その２ 色で制御
// id でなく、128固定
void DrawLines(VISUAL_PLAY* vp_data, int y, int flag){
	int f = vp_data->frame & 0xff;
	y &= 0x7f;
	dmaCopy((u16*)zero, (u16*)vp_data->mem[f], 128);
	if (flag){
		vp_data->mem[f][y] = rand();
	}
}

// Test 
void DrawLinesTest(VISUAL_PLAY* vp_data){
	int  f    = vp_data->frame & 0xff;
	//u16* dest = (u16*)(VRAM + SCREEN_WIDTH * 0x02 * f);
	for (int i = 0; i < 128; i++){
		int tmp = SCREEN_WIDTH * 0x02 * i ;
		// dmaCopy((u16*)vp_data->mem[f], (u16*)dest, 128);
		dmaCopy((u16*)vp_data->mem[i], (u16*)(VRAM + tmp), 128);
	}
}

void ConvertMem(VISUAL_PLAY* vp_data){	
	int f  = (vp_data->frame) & 0xff;
	int f2 = (f + 0x100) & 0xff;
	for(int i = 0; i < 128; i++){
		vp_data->vram[i][f + 32] = vp_data->mem[f][i];
		vp_data->vram[i][f2] = vp_data->mem[f][i];
		//dprintf("%08X", &vp_data->vram[i][f]);
	}
}

void FlushVram(VISUAL_PLAY* vp_data) {
	int f = vp_data->frame & 0x1FF;
	int d = VRAM;
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		dmaCopy((u16*)&vp_data->vram[i][f], (u16 *)d, SCREEN_WIDTH);
		d += SCREEN_WIDTH << 1;
	}
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	SOUND_PLAY   sp_data;
	VISUAL_PLAY  vp_data;
	BUTTON_INFO  b;

	int			 count;

	// Init
	irqInit();
	irqEnable(IRQ_VBLANK);
	// consoleDemoInit();
	SetMode(3 | BG2_ON);

	// Init private
	InitSoundPlay(&sp_data);
	halSetKeysSort(&b, 4, 5, 6, 7, 0, 1, 3, 2);
	count = 0;

	// Init Video
	InitVisualPlay(&vp_data);

	// Sound Init
	REG_SOUNDCNT_X = 0x80;		// turn on sound circuit
	REG_SOUNDCNT_L = 0x1177;	// full volume, enable sound 1 to left and right
	REG_SOUNDCNT_H = 2;			// Overall output ratio - Full

	while (1) {
		scanKeys();
		halSetKeys(&b, keysHeld());

		// 音処理
		SoundPlay(&sp_data, &b);

		// 映像
		MoveLine(&vp_data, 0);
		DrawLines(&vp_data, sp_data.note, halIsAB_hold(&b));
		DrawLinesTest(&vp_data);
		//ConvertMem(&vp_data);
		//FlushVram(&vp_data);

		// 垂直同期
		VBlankIntrWait();
	}
}


