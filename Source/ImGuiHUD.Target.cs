// Copyright 2017 Sebastien Ronsse

using UnrealBuildTool;
using System.Collections.Generic;

public class ImGuiHUDTarget : TargetRules
{
	public ImGuiHUDTarget(TargetInfo Target)
	{
		Type = TargetType.Game;
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
