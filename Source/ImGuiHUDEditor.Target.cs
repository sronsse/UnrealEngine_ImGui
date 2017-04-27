// Copyright 2017 Sebastien Ronsse

using UnrealBuildTool;
using System.Collections.Generic;

public class ImGuiHUDEditorTarget : TargetRules
{
	public ImGuiHUDEditorTarget(TargetInfo Target)
	{
		Type = TargetType.Editor;
	}

	//
	// TargetRules interface.
	//

	public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
		OutExtraModuleNames.AddRange( new string[] { "ImGuiHUD" } );
	}
}
