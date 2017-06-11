#include <gba.h>
#include "common.h"
#include "graphic_mode4.h"

// 縦横2倍
void InitMode4(GRAPHIC_MODE4* gm4){
	int y = SCREEN_HEIGHT * 2; // 320
	int x = SCREEN_WIDTH  * 2; // 480
	gm4->vram = (u8**)malloc_arr((void**)gm4->vram, sizeof(u8), y, x);
}

// 
void FinishMode4(GRAPHIC_MODE4* gm4){
	free_arr((void**)gm4->vram);
}

// バッファからVRAMに書き込み
void FlushVram(GRAPHIC_MODE4* gm4) {
	dmaCopy((u8*)&gm4->vram[0][0], (u16 *)VRAM, sizeof(u8) * SCREEN_WIDTH * SCREEN_HEIGHT);
}

// バッファからVRAMに書き込み
void FlushVramOfs(GRAPHIC_MODE4* gm4, int y, int x) {
	unsigned int d = VRAM;
	// スクロールさせるため、1行ずつ書き込む
	for (int i = y; i < y + SCREEN_HEIGHT; i++) {
		dmaCopy((u8*)&gm4->vram[i][x], (u16 *)d, SCREEN_WIDTH);
		d += sizeof(u8) * SCREEN_WIDTH;
	}
}

//////////////////

// test
// 画面の端や真ん中に点を描画して、期待通りの描画ができているか確認
void testvram(GRAPHIC_MODE4* gm4){
	int d = VRAM;

	gm4->vram[0][0]     = 0x01;		// 左上
	gm4->vram[0][1]     = 0x01;		// 左上から右に1ドットずれたところ
	gm4->vram[0][2]     = 0x01;
	gm4->vram[0][20]    = 0x01;		// 左上から右に20ドット
	gm4->vram[1][0]     = 0x01;		// 左上から下に1ドット
	gm4->vram[2][0]     = 0x01;
	gm4->vram[20][0]    = 0x01;
	gm4->vram[21][0]    = 0x01;
	gm4->vram[21][1]    = 0x01;
	gm4->vram[80][120]  = 0x01;		// 真ん中(GBAは,縦160,横240 の液晶です)
	gm4->vram[0][239]   = 0x01;		// 右上
	gm4->vram[1][239]   = 0x01;
	gm4->vram[159][239] = 0x01;		// 右下
	
	// 実際の画面に書き込む
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		dmaCopy((u8*)&gm4->vram[i][0], (u16 *)d, SCREEN_WIDTH);
		d += sizeof(u8) * SCREEN_WIDTH;
	}
}