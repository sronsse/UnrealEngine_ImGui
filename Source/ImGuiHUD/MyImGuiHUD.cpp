// Copyright 2017 Sebastien Ronsse

#include "ImGuiHUD.h"
#include "MyImGuiHUD.h"

const struct FKey *KeyMap[] =
{
	&EKeys::Tab,
	&EKeys::Left,
	&EKeys::Right,
	&EKeys::Up,
	&EKeys::Down,
	&EKeys::PageUp,
	&EKeys::PageDown,
	&EKeys::Home,
	&EKeys::End,
	&EKeys::Delete,
	&EKeys::BackSpace,
	&EKeys::Enter,
	&EKeys::Escape,
	&EKeys::A,
	&EKeys::C,
	&EKeys::V,
	&EKeys::X,
	&EKeys::Y,
	&EKeys::Z,
};

AMyImGuiHUD::AMyImGuiHUD() :
	Super(),
	FontTexture(NULL)
{
	// Get master material reference
	static ConstructorHelpers::FObjectFinder<UMaterial> mat(TEXT("Material'/Game/Material.Material'"));
	MasterMaterial = mat.Object;
}

void AMyImGuiHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Create dynamic material instance
	if (MasterMaterial)
		MaterialInstance = UMaterialInstanceDynamic::Create(MasterMaterial, NULL);

	// Setup ImGui binding
	ImGui_ImplUE_Init();
}

void AMyImGuiHUD::BeginDestroy()
{
	Super::BeginDestroy();

	// Cleanup
	ImGui_ImplUE_Shutdown();

	MasterMaterial = NULL;
	MaterialInstance = NULL;
}

void AMyImGuiHUD::DrawHUD()
{
	// Process events
	ImGui_ImplUE_ProcessEvent();

	// Prepare new frame
	ImGui_ImplUE_NewFrame();

	// Show ImGui test window
	ImGui::ShowTestWindow();

	// Rendering
	ImGui::Render();
}

bool AMyImGuiHUD::ImGui_ImplUE_Init()
{
	ImGuiIO &io = ImGui::GetIO();

	// Keyboard mapping
	for (int i = 0; i < ImGuiKey_COUNT; i++)
		io.KeyMap[i] = i;

	// Fill callbacks
	io.RenderDrawListsFn = ImGui_ImplUE_RenderDrawLists;
	io.SetClipboardTextFn = ImGui_ImplUE_SetClipboardText;
	io.GetClipboardTextFn = ImGui_ImplUE_GetClipboardText;
	io.UserData = this;

	// Enable mouse cursor rendering
	io.MouseDrawCursor = true;

	return true;
}

void AMyImGuiHUD::ImGui_ImplUE_Shutdown()
{
	ImGui_ImplUE_InvalidateDeviceObjects();
	ImGui::Shutdown();
}

bool AMyImGuiHUD::ImGui_ImplUE_CreateDeviceObjects()
{
	// Build texture atlas
	ImGuiIO &io = ImGui::GetIO();
	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	// Upload texture to graphics system
	FontTexture = UTexture2D::CreateTransient(width, height, PF_R8G8B8A8);
	FTexture2DMipMap &mip = FontTexture->PlatformData->Mips[0];
	void *data = mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(data, pixels, width * height * 4);
	mip.BulkData.Unlock();
	FontTexture->UpdateResource();

	// Store our identifier
	io.Fonts->TexID = (void *)FontTexture;

	return true;
}

void AMyImGuiHUD::ImGui_ImplUE_InvalidateDeviceObjects()
{
	FontTexture = NULL;
}

