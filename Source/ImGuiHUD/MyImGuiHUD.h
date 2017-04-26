// Copyright 2017 Sebastien Ronsse

#pragma once

#include "GameFramework/HUD.h"
#include "imgui/imgui.h"
#include "MyImGuiHUD.generated.h"

/**
 * 
 */
UCLASS()
class IMGUIHUD_API AMyImGuiHUD : public AHUD
{
	GENERATED_BODY()

	AMyImGuiHUD();
	
	virtual void PostInitializeComponents() override;
	virtual void BeginDestroy() override;
	virtual void DrawHUD() override;

	bool ImGui_ImplUE_Init();
	void ImGui_ImplUE_Shutdown();
	bool ImGui_ImplUE_CreateDeviceObjects();
	void ImGui_ImplUE_InvalidateDeviceObjects();
	void ImGui_ImplUE_ProcessEvent();
	void ImGui_ImplUE_NewFrame();

	static void ImGui_ImplUE_RenderDrawLists(ImDrawData *draw_data);
	static const char *ImGui_ImplUE_GetClipboardText();
	static void ImGui_ImplUE_SetClipboardText(const char *text);

	UPROPERTY()
	UTexture2D *FontTexture;

	UPROPERTY()
	UMaterialInterface *MasterMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic *MaterialInstance;
};
