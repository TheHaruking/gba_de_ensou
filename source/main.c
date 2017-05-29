
#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mappy.h>
#include "hal_eightinput.h"
#include "sound_play.h"

#define SIZE_BAR	8
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

void InitVisualPlay(VISUAL_PLAY* vpd){
	int m, n, m_org;

	m_org = 128;
	m = m_org * SIZE_BAR;
	n = SCREEN_WIDTH * 2; // 480;
	vpd->frame  = 0;
	vpd->mem    = (u16* )malloc(sizeof(u16) * m_org);
	// [m][.] ... 高さ
	// [.][n] ... 横
	vpd->vram   = (u16**)malloc(sizeof(u16*) * m);       // (4 * 1024)
	vpd->vram[0]= (u16* )malloc(sizeof(u16)  * m * n);   // (2 * 480)
	for (int i = 1; i < m ; i++) {
		vpd->vram[i] = vpd->vram[i-1] + sizeof(u16) * n;
	}
	for (int i = 0; i < m; i++) {
		memset((u16*)vpd->vram[i], (u16)0x0000, sizeof(u16) * n);
	}
	dprintf("vram : %08X\n", vpd->vram);
}

void MoveLine(VISUAL_PLAY* vpd) {
	if ((++vpd->frame) >= (SCREEN_WIDTH)) {
		vpd->frame = 0;
	}
}

// 方法その２ 色で制御
void DrawLines(VISUAL_PLAY* vpd, unsigned int y, int flag){
	y &= 0x7f;
	dmaCopy(zero, vpd->mem, 128);

	// 音程に色をセット
	if (flag){
		vpd->mem[y] = RGB5(31, 15, 0);
	}
}
void DrawLinesTest(VISUAL_PLAY* vpd){
	int f = vpd->frame;
	u16* dest = (u16*)(VRAM + SCREEN_WIDTH * 2 * f);
	dmaCopy((u16*)vpd->mem, (u16*)dest, 128);
}

void ConvertMem(VISUAL_PLAY* vpd){
	int n  = vpd->frame;
	//int n2 = vpd->frame + SCREEN_WIDTH;

	// セットした色を、実際の描画サイズ・向きに変換
	for (int i = 0; i < 128; i++){
		vpd->vram[0][i ] = vpd->mem[i];
		//vpd->vram[i][n2] = vpd->mem[i];
	}
}

void DrawMemTest(VISUAL_PLAY* vpd){
	int f   = vpd->frame;
	int dst = VRAM;

	// test
	int j = 0;
	//for (int j = 0; j < 160; j++) {
		for (int i = 0; i < 240; i++) {
			//vpd->vram[j][i] = 0x001F;
			*(u16*)(dst + (j * 240 * 2) + (i * 2)) = vpd->vram[j][i];
		}
	//}
}

void FlushVram(VISUAL_PLAY* vpd) {
	int d = VRAM;
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		dmaCopy((u16*)&vpd->vram[i][0], (u16 *)0x06000000, SCREEN_WIDTH);
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
		dprintf("note : %d\n", sp_data.note);
		//DrawLinesTest(&vp_data);
		ConvertMem(&vp_data);
		DrawMemTest(&vp_data);
		//FlushVram(&vp_data);

		// 垂直同期
		VBlankIntrWait();
	}
}


