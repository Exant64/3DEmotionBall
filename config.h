#pragma once

enum {
	LoD_MaxHigh = 0,
	LoD_MaxMedium,
	LoD_ForceLow,
	LoD_ForceMedium,
	LoD_ForceHigh,

	LoD_Default = LoD_MaxHigh
};

enum {
	Specular_Enabled = 0,
	Specular_EnabledNoHalo,
	Specular_Disabled,

	Specular_Default = Specular_Enabled
};

struct CONFIG {
	int LoD;
	int Specular;

	bool UseNeutralChaosSprite;
	bool UseDarkChaosSprite;
	bool UseEmoteBallTypeSprite;
	bool RotateExpression;

	bool PurpleDarkChaosEmotion;
	bool YellowNeutChaosEmotion;
};

extern CONFIG ModConfig;

void Config_Init(const char* path);