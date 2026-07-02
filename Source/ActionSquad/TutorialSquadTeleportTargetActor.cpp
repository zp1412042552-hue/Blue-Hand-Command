#include "TutorialSquadTeleportTargetActor.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ATutorialSquadTeleportTargetActor::ATutorialSquadTeleportTargetActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PlayerPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Player Point"));
	PlayerPoint->SetupAttachment(SceneRoot);
	PlayerPoint->SetRelativeLocation(FVector::ZeroVector);

	TeamAPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Team A Point"));
	TeamAPoint->SetupAttachment(SceneRoot);
	TeamAPoint->SetRelativeLocation(FVector(0.0f, -110.0f, 0.0f));

	TeamBPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Team B Point"));
	TeamBPoint->SetupAttachment(SceneRoot);
	TeamBPoint->SetRelativeLocation(FVector(0.0f, 110.0f, 0.0f));

	ConfigurePoint(PlayerPoint);
	ConfigurePoint(TeamAPoint);
	ConfigurePoint(TeamBPoint);
}

FTransform ATutorialSquadTeleportTargetActor::GetPlayerTeleportTransform() const
{
	return BuildTeleportTransform(PlayerPoint, PlayerHeightOffset);
}

FTransform ATutorialSquadTeleportTargetActor::GetTeamATeleportTransform() const
{
	return BuildTeleportTransform(TeamAPoint, TeamHeightOffset);
}

FTransform ATutorialSquadTeleportTargetActor::GetTeamBTeleportTransform() const
{
	return BuildTeleportTransform(TeamBPoint, TeamHeightOffset);
}

void ATutorialSquadTeleportTargetActor::HideTeleportMarkers()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

FTransform ATutorialSquadTeleportTargetActor::BuildTeleportTransform(const USceneComponent* Point, float HeightOffset) const
{
	const USceneComponent* Source = Point ? Point : RootComponent.Get();
	FTransform Transform = Source ? Source->GetComponentTransform() : GetActorTransform();
	Transform.AddToTranslation(FVector(0.0f, 0.0f, HeightOffset));

	FRotator Rotation = Transform.GetRotation().Rotator();
	Rotation.Pitch = 0.0f;
	Rotation.Roll = 0.0f;
	Transform.SetRotation(Rotation.Quaternion());
	return Transform;
}

void ATutorialSquadTeleportTargetActor::ConfigurePoint(UStaticMeshComponent* Point)
{
	if (!Point)
	{
		return;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MarkerMaterial(
		TEXT("/Engine/EngineMaterials/EmissiveMeshMaterial.EmissiveMeshMaterial"));

	if (SphereMesh.Succeeded())
	{
		Point->SetStaticMesh(SphereMesh.Object);
	}

	Point->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.04f));
	Point->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Point->SetGenerateOverlapEvents(false);
	Point->SetHiddenInGame(true);

	if (MarkerMaterial.Succeeded())
	{
		Point->SetMaterial(0, MarkerMaterial.Object);
	}
}
