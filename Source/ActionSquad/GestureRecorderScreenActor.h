#pragma once

#include "CoreMinimal.h"
#include "CommandGestureComponent.h"
#include "GameFramework/Actor.h"
#include "GestureRecorderScreenActor.generated.h"

class APlayerCameraManager;
class UTutorialCommandWidget;
class UWidgetComponent;

UCLASS(Blueprintable)
class ACTIONSQUAD_API AGestureRecorderScreenActor : public AActor
{
	GENERATED_BODY()

public:
	AGestureRecorderScreenActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture Recording")
	void SetStatusText(const FText& InStatusText);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture Recording")
	void SetRecordingPrompt(const FText& InTitleText, const FText& InInstructionText, const FText& InStatusText);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture Recording")
	void UpdatePoseText(const FFingerExtensionPose& Pose);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UWidgetComponent> RecorderWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Layout")
	FVector2D WidgetDrawSize = FVector2D(920.0f, 340.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Layout", meta = (ClampMin = "0.01"))
	float ScreenScale = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Layout", meta = (Units = "cm"))
	float FollowDistance = 58.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Layout", meta = (Units = "cm"))
	float VerticalOffset = -4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Layout")
	float LocationLerpSpeed = 7.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Layout")
	float RotationLerpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Layout")
	FRotator WidgetRotationCorrection = FRotator(0.0f, 0.0f, 180.0f);

private:
	void InitializeWidget();
	void ApplyScreenLayout();
	void ApplyContentToWidget();
	void UpdateRecorderStepFromTitle();
	void UpdateTransform(float DeltaSeconds);
	bool EnsureCamera();

	UPROPERTY()
	TObjectPtr<UTutorialCommandWidget> CachedWidget;

	UPROPERTY()
	TObjectPtr<APlayerCameraManager> PlayerCameraManager;

	UPROPERTY()
	FText TitleLineText;

	UPROPERTY()
	FText InstructionLineText;

	UPROPERTY()
	FText StatusLineText;

	UPROPERTY()
	FText PoseLineText;

	int32 CurrentStepIndicator = 0;
	bool bHasInitialTransformLock = false;
};