void AMyImGuiHUD::ImGui_ImplUE_ProcessEvent()
{
	ImGuiIO &io = ImGui::GetIO();

	// Fill KeysDown array
	APlayerController *pc = GetOwningPlayerController();
	for (int i = 0; i < ImGuiKey_COUNT; i++)
		io.KeysDown[i] = pc->IsInputKeyDown(*KeyMap[i]);

	// Fill special keys
	io.KeyShift = pc->IsInputKeyDown(EKeys::LeftShift) || pc->IsInputKeyDown(EKeys::RightShift);
	io.KeyCtrl = pc->IsInputKeyDown(EKeys::LeftControl) || pc->IsInputKeyDown(EKeys::RightControl);
	io.KeyAlt = pc->IsInputKeyDown(EKeys::LeftAlt) || pc->IsInputKeyDown(EKeys::RightAlt);
	io.KeySuper = false;

	// Parse all keys
	TArray<FKey> keys;
	EKeys::GetAllKeys(keys);
	for (int i = 0; i < keys.Num(); i++)
	{
		// Validate keyboard input
		bool keyboard = !keys[i].IsMouseButton() &&
			!keys[i].IsModifierKey() &&
			!keys[i].IsGamepadKey() &&
			!keys[i].IsFloatAxis() &&
			!keys[i].IsVectorAxis();
		if (!keyboard)
			continue;

		// Check if key was pressed
		if (pc->WasInputKeyJustPressed(keys[i]))
		{
			// Get codes
			const uint32 *key_code = NULL;
			const uint32 *char_code = NULL;
			FInputKeyManager::Get().GetCodesFromKey(keys[i], key_code, char_code);

			// Only consider key press if char code is valid
			if (char_code)
			{
				// Get lower case version of character
				int c = tolower((int)*char_code);

				// Set character to upper case if shift was pressed
				if (pc->IsInputKeyDown(EKeys::LeftShift) || pc->IsInputKeyDown(EKeys::RightShift))
					c = toupper(c);

				// Report character input to ImGui
				io.AddInputCharacter((ImWchar)c);
			}
		}
	}
}

void AMyImGuiHUD::ImGui_ImplUE_NewFrame()
{
	if (!FontTexture)
		ImGui_ImplUE_CreateDeviceObjects();

	ImGuiIO &io = ImGui::GetIO();

	// Setup display size
	io.DisplaySize = ImVec2((float)Canvas->SizeX, (float)Canvas->SizeY);
	io.DisplayFramebufferScale = ImVec2(1, 1);

	// Setup inputs
	APlayerController *pc = GetOwningPlayerController();
	pc->GetMousePosition(io.MousePos.x, io.MousePos.y);
	io.MouseDown[0] = pc->IsInputKeyDown(EKeys::LeftMouseButton);
	io.MouseDown[1] = pc->IsInputKeyDown(EKeys::RightMouseButton);
	io.MouseDown[2] = pc->IsInputKeyDown(EKeys::MiddleMouseButton);

	ImGui::NewFrame();
}

