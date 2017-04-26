// Copyright 2017 Sebastien Ronsse

#include "ImGuiHUD.h"
#include "ImGuiHUDGameModeBase.h"
#include "MyImGuiHUD.h"

AImGuiHUDGameModeBase::AImGuiHUDGameModeBase() :
	Super()
{
	HUDClass = AMyImGuiHUD::StaticClass();
}
