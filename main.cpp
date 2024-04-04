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
#include "data/al_icon_halo.nja"

#include "sa2-mod-loader/libmodutils/ModelInfo.cpp"

NJS_TEXNAME AL_3DICON_TEXNAME[2];
NJS_TEXLIST AL_3DICON_TEXLIST = { AL_3DICON_TEXNAME, 2 };

DataPointer(int, nj_cnk_blend_mode, 0x025F0264);

FunctionHook<void> ChaoMain_Constructor_FuncHook(0x52AB60);
FunctionHook<void, task*> AL_IconDrawSub(0x53CEB0);

UsercallFuncVoid(AL_LoadTex, (const char* pFileName, NJS_TEXLIST* texlist, Uint16 a1), (pFileName, texlist, a1), 0x530280, rEBX, stack4, rAX);
UsercallFuncVoid(SetChunkTexIndexPrimary, (int index, int a2, int a3), (index, a2, a3), 0x56E3D0, rEAX, rEBX, stack4);
UsercallFuncVoid(SetChunkTextureID, (NJS_CNK_MODEL* a1, __int16 a2), (a1, a2), 0x0055EA00, rECX, rDI);
UsercallFuncVoid(njQuaternionEx, (NJS_QUATERNION* pQuat), (pQuat), 0x0784B50, rEAX);
UsercallFunc(bool, AL_IsDark, (task* tp), (tp), 0x00535390, rEAX, rEAX);
UsercallFunc(bool, AL_IsHero, (task* tp), (tp), 0x00535360, rEAX, rEAX);

void ChaoMain_Constructor_TexLoadHook() {
	ChaoMain_Constructor_FuncHook.Original();

	AL_LoadTex.Original("AL_3DICON", &AL_3DICON_TEXLIST, 0);
}

static float MaterialColor[3];
static float MaterialAlpha;
static bool DisableSpecularRender = false;

static void OffConstantAttr(int _and, int _or) {
	nj_constant_attr_and_ &= ~_and;
	nj_constant_attr_or_ &= ~_or ;
}

static void DrawSpecularObject(NJS_OBJECT* obj) {
	const int flags = NJD_FST_ENV | NJD_FST_UA | NJD_FST_IL;

	OffConstantAttr(0, flags);
	if (MaterialAlpha < 1) {
		OnConstantAttr(0, NJD_FST_UA);
	}
	SetMaterial(MaterialAlpha, MaterialColor[0], MaterialColor[1], MaterialColor[2]);
	SetChunkTextureID(obj->chunkmodel, 0);
	DrawObject(obj);

	OnConstantAttr(0, flags);
	SetMaterial(1, 1, 1, 1);
	SetChunkTextureID(obj->chunkmodel, 1);
	if(!DisableSpecularRender)
		DrawObject(obj);
}

double __fastcall njOuterProduct(NJS_VECTOR* a1, NJS_VECTOR* a2, NJS_VECTOR* a3)
{
	double v3; // st7
	double v4; // st6
	double v5; // st5
	float v6; // ST00_4

	v3 = a2->z * a1->y - a2->y * a1->z;
	v4 = a2->x * a1->z - a2->z * a1->x;
	v5 = a2->y * a1->x - a2->x * a1->y;
	a3->x = v3;
	a3->y = v4;
	a3->z = v5;
	v6 = v5 * v5 + v4 * v4 + v3 * v3;
	return sqrtf(v6);
}

