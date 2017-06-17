//---------------------------------------------------------------------------------
#ifndef _common_h_
#define _common_h_
//---------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------------
#include <gba.h>
#define SGN(n)			((n > 0) - (n < 0))

#define WININ_0(n)		((n & 0xFF00) >> 8)
#define WININ_1(n)		( n & 0xFF00)
#define WINOUT(n)		((n & 0xFF00) >> 8)
#define OBJWIN(n)		( n & 0xFF00)
#define WIN_RIGHT(n)	( n & 0xFF)
#define WIN_LEFT(n)		((n & 0xFF) << 8)
#define WIN_BOTTOM(n)	( n & 0xFF)
#define WIN_TOP(n)		((n & 0xFF) << 8)

// 2次元配列を確保・開放
extern void** malloc_arr(void** adr, int size, int y, int x);
extern void   free_arr(void** adr);

static inline void SetMode_add(int mode)	{REG_DISPCNT |= mode;}

extern void SetWIN0(int x, int y, int x2, int y2);
extern void SetWIN1(int x, int y, int x2, int y2);

//---------------------------------------------------------------------------------
#ifdef __cplusplus
	}	   // extern "C"
#endif
#endif