//---------------------------------------------------------------------------------
#ifndef _graphic_mode0_h_
#define _graphic_mode0_h_
//---------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------------

typedef struct _GRAPHIC_MODE0_ {
	unsigned short*** vram;		// 描画バッファ1
} GRAPHIC_MODE0, *PGRAPHIC_MODE0;

extern void InitMode0(GRAPHIC_MODE0* gmd);
extern void FinishMode0(GRAPHIC_MODE0* gmd);
extern void FlushBG(GRAPHIC_MODE0* gmd);
extern void testvram(GRAPHIC_MODE0* gmd);


//---------------------------------------------------------------------------------
#ifdef __cplusplus
	}	   // extern "C"
#endif
#endif