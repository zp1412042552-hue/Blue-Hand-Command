#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialDoorActor.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

UENUM(BlueprintType)
enum class ETutorialDoorState : uint8
{
	Closed UMETA(DisplayName = "Closed"),
	Targeted UMETA(DisplayName = "Targeted"),
	Breached UMETA(DisplayName = "Breached")
};

UCLASS(Blueprintable)
class ACTIONSQUAD_API ATutorialDoorActor : public AActor
{
	GENERATED_BODY()

public:
	ATutorialDoorActor();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Door")
	void SetDoorState(ETutorialDoorState NewState);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Door")
	void BreachFrom(const FVector& BreacherLocation);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UTextRenderComponent> DoorLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Door")
	ETutorialDoorState DoorState = ETutorialDoorState::Closed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Door")
	float BreachImpulse = 85000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Door")
	float BreachUpImpulse = 12000.0f;
};
