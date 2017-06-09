
#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mappy.h>
#include "hal_eightinput.h"
#include "sound_play.h"
#include "chr003.h"
#include "common.h"


// 演奏を楽しむ "PLAYモード"
// 右から流れてくる音符の演奏に挑戦する "GAMEモード"
// ... の予定
#define MODE_PLAY	0
#define MODE_GAME	1
#define OBJ_LEFT9	9
#define OBJ_MAX		128

// 黒鍵を1に。背景描画に使用（予定）
const int keycolor_tbl[12] = {
	0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 
};

// 映像用データ
typedef struct _VISUAL_PLAY_ {
	int			    frame;		// スクロール用カウンタ
	unsigned char** mem;		// 色保存
	unsigned char** vram;		// 描画バッファ
	int				mode_flag;	// "演奏モード" or "楽譜モード"
	OBJATTR*		icon_all;	// 
	OBJATTR*		icon_key;		// 左のアイコン
	OBJATTR*		icon_ab;		// AB押したときのアイコン
} VISUAL_PLAY, *PVISUAL_PLAY;

// 初期化
void InitVisualPlay(VISUAL_PLAY* vpd){
	int m, n, m_mem, n_mem;
	int n_key	 = 4 * OBJ_LEFT9;		// 1アイコン4つぶん 
/*	int n_ab 	 = 2 * OBJ_LEFT9 * 2;	// 1アイコン2つぶん * AとB  (未使用)*/

	// パレットに色を設定
	BG_COLORS[0x00]  = RGB5( 0, 0, 0);
	BG_COLORS[0x01]  = RGB5(31,15, 0);
	BG_COLORS[0x02]  = RGB5( 0, 0,31);
	BG_COLORS[0x10]  = RGB5(13,13,13);
	BG_COLORS[0x11]  = RGB5( 7, 7, 7);
	
	// メモリ初期化
	vpd->frame  = 0;
	vpd->mode_flag = MODE_PLAY;
	// 配列メモリ確保設定
	m_mem = SCREEN_WIDTH * 2;
	n_mem = 128;
	m = SCREEN_HEIGHT;
	n = sizeof(u8) * SCREEN_WIDTH * 2; // 480;
	// 各配列メモリ確保
	// mem[幅480][音程128]
	// vram[高160][幅480]
	vpd->mem     = (u8**)malloc_arr((void**)vpd->mem,  sizeof(u8), m_mem, n_mem);
	vpd->vram    = (u8**)malloc_arr((void**)vpd->vram, sizeof(u8), m,     n);
	vpd->icon_all = (OBJATTR*)malloc(sizeof(OBJATTR*) * OBJ_MAX);   // とりあえず、128個分確保しておく
	vpd->icon_key = &vpd->icon_all[0];		// まずkey
	vpd->icon_ab  = &vpd->icon_all[n_key];  // key の次に ab
}

void FinishVisualPlay(VISUAL_PLAY* vpd){
	free(vpd->icon_ab);
	free(vpd->icon_key);
	free_arr((void**)vpd->vram);
	free_arr((void**)vpd->mem);
}

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
void obj2draw(OBJATTR* attr, int chr, int x, int y){
	int x2 = x + 8;
	attr[0].attr0 = OBJ_Y(y ) | OBJ_16_COLOR;
	attr[0].attr1 = OBJ_X(x ) | 0         | OBJ_VFLIP;
	attr[0].attr2 = OBJ_CHAR(chr);
	
	attr[1].attr0 = OBJ_Y(y ) | OBJ_16_COLOR;
	attr[1].attr1 = OBJ_X(x2) | OBJ_HFLIP | 0;
	attr[1].attr2 = OBJ_CHAR(chr);
}

void obj2pal(OBJATTR* attr, int pal){
	attr[0].attr2 &= 0x0FFF;
	attr[0].attr2 |= OBJ_PALETTE(pal);
	attr[1].attr2 &= 0x0FFF;
	attr[1].attr2 |= OBJ_PALETTE(pal);
}

void obj2chr(OBJATTR* attr, int chr){
	attr[0].attr2 &= 0xFC00;
	attr[0].attr2 |= OBJ_CHAR(chr);
	attr[1].attr2 &= 0xFC00;
	attr[1].attr2 |= OBJ_CHAR(chr);
}

