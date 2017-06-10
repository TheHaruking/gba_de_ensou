#include <gba.h>
#include "common.h"
#include "graphic_mode0.h"

void InitMode0(GRAPHIC_MODE0* gmd){
	int m = 32;
	int n = 32;
	gmd->vram = (u8**)malloc_arr((void**)gmd->vram, sizeof(u8), m, n);
}

void FinishMode0(GRAPHIC_MODE0* gmd){
}

void SetBgChr(u16* tiles_adr, u16* pal_adr) {
	// グラフィックデータをメモリへコピー
	dmaCopy((u16*)tiles_adr, BITMAP_OBJ_BASE_ADR, 8192);
	dmaCopy((u16*)pal_adr,   BG_COLORS,          512);
}

void FlushBG(GRAPHIC_MODE0* gmd){
	dmaCopy((u8*)&gmd->vram[0][0], (u16 *)VRAM, sizeof(u8) * SCREEN_WIDTH * SCREEN_HEIGHT);
}
