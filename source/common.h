//---------------------------------------------------------------------------------
#ifndef _common_h_
#define _common_h_
//---------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------------

// 2次元配列を確保・開放
extern void** malloc_arr(void** adr, int size, int y, int x);
extern void   free_arr(void** adr);

//---------------------------------------------------------------------------------
#ifdef __cplusplus
	}	   // extern "C"
#endif
#endif