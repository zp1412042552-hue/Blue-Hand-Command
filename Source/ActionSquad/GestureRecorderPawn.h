#pragma once

#include "CoreMinimal.h"
#include "CommandGestureComponent.h"
#include "GameFramework/Pawn.h"
#include "HandPose.h"
#include "GestureRecorderPawn.generated.h"

class AGestureRecorderScreenActor;
class UCameraComponent;
class UMotionControllerComponent;
class UOculusXRHandComponent;
class UCapsuleComponent;

UCLASS(Blueprintable)
class ACTIONSQUAD_API AGestureRecorderPawn : public APawn
{
	GENERATED_BODY()

public:
	AGestureRecorderPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture Recording")
	void SelectRecordSlotA();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture Recording")
	void SelectRecordSlotB();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture Recording")
	void SelectRecordSlotAction();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture Recording")
	void SelectRecordSlotProtectMe();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture Recording")
	void SelectRecordSlotFreeAttack();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture Recording")
	void StartRecordingSelectedSlot();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UCapsuleComponent> BodyCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<USceneComponent> VROrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UMotionControllerComponent> LeftHandTrackingRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UOculusXRHandComponent> LeftHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UMotionControllerComponent> RightHandTrackingRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UOculusXRHandComponent> RightHandMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UCommandGestureComponent> CommandGesture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture Recording")
	TSubclassOf<AGestureRecorderScreenActor> ScreenActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture Recording")
	FVector ScreenOffset = FVector(105.0f, 0.0f, 135.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Gesture Recording")
	TObjectPtr<AGestureRecorderScreenActor> ScreenActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture Recording")
	float CountdownSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture Recording")
	float SampleWindowSeconds = 1.25f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Gesture Recording")
	ECommandGesture SelectedRecordGesture = ECommandGesture::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Gesture Recording")
	bool bCountdownActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Gesture Recording")
	bool bSamplingActive = false;

private:
	void SelectRecordSlot(ECommandGesture Gesture);
	void BeginSampling();
	void FinishSampling();
	void SavePose(ECommandGesture Gesture, const FFingerExtensionPose& Pose, const FHandPose& HandPose);
	void UpdateScreenPrompt();
	void ConfigureHandVisuals();
	static bool IsRecordableGesture(ECommandGesture Gesture);
	static FString GetPoseName(ECommandGesture Gesture);
	static FText GetGestureTitle(ECommandGesture Gesture);
	static FText GetGestureInstruction(ECommandGesture Gesture);
	static FFingerExtensionPose DividePose(const FFingerExtensionPose& Pose, float Divisor);

	float CountdownRemainingSeconds = 0.0f;
	float SampleRemainingSeconds = 0.0f;
	float SampleCount = 0.0f;
	FFingerExtensionPose SampleAccumulator;
	FHandPose HandPoseAccumulator;
};
