
#include <gba.h>
#include <stdio.h>
#include <stdlib.h>

#include "hal_eightinput.h"
#include "sound_play.h"

// 映像用データ
typedef struct _VP_LINE_ {
	int		id;
	int		x, y;
	int		len;
	int		col;
	int		frame;
} VP_LINE, *PVP_LINE;

typedef struct _VP_COLOR_ {
	int		main;
	int		color;
	int		gray1;
	int		gray2;
} VP_COLOR, *PVP_COLOR;

typedef struct _VISUAL_PLAY_ {
	int		frame;
	VP_LINE	lines[128];
	unsigned short dots[128];
	unsigned short mem[256][128];
	unsigned short vram[256][512];
} VISUAL_PLAY, *PVISUAL_PLAY;

// 0 - FFFF
void CreateLine(VISUAL_PLAY* vp_data, int id, int y) {
	vp_data->lines[id].id    = id;
	vp_data->lines[id].x     = 0;
	vp_data->lines[id].y     = y;
	vp_data->lines[id].len   = 16;
	vp_data->lines[id].col   = rand();

	vp_data->lines[id].frame = 0;
}

void MoveLine(VISUAL_PLAY* vp_data, int id) {
	vp_data->lines[id].frame++;
}

void StretchLine(VISUAL_PLAY* vp_data, int id){
	vp_data->lines[id].len = vp_data->lines[id].frame;
}

// 240 * 160
void DrawLine(VISUAL_PLAY* vp_data, int id) {
	int y = SCREEN_HEIGHT - vp_data->lines[id].y;
	int x = SCREEN_WIDTH  - vp_data->lines[id].frame;
	int ofs = VRAM + (y * 240 + x) * 2; // 16bit なので 掛ける2
	int len = vp_data->lines[id].len;
	int col = vp_data->lines[id].col;

	for (int i = 0; i < len; i++) {
		*(u16 *)(ofs + i * 2) = col;
	}
}

// 方法その２ 色で制御
// id でなく、128固定
void DrawLines(VISUAL_PLAY* vp_data, int y, int flag){
	int f = vp_data->frame & 0xff;
	y &= 0x7f;
	dmaCopy((u16*)0, (u16*)vp_data->mem[f], 128);
	if(flag){
		vp_data->mem[f][y] = rand();
	}
	//dprintf("%04X\n", (u16)vp_data->mem[f][y]);
}

void ConvertMem(VISUAL_PLAY* vp_data){
	int f  = (vp_data->frame) & 0xff;
	int f2 = (f + 0x100) & 0xff;
	for(int i = 0; i < 128; i++){
		vp_data->vram[i][f + 32] = vp_data->mem[f][i];
		vp_data->vram[i][f2] = vp_data->mem[f][i];
		//dprintf("%08X", &vp_data->vram[i][f]);
	}
}

void FlushVram(VISUAL_PLAY* vp_data) {
	int f = vp_data->frame & 0x1FF;
	int d = VRAM;
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		dmaCopy((u16*)&vp_data->vram[i][f], (u16 *)d, SCREEN_WIDTH);
		d += SCREEN_WIDTH << 1;
	}
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	SOUND_PLAY   sp_data;
	VISUAL_PLAY  vp_data;
	BUTTON_INFO  b;

	int			 count;

	// Init
	irqInit();
	irqEnable(IRQ_VBLANK);
	// consoleDemoInit();
	SetMode(3 | BG2_ON);

	// Init private
	InitSoundPlay(&sp_data);
	halSetKeysSort(&b, 4, 5, 6, 7, 0, 1, 3, 2);
	count = 0;

	// Init Video
	//InitVisualPlay(&vp_data)

	// Sound Init
	REG_SOUNDCNT_X = 0x80;		// turn on sound circuit
	REG_SOUNDCNT_L = 0x1177;	// full volume, enable sound 1 to left and right
	REG_SOUNDCNT_H = 2;			// Overall output ratio - Full

	while (1) {
		scanKeys();
		halSetKeys(&b, keysHeld());

		// 音処理
		SoundPlay(&sp_data, &b);

		// 表示
		if (halIsAB(&b)) {
			CreateLine(&vp_data, 0, sp_data.note);
			count = (count + 1) & 0xff;
		}
		if (halIsAB_hold(&b)) {
			StretchLine(&vp_data, 0);
		}
		MoveLine(&vp_data, 0);
		//DrawLine(&vp_data, 0);
		DrawLines(&vp_data, sp_data.note, halIsAB_hold(&b));
		ConvertMem(&vp_data);
		FlushVram(&vp_data);

		// 垂直同期
		VBlankIntrWait();
	}
}


