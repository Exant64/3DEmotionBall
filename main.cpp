#include "pch.h"

#include "SA2ModLoader.h"
#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"

#include "data/al_icon_ball.nja"
#include "data/al_icon_spike.nja"

#include "data/al_icon_exclamation.nja"
#include "data/al_icon_question.nja"
#include "data/al_icon_swirl.nja"
#include "data/al_icon_heart.nja"

NJS_TEXNAME AL_3DICON_TEXNAME[2];
NJS_TEXLIST AL_3DICON_TEXLIST = { AL_3DICON_TEXNAME, 2 };

enum eAL_EYE_TEXNUM
{
	AL_EYE_TEXID_NORMAL = 0x0,
	AL_EYE_TEXID_KYA = 0x1,
	AL_EYE_TEXID_NAMU = 0x2,
	AL_EYE_TEXID_TOHOHO = 0x3,
	AL_EYE_TEXID_NIKO = 0x4,
	AL_EYE_TEXID_BIKKURI = 0x5,
	AL_EYE_TEXID_GURUGURU = 0x6,
	AL_EYE_TEXID_SUYASUYA = 0x7,
	AL_EYE_TEXID_DARK = 0x8, 
	AL_EYE_TEXID_HERO = 0x9,
	AL_EYE_TEXID_NCHAOS = 0xA,
	AL_EYE_TEXID_HCHAOS = 0xB,
	AL_EYE_TEXID_DCHAOS = 0xC,
};


DataPointer(int, nj_cnk_blend_mode, 0x025F0264);

FunctionHook<void> ChaoMain_Constructor_FuncHook(0x52AB60);
FunctionHook<void, task*> AL_IconDrawSub(0x53CEB0);

UsercallFuncVoid(AL_LoadTex, (const char* pFileName, NJS_TEXLIST* texlist, Uint16 a1), (pFileName, texlist, a1), 0x530280, rEBX, stack4, rAX);
UsercallFuncVoid(SetChunkTexIndexPrimary, (int index, int a2, int a3), (index, a2, a3), 0x56E3D0, rEAX, rEBX, stack4);
UsercallFuncVoid(SetChunkTextureID, (NJS_CNK_MODEL* a1, __int16 a2), (a1, a2), 0x0055EA00, rECX, rDI);
UsercallFunc(bool, AL_IsDark, (task* tp), (tp), 0x00535390, rEAX, rEAX);

void ChaoMain_Constructor_TexLoadHook() {
	ChaoMain_Constructor_FuncHook.Original();

	AL_LoadTex.Original("AL_3DICON", &AL_3DICON_TEXLIST, 0);
}

static float MaterialColor[3];
static bool DisableSpecularRender = false;

static void OffConstantAttr(int _and, int _or) {
	nj_constant_attr_and_ &= ~_and;
	nj_constant_attr_or_ &= ~_or ;
}

static void DrawSpecularObject(NJS_OBJECT* obj) {
	const int flags = NJD_FST_ENV | NJD_FST_UA | NJD_FST_IL;

	OffConstantAttr(0, flags);
	SetMaterial(1, MaterialColor[0], MaterialColor[1], MaterialColor[2]);
	SetChunkTextureID(obj->chunkmodel, 0);
	DrawObject(obj);

	OnConstantAttr(0, flags);
	SetMaterial(1, 1, 1, 1);
	SetChunkTextureID(obj->chunkmodel, 1);
	if(!DisableSpecularRender)
		DrawObject(obj);
}

