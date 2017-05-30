
#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mappy.h>
#include "hal_eightinput.h"
#include "sound_play.h"

// 映像用データ
typedef struct _VP_LINE_ {
	int		id;
	int		x, y;
	int		len;
	int		col;
	int		frame;
} VP_LINE, *PVP_LINE;

typedef struct _VISUAL_PLAY_ {
	unsigned int    frame;
	unsigned char * mem;
	unsigned char **vram;
} VISUAL_PLAY, *PVISUAL_PLAY;

void InitVisualPlay(VISUAL_PLAY* vpd){
	int m, n, m_org;

	// パレットに色を設定
	BG_COLORS[0] = RGB5(0,0,0);
	BG_COLORS[1] = RGB5(31,31,0);
	BG_COLORS[2] = RGB5(0,0,31);
	
	m_org = 128;
	m = SCREEN_HEIGHT;
	n = sizeof(u8) * SCREEN_WIDTH * 2; // 480;
	vpd->frame  = 0;
	// mem[128]
	vpd->mem    = (u8* )malloc(sizeof(u8 ) * m_org);
	// vram[m][.] ... 高さ
	// vram[.][n] ... 横
	vpd->vram   = (u8**)malloc(sizeof(u8*) * m);       // (4 * 160)
	vpd->vram[0]= (u8* )malloc(sizeof(u8 ) * n * m);   // (2 * 160 * 480)
	// 先頭アドレスをセット ： 2 x 480 間隔で
	for (int i = 1; i < m ; i++) {
		vpd->vram[i] = vpd->vram[i-1] + sizeof(u8) * n;
	}
	dprintf("vram : %d\n", sizeof(u8 ) * n * m);
}

// test
// 端や真ん中に点を描画
void testvram(VISUAL_PLAY* vpd){
	int d = VRAM;

	vpd->vram[0][0]   = 0x01;
	vpd->vram[0][1]   = 0x01;
	vpd->vram[0][2]   = 0x01;
	vpd->vram[0][20]  = 0x01;
	vpd->vram[1][0]   = 0x01;
	vpd->vram[2][0]   = 0x01;
	vpd->vram[20][0]   = 0x01;
	vpd->vram[21][0]   = 0x01;
	vpd->vram[21][1]   = 0x01;
	vpd->vram[80][120]= 0x01;
	vpd->vram[0][239] = 0x01;
	vpd->vram[1][239] = 0x01;
	vpd->vram[159][239] = 0x01;
	
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		dmaCopy((u8*)&vpd->vram[i][5], (u16 *)d, SCREEN_WIDTH);
		d += sizeof(u8) * SCREEN_WIDTH;
	}
}

// スクロール用数値(frame)を計算
void MoveLine(VISUAL_PLAY* vpd) {
	// 0 ~ 239 をループする数値(スクロール用)
	if ((++vpd->frame) >= (SCREEN_WIDTH)) {
		vpd->frame = 0;
	}
}

// 音の高さデータを色に変換
void DrawLines(VISUAL_PLAY* vpd, unsigned int y, int flag){
	y &= 0x7f;
	// 黒で初期化
	memset(vpd->mem, 0x00, 128);

	// 音程に色をセット
	if (flag){
		vpd->mem[y] = 0x01;
	}
}

// 上から128音程描画確認
void DrawLinesTest(VISUAL_PLAY* vpd){
	int f = vpd->frame;
	u16* dest = (u16*)(VRAM + SCREEN_WIDTH * 2 * f);
	dmaCopy((u16*)vpd->mem, (u16*)dest, 128);
}

// mem を、実際に描画する際の絵に変換
void ConvertMem(VISUAL_PLAY* vpd){
	int n  = vpd->frame;
	int n2 = vpd->frame + SCREEN_WIDTH;
	int note = 26;

	// セットした色を、実際の描画サイズ・向きに変換
	// i >> 3 して、棒を縦8 にしている
	for (int i = 0; i < SCREEN_HEIGHT; i++){
		vpd->vram[i][n ] = vpd->mem[(i >> 3) + note]; 
		vpd->vram[i][n2] = vpd->mem[(i >> 3) + note]; 
	}
}

// バッファからVRAMに書き込み
void FlushVram(VISUAL_PLAY* vpd) {
	unsigned int d = VRAM;
	// スクロールさせるため、1行ずつ書き込む
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		dmaCopy((u8*)&vpd->vram[i][0], (u16 *)d, SCREEN_WIDTH);
		d += sizeof(u8) * SCREEN_WIDTH;
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
	SetMode(4 | BG2_ON);

	// Init private
	InitSoundPlay(&sp_data);
	halSetKeysSort(&b, 4, 5, 6, 7, 0, 1, 3, 2);

	// Init Video
	InitVisualPlay(&vp_data);
	//testvram(&vp_data);

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
		//dprintf("note : %d\n", sp_data.note);
		//DrawLinesTest(&vp_data);
		ConvertMem(&vp_data);
		//DrawMemTest(&vp_data);
		FlushVram(&vp_data);

		// 垂直同期
		VBlankIntrWait();
	}
}


