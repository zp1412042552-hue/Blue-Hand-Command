#include "TutorialPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/ChildActorComponent.h"
#include "CommandGestureComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerController.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "InputCoreTypes.h"
#include "Materials/MaterialInterface.h"
#include "MotionControllerComponent.h"
#include "NavigationSystem.h"
#include "OculusXRHandComponent.h"
#include "OculusXRInputFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TutorialCommandAimVisualActor.h"
#include "TutorialCommandMarkerActor.h"
#include "TutorialBallisticEffectActor.h"
#include "TutorialBulletMarkActor.h"
#include "TutorialInstructionActor.h"
#include "TutorialDoorActor.h"
#include "TutorialFloorMarkerActor.h"
#include "TutorialShellCasingActor.h"
#include "TutorialTeamMemberActor.h"
#include "TutorialWeaponActor.h"
#include "EngineUtils.h"
#include "GameFramework/DamageType.h"
#include "DrawDebugHelpers.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ATutorialPawn::ATutorialPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	BodyCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyCollision"));
	BodyCollision->InitCapsuleSize(34.0f, 88.0f);
	BodyCollision->SetCollisionProfileName(TEXT("Pawn"));
	SetRootComponent(BodyCollision);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SceneRoot->SetupAttachment(BodyCollision);

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(SceneRoot);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VROrigin);
	Camera->bLockToHmd = true;

	LeftHandTrackingRoot = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHandTrackingRoot"));
	LeftHandTrackingRoot->SetupAttachment(VROrigin);
	LeftHandTrackingRoot->SetTrackingSource(EControllerHand::Left);

	RightHandTrackingRoot = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHandTrackingRoot"));
	RightHandTrackingRoot->SetupAttachment(VROrigin);
	RightHandTrackingRoot->SetTrackingSource(EControllerHand::Right);

	LeftHandMesh = CreateDefaultSubobject<UOculusXRHandComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftHandTrackingRoot);
	LeftHandMesh->SkeletonType = EOculusXRHandType::HandLeft;
	LeftHandMesh->MeshType = EOculusXRHandType::HandLeft;
	LeftHandMesh->bInitializePhysics = false;
	LeftHandMesh->bUpdateHandScale = true;
	LeftHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LeftHandGunAttachRoot = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandGunAttachRoot"));
	LeftHandGunAttachRoot->SetupAttachment(LeftHandTrackingRoot);
	LeftHandGunAttachRoot->SetRelativeTransform(FTransform(FRotator::ZeroRotator, FVector(8.0f, 0.0f, -2.0f), FVector(1.0f)));

	PlayerWeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("PlayerWeaponComponent"));
	PlayerWeaponComponent->SetupAttachment(LeftHandGunAttachRoot);
	PlayerWeaponComponent->SetChildActorClass(ATutorialWeaponActor::StaticClass());
	PlayerWeaponComponent->SetRelativeTransform(FTransform::Identity);

	PlayerWeaponMuzzleReference = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerWeaponMuzzleReference"));
	PlayerWeaponMuzzleReference->SetupAttachment(PlayerWeaponComponent);
	PlayerWeaponMuzzleReference->SetRelativeLocation(FVector(45.0f, 0.0f, 2.0f));

	RightHandMesh = CreateDefaultSubobject<UOculusXRHandComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightHandTrackingRoot);
	RightHandMesh->SkeletonType = EOculusXRHandType::HandRight;
	RightHandMesh->MeshType = EOculusXRHandType::HandRight;
	RightHandMesh->bInitializePhysics = false;
	RightHandMesh->bUpdateHandScale = true;
	RightHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CommandGesture = CreateDefaultSubobject<UCommandGestureComponent>(TEXT("CommandGesture"));

	TeamMemberClass = ATutorialTeamMemberActor::StaticClass();
	TutorialInstructionClass = ATutorialInstructionActor::StaticClass();
	CommandMarkerClass = ATutorialCommandMarkerActor::StaticClass();
	CommandAimVisualClass = ATutorialCommandAimVisualActor::StaticClass();
	PlayerWeaponClass = ATutorialWeaponActor::StaticClass();
	MuzzleFlashEffectClass = ATutorialBallisticEffectActor::StaticClass();
	BulletTracerEffectClass = ATutorialBallisticEffectActor::StaticClass();
	ImpactEffectClass = ATutorialBallisticEffectActor::StaticClass();
	ShellCasingClass = ATutorialShellCasingActor::StaticClass();
	BulletMarkClass = ATutorialBulletMarkActor::StaticClass();

	static ConstructorHelpers::FObjectFinder<USoundBase> WeaponFireSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Weapon_Fire.AS_SFX_Weapon_Fire"));
	static ConstructorHelpers::FObjectFinder<USoundBase> SurfaceImpactSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Bullet_Surface.AS_SFX_Bullet_Surface"));
	static ConstructorHelpers::FObjectFinder<USoundBase> CharacterImpactSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Bullet_Character.AS_SFX_Bullet_Character"));
	static ConstructorHelpers::FObjectFinder<USoundBase> ShellSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Shell_Eject.AS_SFX_Shell_Eject"));
	static ConstructorHelpers::FObjectFinder<USoundBase> SelectASoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Command_Select_A.AS_SFX_Command_Select_A"));
	static ConstructorHelpers::FObjectFinder<USoundBase> SelectBSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Command_Select_B.AS_SFX_Command_Select_B"));
	static ConstructorHelpers::FObjectFinder<USoundBase> MoveIssuedSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Command_Move_Issue.AS_SFX_Command_Move_Issue"));
	static ConstructorHelpers::FObjectFinder<USoundBase> InvalidSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Command_Invalid.AS_SFX_Command_Invalid"));
	static ConstructorHelpers::FObjectFinder<USoundBase> LocomotionStartSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Locomotion_Start.AS_SFX_Locomotion_Start"));
	static ConstructorHelpers::FObjectFinder<USoundBase> LocomotionStopSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Locomotion_Stop.AS_SFX_Locomotion_Stop"));
	static ConstructorHelpers::FObjectFinder<USoundBase> HandTouchArmedSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Hand_Touch_Armed.AS_SFX_Hand_Touch_Armed"));
	static ConstructorHelpers::FObjectFinder<USoundBase> HandTouchResetSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Hand_Touch_Reset.AS_SFX_Hand_Touch_Reset"));
	static ConstructorHelpers::FObjectFinder<USoundBase> BulletTracerWhizSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Bullet_Tracer_Whiz.AS_SFX_Bullet_Tracer_Whiz"));
	static ConstructorHelpers::FObjectFinder<USoundBase> WeaponDryClickSoundAsset(
		TEXT("/Game/Audio/Tutorial/AS_SFX_Weapon_Dry_Click.AS_SFX_Weapon_Dry_Click"));
	PlayerWeaponFireSound = WeaponFireSoundAsset.Object;
	PlayerWeaponSurfaceImpactSound = SurfaceImpactSoundAsset.Object;
	PlayerWeaponCharacterImpactSound = CharacterImpactSoundAsset.Object;
	PlayerWeaponShellSound = ShellSoundAsset.Object;
	CommandSelectASound = SelectASoundAsset.Object;
	CommandSelectBSound = SelectBSoundAsset.Object;
	CommandMoveIssuedSound = MoveIssuedSoundAsset.Object;
	CommandInvalidSound = InvalidSoundAsset.Object;
	LocomotionStartSound = LocomotionStartSoundAsset.Object;
	LocomotionStopSound = LocomotionStopSoundAsset.Object;
	HandTouchArmedSound = HandTouchArmedSoundAsset.Object;
	HandTouchResetSound = HandTouchResetSoundAsset.Object;
	BulletTracerWhizSound = BulletTracerWhizSoundAsset.Object;
	WeaponDryClickSound = WeaponDryClickSoundAsset.Object;

	ConfigureHandVisuals();
}

void ATutorialPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ConfigurePlayerWeaponComponent();
}

void ATutorialPawn::BeginPlay()
{
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::Disabled);

	if (CommandGesture)
	{
		CommandGesture->OnCommandGestureRecognized.AddDynamic(this, &ATutorialPawn::HandleCommandGestureRecognized);
	}

	SpawnTutorialActors();
	ConfigurePlayerWeaponComponent();

	if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		EnableInput(PlayerController);
	}
}

void ATutorialPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateGunPitchLocomotion(DeltaSeconds);
	UpdateHandTouchFireInput(DeltaSeconds);
	UpdateCommandPreview(DeltaSeconds);
	UpdateSquadCommandState(DeltaSeconds);
}

void ATutorialPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		return;
	}

	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &ATutorialPawn::TestSelectA);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ATutorialPawn::TestSelectB);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ATutorialPawn::TestMoveSelectedTeam);
	PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ATutorialPawn::TestProtectMe);
	PlayerInputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ATutorialPawn::TestFreeAttack);
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &ATutorialPawn::TestMoveSelectedTeam);
}

void ATutorialPawn::SelectTeam(ESelectedTeamTarget Target)
{
	SquadCommandMode = ESquadTacticalCommandMode::None;

	const bool bSameTarget = CurrentSelectedTeam == Target;
	if (bSameTarget && bCommandIssuedSinceSelection && !bCanRearmSameTeamCommand)
	{
		return;
	}

	if (!bSameTarget || bCommandIssuedSinceSelection)
	{
		bCommandIssuedSinceSelection = false;
		bCanRearmSameTeamCommand = false;
		PreviewHoldSeconds = 0.0f;
		LastPreviewActor.Reset();
		bHasContinuousFollowTarget = false;
	}

	CurrentSelectedTeam = Target;

	if (UWorld* World = GetWorld())
	{
		if (Target == ESelectedTeamTarget::TeamA && CommandSelectASound)
		{
			UGameplayStatics::PlaySound2D(World, CommandSelectASound);
		}
		else if (Target == ESelectedTeamTarget::TeamB && CommandSelectBSound)
		{
			UGameplayStatics::PlaySound2D(World, CommandSelectBSound);
		}
	}

	if (TeamA)
	{
		const bool bSelectA = Target == ESelectedTeamTarget::TeamA || Target == ESelectedTeamTarget::All;
		TeamA->SetSelected(bSelectA);
	}

	if (TeamB)
	{
		const bool bSelectB = Target == ESelectedTeamTarget::TeamB || Target == ESelectedTeamTarget::All;
		TeamB->SetSelected(bSelectB);
	}
}

void ATutorialPawn::SpawnTutorialActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ATutorialTeamMemberActor> It(World); It; ++It)
	{
		ATutorialTeamMemberActor* ExistingMember = *It;
		if (!ExistingMember)
		{
			continue;
		}

		if (ExistingMember->TeamRole == ETeamMemberRole::TeamA && !TeamA)
		{
			TeamA = ExistingMember;
		}
		else if (ExistingMember->TeamRole == ETeamMemberRole::TeamB && !TeamB)
		{
			TeamB = ExistingMember;
		}
	}

	for (TActorIterator<ATutorialInstructionActor> It(World); It; ++It)
	{
		TutorialInstruction = *It;
		break;
	}

	const FRotator SpawnRotation(0.0f, GetActorRotation().Yaw, 0.0f);

	if (!TeamA && TeamMemberClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		TeamA = World->SpawnActor<ATutorialTeamMemberActor>(
			TeamMemberClass,
			GetActorLocation() + SpawnRotation.RotateVector(TeamAOffset),
			SpawnRotation,
			SpawnParams);
		if (TeamA)
		{
			TeamA->InitializeTeamMember(ETeamMemberRole::TeamA);
		}
	}

	if (!TeamB && TeamMemberClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		TeamB = World->SpawnActor<ATutorialTeamMemberActor>(
			TeamMemberClass,
			GetActorLocation() + SpawnRotation.RotateVector(TeamBOffset),
			SpawnRotation,
			SpawnParams);
		if (TeamB)
		{
			TeamB->InitializeTeamMember(ETeamMemberRole::TeamB);
		}
	}

	if (!TutorialInstruction && TutorialInstructionClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		TutorialInstruction = World->SpawnActor<ATutorialInstructionActor>(
			TutorialInstructionClass,
			GetActorLocation() + SpawnRotation.RotateVector(FVector(90.0f, 0.0f, 145.0f)),
			SpawnRotation,
			SpawnParams);
	}

	if (!CommandMarker && CommandMarkerClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CommandMarker = World->SpawnActor<ATutorialCommandMarkerActor>(
			CommandMarkerClass,
			GetActorLocation(),
			FRotator::ZeroRotator,
			SpawnParams);
		if (CommandMarker)
		{
			CommandMarker->HideMarker();
		}
	}

	if (!CommandAimVisual && CommandAimVisualClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CommandAimVisual = World->SpawnActor<ATutorialCommandAimVisualActor>(
			CommandAimVisualClass,
			GetActorLocation(),
			FRotator::ZeroRotator,
			SpawnParams);
		if (CommandAimVisual)
		{
			CommandAimVisual->HideAim();
		}
	}
}

bool ATutorialPawn::CommandSelectedTeamToPointedLocation()
{
	FHitResult Hit;
	return TraceCommandTarget(Hit) && IssueCommandAtHit(Hit);
}

bool ATutorialPawn::FirePlayerWeapon()
{
	UWorld* World = GetWorld();
	if (!World || !bEnablePlayerWeaponFire)
	{
		if (WeaponDryClickSound)
		{
			UGameplayStatics::PlaySound2D(this, WeaponDryClickSound);
		}
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastPlayerWeaponFireTime < PlayerWeaponFireInterval)
	{
		if (WeaponDryClickSound)
		{
			UGameplayStatics::PlaySound2D(World, WeaponDryClickSound, 0.35f);
		}
		return false;
	}

	FTransform MuzzleTransform;
	if (!GetPlayerWeaponMuzzleTransform(MuzzleTransform))
	{
		if (WeaponDryClickSound)
		{
			UGameplayStatics::PlaySound2D(World, WeaponDryClickSound);
		}
		return false;
	}
	FVector ShotDirection = MuzzleTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	if (ShotDirection.IsNearlyZero())
	{
		ShotDirection = GetActorForwardVector();
	}

	const FVector TraceStart = MuzzleTransform.GetLocation();
	const FVector TraceEnd = TraceStart + ShotDirection * PlayerWeaponRange;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ActionSquadPlayerWeaponTrace), true, this);
	if (PlayerWeapon)
	{
		QueryParams.AddIgnoredActor(PlayerWeapon);
	}
	if (PlayerWeaponComponent && PlayerWeaponComponent->GetChildActor())
	{
		QueryParams.AddIgnoredActor(PlayerWeaponComponent->GetChildActor());
	}

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	const FVector ShotEnd = bHit ? Hit.ImpactPoint : TraceEnd;

	SpawnPlayerWeaponMuzzleFlash(MuzzleTransform);
	SpawnPlayerWeaponBulletTracer(TraceStart, ShotEnd);
	SpawnPlayerWeaponShellCasing(MuzzleTransform);
	if (PlayerWeaponFireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(World, PlayerWeaponFireSound, TraceStart);
	}

	if (bHit)
	{
		AActor* HitActor = Hit.GetActor();
		const bool bBloodImpact = Cast<ATutorialTeamMemberActor>(HitActor) != nullptr;
		SpawnPlayerWeaponImpactEffect(Hit, bBloodImpact);
		SpawnPlayerWeaponBulletMark(Hit, bBloodImpact);
		if (USoundBase* ImpactSound = bBloodImpact ? PlayerWeaponCharacterImpactSound.Get() : PlayerWeaponSurfaceImpactSound.Get())
		{
			UGameplayStatics::PlaySoundAtLocation(World, ImpactSound, Hit.ImpactPoint);
		}

		if (HitActor && PlayerWeaponDamage > 0.0f)
		{
			UGameplayStatics::ApplyPointDamage(
				HitActor,
				PlayerWeaponDamage,
				ShotDirection,
				Hit,
				GetController(),
				this,
				UDamageType::StaticClass());
		}
	}

	LastPlayerWeaponFireTime = CurrentTime;
	if (TutorialInstruction)
	{
		TutorialInstruction->NotifyPlayerFiredWeapon();
	}
	return true;
}

void ATutorialPawn::StartProtectMeCommand()
{
	SquadCommandMode = ESquadTacticalCommandMode::ProtectMe;
	ProtectMeRetargetTimer = 0.0f;
	FreeAttackThinkTimer = 0.0f;
	bTeamAAtProtectSpot = false;
	bTeamBAtProtectSpot = false;
	ClearPointCommandSelection();
	HideCommandVisuals();

	if (TeamA)
	{
		TeamA->SetSelected(false);
		TeamA->StopCommandMovement();
	}
	if (TeamB)
	{
		TeamB->SetSelected(false);
		TeamB->StopCommandMovement();
	}
}