void AMyImGuiHUD::ImGui_ImplUE_RenderDrawLists(ImDrawData *draw_data)
{
	// Avoid rendering when minimized
	ImGuiIO &io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if ((fb_width == 0) || (fb_height == 0))
		return;

	// Scale coordinates for retina displays(screen coordinates != framebuffer coordinates)
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// Render command lists
	AMyImGuiHUD *hud = (AMyImGuiHUD *)io.UserData;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList *cmd_list = draw_data->CmdLists[n];
		const ImDrawVert *vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx *idx_buffer = cmd_list->IdxBuffer.Data;

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

			// Call user-defined callback and continue if needed
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
				continue;
			}

			// Bind texture
			hud->MaterialInstance->SetTextureParameterValue(FName("param"), (UTexture *)pcmd->TextureId);

			// Parse all render commands and build triangle list
			TArray<FCanvasUVTri> triangles;
			for (unsigned int elem = 0; elem < pcmd->ElemCount / 3; elem++)
			{
				// Get vertices using vertex and index buffers
				ImDrawVert v[] =
				{
					cmd_list->VtxBuffer[idx_buffer[elem * 3]],
					cmd_list->VtxBuffer[idx_buffer[elem * 3 + 1]],
					cmd_list->VtxBuffer[idx_buffer[elem * 3 + 2]]
				};

				// Get vertex colors as vectors
				ImVec4 col[] =
				{
					ImGui::ColorConvertU32ToFloat4(v[0].col),
					ImGui::ColorConvertU32ToFloat4(v[1].col),
					ImGui::ColorConvertU32ToFloat4(v[2].col)
				};

				// Compute min/max positions
				ImVec2 min_pos = v[0].pos;
				ImVec2 max_pos = v[0].pos;
				for (int i = 0; i < 3; i++)
				{
					if (v[i].pos.x < min_pos.x)
						min_pos.x = v[i].pos.x;
					if (v[i].pos.y < min_pos.y)
						min_pos.y = v[i].pos.y;
					if (v[i].pos.x > max_pos.x)
						max_pos.x = v[i].pos.x;
					if (v[i].pos.y > max_pos.y)
						max_pos.y = v[i].pos.y;
				}

				// Compute min/max UVs
				ImVec2 min_uv = v[0].uv;
				ImVec2 max_uv = v[0].uv;
				for (int i = 0; i < 3; i++)
				{
					if (v[i].uv.x < min_uv.x)
						min_uv.x = v[i].uv.x;
					if (v[i].uv.y < min_uv.y)
						min_uv.y = v[i].uv.y;
					if (v[i].uv.x > max_uv.x)
						max_uv.x = v[i].uv.x;
					if (v[i].uv.y > max_uv.y)
						max_uv.y = v[i].uv.y;
				}

				// Adapt vertex positions and UVs based on clipping parameters
				for (int i = 0; i < 3; i++)
				{
					if (v[i].pos.x < pcmd->ClipRect.x)
					{
						v[i].uv.x += (max_uv.x - v[i].uv.x) * (pcmd->ClipRect.x - v[i].pos.x) / (max_pos.x - v[i].pos.x);
						v[i].pos.x = pcmd->ClipRect.x;
					}
					else if (v[i].pos.x > pcmd->ClipRect.z)
					{
						v[i].uv.x -= (v[i].uv.x - min_uv.x) * (v[i].pos.x - pcmd->ClipRect.z) / (v[i].pos.x - min_pos.x);
						v[i].pos.x = pcmd->ClipRect.z;
					}
					if (v[i].pos.y < pcmd->ClipRect.y)
					{
						v[i].uv.y += (max_uv.y - v[i].uv.y) * (pcmd->ClipRect.y - v[i].pos.y) / (max_pos.y - v[i].pos.y);
						v[i].pos.y = pcmd->ClipRect.y;
					}
					else if (v[i].pos.y > pcmd->ClipRect.w)
					{
						v[i].uv.y -= (v[i].uv.y - min_uv.y) * (v[i].pos.y - pcmd->ClipRect.w) / (v[i].pos.y - min_pos.y);
						v[i].pos.y = pcmd->ClipRect.w;
					}
				}

				// Build and push triangle to list
				FCanvasUVTri triangle;
				triangle.V0_Pos = FVector2D(v[0].pos.x, v[0].pos.y);
				triangle.V1_Pos = FVector2D(v[1].pos.x, v[1].pos.y);
				triangle.V2_Pos = FVector2D(v[2].pos.x, v[2].pos.y);
				triangle.V0_UV = FVector2D(v[0].uv.x, v[0].uv.y);
				triangle.V1_UV = FVector2D(v[1].uv.x, v[1].uv.y);
				triangle.V2_UV = FVector2D(v[2].uv.x, v[2].uv.y);
				triangle.V0_Color = FLinearColor(col[0].x, col[0].y, col[0].z, col[0].w);
				triangle.V1_Color = FLinearColor(col[1].x, col[1].y, col[1].z, col[1].w);
				triangle.V2_Color = FLinearColor(col[2].x, col[2].y, col[2].z, col[2].w);
				triangles.Push(triangle);
			}

			// Draw triangles
			hud->Canvas->K2_DrawMaterialTriangle(hud->MaterialInstance, triangles);

			// Update index buffer pointer
			idx_buffer += pcmd->ElemCount;
		}
	}
}

const char *AMyImGuiHUD::ImGui_ImplUE_GetClipboardText()
{
	FString text;
	FWindowsPlatformMisc::ClipboardPaste(text);
	FTCHARToUTF8 result(*text);
	return result.Get();
}

void AMyImGuiHUD::ImGui_ImplUE_SetClipboardText(const char *text)
{
	int new_size = strlen(text) + 1;
	TCHAR *new_str = new TCHAR[new_size];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, new_str, new_size, text, _TRUNCATE);
	FWindowsPlatformMisc::ClipboardCopy(new_str);
	delete [] new_str;
}
