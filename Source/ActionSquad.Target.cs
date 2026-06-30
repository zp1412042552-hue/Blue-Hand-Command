using UnrealBuildTool;
using System.Collections.Generic;

public class ActionSquadTarget : TargetRules
{
	public ActionSquadTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("ActionSquad");
	}
}
