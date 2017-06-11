#include <stdlib.h>
#include <gba.h>
#include "graphic_mode0.h"

void InitMode0(GRAPHIC_MODE0* gmd){
	// 0x06000000, 0x06002000, 0x06004000, 0x06006000 
	BGCTRL[0] = SCREEN_BASE(0)  | CHAR_BASE(3) | BG_SIZE(3);
	BGCTRL[1] = SCREEN_BASE(4)  | CHAR_BASE(3) | BG_SIZE(3);
	BGCTRL[2] = SCREEN_BASE(8)  | CHAR_BASE(3) | BG_SIZE(3);
	BGCTRL[3] = SCREEN_BASE(12) | CHAR_BASE(3) | BG_SIZE(3);

	// メモリ確保
	gmd->vram       = (u16***)malloc(sizeof(u16**)           * 4); // BG面分(4つ)
	gmd->vram[0]    = (u16** )malloc(sizeof(u16* )      * 64 * 4); // 縦(64)
	gmd->vram[0][0] = (u16*  )malloc(sizeof(u16  ) * 64 * 64 * 4); // 横(64)
	
	// アドレス代入
	for (int j = 1; j < 4; j++) {
		gmd->vram[j]    = gmd->vram[j - 1]    + sizeof(u16**);
		gmd->vram[j][0] = gmd->vram[j - 1][0] + sizeof(u16* );
	}
	for (int j = 0; j < 4; j++) {
		for (int i = 1; i < 64; i++) {
			gmd->vram[j][i] = gmd->vram[j][i - 1] + sizeof(u16);
		}
	}
}

void FinishMode0(GRAPHIC_MODE0* gmd){
}

void SetBgChr(u16* tiles_adr, u16* pal_adr) {
	// グラフィックデータをメモリへコピー
	dmaCopy((u16*)tiles_adr, BITMAP_OBJ_BASE_ADR, 8192);
	dmaCopy((u16*)pal_adr,   BG_COLORS,          512);
}

void FlushBG(GRAPHIC_MODE0* gmd){
	dmaCopy((u8*)&gmd->vram[0][0][0], (u16 *)VRAM, sizeof(u16) * 64 * 64 * 4);
}
