#include "pch.h"

#include "SA2ModLoader.h"
#include "sa2-mod-loader/mod-loader-common/ModLoaderCommon/TextConv.cpp"
#include "sa2-mod-loader/mod-loader-common/ModLoaderCommon/IniFile.cpp"

#include "config.h"

CONFIG ModConfig;

void Config_Init(const char* path) {
	std::string inipath = path;
	inipath += "\\config.ini";

	IniFile config(inipath.c_str());

	ModConfig.LoD = config.getInt("General", "LoD", LoD_Default);
	ModConfig.Specular = config.getInt("General", "Specular", Specular_Default);

	ModConfig.UseNeutralChaosSprite = config.getBool("General", "UseNeutralChaosSprite", false);
	ModConfig.UseDarkChaosSprite = config.getBool("General", "UseDarkChaosSprite", false);
	ModConfig.UseEmoteBallTypeSprite = config.getBool("General", "UseEmoteBallTypeSprite", false);
	ModConfig.RotateExpression = config.getBool("General", "RotateExpression", true);
	ModConfig.PurpleDarkChaosEmotion = config.getBool("Extras", "PurpleDarkChaosEmotion", false);
	ModConfig.YellowNeutChaosEmotion = config.getBool("Extras", "YellowNeutChaosEmotion", false);
}