void ATutorialPawn::StartFreeAttackCommand()
{
	SquadCommandMode = ESquadTacticalCommandMode::FreeAttack;
	ProtectMeRetargetTimer = 0.0f;
	FreeAttackThinkTimer = 0.0f;
	bTeamAAtProtectSpot = false;
	bTeamBAtProtectSpot = false;
	ClearPointCommandSelection();
	HideCommandVisuals();

	if (TeamA)
	{
		TeamA->SetSelected(false);
	}
	if (TeamB)
	{
		TeamB->SetSelected(false);
	}
}

void ATutorialPawn::HandleCommandGestureRecognized(ECommandGesture Gesture)
{
	switch (Gesture)
	{
	case ECommandGesture::SelectA:
		SelectTeam(ESelectedTeamTarget::TeamA);
		break;
	case ECommandGesture::SelectB:
		SelectTeam(ESelectedTeamTarget::TeamB);
		break;
	case ECommandGesture::Action:
		CommandSelectedTeamToPointedLocation();
		break;
	case ECommandGesture::Recall:
		StartProtectMeCommand();
		break;
	case ECommandGesture::Watch:
		StartFreeAttackCommand();
		break;
	default:
		break;
	}

	if (TutorialInstruction)
	{
		TutorialInstruction->NotifyGesture(Gesture);
	}
}

void ATutorialPawn::UpdateCommandPreview(float DeltaSeconds)
{
	const ESelectedTeamTarget MarkerTarget = GetMarkerTarget();
	const bool bOutsideRecognitionZone =
		CommandGesture &&
		CommandGesture->bUseRecognitionZone &&
		!CommandGesture->bHandInsideRecognitionZone;

	if (MarkerTarget == ESelectedTeamTarget::None)
	{
		PreviewHoldSeconds = 0.0f;
		LastPreviewActor.Reset();
		bHasContinuousFollowTarget = false;
		HideCommandVisuals();
		return;
	}

	if (bEnableContinuousPointFollow)
	{
		if (bOutsideRecognitionZone || !IsContinuousFollowGestureHeld(MarkerTarget))
		{
			StopSelectedTeamMovement(MarkerTarget);
			PreviewHoldSeconds = 0.0f;
			LastPreviewActor.Reset();
			bHasContinuousFollowTarget = false;
			HideCommandVisuals();
			return;
		}

		if (UpdateContinuousPointFollow(DeltaSeconds, MarkerTarget))
		{
			return;
		}
	}

	if (bCommandIssuedSinceSelection || bOutsideRecognitionZone)
	{
		if (bOutsideRecognitionZone && bCommandIssuedSinceSelection)
		{
			bCanRearmSameTeamCommand = true;
		}
		PreviewHoldSeconds = 0.0f;
		LastPreviewActor.Reset();
		bHasContinuousFollowTarget = false;
		HideCommandVisuals();
		return;
	}

	FHitResult Hit;
	if (!TraceCommandTarget(Hit))
	{
		PreviewHoldSeconds = 0.0f;
		LastPreviewActor.Reset();
		bHasContinuousFollowTarget = false;
		HideCommandVisuals();
		return;
	}

	const bool bCanConfirmHit = IsCommandHitConfirmable(Hit);
	if (bCanConfirmHit)
	{
		const bool bSameActor = LastPreviewActor.Get() == Hit.GetActor();
		const bool bSameLocation = FVector::DistSquared(LastPreviewLocation, Hit.ImpactPoint) <= FMath::Square(CommandStableRadius);
		if (bSameActor && bSameLocation)
		{
			PreviewHoldSeconds += FMath::Max(0.0f, DeltaSeconds);
		}
		else
		{
			PreviewHoldSeconds = 0.0f;
			LastPreviewLocation = Hit.ImpactPoint;
			LastPreviewActor = Hit.GetActor();
		}
	}
	else
	{
		PreviewHoldSeconds = 0.0f;
		LastPreviewLocation = Hit.ImpactPoint;
		LastPreviewActor = Hit.GetActor();
	}

	if (CommandMarker)
	{
		CommandMarker->ShowMarker(
			MarkerTarget,
			Hit.ImpactPoint,
			Hit.ImpactNormal,
			GetCommandAimDirection(),
			CommandHoldSeconds > KINDA_SMALL_NUMBER ? PreviewHoldSeconds / CommandHoldSeconds : 1.0f,
			bCanConfirmHit);
	}

	if (bCanConfirmHit && PreviewHoldSeconds >= CommandHoldSeconds)
	{
		if (IssueCommandAtHit(Hit))
		{
			bCommandIssuedSinceSelection = true;
			bCanRearmSameTeamCommand = false;
			if (TutorialInstruction)
			{
				TutorialInstruction->NotifyGesture(ECommandGesture::Action);
			}
			PreviewHoldSeconds = 0.0f;
			LastPreviewActor.Reset();
			HideCommandVisuals();
		}
	}
}

bool ATutorialPawn::UpdateContinuousPointFollow(float DeltaSeconds, ESelectedTeamTarget MarkerTarget)
{
	FVector AimStart = FVector::ZeroVector;
	FVector AimDirection = FVector::ZeroVector;
	const bool bHasAimRay = GetCommandAimRay(AimStart, AimDirection);

	FHitResult Hit;
	if (!TraceCommandTarget(Hit))
	{
		if (bDrawContinuousFollowAimLine && bHasAimRay && CommandAimVisual)
		{
			CommandAimVisual->ShowAim(MarkerTarget, AimStart, AimStart + AimDirection * CommandTraceDistance, false);
		}
		else if (CommandAimVisual)
		{
			CommandAimVisual->HideAim();
		}
		StopSelectedTeamMovement(MarkerTarget);
		PreviewHoldSeconds = 0.0f;
		LastPreviewActor.Reset();
		bHasContinuousFollowTarget = false;
		if (CommandMarker)
		{
			CommandMarker->HideMarker();
		}
		return true;
	}

	FVector ProjectedTargetLocation = Hit.ImpactPoint;
	const ATutorialFloorMarkerActor* HitMarker = Cast<ATutorialFloorMarkerActor>(Hit.GetActor());
	const ATutorialDoorActor* HitDoor = Cast<ATutorialDoorActor>(Hit.GetActor());
	const ATutorialTeamMemberActor* SelectedPreviewTeam = MarkerTarget == ESelectedTeamTarget::TeamA ? TeamA : TeamB;
	const bool bCanMoveToHit = HitMarker || HitDoor ? true : ProjectCommandHitToNavigation(Hit, ProjectedTargetLocation);
	if (HitMarker)
	{
		ProjectedTargetLocation = HitMarker->GetCommandLocation();
	}
	else if (HitDoor)
	{
		ProjectedTargetLocation = HitDoor->GetBreachStandLocation(SelectedPreviewTeam ? SelectedPreviewTeam->GetActorLocation() : GetActorLocation());
	}
	if (bDrawContinuousFollowAimLine && bHasAimRay && CommandAimVisual)
	{
		CommandAimVisual->ShowAim(MarkerTarget, AimStart, bCanMoveToHit ? ProjectedTargetLocation : Hit.ImpactPoint, bCanMoveToHit);
	}
	else if (CommandAimVisual)
	{
		CommandAimVisual->HideAim();
	}

	if (CommandMarker)
	{
		CommandMarker->ShowMarker(
			MarkerTarget,
			bCanMoveToHit ? ProjectedTargetLocation : Hit.ImpactPoint,
			Hit.ImpactNormal,
			GetCommandAimDirection(),
			bCanMoveToHit ? 1.0f : 0.0f,
			bCanMoveToHit);
	}

	if (!bCanMoveToHit)
	{
		StopSelectedTeamMovement(MarkerTarget);
		PreviewHoldSeconds = 0.0f;
		LastPreviewLocation = Hit.ImpactPoint;
		LastPreviewActor = Hit.GetActor();
		bHasContinuousFollowTarget = false;
		return true;
	}

	PreviewHoldSeconds = FMath::Max(0.0f, PreviewHoldSeconds - DeltaSeconds);
	const bool bSameActor = LastPreviewActor.Get() == Hit.GetActor();
	const bool bFarEnoughFromLastTarget =
		FVector::DistSquared(LastPreviewLocation, ProjectedTargetLocation) >= FMath::Square(ContinuousFollowRetargetDistance);
	const bool bReadyToRetarget = PreviewHoldSeconds <= KINDA_SMALL_NUMBER;

	if (bReadyToRetarget && (!bHasContinuousFollowTarget || !bSameActor || bFarEnoughFromLastTarget))
	{
		if (IssueCommandAtHit(Hit))
		{
			LastPreviewLocation = ProjectedTargetLocation;
			LastPreviewActor = Hit.GetActor();
			PreviewHoldSeconds = ContinuousFollowRetargetInterval;
			bHasContinuousFollowTarget = true;
		}
	}

	return true;
}

