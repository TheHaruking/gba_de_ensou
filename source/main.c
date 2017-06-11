#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mappy.h>
#include "hal_eightinput.h"
#include "sound_play.h"
#include "chr003.h"
#include "common.h"
#include "graphic_mode0.h"
#include "obj_utils.h"


// 演奏を楽しむ "PLAYモード"
// 右から流れてくる音符の演奏に挑戦する "GAMEモード"
// ... の予定
#define MODE_PLAY	0
#define MODE_GAME	1
#define OBJ_LEFT9	9

// 黒鍵を1に。背景描画に使用
const int keycolor_tbl[12] = {
	0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 
};

// 映像用データ
typedef struct _VISUAL_PLAY_ {
	int			    frame;		// スクロール用カウンタ
	unsigned char** mem;		// 色保存
	int				mode_flag;	// "演奏モード" or "楽譜モード"
	OBJATTR*		icon_key;		// 左のアイコン
	OBJATTR*		icon_ab;		// AB押したときのアイコン
} VISUAL_PLAY, *PVISUAL_PLAY;

// 初期化
void InitVisualPlay(VISUAL_PLAY* vpd){
	int m_mem, n_mem;	// 確保するメモリの横と縦

	// メモリ初期化
	vpd->frame  = 0;
	vpd->mode_flag = MODE_PLAY;

	// 配列メモリ確保
	// mem[幅480][音程128]
	m_mem = SCREEN_WIDTH * 2;
	n_mem = 128;
	vpd->mem = (u8**)malloc_arr((void**)vpd->mem,  sizeof(u8), m_mem, n_mem);
}

void FinishVisualPlay(VISUAL_PLAY* vpd){
}

void InitGraphic(VISUAL_PLAY* vpd, OBJ_UTILS* oud){
	int n_key/*, n_ab*/;
	int pos_bottom;

	// グラフィックデータをメモリへコピー
	// 1. BG専用領域,  2. 共用領域,  3. OBJ専用領域
	dmaCopy((u16*)chr003Tiles, CHAR_BASE_BLOCK(3),  chr003TilesLen);
	dmaCopy((u16*)chr003Tiles, OBJ_BASE_ADR,		chr003TilesLen); // BGでもCHAR_BASE_BLOCK(3)なら512以降で使用可
	dmaCopy((u16*)chr003Tiles, BITMAP_OBJ_BASE_ADR, chr003TilesLen);
	dmaCopy((u16*)chr003Pal,   BG_COLORS, 		chr003PalLen);
	dmaCopy((u16*)chr003Pal,   OBJ_COLORS,		chr003PalLen);

	// obj_utilsが確保したメモリのアドレスをこちらに登録
	n_key	= 4 * OBJ_LEFT9;		// 1アイコン4つぶん 
//	n_ab	= 2 * OBJ_LEFT9 * 2;	// 1アイコン2つぶん * AとB (未使用)
	objReg(&vpd->icon_key, oud, 0);
	objReg(&vpd->icon_ab,  oud, n_key);

	// OBJ に データセット
	// ※下から順に置いていく。
	pos_bottom = 16 * (OBJ_LEFT9 - 1); // Y : 128
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj2draw(&vpd->icon_ab[i*4    ], 0x03, 0, pos_bottom - i*16);
		obj2draw(&vpd->icon_ab[i*4 + 2], 0x03, 0, pos_bottom - i*16+8);
		obj2pal(&vpd->icon_ab[i*4    ], 2); // 緑
		obj2pal(&vpd->icon_ab[i*4 + 2], 5); // 灰
	}

	// 画面左 ９箱
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj4draw(&vpd->icon_key[i*4], 0x86, 0, pos_bottom - i*16);
	}
}

void LightObj(OBJATTR* attr, int num, int enable){
	// 十字キー入力無しの場合、光らせない
	if (!enable) {
		num = -1;
	}
	// 画面左 ９箱
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj4pal(&attr[i*4], 0x04);
	}
	// 入力のあった方向を光らせる
	if (num >= 0) {
		obj4pal(&attr[num*4], 0x00);
	}
}

void LightObjAB(OBJATTR* attr, int num, int enable){
	int n;
	// abキー入力無しの場合、光らせない
	switch (enable) {
		case BTN_A:		n = 0;	break;
		case BTN_B:		n = 1;	break;
		default:		n = 0;	num = -1;	break;
	}
	// 画面左 ９箱
	for (int i = 0; i < OBJ_LEFT9; i++) {
		obj2chr(&attr[i*4  ], 0x00);
		obj2chr(&attr[i*4+2], 0x00);
	}
	// 入力のあった方向を光らせる
	if (num >= 0) {
		obj2chr(&attr[num*4 + n*2], 0x03);
	}
}

// スクロール用数値(frame)を計算
void MoveLine(VISUAL_PLAY* vpd) {
	// 0 ~ 239 をループする数値(スクロール用)
	// 演奏モード : →方向
	// 楽譜モード : ←方向
	switch (vpd->mode_flag) {
		case MODE_PLAY: vpd->frame--; break;
		case MODE_GAME: vpd->frame++; break;
	}

	switch (vpd->frame) {
		case -1:			vpd->frame = SCREEN_WIDTH - 1;	break;
		case SCREEN_WIDTH:	vpd->frame = 0;					break;
	}
}

// 音の高さデータを色に変換
void DrawLines(VISUAL_PLAY* vpd, unsigned int y, int flag){
}

// 音の高さデータを色に変換(背景)
void DrawLinesBack(VISUAL_PLAY* vpd){
}

// 上から128音程描画確認
void DrawLinesTest(VISUAL_PLAY* vpd){
	int f = vpd->frame;
	u16* dest = (u16*)(VRAM + SCREEN_WIDTH * 2 * f);
	dmaCopy((u16*)vpd->mem[f], (u16*)dest, 128);
}

// mem を、実際に描画する際の絵に変換
void ConvertMem(VISUAL_PLAY* vpd, GRAPHIC_MODE0* gmd){
	gmd->vram[0][0][4] = 0x0001;
	gmd->vram[3][4][4] = 0x00FF;
}


//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	BUTTON_INFO  	b;
	SOUND_PLAY   	sp_data;
	GRAPHIC_MODE0	gm_data;
	OBJ_UTILS		ou_data;
	VISUAL_PLAY  	vp_data;

	// Init
	irqInit();
	irqEnable(IRQ_VBLANK);
	SetMode(0 | BG_ALL_ON | OBJ_ON);

	// Init private
	halInitKeys(&b, 4, 5, 6, 7, 0, 1, 3, 2);
	InitSoundPlay(&sp_data);
	InitMode0(&gm_data);
	InitObj(&ou_data);

	// Init Video
	InitVisualPlay(&vp_data);
	InitGraphic(&vp_data, &ou_data);

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
		LightObj(vp_data.icon_key,  sp_data.vector, halIsKey_hold(&b));
		LightObjAB(vp_data.icon_ab, sp_data.vector, halIsAB_hold(&b));

		// 描画準備
		ConvertMem(&vp_data, &gm_data);

		// 垂直同期待機・書き込み
		VBlankIntrWait();
		FlushBG(&gm_data);
		FlushSprite(&ou_data);
	}

	FinishMode0(&gm_data);
	FinishObj(&ou_data);
}