void obj4pal(OBJATTR* attr, int pal){
	attr[0].attr2 &= 0x0FFF;
	attr[0].attr2 |= OBJ_PALETTE(pal);
	attr[1].attr2 &= 0x0FFF;
	attr[1].attr2 |= OBJ_PALETTE(pal);
	attr[2].attr2 &= 0x0FFF;
	attr[2].attr2 |= OBJ_PALETTE(pal);
	attr[3].attr2 &= 0x0FFF;
	attr[3].attr2 |= OBJ_PALETTE(pal);
}

void objInit(OBJATTR* attr, int chr, int col256){
	col256 &= 1;
	attr->attr0 = 0 | (col256 ? ATTR0_COLOR_256 : 0);
	attr->attr1 = 0;
	attr->attr2 = OBJ_CHAR(chr);
}

// test
// obj描画Test
void InitGraphic(VISUAL_PLAY* vpd){
	// グラフィックデータをメモリへコピー
	dmaCopy((u16*)chr003Tiles, BITMAP_OBJ_BASE_ADR, chr003TilesLen);
	dmaCopy((u16*)chr003Pal,   OBJ_COLORS,          chr003PalLen);

	// OBJ に データセット
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj2draw(&vpd->icon_ab[i*4  ], 512 + 0x03, 0, i*16);
		obj2draw(&vpd->icon_ab[i*4+2], 512 + 0x03, 0, i*16+8);
		obj2pal(&vpd->icon_ab[i*4  ], 2);
		obj2pal(&vpd->icon_ab[i*4+2], 5);
	}

	// 画面左 ９箱
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj4draw(&vpd->icon_key[i*4], 512 + 0x86, 0, i*16);
	}

	// 実際のOBJ情報メモリに書き込み
	dmaCopy(vpd->icon_key, (u16*)OAM, sizeof(OBJATTR) * 128);
}

void ObjFeeder(OBJATTR* attr, int num, int enable){
	// 十字キー入力無しの場合、光らせない
	if (!enable) {
		num = -1;
	}
	// 画面左 ９箱
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj4pal(&attr[i*4], 4);
	}
	// 入力のあった方向を光らせる
	if (num >= 0) {
		obj4pal(&attr[num*4], 0);
	}
}

void ObjFeederAB(OBJATTR* attr, int num, int enable){
	int n;
	// abキー入力無しの場合、光らせない
	switch (enable) {
		case BTN_A:
			n = 0;
			break;
		case BTN_B:
			n = 1;
			break;
		default:
			n = 0;
			num = -1;
			break;
	}
	// 画面左 ９箱
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj2chr(&attr[i*4  ], 512);
		obj2chr(&attr[i*4+2], 512);
	}
	// 入力のあった方向を光らせる
	if (num >= 0) {
		obj2chr(&attr[num*4 + n*2], 512 + 3);
	}
}

// test
// 画面の端や真ん中に点を描画して、期待通りの描画ができているか確認
void testvram(VISUAL_PLAY* vpd){
	int d = VRAM;

	vpd->vram[0][0]     = 0x01;		// 左上
	vpd->vram[0][1]     = 0x01;		// 左上から右に1ドットずれたところ
	vpd->vram[0][2]     = 0x01;
	vpd->vram[0][20]    = 0x01;		// 左上から右に20ドット
	vpd->vram[1][0]     = 0x01;		// 左上から下に1ドット
	vpd->vram[2][0]     = 0x01;
	vpd->vram[20][0]    = 0x01;
	vpd->vram[21][0]    = 0x01;
	vpd->vram[21][1]    = 0x01;
	vpd->vram[80][120]  = 0x01;		// 真ん中(GBAは,縦160,横240 の液晶です)
	vpd->vram[0][239]   = 0x01;		// 右上
	vpd->vram[1][239]   = 0x01;
	vpd->vram[159][239] = 0x01;		// 右下
	
	// 実際の画面に書き込む
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		dmaCopy((u8*)&vpd->vram[i][0], (u16 *)d, SCREEN_WIDTH);
		d += sizeof(u8) * SCREEN_WIDTH;
	}
}

// スクロール用数値(frame)を計算
void MoveLine(VISUAL_PLAY* vpd) {
	// 0 ~ 239 をループする数値(スクロール用)
	// 演奏モード : →方向
	// 楽譜モード : ←方向
	switch (vpd->mode_flag) {
		case MODE_PLAY:
			if ((--vpd->frame) < 0) {
				vpd->frame = SCREEN_WIDTH - 1;
			}
			break;
		case MODE_GAME:
			if ((++vpd->frame) >= (SCREEN_WIDTH)) {
				vpd->frame = 0;
			}
			break;
	}
}