bool ATutorialPawn::IsContinuousFollowGestureHeld(ESelectedTeamTarget MarkerTarget) const
{
	if (!CommandGesture || !CommandGesture->bHandInsideRecognitionZone)
	{
		return false;
	}

	const FFingerExtensionPose& FingerPose = CommandGesture->LastFingerPose;
	const bool bIndexExtended = FingerPose.Index >= ContinuousFollowFingerExtendedMin;
	const bool bMiddleExtended = FingerPose.Middle >= ContinuousFollowFingerExtendedMin;
	const bool bMiddleCurled = FingerPose.Middle <= ContinuousFollowSecondFingerCurledMax;

	if (MarkerTarget == ESelectedTeamTarget::TeamA)
	{
		return bIndexExtended && bMiddleCurled;
	}

	if (MarkerTarget == ESelectedTeamTarget::TeamB)
	{
		return bIndexExtended && bMiddleExtended;
	}

	return false;
}

void ATutorialPawn::StopSelectedTeamMovement(ESelectedTeamTarget MarkerTarget)
{
	if ((MarkerTarget == ESelectedTeamTarget::TeamA || MarkerTarget == ESelectedTeamTarget::All) && TeamA)
	{
		TeamA->StopCommandMovement();
	}

	if ((MarkerTarget == ESelectedTeamTarget::TeamB || MarkerTarget == ESelectedTeamTarget::All) && TeamB)
	{
		TeamB->StopCommandMovement();
	}
}

void ATutorialPawn::HideCommandVisuals()
{
	if (CommandMarker)
	{
		CommandMarker->HideMarker();
	}

	if (CommandAimVisual)
	{
		CommandAimVisual->HideAim();
	}
}

void ATutorialPawn::ClearPointCommandSelection()
{
	CurrentSelectedTeam = ESelectedTeamTarget::None;
	PreviewHoldSeconds = 0.0f;
	LastPreviewActor.Reset();
	bHasContinuousFollowTarget = false;
	bCommandIssuedSinceSelection = false;
	bCanRearmSameTeamCommand = true;
}

void ATutorialPawn::UpdateSquadCommandState(float DeltaSeconds)
{
	switch (SquadCommandMode)
	{
	case ESquadTacticalCommandMode::ProtectMe:
		UpdateProtectMeCommand(DeltaSeconds);
		break;
	case ESquadTacticalCommandMode::FreeAttack:
		UpdateFreeAttackCommand(DeltaSeconds);
		break;
	default:
		break;
	}
}

void ATutorialPawn::UpdateProtectMeCommand(float DeltaSeconds)
{
	ProtectMeRetargetTimer -= FMath::Max(0.0f, DeltaSeconds);
	if (ProtectMeRetargetTimer > 0.0f)
	{
		return;
	}

	ProtectMeRetargetTimer = FMath::Max(0.02f, ProtectMeRetargetInterval);

	auto UpdateMember = [&](ATutorialTeamMemberActor* Member, bool bTeamA, bool& bAtProtectSpot)
	{
		if (!Member || Member->bDead)
		{
			return;
		}

		const FVector TargetLocation = GetProtectMeLocation(bTeamA);
		const float AcceptanceDistance = FMath::Max(5.0f, ProtectMeAcceptanceDistance);
		if (FVector::DistSquared2D(Member->GetActorLocation(), TargetLocation) > FMath::Square(AcceptanceDistance))
		{
			Member->MoveToCommandLocation(TargetLocation);
			bAtProtectSpot = false;
			return;
		}

		if (!bAtProtectSpot || Member->bHasMoveTarget)
		{
			Member->StopCommandMovement();
			bAtProtectSpot = true;
		}

		const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
		const FVector Right = GetActorRightVector().GetSafeNormal2D();
		FVector LookDirection = Forward + (bTeamA ? -Right : Right) * 0.85f;
		if (LookDirection.Normalize())
		{
			Member->SetActorRotation(LookDirection.Rotation());
		}
		Member->PlayTeamAnimation(ETeamMemberAnimState::AlertIdle);
	};

	UpdateMember(TeamA, true, bTeamAAtProtectSpot);
	UpdateMember(TeamB, false, bTeamBAtProtectSpot);
}

void ATutorialPawn::UpdateFreeAttackCommand(float DeltaSeconds)
{
	FreeAttackThinkTimer -= FMath::Max(0.0f, DeltaSeconds);
	if (FreeAttackThinkTimer > 0.0f)
	{
		return;
	}

	FreeAttackThinkTimer = FMath::Max(0.02f, FreeAttackThinkInterval);
	UpdateFreeAttackMember(TeamA, DeltaSeconds, true);
	UpdateFreeAttackMember(TeamB, DeltaSeconds, false);
}

void ATutorialPawn::UpdateFreeAttackMember(ATutorialTeamMemberActor* Member, float DeltaSeconds, bool bTeamA)
{
	if (!Member || Member->bDead)
	{
		return;
	}

	ATutorialTeamMemberActor* Target = FindBestFreeAttackTarget(Member);
	if (!Target)
	{
		if (Member->bHasMoveTarget)
		{
			Member->StopCommandMovement();
		}
		Member->PlayTeamAnimation(ETeamMemberAnimState::AlertIdle);
		return;
	}

	FVector TargetLocation = FVector::ZeroVector;
	if (HasClearShotToEnemy(Member, Target, &TargetLocation))
	{
		if (Member->bHasMoveTarget)
		{
			Member->StopCommandMovement();
		}

		FVector LookDirection = TargetLocation - Member->GetActorLocation();
		LookDirection.Z = 0.0f;
		if (LookDirection.Normalize())
		{
			Member->SetActorRotation(LookDirection.Rotation());
		}
		if (Member->CurrentAnimState != ETeamMemberAnimState::Fire)
		{
			Member->PlayFireAnimationOnly();
		}
		Member->FireWeaponAtLocation(TargetLocation);
		return;
	}

	const FVector MoveLocation = GetFreeAttackMoveLocation(Member, Target);
	if (FVector::DistSquared2D(Member->GetActorLocation(), MoveLocation) > FMath::Square(90.0f))
	{
		Member->MoveToCommandLocation(MoveLocation);
	}
	else
	{
		Member->PlayTeamAnimation(ETeamMemberAnimState::AlertIdle);
	}
}

FVector ATutorialPawn::GetProtectMeLocation(bool bTeamA) const
{
	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = GetActorRightVector().GetSafeNormal2D();
	const FVector SideOffset = (bTeamA ? -Right : Right) * FMath::Max(0.0f, ProtectMeSideOffset);
	const FVector BackOffset = -Forward * FMath::Max(0.0f, ProtectMeBackOffset);
	return GetActorLocation() + SideOffset + BackOffset;
}

FVector ATutorialPawn::GetFreeAttackMoveLocation(const ATutorialTeamMemberActor* Member, const ATutorialTeamMemberActor* Target) const
{
	if (!Member || !Target)
	{
		return GetActorLocation();
	}

	FVector VantageLocation = FVector::ZeroVector;
	if (FindFreeAttackVantageLocation(Member, Target, VantageLocation))
	{
		return VantageLocation;
	}

	const FVector PlayerLocation = GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	FVector DirectionFromTarget = Member->GetActorLocation() - TargetLocation;
	DirectionFromTarget.Z = 0.0f;
	if (!DirectionFromTarget.Normalize())
	{
		DirectionFromTarget = (PlayerLocation - TargetLocation).GetSafeNormal2D();
	}
	if (DirectionFromTarget.IsNearlyZero())
	{
		DirectionFromTarget = -GetActorForwardVector().GetSafeNormal2D();
	}

	FVector SideDirection = FVector::CrossProduct(FVector::UpVector, DirectionFromTarget).GetSafeNormal();
	if (Member->TeamRole == ETeamMemberRole::TeamA)
	{
		SideDirection *= -1.0f;
	}

	FVector DesiredLocation =
		TargetLocation +
		DirectionFromTarget * FMath::Max(100.0f, FreeAttackStandOffDistance) +
		SideDirection * 120.0f;

	const float MaxRadius = FMath::Max(100.0f, FreeAttackPlayerRadius);
	FVector PlayerToDesired = DesiredLocation - PlayerLocation;
	PlayerToDesired.Z = 0.0f;
	const float DesiredDistance = PlayerToDesired.Size();
	if (DesiredDistance > MaxRadius)
	{
		PlayerToDesired = PlayerToDesired.GetSafeNormal() * MaxRadius;
		DesiredLocation = PlayerLocation + PlayerToDesired;
		DesiredLocation.Z = TargetLocation.Z;
	}

	FVector ProjectedLocation = FVector::ZeroVector;
	if (ProjectFreeAttackNavLocation(DesiredLocation, ProjectedLocation))
	{
		return ProjectedLocation;
	}

	return DesiredLocation;
}

