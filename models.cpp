#include "pch.h"
#include "models.h"

#include "sa2-mod-loader/libmodutils/ModelInfo.cpp"

ICON_MODELS IconModels;

static HelperFunctions* pHelper;
static void LoadModel(NJS_OBJECT** pObj, const char* filename) {
	char path[MAX_PATH];

	sprintf_s(path, "./resource/gd_PC/IconModels/%s", filename);
	ModelInfo* pModelInfo = new ModelInfo(pHelper->GetReplaceablePath(path));
	*pObj = pModelInfo->getmodel();

	// yay memory leak lol
}

void Model_Init(HelperFunctions& helper) {
	pHelper = &helper;

	LoadModel(&IconModels.pObjBall, "object_al_icon_ball.sa2mdl");
	LoadModel(&IconModels.pObjHaloHigh, "object_al_icon_halo.sa2mdl");
	LoadModel(&IconModels.pObjHaloMid, "object_al_icon_halo_mid.sa2mdl");
	LoadModel(&IconModels.pObjHaloLow, "object_al_icon_halo_low.sa2mdl");
	LoadModel(&IconModels.pObjSpiky, "object_al_icon_spiky.sa2mdl");
	LoadModel(&IconModels.pObjQuestion, "object_al_icon_question.sa2mdl");
	LoadModel(&IconModels.pObjExclamation, "object_al_icon_exclamation.sa2mdl");
	LoadModel(&IconModels.pObjSwirl, "object_al_icon_swirl.sa2mdl");
	LoadModel(&IconModels.pObjHeart, "object_al_icon_heart.sa2mdl");
}