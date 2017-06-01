
#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mappy.h>
#include "hal_eightinput.h"
#include "sound_play.h"
#include "chr003.h"


// 演奏を楽しむ "PLAYモード"
// 右から流れてくる音符の演奏に挑戦する "GAMEモード"
// ... の予定
#define MODE_PLAY	0
#define MODE_GAME	1

#define $A			0xB0
#define $I			0xB1
#define $U			0xB2
#define $N			0xFF


// 映像用データ
typedef struct _VISUAL_PLAY_ {
	int			    frame;		// スクロール用カウンタ
	unsigned char** mem;		// 色保存
	unsigned char** vram;		// 描画バッファ
	int				mode_flag;	// "演奏モード" or "楽譜モード"

	OBJATTR*		icon_key;	// 左のアイコン
} VISUAL_PLAY, *PVISUAL_PLAY;

// 初期化
void InitVisualPlay(VISUAL_PLAY* vpd){
	int m, n, m_mem, n_mem;

	// パレットに色を設定
	BG_COLORS[0]  = RGB5( 0, 0, 0);
	BG_COLORS[1]  = RGB5(31,31, 0);
	BG_COLORS[2]  = RGB5( 0, 0,31);
	OBJ_COLORS[0] = RGB5( 0, 0, 0);
	OBJ_COLORS[1] = RGB5(31,31, 0);
	OBJ_COLORS[2] = RGB5( 0, 0,31);
	
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
	vpd->mem    = (u8**)malloc(sizeof(u8*) * m_mem);
	vpd->mem[0] = (u8* )malloc(sizeof(u8 ) * n_mem * m_mem);
	vpd->vram   = (u8**)malloc(sizeof(u8*) * m);       // (4 * 160)
	vpd->vram[0]= (u8* )malloc(sizeof(u8 ) * n * m);   // (2 * 160 * 480)
	vpd->icon_key= (OBJATTR*)malloc(sizeof(OBJATTR) * 8);	// 左のアイコン	
	// 先頭アドレスをセット
	for (int i = 1; i < m_mem ; i++) {
		vpd->mem[i]  = vpd->mem[i-1]  + sizeof(u8) * n_mem;
	}
	for (int i = 1; i < m ; i++) {
		vpd->vram[i] = vpd->vram[i-1] + sizeof(u8) * n;
	}
	dprintf("vram : %d\n", sizeof(u8 ) * n * m);
}


// test
// obj描画Test
void testobj(VISUAL_PLAY* vpd){
	// char
	dmaCopy((u16*)chr003Tiles, BITMAP_OBJ_BASE_ADR, chr003TilesLen);
	dmaCopy((u16*)chr003Pal,   OBJ_COLORS,          chr003PalLen);

	//vpd->icon_key[0].attr0 = 
	vpd->icon_key[0].attr0 = OBJ_Y(16) | OBJ_16_COLOR;
	vpd->icon_key[0].attr1 = OBJ_X(19);
	// vpd->icon_key[0].attr2 = OBJ_CHAR(0);
	// ビットマップモードの場合、先頭番号が512番目からとのこと・・・
	vpd->icon_key[0].attr2 = OBJ_CHAR(513);

	vpd->icon_key[1].attr0 = OBJ_Y(32) | OBJ_16_COLOR;
	vpd->icon_key[1].attr1 = OBJ_X(55);
	vpd->icon_key[1].attr2 = OBJ_CHAR(512 + $U);
	
	OAM[0] = vpd->icon_key[0];
	OAM[1] = vpd->icon_key[1];
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
	// 黒で初期化
	memset(vpd->mem[f], 0x00, 128);

	// 音程に色をセット
	if (flag){
		vpd->mem[f][y] = 0x01;
	}
}

// 画面左に押しているキーを表示
// 9 枠 
void DrawKeyFrame(VISUAL_PLAY* vpd, unsigned int y, int flag){

}

// 画面左に押しているキーを表示
// 18 鍵盤 
void DrawKey(VISUAL_PLAY* vpd, unsigned int y, int flag){

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
	int n2 = vpd->frame + SCREEN_WIDTH;
	int note = 26;

	// セットした色を、実際の描画サイズ・向きに変換
	// i >> 3 して、棒を縦8 にしている
	for (int i = 0; i < SCREEN_HEIGHT; i++){
		vpd->vram[i][n ] = vpd->mem[n][(i >> 3) + note]; 
		vpd->vram[i][n2] = vpd->mem[n][(i >> 3) + note]; 
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
	SetMode(4 | BG2_ON | OBJ_ON);

	// Init private
	InitSoundPlay(&sp_data);
	halSetKeysSort(&b, 4, 5, 6, 7, 0, 1, 3, 2);

	// Init Video
	InitVisualPlay(&vp_data);
	//testvram(&vp_data);
	testobj(&vp_data);

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