bool ATutorialPawn::FindFreeAttackVantageLocation(const ATutorialTeamMemberActor* Member, const ATutorialTeamMemberActor* Target, FVector& OutLocation) const
{
	if (!Member || !Target || Target->bDead)
	{
		return false;
	}

	const FVector PlayerLocation = GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const float MaxPlayerRadius = FMath::Max(100.0f, FreeAttackPlayerRadius);
	const float MaxPlayerRadiusSquared = FMath::Square(MaxPlayerRadius);
	const float PreferredStandOff = FMath::Max(120.0f, FreeAttackStandOffDistance);
	const float SearchRadii[] =
	{
		FMath::Max(260.0f, PreferredStandOff * 0.65f),
		PreferredStandOff,
		PreferredStandOff * 1.25f,
		PreferredStandOff * 1.55f
	};

	float BestScore = TNumericLimits<float>::Max();
	bool bFoundVantage = false;
	constexpr int32 AngleSamples = 16;
	const FVector MemberLocation = Member->GetActorLocation();
	FVector PlayerRight = GetActorRightVector();
	PlayerRight.Z = 0.0f;
	PlayerRight.Normalize();

	for (float Radius : SearchRadii)
	{
		for (int32 Index = 0; Index < AngleSamples; ++Index)
		{
			const float AngleRadians = (2.0f * PI * static_cast<float>(Index)) / static_cast<float>(AngleSamples);
			const FVector Direction(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians), 0.0f);
			const FVector DesiredLocation = TargetLocation + Direction * Radius;

			FVector CandidateLocation = FVector::ZeroVector;
			if (!ProjectFreeAttackNavLocation(DesiredLocation, CandidateLocation))
			{
				continue;
			}

			if (FVector::DistSquared2D(CandidateLocation, PlayerLocation) > MaxPlayerRadiusSquared)
			{
				continue;
			}

			if (!HasClearShotFromLocationToEnemy(CandidateLocation, Target))
			{
				continue;
			}

			const bool bEnemyCanShootCandidate = HasEnemyLineOfFireToLocation(Target, CandidateLocation);
			const float MemberTravelScore = FVector::DistSquared2D(MemberLocation, CandidateLocation) * 0.001f;
			const float StandOffScore = FMath::Abs(FVector::Dist2D(CandidateLocation, TargetLocation) - PreferredStandOff) * 0.7f;
			const float PlayerLeashScore = FVector::Dist2D(CandidateLocation, PlayerLocation) * 0.12f;
			const float ExposureScore = bEnemyCanShootCandidate ? 180.0f : -120.0f;
			const float DesiredSide = Member->TeamRole == ETeamMemberRole::TeamA ? -1.0f : 1.0f;
			const float SideAlignment = FVector::DotProduct(CandidateLocation - PlayerLocation, PlayerRight) * DesiredSide;
			const float TeamSideScore = -SideAlignment * 0.04f;
			const float Score = MemberTravelScore + StandOffScore + PlayerLeashScore + ExposureScore + TeamSideScore;

			if (Score < BestScore)
			{
				BestScore = Score;
				OutLocation = CandidateLocation;
				bFoundVantage = true;
			}
		}
	}

	return bFoundVantage;
}

bool ATutorialPawn::ProjectFreeAttackNavLocation(const FVector& DesiredLocation, FVector& OutLocation) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSystem)
	{
		return false;
	}

	FNavLocation ProjectedLocation;
	const FVector Extent(160.0f, 160.0f, 260.0f);
	if (!NavSystem->ProjectPointToNavigation(DesiredLocation, ProjectedLocation, Extent))
	{
		return false;
	}

	OutLocation = ProjectedLocation.Location;
	return true;
}

ATutorialTeamMemberActor* ATutorialPawn::FindBestFreeAttackTarget(const ATutorialTeamMemberActor* Member) const
{
	UWorld* World = GetWorld();
	if (!World || !Member)
	{
		return nullptr;
	}

	ATutorialTeamMemberActor* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	const float MaxPlayerDistanceSquared = FMath::Square(FMath::Max(100.0f, FreeAttackPlayerRadius));
	const FVector PlayerLocation = GetActorLocation();

	for (TActorIterator<ATutorialTeamMemberActor> It(World); It; ++It)
	{
		ATutorialTeamMemberActor* Candidate = *It;
		if (!Candidate || Candidate == Member || Candidate->TeamRole != ETeamMemberRole::Enemy || Candidate->bDead)
		{
			continue;
		}

		if (FVector::DistSquared2D(Candidate->GetActorLocation(), PlayerLocation) > MaxPlayerDistanceSquared)
		{
			continue;
		}

		const bool bHasClearShot = HasClearShotToEnemy(Member, Candidate);
		const float MemberDistanceSquared = FVector::DistSquared2D(Candidate->GetActorLocation(), Member->GetActorLocation());
		const float Score = MemberDistanceSquared + (bHasClearShot ? 0.0f : MaxPlayerDistanceSquared);
		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

bool ATutorialPawn::HasClearShotToEnemy(const ATutorialTeamMemberActor* Member, const ATutorialTeamMemberActor* Target, FVector* OutTargetLocation) const
{
	if (!Member || !Target || Target->bDead)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector TargetLocation = Target->GetActorLocation() + FVector(0.0f, 0.0f, FreeAttackAimHeight);
	if (OutTargetLocation)
	{
		*OutTargetLocation = TargetLocation;
	}

	const FVector TraceStart = Member->GetActorLocation() + FVector(0.0f, 0.0f, FreeAttackAimHeight);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ActionSquadFreeAttackSightTrace), false, Member);
	if (Member->EquippedWeapon)
	{
		QueryParams.AddIgnoredActor(Member->EquippedWeapon);
	}

	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, TraceStart, TargetLocation, ECC_Visibility, QueryParams))
	{
		return true;
	}

	const AActor* HitActor = Hit.GetActor();
	return HitActor == Target || (HitActor && HitActor->IsOwnedBy(Target));
}

bool ATutorialPawn::HasClearShotFromLocationToEnemy(const FVector& FromLocation, const ATutorialTeamMemberActor* Target, FVector* OutTargetLocation) const
{
	if (!Target || Target->bDead)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector TargetLocation = Target->GetActorLocation() + FVector(0.0f, 0.0f, FreeAttackAimHeight);
	if (OutTargetLocation)
	{
		*OutTargetLocation = TargetLocation;
	}

	const FVector TraceStart = FromLocation + FVector(0.0f, 0.0f, FreeAttackAimHeight);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ActionSquadFreeAttackVantageTrace), false, this);
	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, TraceStart, TargetLocation, ECC_Visibility, QueryParams))
	{
		return true;
	}

	const AActor* HitActor = Hit.GetActor();
	return HitActor == Target || (HitActor && HitActor->IsOwnedBy(Target));
}

bool ATutorialPawn::HasEnemyLineOfFireToLocation(const ATutorialTeamMemberActor* Enemy, const FVector& TargetLocation) const
{
	if (!Enemy || Enemy->bDead)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector TraceStart = Enemy->GetActorLocation() + FVector(0.0f, 0.0f, FreeAttackAimHeight);
	const FVector TraceEnd = TargetLocation + FVector(0.0f, 0.0f, FreeAttackAimHeight);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ActionSquadFreeAttackExposureTrace), false, Enemy);
	if (Enemy->EquippedWeapon)
	{
		QueryParams.AddIgnoredActor(Enemy->EquippedWeapon);
	}

	FHitResult Hit;
	return !World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
}

bool ATutorialPawn::HasAnyLivingEnemyInFreeAttackRange() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float MaxPlayerDistanceSquared = FMath::Square(FMath::Max(100.0f, FreeAttackPlayerRadius));
	const FVector PlayerLocation = GetActorLocation();
	for (TActorIterator<ATutorialTeamMemberActor> It(World); It; ++It)
	{
		const ATutorialTeamMemberActor* Candidate = *It;
		if (Candidate && Candidate->TeamRole == ETeamMemberRole::Enemy && !Candidate->bDead &&
			FVector::DistSquared2D(Candidate->GetActorLocation(), PlayerLocation) <= MaxPlayerDistanceSquared)
		{
			return true;
		}
	}

	return false;
}

bool ATutorialPawn::TraceCommandTarget(FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector Start = FVector::ZeroVector;
	FVector Direction = FVector::ZeroVector;
	if (!GetCommandAimRay(Start, Direction))
	{
		return false;
	}

	const FVector End = Start + Direction * CommandTraceDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ActionSquadCommandTrace), false, this);
	return World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);
}

