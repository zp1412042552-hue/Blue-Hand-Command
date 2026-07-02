#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialSquadTeleportTargetActor.generated.h"

class UStaticMeshComponent;

UCLASS(Blueprintable)
class ACTIONSQUAD_API ATutorialSquadTeleportTargetActor : public AActor
{
	GENERATED_BODY()

public:
	ATutorialSquadTeleportTargetActor();

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	FTransform GetPlayerTeleportTransform() const;

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	FTransform GetTeamATeleportTransform() const;

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	FTransform GetTeamBTeleportTransform() const;

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	void HideTeleportMarkers();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UStaticMeshComponent> PlayerPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UStaticMeshComponent> TeamAPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Squad|Components")
	TObjectPtr<UStaticMeshComponent> TeamBPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial", meta = (Units = "cm"))
	float PlayerHeightOffset = 92.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial", meta = (Units = "cm"))
	float TeamHeightOffset = 88.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial")
	bool bHideMarkersAfterTeleport = true;

private:
	FTransform BuildTeleportTransform(const USceneComponent* Point, float HeightOffset) const;
	void ConfigurePoint(UStaticMeshComponent* Point);
};
