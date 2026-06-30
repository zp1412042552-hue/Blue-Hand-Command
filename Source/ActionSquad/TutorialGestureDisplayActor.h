#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialGestureDisplayActor.generated.h"

class UChildActorComponent;
class UStaticMesh;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FCommandGestureDisplay
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture Display")
	FString GestureName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture Display")
	TSoftClassPtr<AActor> GestureActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture Display")
	TSoftObjectPtr<UStaticMesh> GestureMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture Display")
	FTransform RelativeTransform = FTransform::Identity;
};

UCLASS(Blueprintable)
class ACTIONSQUAD_API ATutorialGestureDisplayActor : public AActor
{
	GENERATED_BODY()

public:
	ATutorialGestureDisplayActor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	void PlayGestureDemo(int32 GestureIndex);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	void StopAndHideGesture();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	void SetTargetDisplayWidth(float WidthCm);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UChildActorComponent> GestureActorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UStaticMeshComponent> GestureMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial")
	TArray<FCommandGestureDisplay> GestureDisplays;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float TargetWidthFractionOfScreen = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial")
	float TargetDisplayWidthCm = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial", meta = (Units = "deg"))
	float ModelForwardCorrectionYawDegrees = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial")
	FVector WidgetOffset = FVector(0.0f, 0.0f, -22.0f);

private:
	void ApplyGestureDisplay(const FCommandGestureDisplay& Display);
	void FitCurrentGestureToTargetWidth();
	void BuildDefaultGestureDisplays();
};
