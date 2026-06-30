#pragma once

#include "CoreMinimal.h"
#include "ActionSquadTypes.h"
#include "Engine/GameInstance.h"
#include "ActionSquadGameInstance.generated.h"

class UActionSquadSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionSquadGamePhaseChanged, EActionSquadGamePhase, NewPhase);

UCLASS()
class ACTIONSQUAD_API UActionSquadGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Levels")
	bool RequestLevelTransition(const FActionSquadLevelRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Levels")
	void GoToTutorial();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Levels")
	void GoToHome();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Levels")
	void StartMission(FName MissionMapName);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Levels")
	void GoToMissionResult(const FString& ResultsMessage);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	void CompleteTutorial();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Save")
	UActionSquadSaveGame* LoadOrCreateSaveGame();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Save")
	void SaveGame();

	UPROPERTY(BlueprintAssignable, Category = "Action Squad|Events")
	FOnActionSquadGamePhaseChanged OnGamePhaseChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|State")
	EActionSquadGamePhase CurrentPhase = EActionSquadGamePhase::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Save")
	TObjectPtr<UActionSquadSaveGame> CurrentSaveGame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Levels")
	FName TutorialLevelName = TEXT("Tutorial");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Levels")
	FName HomeLevelName = TEXT("Home");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Levels")
	FName DefaultMissionLevelName = TEXT("Assault");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Levels")
	FName MissionResultLevelName = TEXT("MissionResult");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Save")
	FString SaveSlotName = TEXT("ActionSquadSave");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Save")
	int32 SaveUserIndex = 0;

private:
	void SetGamePhase(EActionSquadGamePhase NewPhase);
};
