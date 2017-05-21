#include "hal_eightinput.h"

/*
 *  1. A B SHIFT 4方向 の 計7 ボタン を 教えてやる
 *     04
 *     ↑       
 * 02←  →01   (B  :20)    (A  :10)   
 *     ↓
 *     08   (ESC:80)  (CTR:40)
 */

#define KEY_X0		0x01
#define KEY_X1		0x02
#define KEY_Y0		0x04
#define KEY_Y1		0x08

#define BTN_A		0x10
#define BTN_B		0x20
#define BTN_CTR		0x40
#define BTN_ESC		0x80

#define KEY_XY		0x0F
#define BTN_AB		0x30

#define PUSH_I
#define PUSH_X
#define PUSH_HOLD
#define PUSH_SHIFT
#define PUSH_DOUBLE

#define MODE_NUM	0
#define MODE_ALPH	1
#define MODE_HIRA	2
#define MODE_SMBL	3
#define MODE_USER	4 

#define CENTER_NUM	4

const int num_8_tbl[16] = {
	// → ←
	-1, 2, 6,-1,
	 0, 1, 7,-1, // ↑
	 4, 3, 5,-1, // ↓
	-1,-1,-1,-1,
};

const int num_4_tbl[16] = {
	//  →  ←
	-1,  1,  3, -1,
	 0, -1, -1, -1, // ↑
	 2, -1, -1, -1, // ↓
	-1, -1, -1, -1,
};

const int num_4_8_tbl[4][8] = {
	{	4, 5, 6, 7, 0, 1, 2, 3, },
	{	2, 3, 4, 5, 6, 7, 0, 1, },
	{	0, 1, 2, 3, 4, 5, 6, 7, },
	{	6, 7, 0, 1, 2, 3, 4, 5, },
};

int inc(int* in){
	*in += 1;
	return *in;
}

void halSetKeysSort(BUTTON_INFO* btn, unsigned int x0, unsigned int x1, unsigned int y0, unsigned int y1, unsigned int a, unsigned int b, unsigned int ctr, unsigned int esc) {
	btn->x0  = x0;
	btn->x1  = x1;
	btn->y0  = y0;
	btn->y1  = y1;
	btn->a   = a;
	btn->b   = b;
	btn->ctr = ctr;
	btn->esc = esc;
}


/* ボタンビットを、共通化する。
 * x0, x1, y0, ... には、src の 何番目ビット を 目的のボタンに当てるか指定されている。
 */
void halSetKeys(BUTTON_INFO* btn, int src){
	btn->b_old = btn->b0;
	btn->b0 = (   ((src >> btn->x0 ) & 1) 
	            | ((src >> btn->x1 ) & 1) << 1
			    | ((src >> btn->y0 ) & 1) << 2
			    | ((src >> btn->y1 ) & 1) << 3
			    | ((src >> btn->a  ) & 1) << 4
			    | ((src >> btn->b  ) & 1) << 5
			    | ((src >> btn->ctr) & 1) << 6
			    | ((src >> btn->esc) & 1) << 7
			  ) & 0xFF;
	btn->b1 = (btn->b0 ^ btn->b_old) & btn->b0;
	btn->b2 = (btn->b0 ^ btn->b_old) & btn->b_old;

	// 方向キー単体の情報取得
	btn->k0 = btn->b0 & KEY_XY;
	btn->k1 = btn->b1 & KEY_XY;
	btn->k2 = btn->b2 & KEY_XY;
	btn->k_old = btn->b_old & KEY_XY;

	// 方向のベクトル化(0 ~ 7)
	btn->key_8 = num_8_tbl[btn->k0];
 
	// フラグリセット
	btn->f_key_flag = 0;
	// 最初に押した方向キーを保存。 (0 1 2 3。ななめ無視)
	if (!btn->k_old) {
		btn->f_key = num_4_tbl[btn->k1];
		// 押した瞬間フラグオン
		if (btn->f_key >= 0)
			btn->f_key_flag = 1;
	}
	// 離されていれば -1
	if (!btn->k0)
		btn->f_key = -1;

	// 最初に押した方向に対して、どの程度回したか
	if (btn->f_key >= 0) {
		btn->f_key_move = num_4_8_tbl[btn->f_key][btn->key_8];
	} else {
		btn->f_key_move = -1;
	}
}

int halKeyToNum(BUTTON_INFO* btn){
	int ab  = btn->b1 & BTN_AB;
	int key = btn->b0 & KEY_XY;
	int direction_8 = num_8_tbl[key];
	int ret;
	
	// AかBを押した瞬間でなければ-1
	if (!ab) 
		return -1;

	// 十字キー 変な方向(or 押していない)なら-1
	if (!direction_8 < 0) 
		return -1;

	switch (ab){
		case BTN_A: ab = 0; break;
		case BTN_B: ab = 1; break;
		default:    ab = 0; break; // 同時押し
	}
	
	// ↑ + A : 0
	// ↑ + B : 1
	// ↗ + A : 2 
	// ...
	// ← + B : 13
	// ↖ + A : 14
	// ↖ + B : 15
	ret = (direction_8 << 1) + ab; 

	return ret;
}

int halKeyAB(BUTTON_INFO* btn){
	int ab  = btn->b1 & BTN_AB;
	int key = btn->b0 & KEY_XY;
	int direction_8 = num_8_tbl[key];
	int ret;

	ret = ret;

	return ret;
}

// Ctrホールド + 十字キー [押したとき]
int halKeyCtr4(BUTTON_INFO* btn){
	int key = btn->b1 & KEY_XY;
	int direction_4 = num_4_tbl[key];
	int ret;

	// 十字キー 変な方向(or 押していない)なら-1
	if (direction_4 < 0) 
		return -1;

	// Ctr 押していなければ-1
	if (!(btn->b0 & BTN_CTR)) 
		return -1;
	
	ret = direction_4;
	return ret;
}

// Ctrホールド + 十字キー -> Ctr離したとき
int halKeyCtr8(BUTTON_INFO* btn){
	int key = btn->b0 & KEY_XY;
	int direction_8 = num_8_tbl[key];
	int ret;

	// Ctr 離した瞬間でなければ-1
	if (!(btn->b2 & BTN_CTR)) 
		return -1;
	
	// 十字キー 変な方向(or 押していない)なら-1
	if (!direction_8 < 0)
		return -1;

	ret = direction_8;
	return ret;
}

// Ctrホールド + 十字キー -> 十字キー離したとき
int halKeyCtr12(BUTTON_INFO* btn){
	int direction_8 = num_8_tbl[btn->k0];
	int ret;

	// stage 0
	if (btn->k12_stage == 0) {
		if (btn->f_key_flag) { 
			btn->k12_stage = 1;
		}
	}
	// stage 1
	if (btn->k12_stage == 1) {
		int direction_4_8 = num_4_8_tbl[btn->f_key][direction_8];
		if      (btn->f_key != direction_4_8 ){
			btn->k12_stage == 2;
		}
		else if (!btn->k0) {
			btn->k12_stage == 3;
		}
	}
	// stage 2
	if( btn->k12_stage == 2){
		int direction_4_8 = num_4_8_tbl[btn->f_key][direction_8];
		if      ( btn->f_key < direction_4_8 ){
			ret = 1;
		}
		else if ( btn->f_key > direction_4_8 ){
			ret = -1;
		}
		btn->k12_stage = 0;
	}
	// stage 3
	if( btn->k12_stage == 3){
		ret = 0;
		btn->k12_stage = 0;
	}
	return ret;
}

int test( BUTTON_INFO* btn ){
	return btn->f_key_move;
}