#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mappy.h>
#include "hal_eightinput.h"
#include "sound_play.h"
#include "chr003.h"
#include "common.h"
#include "graphic_mode4.h"
#include "obj_utils.h"

// 演奏を楽しむ "PLAYモード"
// 右から流れてくる音符の演奏に挑戦する "GAMEモード"
// ... の予定
#define MODE_PLAY	0
#define MODE_GAME	1
#define BOXES_9		9
#define OFS2MEM(n)	(n + 14)

// 映像用データ
typedef struct _VISUAL_PLAY_ {
	int			    frame;			// スクロール用カウンタ
	int				height;			// 高さ
	int				height_view;	// 表示中の高さ
	int				height_spd; 	// 移動中の速さ
	unsigned char** mem;			// 色保存
	int				mode_flag;		// "演奏モード" or "楽譜モード"
	OBJATTR*		icon_key;		// 左のアイコン
	OBJATTR*		icon_ab;		// AB押したときのアイコン
	OBJATTR*		icon_mes;		// 下段の文字
} VISUAL_PLAY, *PVISUAL_PLAY;

// 黒鍵を1に。背景描画に使用
const int keycolor_tbl[12] = {
	0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 
};

const char word_tbl[2][16] = {
	/* おくたーぶ：ＸＸ    ↓：ＸＸ    */ /* 後に'↑'へと反転させる */
	/* きー      ：ＸＸ    ↓：ＸＸ    */
	{ 0xB4, 0xB7, 0xC0, 0x2D, 0xFC, ':', 0, 0, ' ', ' ', 0x97, ':', 0, 0, ' ', ' '},
	{ 0xB6, 0x2D, 0x00, 0x00, 0x00, ':', 0, 0, ' ', ' ', 0x97, ':', 0, 0, ' ', ' '}, 
};

const char word_tbl_note[12][2] = {
	{0xF4, ' '}, {0xF4, '#'}, {0xD8, ' '}, {0xD8, '#'}, 
	{0xD1, ' '}, {0xCD, ' '}, {0xCD, '#'}, {0xBE, ' '}, 
	{0xBE, '#'}, {0xD5, ' '}, {0xD5, '#'}, {0xBB, ' '}, 
};

const char word_tbl_key[12][2] = {
	{'0', '0'}, {'0', '1'}, {'0', '2'}, {'0', '3'}, 
	{'0', '4'}, {'0', '5'}, {'0', '6'}, {'0', '7'}, 
	{'0', '8'}, {'0', '9'}, {'1', '0'}, {'1', '1'}, 
};

const char word_tbl_oct[8][2] = {
	{'-', '1'}, {'0', '0'}, {'0', '1'}, {'0', '2'},
	{'0', '3'}, {'0', '4'}, {'0', '5'}, {'0', '6'},
};

// 初期化
void InitVisualPlay(VISUAL_PLAY* vpd, int ofs){
	int x, y;	// 確保するメモリの横と縦

	// メモリ初期化
	vpd->frame  = 0;
	vpd->height = (OFS2MEM(ofs)) * 8;
	vpd->mode_flag = MODE_PLAY;

	// 配列メモリ確保
	// mem[音程128][幅480] ... 最低音 0, 最高音 127
	x = SCREEN_WIDTH * 2;
	y = 128;
	vpd->mem = (u8**)malloc_arr((void**)vpd->mem,  sizeof(u8), y, x);
}

// 終了処理
void FinishVisualPlay(VISUAL_PLAY* vpd){
}

