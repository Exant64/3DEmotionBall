#include "pch.h"

#include "SA2ModLoader.h"
#include "FunctionHook.h"

FunctionHook<void, task*> AL_IconDrawSub(0x53CEB0);

void AL_IconDraw_Hook(task* tp) {
	ChaoData1* cwk = tp->Data1.Chao;
	AL_ICON* pIcon = (AL_ICON*)&cwk->EmotionBallData;

	NJS_POINT3 lower_pos = pIcon->Lower.Pos;
	njPushMatrixEx();
	//DrawObject(&);
	njPopMatrixEx();
	
	// AL_IconDrawSub.Original(tp);
}

extern "C" __declspec(dllexport) void Init(const char* path) {
	AL_IconDrawSub.Hook(AL_IconDraw_Hook);
}

extern "C" __declspec (dllexport) ModInfo SA2ModInfo = { ModLoaderVer };