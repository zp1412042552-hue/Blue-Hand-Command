#include "GestureRecorderPawn.h"

#include "Camera/CameraComponent.h"
#include "CommandGestureComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerController.h"
#include "GesturePoseProfileSaveGame.h"
#include "GestureRecorderScreenActor.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "MotionControllerComponent.h"
#include "OculusXRHandComponent.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

AGestureRecorderPawn::AGestureRecorderPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	BodyCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyCollision"));
	BodyCollision->InitCapsuleSize(34.0f, 88.0f);
	BodyCollision->SetCollisionProfileName(TEXT("Pawn"));
	SetRootComponent(BodyCollision);

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(BodyCollision);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VROrigin);
	Camera->bLockToHmd = true;

	LeftHandTrackingRoot = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHandTrackingRoot"));
	LeftHandTrackingRoot->SetupAttachment(VROrigin);
	LeftHandTrackingRoot->SetTrackingSource(EControllerHand::Left);

	LeftHandMesh = CreateDefaultSubobject<UOculusXRHandComponent>(TEXT("LeftHandMesh"));
	LeftHandMesh->SetupAttachment(LeftHandTrackingRoot);
	LeftHandMesh->SkeletonType = EOculusXRHandType::HandLeft;
	LeftHandMesh->MeshType = EOculusXRHandType::HandLeft;
	LeftHandMesh->bInitializePhysics = false;
	LeftHandMesh->bUpdateHandScale = true;
	LeftHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RightHandTrackingRoot = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHandTrackingRoot"));
	RightHandTrackingRoot->SetupAttachment(VROrigin);
	RightHandTrackingRoot->SetTrackingSource(EControllerHand::Right);

	RightHandMesh = CreateDefaultSubobject<UOculusXRHandComponent>(TEXT("RightHandMesh"));
	RightHandMesh->SetupAttachment(RightHandTrackingRoot);
	RightHandMesh->SkeletonType = EOculusXRHandType::HandRight;
	RightHandMesh->MeshType = EOculusXRHandType::HandRight;
	RightHandMesh->bInitializePhysics = false;
	RightHandMesh->bUpdateHandScale = true;
	RightHandMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CommandGesture = CreateDefaultSubobject<UCommandGestureComponent>(TEXT("CommandGesture"));
	ScreenActorClass = AGestureRecorderScreenActor::StaticClass();

	ConfigureHandVisuals();
}

void AGestureRecorderPawn::BeginPlay()
{
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::Disabled);

	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<AGestureRecorderScreenActor> It(World); It; ++It)
		{
			ScreenActor = *It;
			break;
		}

		if (!ScreenActor)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			UClass* ClassToSpawn = ScreenActorClass ? ScreenActorClass.Get() : AGestureRecorderScreenActor::StaticClass();
			ScreenActor = World->SpawnActor<AGestureRecorderScreenActor>(
				ClassToSpawn,
				GetActorLocation() + GetActorRotation().RotateVector(ScreenOffset),
				GetActorRotation(),
				SpawnParams);
		}
	}

	if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		EnableInput(PlayerController);
	}

	UpdateScreenPrompt();
}

void AGestureRecorderPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ScreenActor && Camera)
	{
		if (CommandGesture && !bCountdownActive && !bSamplingActive)
		{
			ScreenActor->UpdatePoseText(CommandGesture->LastFingerPose);
		}
	}

	if (bCountdownActive)
	{
		CountdownRemainingSeconds = FMath::Max(0.0f, CountdownRemainingSeconds - DeltaSeconds);
		if (ScreenActor)
		{
			const int32 DisplaySeconds = FMath::Max(1, FMath::CeilToInt(CountdownRemainingSeconds));
			ScreenActor->SetRecordingPrompt(
				FText::FromString(TEXT("准备录制")),
				FText::FromString(SelectedRecordGesture == ECommandGesture::SelectA
					? TEXT("保持 1 根手指。")
					: TEXT("保持 2 根手指。")),
				FText::FromString(FString::Printf(TEXT("%d 秒后开始采样。"), DisplaySeconds)));
		}

		if (CountdownRemainingSeconds <= KINDA_SMALL_NUMBER)
		{
			BeginSampling();
		}
		return;
	}

	if (bSamplingActive)
	{
		SampleRemainingSeconds = FMath::Max(0.0f, SampleRemainingSeconds - DeltaSeconds);
		if (CommandGesture)
		{
			FHandPose CurrentHandPose;
			FFingerExtensionPose CurrentFingerPose;
			if (CommandGesture->GetCurrentHandPose(CurrentHandPose, &CurrentFingerPose))
			{
				SampleAccumulator.Thumb += CurrentFingerPose.Thumb;
				SampleAccumulator.Index += CurrentFingerPose.Index;
				SampleAccumulator.Middle += CurrentFingerPose.Middle;
				SampleAccumulator.Ring += CurrentFingerPose.Ring;
				SampleAccumulator.Pinky += CurrentFingerPose.Pinky;

				if (SampleCount <= KINDA_SMALL_NUMBER)
				{
					HandPoseAccumulator = CurrentHandPose;
				}
				else
				{
					HandPoseAccumulator.AddWeighted(CurrentHandPose, 1.0f / (SampleCount + 1.0f));
				}

				SampleCount += 1.0f;
			}
		}

		if (ScreenActor)
		{
			ScreenActor->SetRecordingPrompt(
				FText::FromString(TEXT("正在录制")),
				FText::FromString(TEXT("保持手势稳定，先不要移动。")),
				FText::FromString(FString::Printf(TEXT("采样剩余 %.1f 秒。"), SampleRemainingSeconds)));
		}

		if (SampleRemainingSeconds <= KINDA_SMALL_NUMBER)
		{
			FinishSampling();
		}
	}
}

void AGestureRecorderPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		return;
	}

	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &AGestureRecorderPawn::SelectRecordSlotA);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AGestureRecorderPawn::SelectRecordSlotB);
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &AGestureRecorderPawn::StartRecordingSelectedSlot);
}

void AGestureRecorderPawn::SelectRecordSlotA()
{
	SelectRecordSlot(ECommandGesture::SelectA);
}

void AGestureRecorderPawn::SelectRecordSlotB()
{
	SelectRecordSlot(ECommandGesture::SelectB);
}

void AGestureRecorderPawn::StartRecordingSelectedSlot()
{
	if (SelectedRecordGesture != ECommandGesture::SelectA && SelectedRecordGesture != ECommandGesture::SelectB)
	{
		SelectRecordSlot(ECommandGesture::SelectA);
		return;
	}

	bCountdownActive = true;
	bSamplingActive = false;
	CountdownRemainingSeconds = CountdownSeconds;
	UpdateScreenPrompt();
}

void AGestureRecorderPawn::SelectRecordSlot(ECommandGesture Gesture)
{
	if (bCountdownActive || bSamplingActive)
	{
		return;
	}

	SelectedRecordGesture = Gesture;
	UpdateScreenPrompt();
}

void AGestureRecorderPawn::BeginSampling()
{
	bCountdownActive = false;
	bSamplingActive = true;
	SampleRemainingSeconds = SampleWindowSeconds;
	SampleCount = 0.0f;
	SampleAccumulator = FFingerExtensionPose();
	HandPoseAccumulator = FHandPose();
}

void AGestureRecorderPawn::FinishSampling()
{
	bSamplingActive = false;
	const FFingerExtensionPose AveragePose = DividePose(SampleAccumulator, FMath::Max(1.0f, SampleCount));
	FHandPose AverageHandPose = HandPoseAccumulator;
	AverageHandPose.PoseName = SelectedRecordGesture == ECommandGesture::SelectA ? TEXT("SelectA") : TEXT("SelectB");
	AverageHandPose.Encode();
	SavePose(SelectedRecordGesture, AveragePose, AverageHandPose);
	UpdateScreenPrompt();
}

void AGestureRecorderPawn::SavePose(ECommandGesture Gesture, const FFingerExtensionPose& Pose, const FHandPose& HandPose)
{
	if (Gesture != ECommandGesture::SelectA && Gesture != ECommandGesture::SelectB)
	{
		return;
	}

	UGesturePoseProfileSaveGame* Profile = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(UGesturePoseProfileSaveGame::SlotName, UGesturePoseProfileSaveGame::UserIndex))
	{
		Profile = Cast<UGesturePoseProfileSaveGame>(
			UGameplayStatics::LoadGameFromSlot(UGesturePoseProfileSaveGame::SlotName, UGesturePoseProfileSaveGame::UserIndex));
	}

	if (!Profile)
	{
		Profile = Cast<UGesturePoseProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(UGesturePoseProfileSaveGame::StaticClass()));
	}

	if (!Profile)
	{
		return;
	}

	if (Gesture == ECommandGesture::SelectA)
	{
		Profile->SelectAPose = Pose;
		Profile->SelectAEncodedPose = HandPose.CustomEncodedPose;
		Profile->bHasSelectA = true;
	}
	else
	{
		Profile->SelectBPose = Pose;
		Profile->SelectBEncodedPose = HandPose.CustomEncodedPose;
		Profile->bHasSelectB = true;
	}

	UGameplayStatics::SaveGameToSlot(Profile, UGesturePoseProfileSaveGame::SlotName, UGesturePoseProfileSaveGame::UserIndex);
}

void AGestureRecorderPawn::UpdateScreenPrompt()
{
	if (!ScreenActor)
	{
		return;
	}

	if (SelectedRecordGesture == ECommandGesture::SelectA)
	{
		ScreenActor->SetRecordingPrompt(
			FText::FromString(TEXT("手势录制  1/2")),
			FText::FromString(TEXT("队友 A：伸出 1 根手指，然后按 R。")),
			FText::FromString(TEXT("倒计时结束后保持手势，系统会自动保存。")));
	}
	else if (SelectedRecordGesture == ECommandGesture::SelectB)
	{
		ScreenActor->SetRecordingPrompt(
			FText::FromString(TEXT("手势录制  2/2")),
			FText::FromString(TEXT("队友 B：伸出 2 根手指，然后按 R。")),
			FText::FromString(TEXT("倒计时结束后保持手势，系统会自动保存。")));
	}
	else
	{
		ScreenActor->SetRecordingPrompt(
			FText::FromString(TEXT("手势录制  0/2")),
			FText::FromString(TEXT("按 1 选择 A，按 2 选择 B，然后按 R 开始录制。")),
			FText::FromString(TEXT("把右手放到视野中，蓝色手模应该能在屏幕下方看到。")));
	}
}

void AGestureRecorderPawn::ConfigureHandVisuals()
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

FFingerExtensionPose AGestureRecorderPawn::DividePose(const FFingerExtensionPose& Pose, float Divisor)
{
	FFingerExtensionPose Result;
	Result.Thumb = Pose.Thumb / Divisor;
	Result.Index = Pose.Index / Divisor;
	Result.Middle = Pose.Middle / Divisor;
	Result.Ring = Pose.Ring / Divisor;
	Result.Pinky = Pose.Pinky / Divisor;
	return Result;
}
