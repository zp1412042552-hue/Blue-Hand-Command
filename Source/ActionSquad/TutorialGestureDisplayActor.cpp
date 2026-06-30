#include "TutorialGestureDisplayActor.h"

#include "Components/ChildActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ATutorialGestureDisplayActor::ATutorialGestureDisplayActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	GestureActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("GestureActor"));
	GestureActorComponent->SetupAttachment(SceneRoot);
	GestureActorComponent->SetVisibility(false, true);
	GestureActorComponent->SetHiddenInGame(true);

	GestureMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GestureMesh"));
	GestureMeshComponent->SetupAttachment(SceneRoot);
	GestureMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GestureMeshComponent->SetGenerateOverlapEvents(false);
	GestureMeshComponent->SetCastShadow(false);
	GestureMeshComponent->SetVisibility(false, true);
	GestureMeshComponent->SetHiddenInGame(true);

	BuildDefaultGestureDisplays();
}

void ATutorialGestureDisplayActor::BeginPlay()
{
	Super::BeginPlay();
	BuildDefaultGestureDisplays();
}

void ATutorialGestureDisplayActor::PlayGestureDemo(int32 GestureIndex)
{
	if (!GestureDisplays.IsValidIndex(GestureIndex))
	{
		StopAndHideGesture();
		return;
	}

	ApplyGestureDisplay(GestureDisplays[GestureIndex]);
}

void ATutorialGestureDisplayActor::StopAndHideGesture()
{
	if (GestureActorComponent)
	{
		GestureActorComponent->SetChildActorClass(nullptr);
		GestureActorComponent->SetVisibility(false, true);
		GestureActorComponent->SetHiddenInGame(true);
	}

	if (GestureMeshComponent)
	{
		GestureMeshComponent->SetStaticMesh(nullptr);
		GestureMeshComponent->SetVisibility(false, true);
		GestureMeshComponent->SetHiddenInGame(true);
	}
}

void ATutorialGestureDisplayActor::SetTargetDisplayWidth(float WidthCm)
{
	TargetDisplayWidthCm = FMath::Max(1.0f, WidthCm);
	FitCurrentGestureToTargetWidth();
}

void ATutorialGestureDisplayActor::ApplyGestureDisplay(const FCommandGestureDisplay& Display)
{
	StopAndHideGesture();

	if (GestureActorComponent && !Display.GestureActorClass.IsNull())
	{
		UClass* ActorClass = Display.GestureActorClass.LoadSynchronous();
		GestureActorComponent->SetChildActorClass(ActorClass);
		GestureActorComponent->SetRelativeTransform(Display.RelativeTransform);
		GestureActorComponent->SetVisibility(ActorClass != nullptr, true);
		GestureActorComponent->SetHiddenInGame(ActorClass == nullptr);
		if (ActorClass)
		{
			FitCurrentGestureToTargetWidth();
			return;
		}
	}

	if (!GestureMeshComponent || Display.GestureMesh.IsNull())
	{
		return;
	}

	UStaticMesh* Mesh = Display.GestureMesh.LoadSynchronous();
	GestureMeshComponent->SetStaticMesh(Mesh);
	GestureMeshComponent->SetRelativeTransform(Display.RelativeTransform);
	GestureMeshComponent->SetVisibility(Mesh != nullptr, true);
	GestureMeshComponent->SetHiddenInGame(Mesh == nullptr);
	FitCurrentGestureToTargetWidth();
}

void ATutorialGestureDisplayActor::FitCurrentGestureToTargetWidth()
{
	if (GestureMeshComponent && GestureMeshComponent->GetStaticMesh())
	{
		GestureMeshComponent->SetRelativeScale3D(FVector::OneVector);
		FVector MinBounds = FVector::ZeroVector;
		FVector MaxBounds = FVector::ZeroVector;
		GestureMeshComponent->GetLocalBounds(MinBounds, MaxBounds);
		const FVector BoundsSize = MaxBounds - MinBounds;
		const float CurrentScreenPlaneWidth = FMath::Max(BoundsSize.Y, BoundsSize.Z);
		if (CurrentScreenPlaneWidth > KINDA_SMALL_NUMBER)
		{
			const float NewScale = FMath::Clamp(TargetDisplayWidthCm / CurrentScreenPlaneWidth, 0.001f, 100.0f);
			GestureMeshComponent->SetRelativeScale3D(FVector(NewScale));
		}
		return;
	}

	if (!GestureActorComponent || !GestureActorComponent->GetChildActor())
	{
		return;
	}

	GestureActorComponent->SetRelativeScale3D(FVector::OneVector);

	FVector Origin = FVector::ZeroVector;
	FVector Extent = FVector::ZeroVector;
	GestureActorComponent->GetChildActor()->GetActorBounds(true, Origin, Extent);
	const float CurrentScreenPlaneWidth = FMath::Max(Extent.Y, Extent.Z) * 2.0f;
	if (CurrentScreenPlaneWidth > KINDA_SMALL_NUMBER)
	{
		const float NewScale = FMath::Clamp(TargetDisplayWidthCm / CurrentScreenPlaneWidth, 0.001f, 100.0f);
		GestureActorComponent->SetRelativeScale3D(FVector(NewScale));
	}
}

void ATutorialGestureDisplayActor::BuildDefaultGestureDisplays()
{
	if (GestureDisplays.Num() > 0)
	{
		return;
	}

	const TCHAR* Names[] =
	{
		TEXT("SelectA_OneFinger"),
		TEXT("SelectB_TwoFinger"),
		TEXT("Action_Point"),
		TEXT("Watch_StopPalm"),
		TEXT("Recall_FistPull")
	};

	for (const TCHAR* Name : Names)
	{
		FCommandGestureDisplay Display;
		Display.GestureName = Name;
		Display.RelativeTransform = FTransform::Identity;
		GestureDisplays.Add(Display);
	}
}
