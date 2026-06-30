#include "GestureRecorderScreenActor.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TutorialCommandWidget.h"

AGestureRecorderScreenActor::AGestureRecorderScreenActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	RecorderWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("RecorderWidget"));
	RecorderWidgetComponent->SetupAttachment(SceneRoot);
	RecorderWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	RecorderWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RecorderWidgetComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	RecorderWidgetComponent->SetGenerateOverlapEvents(false);
	RecorderWidgetComponent->SetTwoSided(true);
	RecorderWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	RecorderWidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);

	TitleLineText = FText::FromString(TEXT("手势录制  0/2"));
	InstructionLineText = FText::FromString(TEXT("按 1 选择 A，按 2 选择 B，然后按 R 开始录制。"));
	StatusLineText = FText::FromString(TEXT("把右手放到视野中，蓝色手模应该能在屏幕下方看到。"));
	PoseLineText = FText::FromString(TEXT("等待右手追踪数据..."));

	ApplyScreenLayout();
}

void AGestureRecorderScreenActor::BeginPlay()
{
	Super::BeginPlay();

	InitializeWidget();
}

void AGestureRecorderScreenActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (EnsureCamera())
	{
		UpdateTransform(DeltaSeconds);
	}
}

void AGestureRecorderScreenActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyScreenLayout();
}

void AGestureRecorderScreenActor::SetStatusText(const FText& InStatusText)
{
	StatusLineText = InStatusText;
	ApplyContentToWidget();
}

void AGestureRecorderScreenActor::SetRecordingPrompt(const FText& InTitleText, const FText& InInstructionText, const FText& InStatusText)
{
	TitleLineText = InTitleText;
	InstructionLineText = InInstructionText;
	StatusLineText = InStatusText;
	UpdateRecorderStepFromTitle();
	ApplyContentToWidget();
}

void AGestureRecorderScreenActor::UpdatePoseText(const FFingerExtensionPose& Pose)
{
	const FString Text = FString::Printf(
		TEXT("实时右手：拇指 %.2f   食指 %.2f   中指 %.2f   无名指 %.2f   小指 %.2f"),
		Pose.Thumb,
		Pose.Index,
		Pose.Middle,
		Pose.Ring,
		Pose.Pinky);
	PoseLineText = FText::FromString(Text);
	ApplyContentToWidget();
}

void AGestureRecorderScreenActor::InitializeWidget()
{
	if (!RecorderWidgetComponent)
	{
		return;
	}

	ApplyScreenLayout();
	RecorderWidgetComponent->SetWidgetClass(UTutorialCommandWidget::StaticClass());
	RecorderWidgetComponent->InitWidget();
	CachedWidget = Cast<UTutorialCommandWidget>(RecorderWidgetComponent->GetWidget());
	ApplyContentToWidget();
}

void AGestureRecorderScreenActor::ApplyScreenLayout()
{
	if (RecorderWidgetComponent)
	{
		RecorderWidgetComponent->SetDrawSize(WidgetDrawSize);
		RecorderWidgetComponent->SetRelativeRotation(WidgetRotationCorrection);
	}

	SetActorScale3D(FVector(FMath::Max(0.01f, ScreenScale)));

	if (CachedWidget)
	{
		CachedWidget->PanelSize = WidgetDrawSize;
	}
}

void AGestureRecorderScreenActor::ApplyContentToWidget()
{
	if (!CachedWidget)
	{
		if (RecorderWidgetComponent && RecorderWidgetComponent->GetWidget())
		{
			CachedWidget = Cast<UTutorialCommandWidget>(RecorderWidgetComponent->GetWidget());
		}
	}

	if (!CachedWidget)
	{
		return;
	}

	CachedWidget->PanelSize = WidgetDrawSize;
	CachedWidget->SetTitleText(TitleLineText);
	CachedWidget->SetHighlightText(InstructionLineText);
	CachedWidget->SetBodyText(StatusLineText);
	CachedWidget->SetFooterText(PoseLineText);
	CachedWidget->SetStepIndicator(CurrentStepIndicator, 2);
}

void AGestureRecorderScreenActor::UpdateRecorderStepFromTitle()
{
	const FString Title = TitleLineText.ToString();
	if (Title.Contains(TEXT("0/2")))
	{
		CurrentStepIndicator = 0;
	}
	else if (Title.Contains(TEXT("1/2")))
	{
		CurrentStepIndicator = 1;
	}
	else if (Title.Contains(TEXT("2/2")))
	{
		CurrentStepIndicator = 2;
	}
}

void AGestureRecorderScreenActor::UpdateTransform(float DeltaSeconds)
{
	if (!PlayerCameraManager)
	{
		return;
	}

	const FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
	const FRotator CameraRotation = PlayerCameraManager->GetCameraRotation();
	const FVector TargetLocation = CameraLocation + CameraRotation.Vector() * FollowDistance + FVector(0.0f, 0.0f, VerticalOffset);

	if (!bHasInitialTransformLock)
	{
		const FRotator InitialRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, CameraLocation);
		SetActorLocationAndRotation(TargetLocation, FRotator(InitialRotation.Pitch, InitialRotation.Yaw, 0.0f));
		bHasInitialTransformLock = true;
		return;
	}

	FVector NewLocation = GetActorLocation();
	if (FVector::DistSquared(NewLocation, TargetLocation) > FMath::Square(0.5f))
	{
		NewLocation = FMath::VInterpTo(NewLocation, TargetLocation, DeltaSeconds, FMath::Max(LocationLerpSpeed, KINDA_SMALL_NUMBER));
	}

	const FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(NewLocation, CameraLocation);
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaSeconds, FMath::Max(RotationLerpSpeed, KINDA_SMALL_NUMBER));
	NewRotation.Roll = 0.0f;

	SetActorLocationAndRotation(NewLocation, NewRotation);
}

bool AGestureRecorderScreenActor::EnsureCamera()
{
	if (!PlayerCameraManager)
	{
		PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	}

	return PlayerCameraManager != nullptr;
}
