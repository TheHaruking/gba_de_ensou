
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
	unsigned int   frame;
	unsigned short *mem;
	unsigned short **vram;
} VISUAL_PLAY, *PVISUAL_PLAY;

void InitVisualPlay(VISUAL_PLAY* vp_data){
	vp_data->frame  = 0;
	vp_data->mem    = (u16*)malloc(128 * 2);
	vp_data->vram   = (u16**)malloc(160 * sizeof(u16*));
	vp_data->vram[0]= (u16*) malloc(160 * 480 * sizeof(u16));
	//memset(vp_data->mem , 0, 128 * 2);
	memset(vp_data->vram, 0, 160 * 480 * sizeof(u16));
}

void MoveLine(VISUAL_PLAY* vp_data) {
	if ((++vp_data->frame) >= SCREEN_WIDTH) {
		vp_data->frame = 0;
	}
}

// 方法その２ 色で制御
// id でなく、128固定
void DrawLines(VISUAL_PLAY* vp_data,unsigned int y, int flag){
	y &= 0xff;
	dmaCopy(zero, vp_data->mem, 128);
	if (flag){
		vp_data->mem[y] = RGB5(31, 15, 0);
	}
}

// Test 
void DrawLinesTest(VISUAL_PLAY* vp_data){
	int f = vp_data->frame;
	u16* dest = (u16*)(VRAM + SCREEN_WIDTH * 2 * f);
	dmaCopy((u16*)vp_data->mem, (u16*)dest, 128);
}

void ConvertMem(VISUAL_PLAY* vp_data){
	int f  = SCREEN_WIDTH + vp_data->frame;
	int f2 = vp_data->frame;
	for (int i = 0; i < 128; i++){
		vp_data->vram[f][i ] = vp_data->mem[i];
		vp_data->vram[f][i ] = vp_data->mem[i];
	}
	vp_data->vram[0][0] = 0xffff;
}

void DrawMemTest(VISUAL_PLAY* vp_data){
	int f = vp_data->frame;
	int dest = VRAM + (SCREEN_WIDTH - f) * 1;
	for (int i = 0 ; i < 128; i++){
		*(u16 *)dest = vp_data->mem[128 - i];
		dest += SCREEN_WIDTH << 1;
	}
}

void FlushVram(VISUAL_PLAY* vp_data) {
	int f = vp_data->frame;
	int d = VRAM;
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		dmaCopy((u16*)&vp_data->vram[i][0], (u16 *)0x06000000, SCREEN_WIDTH);
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

	// Init
	irqInit();
	irqEnable(IRQ_VBLANK);
	// consoleDemoInit();
	SetMode(3 | BG2_ON);

	// Init private
	InitSoundPlay(&sp_data);
	halSetKeysSort(&b, 4, 5, 6, 7, 0, 1, 3, 2);

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
		MoveLine(&vp_data);
		DrawLines(&vp_data, sp_data.note, halIsAB_hold(&b));
		//DrawLinesTest(&vp_data);
		ConvertMem(&vp_data);
		DrawMemTest(&vp_data);
		//FlushVram(&vp_data);

		// 垂直同期
		VBlankIntrWait();
	}
}


