// Copyright 2017 Sebastien Ronsse

#include "ImGuiHUD.h"
#include "MyImGuiHUD.h"

void AMyImGuiHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Setup ImGui binding
	ImGui_ImplUE_Init();
}

void AMyImGuiHUD::BeginDestroy()
{
	Super::BeginDestroy();

	// Cleanup
	ImGui_ImplUE_Shutdown();
}

void AMyImGuiHUD::DrawHUD()
{
	// Process events
	ImGui_ImplUE_ProcessEvent();

	// Prepare new frame
	ImGui_ImplUE_NewFrame();

	// Rendering
	ImGui::Render();
}

bool AMyImGuiHUD::ImGui_ImplUE_Init()
{
	return true;
}

void AMyImGuiHUD::ImGui_ImplUE_Shutdown()
{
}

bool AMyImGuiHUD::ImGui_ImplUE_CreateDeviceObjects()
{
	return true;
}

void AMyImGuiHUD::ImGui_ImplUE_InvalidateDeviceObjects()
{
}

void AMyImGuiHUD::ImGui_ImplUE_ProcessEvent()
{
}

void AMyImGuiHUD::ImGui_ImplUE_NewFrame()
{
}
