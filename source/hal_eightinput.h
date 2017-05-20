//---------------------------------------------------------------------------------
#ifndef _hal_eightinput_h_
#define _hal_eightinput_h_
//---------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------------
typedef struct _BUTTON_INFO_ {
	unsigned int b0;		// new
	unsigned int b1;		// old
	unsigned int x0, x1, y0, y1, a, b, ctr, esc; // sortinfo
} BUTTON_INFO, *PBUTTON_INFO;

extern int  test();
extern int  inc(int*);

extern void halSetKeysSort(BUTTON_INFO* btn, unsigned int x0, unsigned int x1, unsigned int y0, unsigned int y1, unsigned int a, unsigned int b, unsigned int ctr, unsigned int esc);
extern void halSetKeys(BUTTON_INFO* btn, int src);
extern int  halKeyToNum(BUTTON_INFO* btn);

#endif