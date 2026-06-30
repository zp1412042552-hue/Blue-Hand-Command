#pragma once

#include "CoreMinimal.h"
#include "ActionSquadTypes.h"
#include "GameFramework/Character.h"
#include "TutorialTeamMemberActor.generated.h"

class UAnimSequence;
class USkeletalMeshComponent;
class ATutorialDoorActor;
class UWidgetComponent;
class UTeamNameplateWidget;

USTRUCT(BlueprintType)
struct FTeamMemberAnimationSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> RelaxedIdle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> AlertIdle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> CrouchAlert = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> Walk = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> Run = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> CrouchWalk = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> Fire = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> Reload = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> HitReact = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> Death = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	TObjectPtr<UAnimSequence> BreachKick = nullptr;
};

UCLASS(Blueprintable)
class ACTIONSQUAD_API ATutorialTeamMemberActor : public ACharacter
{
	GENERATED_BODY()

public:
	ATutorialTeamMemberActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Team")
	void InitializeTeamMember(ETeamMemberRole InRole);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Team")
	void SetSelected(bool bInSelected);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Team")
	void PlayTeamAnimation(ETeamMemberAnimState NewState);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Team")
	void MoveToCommandLocation(const FVector& WorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Team")
	void SnapToGround();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Team")
	void BreachDoor(ATutorialDoorActor* Door);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<USkeletalMeshComponent> SoldierMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UWidgetComponent> NameplateWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Team")
	ETeamMemberRole TeamRole = ETeamMemberRole::TeamA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Team")
	bool bSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Team")
	float NameplateHeight = 210.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Movement")
	bool bSnapToGroundOnConstruction = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Movement")
	float GroundTraceUp = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Movement")
	float GroundTraceDown = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Movement")
	float MovementSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Movement")
	float AcceptanceRadius = 65.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Movement")
	float NavProjectionExtent = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Movement")
	float StepUpHeight = 38.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Door")
	float DoorBreachDistance = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Animations")
	FTeamMemberAnimationSet AnimationSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Animations")
	ETeamMemberAnimState CurrentAnimState = ETeamMemberAnimState::RelaxedIdle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Animations")
	TObjectPtr<UAnimSequence> CurrentAnimation = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Movement")
	bool bHasMoveTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Movement")
	FVector MoveTargetLocation = FVector::ZeroVector;

private:
	void LoadDefaultAssets();
	void RefreshNameplate();
	UAnimSequence* ResolveAnimation(ETeamMemberAnimState State) const;
	void FinishMoveCommand();
	void UpdateCommandMovement(float DeltaSeconds);
	void UpdateMovementAnimation();
};

UCLASS(Blueprintable)
class ACTIONSQUAD_API ATutorialTeamAActor : public ATutorialTeamMemberActor
{
	GENERATED_BODY()

public:
	ATutorialTeamAActor();
};

UCLASS(Blueprintable)
class ACTIONSQUAD_API ATutorialTeamBActor : public ATutorialTeamMemberActor
{
	GENERATED_BODY()

public:
	ATutorialTeamBActor();
};