void AL_IconDraw_Hook(task* tp) {
	ChaoData1* cwk = tp->Data1.Chao;
	AL_ICON* pIcon = (AL_ICON*)&cwk->EmotionBallData;

	NJS_POINT3 lower_pos = pIcon->Lower.Pos;
	NJS_POINT3 upper_pos = pIcon->Upper.Pos;

	float puni_phase = (njSin(pIcon->PuniPhase) + 1.0) * 0.08f + 0.92f;
	float sx = puni_phase * pIcon->Lower.Scl.x;
	float sy = (2.0 - puni_phase) * pIcon->Lower.Scl.y;

	// i think this is for if the chao is picked up?
	if ((cwk->entity.Status & 0x8000u) == 0) {
		lower_pos.y += 0.3f;
		upper_pos.y += 0.3f;
	}
	else {
		lower_pos.y -= 0.7f;
		upper_pos.y -= 0.7f;
	}

	// convert the hex color to float color, used above in DrawSpecularObject
	Uint8* pColor = (Uint8*)&pIcon->Color;
	MaterialColor[0] = pColor[2] / 255.f;
	MaterialColor[1] = pColor[1] / 255.f;
	MaterialColor[2] = pColor[0] / 255.f;

	njSetTexture(&AL_3DICON_TEXLIST);

	SaveControl3D();
	SaveConstantAttr();

	int backupblend = nj_cnk_blend_mode;
	nj_cnk_blend_mode = NJD_FBS_SA | NJD_FBD_ONE; // source alpha to one blending
	
	OnControl3D(NJD_CONTROL_3D_CNK_BLEND_MODE); // to make the above variable work
	OnControl3D(NJD_CONTROL_3D_CONSTANT_MATERIAL); // to be able to set the emotion ball color with SetMaterial
	OnControl3D(NJD_CONTROL_3D_CNK_CONSTANT_ATTR); // to be able to enable/disable env map and use alpha on the model
	
	njPushMatrixEx();
	njTranslateEx(&lower_pos);
	njScale(0, sx, sy, sx);

	if(AL_IsDark.Original(tp))
		DrawSpecularObject(&object_al_icon_spiky);
	else 
		DrawSpecularObject(&object_al_icon_ball);

	njPopMatrixEx();
	
	if (pIcon->Upper.TexNum != 10) {
		float sx = puni_phase * pIcon->Upper.Scl.x;
		float sy = (2.0 - puni_phase) * pIcon->Upper.Scl.y;

		njPushMatrixEx();
		njTranslateEx(&upper_pos);
		njTranslate(0, 0, -0.2f, 0);
		njScale(0, sx, sy, sx);
		
		switch (pIcon->Upper.TexNum)
		{
		case 1: //exclamation mark
			njTranslate(0, 0, -0.3f, 0);
			DrawSpecularObject(&object_al_icon_exclamation);
			break;
		case 2: //question mark
			njTranslate(0, 0, -0.3f, 0);
			DrawSpecularObject(&object_al_icon_question);
			break;
		case 3:
			njTranslate(0, 0, -0.6f, 0);
			DrawSpecularObject(&object_al_icon_heart);
			break;
		case 4:
			njTranslate(0, 0, -0.6f, 0);
			DrawSpecularObject(&object_al_icon_swirl);
			break;
		}

		njPopMatrixEx();
	}

	// restore states
	LoadControl3D();
	LoadConstantAttr();
	nj_cnk_blend_mode = backupblend;

	// todo!! only draw this if its the fire emoteball or something?
	// AL_IconDrawSub.Original(tp);
}

extern "C" __declspec(dllexport) void OnInput() {
	// debug
	if (ControllerPointers[0]->press & Buttons_Y) {
		DisableSpecularRender = !DisableSpecularRender;
	}
}

extern "C" __declspec(dllexport) void Init(const char* path) {
	ChaoMain_Constructor_FuncHook.Hook(ChaoMain_Constructor_TexLoadHook);
	AL_IconDrawSub.Hook(AL_IconDraw_Hook);

	// todo: check if this is necessary at all
	Sint8 display_function_to_use = 0x1C;
	WriteData((char*)0x0055031B, (char)display_function_to_use);
	WriteData((char*)0x00550324, (char)display_function_to_use);
}

extern "C" __declspec (dllexport) ModInfo SA2ModInfo = { ModLoaderVer };