bool ATutorialPawn::GetCommandAimRay(FVector& OutStart, FVector& OutDirection) const
{
	if (RightHandMesh && RightHandMesh->GetSkinnedAsset())
	{
		const FName ThumbRootBone(*UOculusXRInputFunctionLibrary::GetBoneName(EOculusXRBone::Thumb_0));
		const FName ThumbTipBone(*UOculusXRInputFunctionLibrary::GetBoneName(EOculusXRBone::Thumb_Tip));
		const int32 RootBoneIndex = RightHandMesh->GetBoneIndex(ThumbRootBone);
		const int32 TipBoneIndex = RightHandMesh->GetBoneIndex(ThumbTipBone);

		if (RootBoneIndex != INDEX_NONE && TipBoneIndex != INDEX_NONE)
		{
			const FVector ThumbRootLocation = RightHandMesh->GetBoneLocation(ThumbRootBone);
			const FVector ThumbTipLocation = RightHandMesh->GetBoneLocation(ThumbTipBone);
			const FVector ThumbDirection = ThumbTipLocation - ThumbRootLocation;
			if (ThumbDirection.SizeSquared() > FMath::Square(1.0f))
			{
				OutStart = ThumbTipLocation;
				OutDirection = ThumbDirection.GetSafeNormal();
				return true;
			}
		}
	}

	USceneComponent* TraceSource = RightHandTrackingRoot ? Cast<USceneComponent>(RightHandTrackingRoot) : nullptr;
	if (!TraceSource || !TraceSource->IsRegistered())
	{
		TraceSource = Camera;
	}

	if (!TraceSource)
	{
		return false;
	}

	OutStart = TraceSource->GetComponentLocation();
	OutDirection = TraceSource->GetForwardVector();
	return true;
}

FVector ATutorialPawn::GetCommandAimDirection() const
{
	FVector Start = FVector::ZeroVector;
	FVector Direction = FVector::ZeroVector;
	if (GetCommandAimRay(Start, Direction))
	{
		return Direction;
	}

	return GetActorForwardVector();
}

bool ATutorialPawn::IsCommandHitConfirmable(const FHitResult& Hit) const
{
	return Cast<ATutorialDoorActor>(Hit.GetActor()) != nullptr
		|| Cast<ATutorialFloorMarkerActor>(Hit.GetActor()) != nullptr
		|| IsWalkableCommandHit(Hit);
}

bool ATutorialPawn::IsWalkableCommandHit(const FHitResult& Hit) const
{
	FVector ProjectedLocation = FVector::ZeroVector;
	return ProjectCommandHitToNavigation(Hit, ProjectedLocation);
}

bool ATutorialPawn::ProjectCommandHitToNavigation(const FHitResult& Hit, FVector& OutProjectedLocation) const
{
	if (!Hit.bBlockingHit || Hit.ImpactNormal.Z < WalkableCommandSurfaceMinZ)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const UNavigationSystemV1* NavSystem = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (!NavSystem)
	{
		return false;
	}

	FNavLocation ProjectedLocation;
	const FVector Extent(CommandNavProjectionExtent, CommandNavProjectionExtent, CommandNavProjectionExtent);
	if (!NavSystem->ProjectPointToNavigation(Hit.ImpactPoint, ProjectedLocation, Extent))
	{
		return false;
	}

	OutProjectedLocation = ProjectedLocation.Location;
	return true;
}

bool ATutorialPawn::IssueCommandAtHit(const FHitResult& Hit)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	ATutorialDoorActor* HitDoor = Cast<ATutorialDoorActor>(Hit.GetActor());
	ATutorialFloorMarkerActor* HitMarker = Cast<ATutorialFloorMarkerActor>(Hit.GetActor());
	if (!HitDoor && !HitMarker && !IsWalkableCommandHit(Hit))
	{
		if (CommandInvalidSound)
		{
			UGameplayStatics::PlaySound2D(World, CommandInvalidSound);
		}
		return false;
	}

	FVector CommandLocation = Hit.ImpactPoint;
	if (HitMarker)
	{
		CommandLocation = HitMarker->GetCommandLocation();
	}

	bool bIssuedCommand = false;
	if ((CurrentSelectedTeam == ESelectedTeamTarget::TeamA || CurrentSelectedTeam == ESelectedTeamTarget::All) && TeamA)
	{
		if (HitDoor)
		{
			TeamA->BreachDoor(HitDoor);
		}
		else
		{
			TeamA->MoveToCommandLocation(CommandLocation);
		}
		bIssuedCommand = true;
	}

	if ((CurrentSelectedTeam == ESelectedTeamTarget::TeamB || CurrentSelectedTeam == ESelectedTeamTarget::All) && TeamB)
	{
		if (HitDoor)
		{
			TeamB->BreachDoor(HitDoor);
		}
		else
		{
			TeamB->MoveToCommandLocation(CommandLocation);
		}
		bIssuedCommand = true;
	}

	if (bIssuedCommand && CommandMoveIssuedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(World, CommandMoveIssuedSound, CommandLocation);
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (bIssuedCommand)
	{
		DrawDebugSphere(World, Hit.ImpactPoint, 12.0f, 12, FColor::Cyan, false, 1.5f, 0, 2.0f);
	}
#endif

	return bIssuedCommand;
}

ESelectedTeamTarget ATutorialPawn::GetMarkerTarget() const
{
	if (CurrentSelectedTeam == ESelectedTeamTarget::TeamA || CurrentSelectedTeam == ESelectedTeamTarget::TeamB)
	{
		return CurrentSelectedTeam;
	}

	return ESelectedTeamTarget::None;
}

void ATutorialPawn::TestSelectA()
{
	if (CommandGesture)
	{
		CommandGesture->ForceRecognizeGesture(ECommandGesture::SelectA);
	}
}

void ATutorialPawn::TestSelectB()
{
	if (CommandGesture)
	{
		CommandGesture->ForceRecognizeGesture(ECommandGesture::SelectB);
	}
}

void ATutorialPawn::TestMoveSelectedTeam()
{
	if (CommandSelectedTeamToPointedLocation() && TutorialInstruction)
	{
		TutorialInstruction->NotifyGesture(ECommandGesture::Action);
	}
}

void ATutorialPawn::TestProtectMe()
{
	if (CommandGesture)
	{
		CommandGesture->ForceRecognizeGesture(ECommandGesture::Recall);
	}
}

void ATutorialPawn::TestFreeAttack()
{
	if (CommandGesture)
	{
		CommandGesture->ForceRecognizeGesture(ECommandGesture::Watch);
	}
}

void ATutorialPawn::ConfigurePlayerWeaponComponent()
{
	if (!PlayerWeaponComponent)
	{
		PlayerWeapon = nullptr;
		return;
	}

	UClass* ClassToUse = PlayerWeaponClass ? PlayerWeaponClass.Get() : ATutorialWeaponActor::StaticClass();
	if (ClassToUse && PlayerWeaponComponent->GetChildActorClass() != ClassToUse)
	{
		PlayerWeaponComponent->SetChildActorClass(ClassToUse);
	}

	PlayerWeapon = Cast<ATutorialWeaponActor>(PlayerWeaponComponent->GetChildActor());
}

bool ATutorialPawn::GetPlayerWeaponMuzzleTransform(FTransform& OutMuzzleTransform) const
{
	const ATutorialWeaponActor* CurrentWeapon = PlayerWeapon;
	if (!CurrentWeapon && PlayerWeaponComponent)
	{
		CurrentWeapon = Cast<ATutorialWeaponActor>(PlayerWeaponComponent->GetChildActor());
	}

	if (bUseWeaponActorMuzzleForFiring && CurrentWeapon)
	{
		OutMuzzleTransform = CurrentWeapon->GetFiringMuzzleTransform();
		return true;
	}

	if (PlayerWeaponMuzzleReference)
	{
		OutMuzzleTransform = PlayerWeaponMuzzleReference->GetComponentTransform();
		return true;
	}

	if (CurrentWeapon)
	{
		OutMuzzleTransform = CurrentWeapon->GetFiringMuzzleTransform();
		return true;
	}

	return false;
}

FTransform ATutorialPawn::GetPlayerWeaponShellEjectionTransform(const FTransform& MuzzleTransform) const
{
	const ATutorialWeaponActor* CurrentWeapon = PlayerWeapon;
	if (!CurrentWeapon && PlayerWeaponComponent)
	{
		CurrentWeapon = Cast<ATutorialWeaponActor>(PlayerWeaponComponent->GetChildActor());
	}

	return CurrentWeapon ? CurrentWeapon->GetShellEjectionTransform() : MuzzleTransform;
}