// 初期化(グラフィック関連)
void InitGraphic(VISUAL_PLAY* vpd, OBJ_UTILS* oud){
	int pos_bottom;

	// グラフィックデータをメモリへコピー
	dmaCopy((u16*)chr003Tiles, BITMAP_OBJ_BASE_ADR, chr003TilesLen);
	//	dmaCopy((u16*)chr003Pal,   BG_COLORS, 		chr003PalLen);
	dmaCopy((u16*)chr003Pal,   OBJ_COLORS,		chr003PalLen);

	// BGパレットに色を設定 (とりあえず)
	BG_COLORS[0x00] = RGB5( 0, 0, 0);
	BG_COLORS[0x01] = RGB5(31,15, 0);
	BG_COLORS[0x02] = RGB5(31,21, 8);
	BG_COLORS[0x10] = RGB5(13,13,13);
	BG_COLORS[0x11] = RGB5( 7, 7, 7);
	BG_COLORS[0x12] = RGB5(31,31,31);


	// obj_utilsが確保したメモリのアドレスをこちらに登録
	// key : 1アイコン4つ * 9 = 36 
	// ab  : 1アイコン2つ * 9 * AとB = 36
	// mes : 16文字 * 2;
	objReg(&vpd->icon_key, oud, 0);
	objReg(&vpd->icon_ab,  oud, 48);
	objReg(&vpd->icon_mes, oud, 96);

	// OBJ に データセット
	// ※下から順に置いていく。
	pos_bottom = 16 * (BOXES_9 - 1); // Y : 128
	for (int i = 0; i < BOXES_9; i++) {
		obj2draw(&vpd->icon_ab[i*4    ], 512 + 0x03, 0, pos_bottom - i*16);
		obj2draw(&vpd->icon_ab[i*4 + 2], 512 + 0x03, 0, pos_bottom - i*16 + 8);
		obj2pal(&vpd->icon_ab[i*4    ], 6); // 白
		obj2pal(&vpd->icon_ab[i*4 + 2], 5); // 灰
	}

	// 画面左 9箱
	for (int i = 0; i < BOXES_9; i++) {
		obj4draw(&vpd->icon_key[i*4], 512 + 0x86, 0, pos_bottom - i*16);
	}
	// 画面下 16文字 2行
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 16; i++) {
			objdraw(&vpd->icon_mes[j*16 + i], 512 + word_tbl[j][i], i*8, 144 + j*8);
		}
	}
	// ここで1行目 "↓：ＸＸ" を "↑：ＸＸ" に反転
	objattr(&vpd->icon_mes[10], 0, 1);

	// 画面←アイコン領域(と、文字の部分) に、描画されないように
	SetMode_add( WIN0_ENABLE | WIN1_ENABLE );
	REG_WININ	= WININ_0(OBJ_ENABLE) | WININ_1(OBJ_ENABLE);	// 内側 : OBJ
	REG_WINOUT	= WINOUT(BG2_ENABLE);							// 外側 : メイン画面
	SetWIN0(  0,   0,  16, 144);
	SetWIN1(  0, 144, 240, 160);

	// 真っ黒部分
	for (int i = 0; i <= 13; i++) {
		memset(vpd->mem[i], 0x00, 480);
	}
	for (int i = 98; i <= 127; i++) {
		memset(vpd->mem[i], 0x00, 480);
	}
}

// 十字キー入力で光らせる
void LightObj(VISUAL_PLAY* vpd, int num, int enable){
	// 十字キー入力無しの場合、光らせない
	if (!enable) {
		num = -1;
	}
	// 画面左 ９箱
	for (int i = 0; i < BOXES_9; i++) {
		obj4pal(&vpd->icon_key[i*4], 0x04);
	}
	// 入力のあった方向を光らせる
	if (num >= 0) {
		obj4pal(&vpd->icon_key[num*4], 0x00);
	}
}

// ABボタン入力で光らせる
void LightObjAB(VISUAL_PLAY* vpd, int num, int enable){
	int n;
	// abキー入力無しの場合、光らせない
	switch (enable) {
		case BTN_A:		n = 0;	break;
		case BTN_B:		n = 1;	break;
		default:		n = 0;	num = -1;	break;
	}
	// 画面左 ９箱
	for (int i = 0; i < BOXES_9; i++) {
		obj2chr(&vpd->icon_ab[i*4    ], 512);
		obj2chr(&vpd->icon_ab[i*4 + 2], 512);
	}
	// 入力のあった方向を光らせる
	if (num >= 0) {
		obj2chr(&vpd->icon_ab[num*4 + n*2], 512 + 0x03);
	}
}

