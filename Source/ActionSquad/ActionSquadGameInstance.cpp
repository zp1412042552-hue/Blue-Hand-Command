#include "ActionSquadGameInstance.h"

#include "ActionSquad.h"
#include "ActionSquadSaveGame.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

void UActionSquadGameInstance::Init()
{
	Super::Init();
	ActionSquad::ApplyLowScalabilitySettings();
	UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::Disabled);
	LoadOrCreateSaveGame();
}

bool UActionSquadGameInstance::RequestLevelTransition(const FActionSquadLevelRequest& Request)
{
	if (Request.TargetLevelName.IsNone())
	{
		UE_LOG(LogActionSquad, Warning, TEXT("RequestLevelTransition failed: TargetLevelName is None"));
		return false;
	}

	SetGamePhase(Request.TargetPhase);
	UGameplayStatics::OpenLevel(this, Request.TargetLevelName);
	return true;
}

void UActionSquadGameInstance::GoToTutorial()
{
	FActionSquadLevelRequest Request;
	Request.TargetLevelName = TutorialLevelName;
	Request.TargetPhase = EActionSquadGamePhase::Tutorial;
	RequestLevelTransition(Request);
}

void UActionSquadGameInstance::GoToHome()
{
	FActionSquadLevelRequest Request;
	Request.TargetLevelName = HomeLevelName;
	Request.TargetPhase = EActionSquadGamePhase::Home;
	RequestLevelTransition(Request);
}

void UActionSquadGameInstance::StartMission(FName MissionMapName)
{
	FActionSquadLevelRequest Request;
	Request.TargetLevelName = MissionMapName.IsNone() ? DefaultMissionLevelName : MissionMapName;
	Request.TargetPhase = EActionSquadGamePhase::Assault;
	RequestLevelTransition(Request);
}

void UActionSquadGameInstance::GoToMissionResult(const FString& ResultsMessage)
{
	FActionSquadLevelRequest Request;
	Request.TargetLevelName = MissionResultLevelName;
	Request.TargetPhase = EActionSquadGamePhase::MissionResult;
	Request.MetaData.Add(TEXT("ResultsMessage"), ResultsMessage);
	RequestLevelTransition(Request);
}

void UActionSquadGameInstance::CompleteTutorial()
{
	UActionSquadSaveGame* Save = LoadOrCreateSaveGame();
	if (!Save)
	{
		return;
	}

	Save->bTutorialCompleted = true;
	Save->bAssaultUnlocked = true;
	SaveGame();
}

UActionSquadSaveGame* UActionSquadGameInstance::LoadOrCreateSaveGame()
{
	if (CurrentSaveGame)
	{
		return CurrentSaveGame;
	}

	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		CurrentSaveGame = Cast<UActionSquadSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
	}

	if (!CurrentSaveGame)
	{
		CurrentSaveGame = Cast<UActionSquadSaveGame>(UGameplayStatics::CreateSaveGameObject(UActionSquadSaveGame::StaticClass()));
		if (CurrentSaveGame && CurrentSaveGame->UnlockedMissions.Num() == 0)
		{
			CurrentSaveGame->UnlockedMissions.Add(TutorialLevelName);
		}
	}

	return CurrentSaveGame;
}

void UActionSquadGameInstance::SaveGame()
{
	if (CurrentSaveGame)
	{
		UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SaveSlotName, SaveUserIndex);
	}
}

void UActionSquadGameInstance::SetGamePhase(EActionSquadGamePhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		return;
	}

	CurrentPhase = NewPhase;
	OnGamePhaseChanged.Broadcast(CurrentPhase);
}
