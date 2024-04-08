#pragma once
struct CONFIG {
	bool UseNeutralChaosSprite;
	bool UseDarkChaosSprite;
	bool UseEmoteBallTypeSprite;
	bool RotateExpression;

	bool PurpleDarkChaosEmotion;
	bool YellowNeutChaosEmotion;
};

extern CONFIG ModConfig;

void Config_Init(const char* path);