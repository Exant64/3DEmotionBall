#include "pch.h"

#include "SA2ModLoader.h"
#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"

#include "data/al_icon_ball.nja"

NJS_TEXNAME AL_3DICON_TEXNAME[2];
NJS_TEXLIST AL_3DICON_TEXLIST = { AL_3DICON_TEXNAME, 2 };

DataPointer(int, nj_cnk_blend_mode, 0x025F0264);

FunctionHook<void> ChaoMain_Constructor_FuncHook(0x52AB60);
FunctionHook<void, task*> AL_IconDrawSub(0x53CEB0);

UsercallFuncVoid(AL_LoadTex, (const char* pFileName, NJS_TEXLIST* texlist, Uint16 a1), (pFileName, texlist, a1), 0x530280, rEBX, stack4, rAX);
UsercallFuncVoid(SetChunkTexIndexPrimary, (int index, int a2, int a3), (index, a2, a3), 0x56E3D0, rEAX, rEBX, stack4);
UsercallFuncVoid(SetChunkTextureID, (NJS_CNK_MODEL* a1, __int16 a2), (a1, a2), 0x0055EA00, rECX, rDI);

void ChaoMain_Constructor_TexLoadHook() {
	ChaoMain_Constructor_FuncHook.Original();

	AL_LoadTex.Original("AL_3DICON", &AL_3DICON_TEXLIST, 0);
}

void AL_IconDraw_Hook(task* tp) {
	ChaoData1* cwk = tp->Data1.Chao;
	AL_ICON* pIcon = (AL_ICON*)&cwk->EmotionBallData;

	NJS_POINT3 lower_pos = pIcon->Lower.Pos;

	float puni_phase = (njSin(pIcon->PuniPhase) + 1.0) * 0.08f + 0.92f;
	float sx = puni_phase * pIcon->Lower.Scl.x;
	float sy = (2.0 - puni_phase) * pIcon->Lower.Scl.y;

	if ((cwk->entity.Status & 0x8000u) == 0) {
		lower_pos.y += 0.3f;
	}
	else {
		lower_pos.y -= 0.7f;
	}

	SaveControl3D();
	SaveConstantAttr();

	nj_cnk_blend_mode = NJD_FBS_SA | NJD_FBD_ONE;
	
	//njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
	//njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);

	OnControl3D(NJD_CONTROL_3D_CNK_BLEND_MODE);
	OnControl3D(NJD_CONTROL_3D_CONSTANT_MATERIAL);
	OnControl3D(NJD_CONTROL_3D_CNK_CONSTANT_ATTR);
	//OffControl3D(NJD_CONTROL_3D_CONSTANT_TEXTURE_MATERIAL);

	njPushMatrixEx();
	njTranslateEx(&lower_pos);
	njScale(0, sx, 1, sy);
	njSetTexture(&AL_3DICON_TEXLIST);
	//SetChunkTexIndexPrimary(0, 0, 0);
	
	SetChunkTextureID(&cnk_Mesh_159, 0);
	Uint8* pColor = (Uint8*)&pIcon->Color;
	SetMaterial(1, pColor[2] / 255.f, pColor[1] / 255.f, pColor[0] / 255.f);
	DrawObject(&object_al_icon_ball);

	SetMaterial(1, 1, 1, 1);
	SetChunkTextureID(&cnk_Mesh_159, 1);
	OnConstantAttr(0, NJD_FST_ENV | NJD_FST_UA);
	DrawObject(&object_al_icon_ball);

	njPopMatrixEx();
	
	LoadControl3D();
	LoadConstantAttr();
	nj_cnk_blend_mode = 0x2500;

	// AL_IconDrawSub.Original(tp);
}

extern "C" __declspec(dllexport) void Init(const char* path) {
	ChaoMain_Constructor_FuncHook.Hook(ChaoMain_Constructor_TexLoadHook);
	AL_IconDrawSub.Hook(AL_IconDraw_Hook);

	Sint8 display_function_to_use = 0x1C;
	WriteData((char*)0x0055031B, (char)display_function_to_use);
	WriteData((char*)0x00550324, (char)display_function_to_use);
}

extern "C" __declspec (dllexport) ModInfo SA2ModInfo = { ModLoaderVer };