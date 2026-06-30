#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ActionSquadSaveGame.generated.h"

UCLASS()
class ACTIONSQUAD_API UActionSquadSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action Squad|Progress")
	bool bTutorialCompleted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action Squad|Progress")
	bool bAssaultUnlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action Squad|Progress")
	int32 TotalMissionsPlayed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action Squad|Progress")
	FString BestRank;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action Squad|Settings")
	float MasterVolume = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action Squad|Settings")
	float SFXVolume = 0.85f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture")
	FString GestureProfilePath = TEXT("ActionSquadGestures/ActionSquadGestureProfile.json");

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Action Squad|Missions")
	TArray<FName> UnlockedMissions;
};
