//---------------------------------------------------------------------------------
#ifndef _graphic_mode4_h_
#define _graphic_mode4_h_
//---------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------------

typedef struct _GRAPHIC_MODE4_ {
	unsigned char** vram;		// 描画バッファ
} GRAPHIC_MODE4, *PGRAPHIC_MODE4;

extern void InitMode4(GRAPHIC_MODE4* gm4);
extern void FinishMode4(GRAPHIC_MODE4* gm4);
extern void FlushVram(GRAPHIC_MODE4* gm4);
extern void FlushVramOfs(GRAPHIC_MODE4* gm4, int x);
extern void testvram(GRAPHIC_MODE4* gm4);


//---------------------------------------------------------------------------------
#ifdef __cplusplus
	}	   // extern "C"
#endif
#endif