void ATutorialPawn::SpawnPlayerWeaponMuzzleFlash(const FTransform& MuzzleTransform)
{
	if (!bSpawnPlayerWeaponMuzzleFlash)
	{
		return;
	}

	UWorld* World = GetWorld();
	UClass* EffectClass = MuzzleFlashEffectClass ? MuzzleFlashEffectClass.Get() : ATutorialBallisticEffectActor::StaticClass();
	if (!World || !EffectClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATutorialBallisticEffectActor* Effect = World->SpawnActor<ATutorialBallisticEffectActor>(EffectClass, MuzzleTransform, SpawnParams);
	if (Effect)
	{
		Effect->ConfigureMuzzleFlash(MuzzleTransform);
	}
}

void ATutorialPawn::SpawnPlayerWeaponBulletTracer(const FVector& StartLocation, const FVector& EndLocation)
{
	if (!bSpawnPlayerWeaponBulletTracer)
	{
		return;
	}

	UWorld* World = GetWorld();
	UClass* EffectClass = BulletTracerEffectClass ? BulletTracerEffectClass.Get() : ATutorialBallisticEffectActor::StaticClass();
	if (!World || !EffectClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATutorialBallisticEffectActor* Effect = World->SpawnActor<ATutorialBallisticEffectActor>(
		EffectClass,
		StartLocation,
		(EndLocation - StartLocation).Rotation(),
		SpawnParams);
	if (Effect)
	{
		Effect->ConfigureBulletTracer(StartLocation, EndLocation);
	}
	if (BulletTracerWhizSound)
	{
		UGameplayStatics::PlaySoundAtLocation(World, BulletTracerWhizSound, (StartLocation + EndLocation) * 0.5f, 0.35f);
	}
}

void ATutorialPawn::SpawnPlayerWeaponImpactEffect(const FHitResult& Hit, bool bBloodImpact)
{
	if (!bSpawnPlayerWeaponImpactEffect)
	{
		return;
	}

	UWorld* World = GetWorld();
	UClass* EffectClass = ImpactEffectClass ? ImpactEffectClass.Get() : ATutorialBallisticEffectActor::StaticClass();
	if (!World || !EffectClass)
	{
		return;
	}

	const FVector ImpactNormal = Hit.ImpactNormal.IsNearlyZero() ? FVector::UpVector : Hit.ImpactNormal;
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATutorialBallisticEffectActor* Effect = World->SpawnActor<ATutorialBallisticEffectActor>(
		EffectClass,
		Hit.ImpactPoint + ImpactNormal * 1.5f,
		ImpactNormal.Rotation(),
		SpawnParams);
	if (Effect)
	{
		Effect->ConfigureImpact(Hit.ImpactPoint, ImpactNormal, bBloodImpact);
	}
}

void ATutorialPawn::SpawnPlayerWeaponShellCasing(const FTransform& MuzzleTransform)
{
	if (!bSpawnPlayerWeaponShellCasing)
	{
		return;
	}

	UWorld* World = GetWorld();
	UClass* CasingClass = ShellCasingClass ? ShellCasingClass.Get() : ATutorialShellCasingActor::StaticClass();
	if (!World || !CasingClass)
	{
		return;
	}

	const FTransform EjectionTransform = GetPlayerWeaponShellEjectionTransform(MuzzleTransform);
	const FVector LinearVelocity = EjectionTransform.TransformVectorNoScale(ShellEjectionLocalVelocity);
	const FVector AngularVelocity = EjectionTransform.TransformVectorNoScale(ShellEjectionAngularVelocityDegrees);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATutorialShellCasingActor* Casing = World->SpawnActor<ATutorialShellCasingActor>(CasingClass, EjectionTransform, SpawnParams);
	if (Casing)
	{
		Casing->LaunchShell(EjectionTransform, LinearVelocity, AngularVelocity);
	}
	if (PlayerWeaponShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(World, PlayerWeaponShellSound, EjectionTransform.GetLocation(), 0.55f);
	}
}

void ATutorialPawn::SpawnPlayerWeaponBulletMark(const FHitResult& Hit, bool bCharacterMark)
{
	if (!bSpawnPlayerWeaponBulletMarks)
	{
		return;
	}

	UWorld* World = GetWorld();
	UClass* MarkClass = BulletMarkClass ? BulletMarkClass.Get() : ATutorialBulletMarkActor::StaticClass();
	if (!World || !MarkClass)
	{
		return;
	}

	FVector ImpactNormal = Hit.ImpactNormal.GetSafeNormal();
	if (ImpactNormal.IsNearlyZero())
	{
		ImpactNormal = FVector::UpVector;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATutorialBulletMarkActor* Mark = World->SpawnActor<ATutorialBulletMarkActor>(
		MarkClass,
		Hit.ImpactPoint + ImpactNormal * 0.5f,
		ImpactNormal.Rotation(),
		SpawnParams);
	if (!Mark)
	{
		return;
	}

	Mark->ConfigureBulletMark(Hit.ImpactPoint, ImpactNormal, Hit.GetComponent(), bCharacterMark);
	PlayerWeaponBulletMarks.Add(Mark);
	PrunePlayerWeaponBulletMarks();
}

void ATutorialPawn::PrunePlayerWeaponBulletMarks()
{
	PlayerWeaponBulletMarks.RemoveAll([](const TWeakObjectPtr<ATutorialBulletMarkActor>& Mark)
	{
		return !Mark.IsValid();
	});

	if (MaxPlayerWeaponBulletMarks <= 0)
	{
		for (const TWeakObjectPtr<ATutorialBulletMarkActor>& Mark : PlayerWeaponBulletMarks)
		{
			if (Mark.IsValid())
			{
				Mark->Destroy();
			}
		}
		PlayerWeaponBulletMarks.Reset();
		return;
	}

	while (PlayerWeaponBulletMarks.Num() > MaxPlayerWeaponBulletMarks)
	{
		if (ATutorialBulletMarkActor* OldestMark = PlayerWeaponBulletMarks[0].Get())
		{
			OldestMark->Destroy();
		}
		PlayerWeaponBulletMarks.RemoveAt(0);
	}
}

void ATutorialPawn::UpdateHandTouchFireInput(float DeltaSeconds)
{
	if (!bEnableHandTouchFireInput)
	{
		CurrentHandTouchFireDistance = 0.0f;
		bHandTouchFireArmed = true;
		return;
	}

	float ClosestDistance = 0.0f;
	if (!GetClosestHandBoneDistance(ClosestDistance))
	{
		CurrentHandTouchFireDistance = 0.0f;
		bHandTouchFireArmed = true;
		return;
	}

	CurrentHandTouchFireDistance = ClosestDistance;

	const float TriggerDistance = FMath::Max(0.1f, HandTouchFireDistance);
	const float ReleaseDistance = FMath::Max(TriggerDistance + 0.1f, HandTouchFireReleaseDistance);
	if (bHandTouchFireArmed && ClosestDistance <= TriggerDistance)
	{
		if (HandTouchArmedSound)
		{
			UGameplayStatics::PlaySound2D(this, HandTouchArmedSound, 0.6f);
		}
		FirePlayerWeapon();
		bHandTouchFireArmed = false;
	}
	else if (!bHandTouchFireArmed && ClosestDistance >= ReleaseDistance)
	{
		bHandTouchFireArmed = true;
		if (HandTouchResetSound)
		{
			UGameplayStatics::PlaySound2D(this, HandTouchResetSound, 0.45f);
		}
	}
}

bool ATutorialPawn::GetClosestHandBoneDistance(float& OutClosestDistance) const
{
	if (!HasValidHandsForTouchFire())
	{
		return false;
	}

	TArray<FVector> LeftBoneLocations;
	TArray<FVector> RightBoneLocations;
	if (!GetHandBoneLocations(LeftHandMesh, LeftBoneLocations) || !GetHandBoneLocations(RightHandMesh, RightBoneLocations))
	{
		return false;
	}

	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	for (const FVector& LeftLocation : LeftBoneLocations)
	{
		for (const FVector& RightLocation : RightBoneLocations)
		{
			ClosestDistanceSquared = FMath::Min(ClosestDistanceSquared, FVector::DistSquared(LeftLocation, RightLocation));
		}
	}

	if (ClosestDistanceSquared == TNumericLimits<float>::Max())
	{
		return false;
	}

	OutClosestDistance = FMath::Sqrt(ClosestDistanceSquared);
	return true;
}

bool ATutorialPawn::HasValidHandsForTouchFire() const
{
	if (!LeftHandMesh || !RightHandMesh || !LeftHandMesh->GetSkinnedAsset() || !RightHandMesh->GetSkinnedAsset())
	{
		return false;
	}

	if (!bRequireHighConfidenceHandsForFire)
	{
		return true;
	}

	if (!UOculusXRInputFunctionLibrary::IsHandTrackingEnabled())
	{
		return false;
	}

	const bool bLeftValid =
		UOculusXRInputFunctionLibrary::IsHandPositionValid(EOculusXRHandType::HandLeft) &&
		UOculusXRInputFunctionLibrary::GetTrackingConfidence(EOculusXRHandType::HandLeft) != EOculusXRTrackingConfidence::Low;
	const bool bRightValid =
		UOculusXRInputFunctionLibrary::IsHandPositionValid(EOculusXRHandType::HandRight) &&
		UOculusXRInputFunctionLibrary::GetTrackingConfidence(EOculusXRHandType::HandRight) != EOculusXRTrackingConfidence::Low;

	return bLeftValid && bRightValid;
}

bool ATutorialPawn::GetHandBoneLocations(const UOculusXRHandComponent* HandMesh, TArray<FVector>& OutLocations) const
{
	OutLocations.Reset();
	if (!HandMesh || !HandMesh->GetSkinnedAsset())
	{
		return false;
	}

	const int32 BoneCount = HandMesh->GetNumBones();
	OutLocations.Reserve(BoneCount);
	for (int32 BoneIndex = 0; BoneIndex < BoneCount; ++BoneIndex)
	{
		const FName BoneName = HandMesh->GetBoneName(BoneIndex);
		if (BoneName == NAME_None)
		{
			continue;
		}

		OutLocations.Add(HandMesh->GetBoneLocation(BoneName));
	}

	return OutLocations.Num() > 0;
}

void ATutorialPawn::UpdateGunPitchLocomotion(float DeltaSeconds)
{
	const EGunPitchLocomotionState PreviousState = GunLocomotionState;
	if (!bEnableGunPitchLocomotion)
	{
		GunLocomotionState = EGunPitchLocomotionState::Stopped;
		GunLocomotionStartHoldTimer = 0.0f;
		CurrentGunLocomotionSpeed = 0.0f;
		if (PreviousState != EGunPitchLocomotionState::Stopped && LocomotionStopSound)
		{
			UGameplayStatics::PlaySound2D(this, LocomotionStopSound);
		}
		return;
	}

	float GunPitchDegrees = 0.0f;
	if (!GetGunLocomotionPitch(GunPitchDegrees) || !HasValidLeftHandTrackingForGunLocomotion())
	{
		GunLocomotionState = EGunPitchLocomotionState::Stopped;
		GunLocomotionStartHoldTimer = 0.0f;
		CurrentGunLocomotionSpeed = 0.0f;
		if (PreviousState != EGunPitchLocomotionState::Stopped && LocomotionStopSound)
		{
			UGameplayStatics::PlaySound2D(this, LocomotionStopSound);
		}
		return;
	}

	RawGunPitchDegrees = GunPitchDegrees;
	SmoothedGunPitchDegrees = FMath::FInterpTo(
		SmoothedGunPitchDegrees,
		RawGunPitchDegrees,
		FMath::Max(0.0f, DeltaSeconds),
		GunPitchSmoothingSpeed);

	if (GunLocomotionState == EGunPitchLocomotionState::MovingForward)
	{
		if (SmoothedGunPitchDegrees <= GunForwardStopPitch)
		{
			GunLocomotionState = EGunPitchLocomotionState::Stopped;
			GunLocomotionStartHoldTimer = 0.0f;
		}
	}
	else if (GunLocomotionState == EGunPitchLocomotionState::MovingBackward)
	{
		if (SmoothedGunPitchDegrees >= GunBackwardStopPitch)
		{
			GunLocomotionState = EGunPitchLocomotionState::Stopped;
			GunLocomotionStartHoldTimer = 0.0f;
		}
	}
	else
	{
		EGunPitchLocomotionState PendingState = EGunPitchLocomotionState::Stopped;
		if (SmoothedGunPitchDegrees >= GunForwardStartPitch)
		{
			PendingState = EGunPitchLocomotionState::MovingForward;
		}
		else if (SmoothedGunPitchDegrees <= GunBackwardStartPitch)
		{
			PendingState = EGunPitchLocomotionState::MovingBackward;
		}

		if (PendingState == EGunPitchLocomotionState::Stopped)
		{
			GunLocomotionStartHoldTimer = 0.0f;
		}
		else
		{
			GunLocomotionStartHoldTimer += FMath::Max(0.0f, DeltaSeconds);
			if (GunLocomotionStartHoldTimer >= GunLocomotionStartHoldSeconds)
			{
				GunLocomotionState = PendingState;
				GunLocomotionStartHoldTimer = 0.0f;
			}
		}
	}

	if (GunLocomotionState != PreviousState)
	{
		if (GunLocomotionState == EGunPitchLocomotionState::Stopped)
		{
			if (LocomotionStopSound)
			{
				UGameplayStatics::PlaySound2D(this, LocomotionStopSound);
			}
		}
		else if (LocomotionStartSound)
		{
			UGameplayStatics::PlaySound2D(this, LocomotionStartSound);
		}
	}

	const float TargetSpeed =
		GunLocomotionState == EGunPitchLocomotionState::MovingForward
			? GunForwardSpeed
			: (GunLocomotionState == EGunPitchLocomotionState::MovingBackward ? -GunBackwardSpeed : 0.0f);
	const float InterpSpeed = FMath::Abs(TargetSpeed) > FMath::Abs(CurrentGunLocomotionSpeed)
		? GunLocomotionAccelerationSpeed
		: GunLocomotionDecelerationSpeed;
	CurrentGunLocomotionSpeed = FMath::FInterpTo(
		CurrentGunLocomotionSpeed,
		TargetSpeed,
		FMath::Max(0.0f, DeltaSeconds),
		InterpSpeed);

	if (FMath::Abs(CurrentGunLocomotionSpeed) <= 1.0f)
	{
		CurrentGunLocomotionSpeed = 0.0f;
		return;
	}

	const FVector MoveDelta = GetPlayerLocomotionDirection() * CurrentGunLocomotionSpeed * DeltaSeconds;
	FHitResult MoveHit;
	AddActorWorldOffset(MoveDelta, true, &MoveHit);
}

bool ATutorialPawn::GetGunLocomotionPitch(float& OutPitchDegrees) const
{
	if (!PlayerWeaponMuzzleReference)
	{
		return false;
	}

	const FVector MuzzleForward = PlayerWeaponMuzzleReference->GetForwardVector().GetSafeNormal();
	if (MuzzleForward.IsNearlyZero())
	{
		return false;
	}

	// Positive means the muzzle is pushed below horizontal; negative means it is raised above horizontal.
	OutPitchDegrees = -FMath::RadiansToDegrees(FMath::Asin(FMath::Clamp(MuzzleForward.Z, -1.0f, 1.0f)));
	return true;
}

bool ATutorialPawn::HasValidLeftHandTrackingForGunLocomotion() const
{
	if (!bRequireLeftHandTrackingForGunLocomotion)
	{
		return true;
	}

	if (!UOculusXRInputFunctionLibrary::IsHandTrackingEnabled())
	{
		return false;
	}

	return UOculusXRInputFunctionLibrary::GetTrackingConfidence(EOculusXRHandType::HandLeft) != EOculusXRTrackingConfidence::Low;
}

FVector ATutorialPawn::GetPlayerLocomotionDirection() const
{
	FVector Forward = Camera ? Camera->GetForwardVector() : GetActorForwardVector();
	Forward.Z = 0.0f;
	if (!Forward.Normalize())
	{
		Forward = GetActorForwardVector();
		Forward.Z = 0.0f;
		Forward.Normalize();
	}

	return Forward.IsNearlyZero() ? FVector::ForwardVector : Forward;
}

void ATutorialPawn::ConfigureHandVisuals()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HandMaterial(
		TEXT("/Game/HandGameplay/Hands/Models/HandMat.HandMat"));

	if (HandMaterial.Succeeded())
	{
		if (LeftHandMesh)
		{
			LeftHandMesh->MaterialOverride = HandMaterial.Object;
			LeftHandMesh->SetMaterial(0, HandMaterial.Object);
		}
		if (RightHandMesh)
		{
			RightHandMesh->MaterialOverride = HandMaterial.Object;
			RightHandMesh->SetMaterial(0, HandMaterial.Object);
		}
	}
}