// 下にメッセージを出力
void UpdateMes(VISUAL_PLAY* vpd, int key, int oct) {
	/* おくたーぶ：ＸＸ    ↑：ＸＸ    */
	/* きー      ：ＸＸ    ↓：ＸＸ    */
	int max = DivMod(key + 5, 12);	// 最高音。key:0 のとき へ(ふぁ)   
	int min = DivMod(key, 12);		// 最低音。key:4 のとき ど
	oct += 1; // 補正(oct : -1 ~ 6, word_tbl_oct : 0 ~ 7)
	for (int i = 0 ; i < 2; i++) {
		objchr(&vpd->icon_mes[12 + i], 512 + word_tbl_note[max][i]);
		objchr(&vpd->icon_mes[28 + i], 512 + word_tbl_note[min][i]);
		objchr(&vpd->icon_mes[6  + i], 512 + word_tbl_oct[oct][i]);
		objchr(&vpd->icon_mes[22 + i], 512 + word_tbl_key[key][i]);
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

// 上下方向のスクロール計算処理
void MoveHeight(VISUAL_PLAY* vpd, int ofs) {
	int dst  = (OFS2MEM(ofs)) * 8; 		// 最低-14になるので、補正
	int diff = dst - vpd->height;	

	// ÷8( >> 3)した数値分を引いて、ヌルっとスクロール
	if (abs(diff) > 1) {
		vpd->height_spd =  (diff >> 3) + SGN(diff);
		vpd->height 	+= vpd->height_spd;
	} else {
		vpd->height_spd = 0;
		vpd->height  	= dst;
	}
	// 表示位置確定
	vpd->height_view = 160 - DivMod(vpd->height, 160);
	dprintf("dist   : %d\n", vpd->height);
	dprintf("diff   : %d\n", diff);
	dprintf("height : %d\n", vpd->height);
	dprintf("ofs    : %d\n", ofs);
}

// 音の高さデータを色に変換
void DrawLines(VISUAL_PLAY* vpd, unsigned int y, int flag){
	// 範囲外のとき逃げる
	if ((y < 0) || (y >= 84))
		return;

	int f  = vpd->frame;
	int f2 = f + SCREEN_WIDTH;
	y = OFS2MEM(y); // note(-12 ~ 83), mem(2 ~ 97) 間の補正

	// 音程に色をセット
	if (flag) {
		vpd->mem[y][f ] = (keycolor_tbl[DivMod(y - 2, 12)]) ? 0x0001 : 0x0002;
		vpd->mem[y][f2] = (keycolor_tbl[DivMod(y - 2, 12)]) ? 0x0001 : 0x0002;
	}
}

// 音の高さデータを色に変換(背景)
void DrawLinesBack(VISUAL_PLAY* vpd){
	int f = vpd->frame;
	int f2 = f + SCREEN_WIDTH;
	// 背景色をセット
	for (int j = 0; j < 8; j++) {
		for (int i = 0; i < 12; i++) {
			vpd->mem[OFS2MEM(i + j*12)][f ] = (keycolor_tbl[i]) ? 0x0011 : 0x0010; // 黒鍵 : 白鍵
			vpd->mem[OFS2MEM(i + j*12)][f2] = (keycolor_tbl[i]) ? 0x0011 : 0x0010; // 黒鍵 : 白鍵
		}
	}
}

// mem を、実際に描画する際の絵に変換
void ConvertMem(VISUAL_PLAY* vpd, GRAPHIC_MODE4* gmd, int ofs){
	int f  = vpd->frame;
	// 棒出現位置を右に15 ずらす
	int m  = DivMod(f + 15, 240);
	int m2 = m + 240;

	// セットした色を、実際の描画サイズ・向きに変換
	// i >> 3 して、棒を縦8 にしている
	// m は +15 されていて、棒出現位置を右に15 ずらす役割
	int bottom_line = (OFS2MEM(ofs)) << 3;
	int y_ofs, y_ofs2, i_div8;
	for (int i = 0; i < 160; i++){
		// 下から描画
		y_ofs  = 159 - DivMod(bottom_line + i, 160);
		y_ofs2 = y_ofs + 160;
		i_div8 = (OFS2MEM(ofs) - 2) + (i >> 3);
		gmd->vram[y_ofs ][m ] = vpd->mem[i_div8][f]; 
		gmd->vram[y_ofs ][m2] = vpd->mem[i_div8][f]; 
		gmd->vram[y_ofs2][m ] = vpd->mem[i_div8][f]; 
		gmd->vram[y_ofs2][m2] = vpd->mem[i_div8][f]; 
	}
}

// スクロールした分描画
void ConvertMem_Scrolling(VISUAL_PLAY* vpd, GRAPHIC_MODE4* gmd, int ofs){
	// 移動中でなければ帰る
	if (!vpd->height_spd)
		return;

	int y, y2;
	int sgn = SGN(vpd->height_spd);
	// src : 書き込み元の位置,		y : 書き込み先の位置
	int height_ofs_src = (sgn > 0) ? 144 : -17;
	int height_ofs_y   = (sgn > 0) ? 0   : -1;
	u8* src,* dst11,* dst12,* dst21,* dst22;

	// 移動分の描画
	for (int i = height_ofs_y; abs(i) <= abs(vpd->height_spd); i += sgn) {
		// src : 書き込み元の位置,		y : 書き込み先の位置
		src   = &vpd->mem[((vpd->height + height_ofs_src - i) >> 3)][0];
		y     = DivMod(959 - vpd->height - height_ofs_y + i, 160); 
		y2    = y + 160;
		// 1行分(480ピクセル) 書き込み
		// 書き込み元が240ピクセル分の情報しかないため、2回必要
		dst11 = &gmd->vram[y ][0  ];	dst12 = &gmd->vram[y ][240];
		dst21 = &gmd->vram[y2][0  ];	dst22 = &gmd->vram[y2][240];
		dmaCopy( src, dst11, 240);		dmaCopy( src, dst12, 240);
		dmaCopy( src, dst21, 240);		dmaCopy( src, dst22, 240);
	}
}


//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	BUTTON_INFO  	b;
	SOUND_PLAY   	sp_data;
	GRAPHIC_MODE4	gm_data;
	OBJ_UTILS		ou_data;
	VISUAL_PLAY  	vp_data;

	// Init
	irqInit();
	irqEnable(IRQ_VBLANK);
	SetMode(4 | BG2_ON | OBJ_ON);

	// Init private
	halInitKeys(&b, 4, 5, 6, 7, 0, 1, 3, 2);
	InitSoundPlay(&sp_data);
	InitMode4(&gm_data);
	InitObj(&ou_data);

	// 初期音設定(上書き)
	sp_data.octave = 2;
	sp_data.key    = 4;
	sp_data.snd_duty = 1;
	SoundPlay(&sp_data, &b);

	// Init Video
	InitVisualPlay(&vp_data, sp_data.ofs);
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
		MoveHeight(&vp_data, sp_data.ofs);
		DrawLinesBack(&vp_data);
		DrawLines(&vp_data, sp_data.note, halIsAB_hold(&b));

		// 左のアイコン類
		LightObj(&vp_data,  sp_data.vector, halIsKey_hold(&b));
		LightObjAB(&vp_data, sp_data.vector_keep, halIsAB_hold(&b));
		UpdateMes(&vp_data, sp_data.key, sp_data.octave);

		// 変換処理
		ConvertMem(&vp_data, &gm_data, sp_data.ofs);
		ConvertMem_Scrolling(&vp_data, &gm_data, sp_data.ofs);

		// 垂直同期待機・書き込み
		VBlankIntrWait();
		FlushVramOfs(&gm_data, vp_data.height_view, vp_data.frame);
		FlushSprite(&ou_data);
	}

	// メモリ開放
	FinishMode4(&gm_data);
	FinishObj(&ou_data);
}


