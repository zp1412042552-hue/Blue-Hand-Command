#pragma once

#include "CoreMinimal.h"
#include "ActionSquadTypes.generated.h"

UENUM(BlueprintType)
enum class EActionSquadGamePhase : uint8
{
	None UMETA(DisplayName = "None"),
	Tutorial UMETA(DisplayName = "Tutorial"),
	Home UMETA(DisplayName = "Home"),
	Assault UMETA(DisplayName = "Assault"),
	MissionResult UMETA(DisplayName = "Mission Result")
};

UENUM(BlueprintType)
enum class ECommandGesture : uint8
{
	None UMETA(DisplayName = "None"),
	SelectA UMETA(DisplayName = "Select Teammate A"),
	SelectB UMETA(DisplayName = "Select Teammate B"),
	Action UMETA(DisplayName = "Action"),
	Watch UMETA(DisplayName = "Watch"),
	Recall UMETA(DisplayName = "Recall")
};

UENUM(BlueprintType)
enum class ESelectedTeamTarget : uint8
{
	None UMETA(DisplayName = "None"),
	TeamA UMETA(DisplayName = "Team A"),
	TeamB UMETA(DisplayName = "Team B"),
	All UMETA(DisplayName = "All")
};

UENUM(BlueprintType)
enum class ETeamMemberRole : uint8
{
	TeamA UMETA(DisplayName = "Team A"),
	TeamB UMETA(DisplayName = "Team B"),
	Enemy UMETA(DisplayName = "Enemy")
};

UENUM(BlueprintType)
enum class ETeamMemberAnimState : uint8
{
	RelaxedIdle UMETA(DisplayName = "Relaxed Idle"),
	AlertIdle UMETA(DisplayName = "Alert Idle"),
	CrouchAlert UMETA(DisplayName = "Crouch Alert"),
	Walk UMETA(DisplayName = "Walk"),
	Run UMETA(DisplayName = "Run"),
	CrouchWalk UMETA(DisplayName = "Crouch Walk"),
	Fire UMETA(DisplayName = "Fire"),
	Reload UMETA(DisplayName = "Reload"),
	HitReact UMETA(DisplayName = "Hit React"),
	Death UMETA(DisplayName = "Death"),
	BreachKick UMETA(DisplayName = "Breach Kick")
};

USTRUCT(BlueprintType)
struct FActionSquadLevelRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Request")
	FName TargetLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Request")
	EActionSquadGamePhase TargetPhase = EActionSquadGamePhase::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Request")
	TMap<FName, FString> MetaData;
};
