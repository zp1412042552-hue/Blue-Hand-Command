#pragma once

#include "CoreMinimal.h"
#include "CommandGestureComponent.h"
#include "GameFramework/SaveGame.h"
#include "HandPose.h"
#include "GesturePoseProfileSaveGame.generated.h"

UCLASS()
class ACTIONSQUAD_API UGesturePoseProfileSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static const FString SlotName;
	static constexpr int32 UserIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture")
	bool bHasSelectA = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture")
	bool bHasSelectB = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture")
	FFingerExtensionPose SelectAPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture")
	FFingerExtensionPose SelectBPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture")
	FString SelectAEncodedPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture")
	FString SelectBEncodedPose;
};
