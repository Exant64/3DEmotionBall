#include "pch.h"

#include "SA2ModLoader.h"
#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"

#include "models.h"
#include "config.h"

NJS_TEXNAME AL_3DICON_TEXNAME[2];
NJS_TEXLIST AL_3DICON_TEXLIST = { AL_3DICON_TEXNAME, 2 };

DataPointer(int, nj_cnk_blend_mode, 0x025F0264);

FunctionHook<void> ChaoMain_Constructor_FuncHook(0x52AB60);
FunctionHook<void, task*> AL_IconDrawSub(0x53CEB0);
FunctionHook<void, char> sub_42CA20(0x42CA20);
UsercallFuncVoid(AL_CalcIconColor, (task* tp), (tp), 0x0053B940, rEAX);

UsercallFuncVoid(AL_LoadTex, (const char* pFileName, NJS_TEXLIST* texlist, Uint16 a1), (pFileName, texlist, a1), 0x530280, rEBX, stack4, rAX);
UsercallFuncVoid(SetChunkTexIndexPrimary, (int index, int a2, int a3), (index, a2, a3), 0x56E3D0, rEAX, rEBX, stack4);
UsercallFuncVoid(SetChunkTextureID, (NJS_CNK_MODEL* a1, __int16 a2), (a1, a2), 0x0055EA00, rECX, rDI);
UsercallFuncVoid(njQuaternionEx, (NJS_QUATERNION* pQuat), (pQuat), 0x0784B50, rEAX);
UsercallFuncVoid(njDrawTexture3DExSetData, (void* a1, int vertexCount), (a1, vertexCount), 0x00781370, rEAX, rECX);
UsercallFuncVoid(DoLighting, (int a1), (a1), 0x00487060, rEAX);
UsercallFunc(bool, AL_IsDark, (task* tp), (tp), 0x00535390, rEAX, rEAX);
UsercallFunc(bool, AL_IsHero, (task* tp), (tp), 0x00535360, rEAX, rEAX);

void ChaoMain_Constructor_TexLoadHook() {
	ChaoMain_Constructor_FuncHook.Original();

	AL_LoadTex.Original("AL_3DICON", &AL_3DICON_TEXLIST, 0);
}

static bool AlphaTestEnableHackFlag = false;
void sub_42CA20_Hook(char a1) {
	sub_42CA20.Original(a1);

	if (AlphaTestEnableHackFlag) {
		VoidFunc(AlphaTestEnable, 0x042C0A0);
		AlphaTestEnable();
	}
}

static float MaterialColor[3];
static float MaterialAlpha;
static bool UpperIconDisable;
static bool DisableSpecularRender = false;

static void OffConstantAttr(int _and, int _or) {
	nj_constant_attr_and_ &= ~_and;
	nj_constant_attr_or_ &= ~_or ;
}

// the heroChaosBlending argument enables usealpha so that DrawObject enables AlphaBlending
// however, this caused the outside of the halo to "blend" with the inside causing it to look ugly when looked at from the side
// so shaddatic helped me figure this out, we needed to reorder the mesh using materials, outside of the halo first, inside second
// then enable alpha testing because that reenables z write, that way the inside wouldnt get drawn from the side => no blending bug
// however they hardcoded some check to not enable alpha test if the DST blending is one, so we had to hook the function to check for this bool
// that we set here, so that its forced to be on.
static void DrawSpecularObject(NJS_OBJECT* obj, bool heroChaosBlending = false) {
	const int flags = NJD_FST_ENV | NJD_FST_UA | NJD_FST_IL;

	OffConstantAttr(0, flags);
	if (heroChaosBlending) {
		OnConstantAttr(0, NJD_FST_UA);
	}
	SetMaterial(MaterialAlpha, MaterialColor[0], MaterialColor[1], MaterialColor[2]);
	SetChunkTextureID(obj->chunkmodel, 0);
	AlphaTestEnableHackFlag = heroChaosBlending;
	DrawObject(obj);
	AlphaTestEnableHackFlag = false;

	OnConstantAttr(0, flags);
	SetMaterial(1, 1, 1, 1);
	SetChunkTextureID(obj->chunkmodel, 1);
	if(!DisableSpecularRender)
		DrawObject(obj);
}

double __fastcall njOuterProduct(const NJS_VECTOR* a1, const NJS_VECTOR* a2, NJS_VECTOR* a3)
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

static void QuaternionLookRotation(NJS_QUATERNION* quaternion, NJS_VECTOR* forward, const NJS_VECTOR* up)
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

#define PUNI_PHASE ((njSin(pIcon->PuniPhase) + 1.0) * 0.08f + 0.92f)

