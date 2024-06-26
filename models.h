#pragma once
#include "SA2ModLoader.h"

struct ICON_MODELS {
	NJS_OBJECT* pObjBall;
	NJS_OBJECT* pObjHaloHigh;
	NJS_OBJECT* pObjHaloMid;
	NJS_OBJECT* pObjHaloLow;
	NJS_OBJECT* pObjSpiky;
	NJS_OBJECT* pObjQuestion;
	NJS_OBJECT* pObjExclamation;
	NJS_OBJECT* pObjSwirl;
	NJS_OBJECT* pObjHeart;
};

extern ICON_MODELS IconModels;
void Model_Init(HelperFunctions& helper);