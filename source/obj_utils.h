//---------------------------------------------------------------------------------
#ifndef _obj_utils_h_
#define _obj_utils_h_
//---------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------------
#include <gba.h>

#define OBJ_MAX		128

typedef struct _OBJ_UTILS_ {
	OBJATTR*		obj;	// スプライトデータ128個分
} OBJ_UTILS, *POBJ_UTILS;

extern void InitObj(OBJ_UTILS* oud);
extern void FinishObj(OBJ_UTILS* oud);

extern void SetObjChr(OBJ_UTILS* oud, const void* tiles_adr, const void* pal_adr);
extern void objReg(OBJATTR** attrp, OBJ_UTILS* oud, int n);
extern void objMove(OBJATTR* attr, int x, int y);
extern void objchr(OBJATTR* attr, int chr);
extern void objattr(OBJATTR* attr, int h, int v);
extern void objdraw(OBJATTR* attr, int chr, int x, int y);
extern void obj2chr(OBJATTR* attr, int chr);
extern void obj2pal(OBJATTR* attr, int pal);
extern void obj2draw(OBJATTR* attr, int chr, int x, int y);
extern void obj4pal(OBJATTR* attr, int pal);
extern void obj4draw(OBJATTR* attr, int chr, int x, int y);
extern void objInit(OBJATTR* attr, int chr, int col256);
extern void FlushSprite(OBJ_UTILS* oud);

//---------------------------------------------------------------------------------
#ifdef __cplusplus
	}	   // extern "C"
#endif
#endif