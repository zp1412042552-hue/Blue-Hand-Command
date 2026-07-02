#pragma once

#include "CoreMinimal.h"
#include "ActionSquadTypes.h"
#include "Components/ActorComponent.h"
#include "HandPose.h"
#include "CommandGestureComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCommandGestureRecognized, ECommandGesture, Gesture);

USTRUCT(BlueprintType)
struct FFingerExtensionPose
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finger Pose", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Thumb = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finger Pose", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Index = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finger Pose", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Middle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finger Pose", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Ring = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Finger Pose", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Pinky = 0.0f;
};

UCLASS(ClassGroup = (ActionSquad), meta = (BlueprintSpawnableComponent))
class ACTIONSQUAD_API UCommandGestureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCommandGestureComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture")
	void SubmitFingerExtensionPose(const FFingerExtensionPose& Pose);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Gesture")
	void ForceRecognizeGesture(ECommandGesture Gesture);

	bool GetCurrentHandPose(FHandPose& OutHandPose, FFingerExtensionPose* OutFingerPose = nullptr) const;

	UPROPERTY(BlueprintAssignable, Category = "Action Squad|Gesture")
	FOnCommandGestureRecognized OnCommandGestureRecognized;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture")
	bool bEnableMetaHandPolling = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture", meta = (ClampMin = "0.0", Units = "s"))
	float RequiredHoldSeconds = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Gesture")
	ECommandGesture StableGesture = ECommandGesture::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Gesture")
	float StableGestureSeconds = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Gesture")
	FFingerExtensionPose LastFingerPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RecordedHandPoseConfidenceFloor = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RecordedFingerPoseConfidenceFloor = 0.72f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RecordedGestureConfidenceMargin = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OrientationCommandHandPoseConfidenceFloor = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OrientationCommandOpenHandConfidenceFloor = 0.74f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OrientationCommandFingerExtendedMin = 0.70f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OrientationCommandPinkyExtendedMin = 0.60f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OrientationCommandThumbExtendedMin = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SelectionCommandFingerExtendedMin = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture|Built In", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OpenHandExtendedMin = 0.68f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture|Built In", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OpenHandThumbExtendedMin = 0.42f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture|Built In", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FistFingerCurledMax = 0.32f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Gesture|Built In", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FistThumbCurledMax = 0.55f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Gesture")
	float LastRecordedHandPoseConfidence = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Gesture")
	float LastRecordedHandPoseError = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Gesture")
	float LastRecordedFingerPoseConfidence = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Recognition Zone")
	bool bUseRecognitionZone = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Recognition Zone")
	bool bDrawRecognitionZone = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Recognition Zone", meta = (Units = "cm"))
	FVector RecognitionZoneCenter = FVector(17.5f, 0.0f, 2.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Recognition Zone", meta = (Units = "cm"))
	FVector RecognitionZoneExtent = FVector(17.5f, 35.0f, 22.5f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Recognition Zone")
	bool bHandInsideRecognitionZone = false;

private:
	void LoadRecordedGestureProfile();
	void UpdateCandidate(ECommandGesture Candidate, float DeltaTime);
	ECommandGesture ClassifyHandPose(const FHandPose& Pose, const FFingerExtensionPose& FingerPose, float* OutBestConfidence = nullptr, float* OutBestError = nullptr, float* OutBestFingerConfidence = nullptr) const;
	ECommandGesture ClassifyBuiltInFingerGesture(const FFingerExtensionPose& FingerPose, float* OutConfidence = nullptr) const;
	bool PollMetaHandPose(FHandPose& OutHandPose, FFingerExtensionPose* OutFingerPose) const;
	bool GetRecognizerHandLocation(FVector& OutHandLocation) const;
	bool IsHandInsideRecognitionZone(const FVector& HandLocation) const;
	void DrawRecognitionZone(ECommandGesture Candidate) const;
	FRotator GetRecognizerWristRotator() const;
	bool IsOpenHandForOrientationCommand(const FFingerExtensionPose& Pose) const;
	bool IsSelectionFingerPoseAllowed(ECommandGesture Gesture, const FFingerExtensionPose& Pose) const;
	int32 CountSelectionExtendedFingers(const FFingerExtensionPose& Pose) const;
	static float Clamp01(float Value);
	static float ComputeFingerPoseConfidence(const FFingerExtensionPose& ReferencePose, const FFingerExtensionPose& Pose);
	static float ComputeOpenHandConfidence(const FFingerExtensionPose& Pose);
	static float ComputeWristYawConfidence(const FHandPose& ReferencePose, const FHandPose& Pose);

	FHandPose RecordedSelectAHandPose;
	FHandPose RecordedSelectBHandPose;
	FHandPose RecordedActionHandPose;
	FHandPose RecordedWatchHandPose;
	FHandPose RecordedRecallHandPose;
	FFingerExtensionPose RecordedSelectAFingerPose;
	FFingerExtensionPose RecordedSelectBFingerPose;
	FFingerExtensionPose RecordedActionFingerPose;
	FFingerExtensionPose RecordedWatchFingerPose;
	FFingerExtensionPose RecordedRecallFingerPose;
	bool bHasRecordedSelectAHandPose = false;
	bool bHasRecordedSelectBHandPose = false;
	bool bHasRecordedActionHandPose = false;
	bool bHasRecordedWatchHandPose = false;
	bool bHasRecordedRecallHandPose = false;
	bool bHasRecordedSelectAFingerPose = false;
	bool bHasRecordedSelectBFingerPose = false;
	bool bHasRecordedActionFingerPose = false;
	bool bHasRecordedWatchFingerPose = false;
	bool bHasRecordedRecallFingerPose = false;
	bool bStableGestureBroadcasted = false;
};
