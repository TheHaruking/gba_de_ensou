#include <stdlib.h>
#include <gba.h>
#include "obj_utils.h"

void InitObj(OBJ_UTILS* oud) {
	oud->obj = (OBJATTR*)malloc(sizeof(OBJATTR) * OBJ_MAX);
}

void FinishObj(OBJ_UTILS* oud) {
	free(oud->obj);
}

void SetObjChr(OBJ_UTILS* oud, const void* tiles_adr, const void* pal_adr) {
	// グラフィックデータをメモリへコピー
	dmaCopy((u16*)tiles_adr, BITMAP_OBJ_BASE_ADR, 8192);
	dmaCopy((u16*)pal_adr,   OBJ_COLORS,    512);
}

void objReg(OBJATTR** attrp, OBJ_UTILS* oud, int n){
	*attrp = &oud->obj[n];
}

// 移動
void objMove(OBJATTR* attr, int x, int y){
	attr->attr0 &= 0xFF00;
	attr->attr1 &= 0xFE00;
	attr->attr0 |= OBJ_Y(y);
	attr->attr1 |= OBJ_X(x);
}

// 4まとまりを一度に操作
void obj4draw(OBJATTR* attr, int chr, int x, int y){
	int x2 = x + 8;
	int y2 = y + 8;

	attr[0].attr0 = OBJ_Y(y ) | OBJ_16_COLOR;
	attr[0].attr1 = OBJ_X(x ) | 0         | OBJ_VFLIP;
	attr[0].attr2 = OBJ_CHAR(chr);
	
	attr[1].attr0 = OBJ_Y(y ) | OBJ_16_COLOR;
	attr[1].attr1 = OBJ_X(x2) | OBJ_HFLIP | OBJ_VFLIP;
	attr[1].attr2 = OBJ_CHAR(chr);

	attr[2].attr0 = OBJ_Y(y2) | OBJ_16_COLOR;
	attr[2].attr1 = OBJ_X(x)  | 0         | 0;
	attr[2].attr2 = OBJ_CHAR(chr);

	attr[3].attr0 = OBJ_Y(y2) | OBJ_16_COLOR;
	attr[3].attr1 = OBJ_X(x2) | OBJ_HFLIP | 0;
	attr[3].attr2 = OBJ_CHAR(chr);
}

// 2まとまりを一度に操作
void obj2draw(OBJATTR* attr, int chr, int x, int y) {
	int x2 = x + 8;

	attr[0].attr0 = OBJ_Y(y ) | OBJ_16_COLOR;
	attr[0].attr1 = OBJ_X(x ) | 0         | OBJ_VFLIP;
	attr[0].attr2 = OBJ_CHAR(chr);
	
	attr[1].attr0 = OBJ_Y(y ) | OBJ_16_COLOR;
	attr[1].attr1 = OBJ_X(x2) | OBJ_HFLIP | 0;
	attr[1].attr2 = OBJ_CHAR(chr);
}

void obj2pal(OBJATTR* attr, int pal) {
	attr[0].attr2 &= 0x0FFF;
	attr[0].attr2 |= OBJ_PALETTE(pal);
	attr[1].attr2 &= 0x0FFF;
	attr[1].attr2 |= OBJ_PALETTE(pal);
}

void obj2chr(OBJATTR* attr, int chr) {
	attr[0].attr2 &= 0xFC00;
	attr[0].attr2 |= OBJ_CHAR(chr);
	attr[1].attr2 &= 0xFC00;
	attr[1].attr2 |= OBJ_CHAR(chr);
}

void obj4pal(OBJATTR* attr, int pal) {
	attr[0].attr2 &= 0x0FFF;
	attr[0].attr2 |= OBJ_PALETTE(pal);
	attr[1].attr2 &= 0x0FFF;
	attr[1].attr2 |= OBJ_PALETTE(pal);
	attr[2].attr2 &= 0x0FFF;
	attr[2].attr2 |= OBJ_PALETTE(pal);
	attr[3].attr2 &= 0x0FFF;
	attr[3].attr2 |= OBJ_PALETTE(pal);
}

void objdraw(OBJATTR* attr, int chr, int x, int y) {
	attr[0].attr0 = OBJ_Y(y ) | OBJ_16_COLOR;
	attr[0].attr1 = OBJ_X(x ) ;
	attr[0].attr2 = OBJ_CHAR(chr);
}

void objchr(OBJATTR* attr, int chr) {
	attr[0].attr2 &= 0xFC00;
	attr[0].attr2 |= OBJ_CHAR(chr);
}

void objattr(OBJATTR* attr, int h, int v) {
	h &= 1; v &= 1;
	attr[0].attr1 &= 0xCFFF;
	attr[0].attr1 |= (h << 12) | (v << 13) ;
}

void objInit(OBJATTR* attr, int chr, int col256) {
	col256 &= 1;
	attr->attr0 = 0 | (col256 ? ATTR0_COLOR_256 : 0);
	attr->attr1 = 0;
	attr->attr2 = OBJ_CHAR(chr);
}

// バッファからSpriteRAMに書き込み
void FlushSprite(OBJ_UTILS* oud) {
	// 実際のOBJ情報メモリに書き込み
	dmaCopy(oud->obj, (u16*)OAM, sizeof(OBJATTR) * OBJ_MAX);
}