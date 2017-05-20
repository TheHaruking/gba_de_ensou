#include "hal_eightinput.h"

/*
 *  1. A B SHIFT 4方向 の 計7 ボタン を 教えてやる
 
 *     04
 *     ↑
 * 01←  →02    (B:20)    (A:10)   
 *     ↓
 *     08    (ESC:80)  (CTR:40)
 */

#define KEY_X0		0x01
#define KEY_X1		0x02
#define KEY_Y0		0x04
#define KEY_Y1		0x08

#define BTN_A		0x10
#define BTN_B		0x20
#define BTN_CTR		0x40
#define BTN_ESC		0x80

#define PUSH_SHIFT
#define PUSH_I
#define PUSH_X

int test(){
	return 1;
}

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
	btn->b1 = btn->b0;
	btn->b0 = (   ((src >> btn->x0 ) & 1) 
	            | ((src >> btn->x1 ) & 1) << 1
			    | ((src >> btn->y0 ) & 1) << 2
			    | ((src >> btn->y1 ) & 1) << 3
			    | ((src >> btn->a  ) & 1) << 4
			    | ((src >> btn->b  ) & 1) << 5
			    | ((src >> btn->ctr) & 1) << 6
			    | ((src >> btn->esc) & 1) << 7
			  ) & 0xFF;
}

int halKeyToNum(BUTTON_INFO* b){
	return b->b0;
}