static void QuaternionLookRotation(NJS_QUATERNION* quaternion, NJS_VECTOR* forward, NJS_VECTOR* up)
{
	NJS_VECTOR crossOut;
	njUnitVector(forward);
	NJS_VECTOR vector = *forward;
	njOuterProduct(up, &vector, &crossOut);
	njUnitVector(&crossOut);
	NJS_VECTOR vector2 = crossOut;
	NJS_VECTOR vector3;
	njOuterProduct(&vector, &vector2, &vector3);
	float m00 = vector2.x;
	float m01 = vector2.y;
	float m02 = vector2.z;
	float m10 = vector3.x;
	float m11 = vector3.y;
	float m12 = vector3.z;
	float m20 = vector.x;
	float m21 = vector.y;
	float m22 = vector.z;


	float num8 = (m00 + m11) + m22;
	//var quaternion = new Quaternion();
	if (num8 > 0)
	{
		float num = (float)sqrtf(num8 + 1);
		quaternion->re = num * 0.5f;
		num = 0.5f / num;
		quaternion->im[0] = (m12 - m21) * num;
		quaternion->im[1] = (m20 - m02) * num;
		quaternion->im[2] = (m01 - m10) * num;
		return;
	}
	if ((m00 >= m11) && (m00 >= m22))
	{
		float num7 = (float)sqrtf(((1 + m00) - m11) - m22);
		float num4 = 0.5f / num7;
		quaternion->im[0] = 0.5f * num7;
		quaternion->im[1] = (m01 + m10) * num4;
		quaternion->im[2] = (m02 + m20) * num4;
		quaternion->re = (m12 - m21) * num4;
		return;
	}
	if (m11 > m22)
	{
		float num6 = (float)sqrtf(((1 + m11) - m00) - m22);
		float num3 = 0.5f / num6;
		quaternion->im[0] = (m10 + m01) * num3;
		quaternion->im[1] = 0.5f * num6;
		quaternion->im[2] = (m21 + m12) * num3;
		quaternion->re = (m20 - m02) * num3;
		return;
	}
	float num5 = (float)sqrtf(((1 + m22) - m00) - m11);
	float num2 = 0.5f / num5;
	quaternion->im[0] = (m20 + m02) * num2;
	quaternion->im[1] = (m21 + m12) * num2;
	quaternion->im[2] = 0.5f * num5;
	quaternion->re = (m01 - m10) * num2;

}

static void AL_IconDraw_Hook(task* tp) {
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
	MaterialAlpha = pColor[3] / 255.f;
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

	if (AL_IsDark.Original(tp)) {
		njRotateY(0, cwk->entity.Rotation.y);
		njScale(0, sx, sy, sx);

		DrawSpecularObject(&object_al_icon_spiky);
	}
	else if (AL_IsHero.Original(tp)) {
		NJS_POINT3 m1, m2;
		m1 = pIcon->Pos;
		m2 = cwk->HeadTranslationPos;
		
		// in AL_GetShadowPos, they translate up by 2 before getting the head pos
		// so we "undo that" by using the up vector (since they used njTranslate while being rotated to the head and all that)
		m2.x -= pIcon->Up.x * 2;
		m2.y -= pIcon->Up.y * 2;
		m2.z -= pIcon->Up.z * 2;

		NJS_QUATERNION quat;
		NJS_VECTOR up{ 0,1,0 };
		NJS_VECTOR forward;
		forward.x = m2.x - m1.x;
		forward.y = m2.y - m1.y;
		forward.z = m2.z - m1.z;
		
		QuaternionLookRotation(&quat, &forward, &up);
		njQuaternionEx.Original(&quat);
		njRotateX(_nj_current_matrix_ptr_, NJM_DEG_ANG(90));
		njScale(0, sx, sy, sx);

		DrawSpecularObject(&object_al_icon_halo);
	}
	else {
		njRotateY(0, cwk->entity.Rotation.y);
		njScale(0, sx, sy, sx);

		DrawSpecularObject(&object_al_icon_ball);
	}

	njPopMatrixEx();
	
	if (pIcon->Upper.TexNum != 10) {
		float sx = puni_phase * pIcon->Upper.Scl.x;
		float sy = (2.0 - puni_phase) * pIcon->Upper.Scl.y;

		njPushMatrixEx();
		njTranslateEx(&upper_pos);
		njRotateY(0, cwk->entity.Rotation.y);

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

static void ReplaceEmotionBallModel(NJS_OBJECT** pObjToReplace, const char* path) {

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