static void AL_IconAdjustHeldPosition(ChaoData1* cwk, NJS_VECTOR& pos) {
	if ((cwk->entity.Status & 0x8000u) == 0) {
		pos.y += 0.3f;
	}
	else {
		pos.y -= 0.7f;
	}
}

static bool AL_IconCanDrawUpperModel(task* tp) {
	ChaoData1* cwk = tp->Data1.Chao;
	auto type = cwk->ChaoDataBase_ptr->Type;

	if (type == ChaoType_Neutral_Chaos && ModConfig.UseNeutralChaosSprite) {
		return false;
	}

	if (type == ChaoType_Dark_Chaos && ModConfig.UseDarkChaosSprite) {
		return false;
	}
	
	// i'm intentionally using > 0 incase any other types are implemented in the future
	if(cwk->ChaoDataBase_ptr->BallType > 0 && ModConfig.UseEmoteBallTypeSprite) {
		return false;
	}

	return true;
}

static void AL_IconDrawLower(task* tp) {
	ChaoData1* cwk = tp->Data1.Chao;
	AL_ICON* pIcon = (AL_ICON*)&cwk->EmotionBallData;

	auto type = cwk->ChaoDataBase_ptr->Type;
	if (type == ChaoType_Neutral_Chaos ||
		type == ChaoType_Dark_Chaos ||
		cwk->ChaoDataBase_ptr->BallType == 1)
	{
		return;
	}

	const float puni_phase = PUNI_PHASE;
	const float sx = puni_phase * pIcon->Lower.Scl.x;
	const float sy = (2.0 - puni_phase) * pIcon->Lower.Scl.y;

	NJS_POINT3 lower_pos = pIcon->Lower.Pos;
	AL_IconAdjustHeldPosition(cwk, lower_pos);

	njPushMatrixEx();
	njTranslateEx(&lower_pos);

	if (AL_IsDark.Original(tp)) {
		njRotateY(0, cwk->entity.Rotation.y);
		njScale(0, sx, sy, sx);

		DrawSpecularObject(IconModels.pObjSpiky);
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
		const NJS_VECTOR up{ 0,1,0 };
		NJS_VECTOR forward;
		forward.x = m2.x - m1.x;
		forward.y = m2.y - m1.y;
		forward.z = m2.z - m1.z;

		QuaternionLookRotation(&quat, &forward, &up);
		njQuaternionEx.Original(&quat);
		njRotateX(_nj_current_matrix_ptr_, NJM_DEG_ANG(90));
		njScale(0, sx, sy, sx);

		DrawSpecularObject(IconModels.pObjHalo, cwk->ChaoDataBase_ptr->Type == ChaoType_Hero_Chaos);
	}
	else {
		njRotateY(0, cwk->entity.Rotation.y);
		njScale(0, sx, sy, sx);
		
		DrawSpecularObject(IconModels.pObjBall);
	}

	njPopMatrixEx();
}

static void AL_IconDrawUpper(task* tp) {
	ChaoData1* cwk = tp->Data1.Chao;
	AL_ICON* pIcon = (AL_ICON*)&cwk->EmotionBallData;
	
	// upper texnum 10 is "none" basically
	if (pIcon->Upper.TexNum == 10 || !AL_IconCanDrawUpperModel(tp)) {
		return;
	}

	const float puni_phase = (njSin(pIcon->PuniPhase) + 1.0) * 0.08f + 0.92f;
	float sx = puni_phase * pIcon->Upper.Scl.x;
	float sy = (2.0 - puni_phase) * pIcon->Upper.Scl.y;

	NJS_POINT3 upper_pos = pIcon->Upper.Pos;
	AL_IconAdjustHeldPosition(cwk, upper_pos);

	njPushMatrixEx();
	njTranslateEx(&upper_pos);
	njRotateY(0, cwk->entity.Rotation.y);

	njScale(0, sx, sy, sx);

	switch (pIcon->Upper.TexNum)
	{
	case 1: //exclamation mark
		njTranslate(0, 0, -0.3f, 0);
		DrawSpecularObject(IconModels.pObjExclamation);
		break;
	case 2: //question mark
		njTranslate(0, 0, -0.3f, 0);
		DrawSpecularObject(IconModels.pObjQuestion);
		break;
	case 3:
		// hack: the normals dont seem to scale properly with, well scaling
		// so the env map texture was zoomed out and repeated which is obviously ugly
		// so i do a pretty nasty fix for it, i scaled the heart up in blender (by 4 iirc?)
		// applied the scale so the normals get "baked" correctly
		// and then scale down, ofc this causes the opposite to happen, zoom in rather than zoom out
		// but its a thousand times better than what was happening before and not noticeable at all
		njScale(0, 0.25, 0.25, 0.25);
		njTranslate(0, 0, -0.6f, 0);
		DrawSpecularObject(IconModels.pObjHeart);
		break;
	case 4:
		njTranslate(0, 0, -0.6f, 0);
		DrawSpecularObject(IconModels.pObjSwirl);
		break;
	}

	njPopMatrixEx();
}