// 音の高さデータを色に変換
void DrawLines(VISUAL_PLAY* vpd, unsigned int y, int flag){
	int f = vpd->frame;
	// 上下反転させるため、128 から引いておく
	y = 128 - (y & 0x7f);

	// 音程に色をセット
	if (flag){
		vpd->mem[f][y] = 0x01;
	}
}

// 音の高さデータを色に変換(背景)
void DrawLinesBack(VISUAL_PLAY* vpd){
	int f = vpd->frame;
	// とりあえずnote 83
	int note = 83;

	// 背景色をセット
	for (int j = 0; j < 10; j++) {
		for (int i = 0; i < 12; i++) {
			vpd->mem[f][128 - (i + j*12)] = (keycolor_tbl[i]) ? 0x11 : 0x10;
		}
	}
}

// 上から128音程描画確認
void DrawLinesTest(VISUAL_PLAY* vpd){
	int f = vpd->frame;
	u16* dest = (u16*)(VRAM + SCREEN_WIDTH * 2 * f);
	dmaCopy((u16*)vpd->mem[f], (u16*)dest, 128);
}

// mem を、実際に描画する際の絵に変換
void ConvertMem(VISUAL_PLAY* vpd){
	int n  = vpd->frame;
	int n2 = n + SCREEN_WIDTH;
	// 棒出現位置を右に15 ずらす
	int m  = (n + 15) % 240;
	int m2 = m + SCREEN_WIDTH;

	// とりあえずnote 83にして即時確認できるように。
	// 本当は、現在のキーボードのオクターブをみて決めないとダメ
	int note = 83;

	// セットした色を、実際の描画サイズ・向きに変換
	// i >> 3 して、棒を縦8 にしている
	// m は +15 されていて、棒出現位置を右に15 ずらす役割
	for (int i = 0; i < SCREEN_HEIGHT; i++){
		vpd->vram[i][m ] = vpd->mem[n][(i >> 3) + note]; 
		vpd->vram[i][m2] = vpd->mem[n][(i >> 3) + note]; 
	}

	// 左パネルが汚されるのを防ぐ
	for (int i = 0; i < SCREEN_HEIGHT; i++){
		vpd->vram[i][n ] = 0x00; 
		vpd->vram[i][n2] = 0x00;; 
	}
}

// バッファからVRAMに書き込み
void FlushVram(VISUAL_PLAY* vpd) {
	unsigned int d = VRAM;
	// スクロールさせるため、1行ずつ書き込む
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		dmaCopy((u8*)&vpd->vram[i][vpd->frame], (u16 *)d, SCREEN_WIDTH);
		d += sizeof(u8) * SCREEN_WIDTH;
	}
}

// バッファからSpriteRAMに書き込み
void FlushSprite(VISUAL_PLAY* vpd) {
	// 実際のOBJ情報メモリに書き込み
	dmaCopy(vpd->icon_key, (u16*)OAM, sizeof(OBJATTR) * 128);
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
	SetMode(4 | BG2_ON | OBJ_ON);

	// Init private
	InitSoundPlay(&sp_data);
	halSetKeysSort(&b, 4, 5, 6, 7, 0, 1, 3, 2);

	// Init Video
	InitVisualPlay(&vp_data);
	InitGraphic(&vp_data);

	// Sound Init
	REG_SOUNDCNT_X = 0x80;		// turn on sound circuit
	REG_SOUNDCNT_L = 0x1177;	// full volume, enable sound 1 to left and right
	REG_SOUNDCNT_H = 2;			// Overall output ratio - Full

	while (1) {
		// ボタン入力初期化
		scanKeys();
		halSetKeys(&b, keysHeld());

		// 音処理
		SoundPlay(&sp_data, &b);

		// 映像
		MoveLine(&vp_data);
		DrawLinesBack(&vp_data);
		DrawLines(&vp_data, sp_data.note, halIsAB_hold(&b));

		// 左のアイコン類
		ObjFeeder(vp_data.icon_key,  9 - sp_data.vector, halIsKey_hold(&b));
		ObjFeederAB(vp_data.icon_ab, 9 - sp_data.vector, halIsAB_hold(&b));

		// 書き込み処理
		ConvertMem(&vp_data);
		FlushVram(&vp_data);
		FlushSprite(&vp_data);

		// 垂直同期
		VBlankIntrWait();
	}
}