static void AL_IconDraw_Hook(task* tp) {
	ChaoData1* cwk = tp->Data1.Chao;
	AL_ICON* pIcon = (AL_ICON*)&cwk->EmotionBallData;

	// convert the hex color to float color, used above in DrawSpecularObject
	Uint8* pColor = (Uint8*)&pIcon->Color;
	MaterialAlpha = pColor[3] / 255.f;
	MaterialColor[0] = pColor[2] / 255.f;
	MaterialColor[1] = pColor[1] / 255.f;
	MaterialColor[2] = pColor[0] / 255.f;

	DoLighting(*(char*)0x01DE4664);

	njSetTexture(&AL_3DICON_TEXLIST);

	SaveControl3D();
	SaveConstantAttr();

	int backupblend = nj_cnk_blend_mode;
	nj_cnk_blend_mode = NJD_FBS_SA | NJD_FBD_ONE; // source alpha to one blending
	
	OnControl3D(NJD_CONTROL_3D_CNK_BLEND_MODE); // to make the above variable work
	OnControl3D(NJD_CONTROL_3D_CONSTANT_MATERIAL); // to be able to set the emotion ball color with SetMaterial
	OnControl3D(NJD_CONTROL_3D_CNK_CONSTANT_ATTR); // to be able to enable/disable env map and use alpha on the model

	AL_IconDrawLower(tp);
	AL_IconDrawUpper(tp);

	// restore states
	LoadControl3D();
	LoadConstantAttr();
	nj_cnk_blend_mode = backupblend;

	auto type = cwk->ChaoDataBase_ptr->Type;
	if (type == ChaoType_Neutral_Chaos || 
		type == ChaoType_Dark_Chaos || 
		cwk->ChaoDataBase_ptr->BallType == 1) 
	{
		UpperIconDisable = AL_IconCanDrawUpperModel(tp);
		AL_IconDrawSub.Original(tp);
		UpperIconDisable = false;
	}
}

static void UpperIconDrawCheck(void *a1, int vertexCount) {
	if (UpperIconDisable) return;

	njDrawTexture3DExSetData.Original(a1, vertexCount);
}

static void __declspec(naked) UpperIconDrawHook() {
	__asm
	{
		push ecx // int vertexCount
		push eax // a1

		// Call your __cdecl function here:
		call UpperIconDrawCheck

		pop eax // a1
		pop ecx // int vertexCount
		retn
	}
}


extern "C" __declspec(dllexport) void OnInput() {
	// debug
	if (ControllerPointers[0]->press & Buttons_Y) {
		DisableSpecularRender = !DisableSpecularRender;
	}
}

static void AL_CalcIconColor_Hook(task* tp) {
	AL_CalcIconColor.Original(tp);

	const int neut_chaos_color = 0xFFFFFF00;
	const int dark_chaos_color = 0xFFA020F0;

	ChaoData1* cwk = tp->Data1.Chao;
	AL_ICON* pIcon = (AL_ICON*) & cwk->EmotionBallData;

	switch (cwk->ChaoDataBase_ptr->Type) {
	case ChaoType_Neutral_Chaos:
		if (ModConfig.YellowNeutChaosEmotion) {
			pIcon->Color = neut_chaos_color;
		}

		break;
	case ChaoType_Dark_Chaos:
		if (ModConfig.PurpleDarkChaosEmotion) {
			pIcon->Color = dark_chaos_color;
		}

		break;
	}
}

extern "C" __declspec(dllexport) void Init(const char* path, HelperFunctions & helper) {
	Model_Init(helper);
	Config_Init(path);

	ChaoMain_Constructor_FuncHook.Hook(ChaoMain_Constructor_TexLoadHook);
	AL_IconDrawSub.Hook(AL_IconDraw_Hook);
	sub_42CA20.Hook(sub_42CA20_Hook); // alpha hack
	AL_CalcIconColor.Hook(AL_CalcIconColor_Hook); // "extras" config emotion ball coloring options

	// i wanted to use the usercallfunc trampoline stuff where i can, but i don't know how to apply it to this so i had to writecall
	WriteCall((void*)0x0053D19A, UpperIconDrawHook);
}

extern "C" __declspec (dllexport) ModInfo SA2ModInfo = { ModLoaderVer };