#include "TutorialInstructionActor.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TutorialCommandWidget.h"
#include "TutorialGestureDisplayActor.h"

ATutorialInstructionActor::ATutorialInstructionActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TutorialWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TutorialWidget"));
	TutorialWidgetComponent->SetupAttachment(SceneRoot);
	TutorialWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	TutorialWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TutorialWidgetComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TutorialWidgetComponent->SetGenerateOverlapEvents(false);
	TutorialWidgetComponent->SetTwoSided(true);
	TutorialWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	TutorialWidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);

	BuildDefaultSteps();
	ApplyScreenLayout();
}

void ATutorialInstructionActor::BeginPlay()
{
	Super::BeginPlay();
	InitializeWidget();
	StartTutorial();
}

void ATutorialInstructionActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (EnsureCamera())
	{
		UpdateTransform(DeltaSeconds);
	}
}

void ATutorialInstructionActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyScreenLayout();
}

void ATutorialInstructionActor::StartTutorial()
{
	BuildDefaultSteps();
	InitializeWidget();
	ResolveGestureDisplayActor();
	SetCurrentStep(0);
}

void ATutorialInstructionActor::NotifyGesture(ECommandGesture Gesture)
{
	if (!Steps.IsValidIndex(CurrentStepIndex))
	{
		return;
	}

	const FCommandTutorialStep& Step = Steps[CurrentStepIndex];
	if (Step.RequiredGesture == Gesture)
	{
		SetCurrentStep(FMath::Min(CurrentStepIndex + 1, Steps.Num() - 1));
		return;
	}

	if (CachedWidget)
	{
		CachedWidget->SetFooterText(FText::FromString(TEXT("已识别手势，队友状态已更新。")));
	}
}

void ATutorialInstructionActor::SetCurrentStep(int32 StepIndex)
{
	CurrentStepIndex = Steps.IsValidIndex(StepIndex) ? StepIndex : INDEX_NONE;
	ApplyCurrentStepToWidget();
}

void ATutorialInstructionActor::BuildDefaultSteps()
{
	if (Steps.Num() > 0)
	{
		return;
	}

	FCommandTutorialStep SelectA;
	SelectA.Title = FText::FromString(TEXT("1/5 选择队友 A"));
	SelectA.Highlight = FText::FromString(TEXT("伸出 1 根手指"));
	SelectA.Body = FText::FromString(TEXT("保持 0.5 秒，队友 A 头顶牌点亮，并进入警戒姿态。"));
	SelectA.FooterHint = FText::FromString(TEXT("测试键：1"));
	SelectA.RequiredGesture = ECommandGesture::SelectA;
	SelectA.GestureDisplayIndex = 0;
	Steps.Add(SelectA);

	FCommandTutorialStep SelectB;
	SelectB.Title = FText::FromString(TEXT("2/5 选择队友 B"));
	SelectB.Highlight = FText::FromString(TEXT("伸出 2 根手指"));
	SelectB.Body = FText::FromString(TEXT("保持 0.5 秒，队友 B 头顶牌点亮，并进入警戒姿态。"));
	SelectB.FooterHint = FText::FromString(TEXT("测试键：2"));
	SelectB.RequiredGesture = ECommandGesture::SelectB;
	SelectB.GestureDisplayIndex = 1;
	Steps.Add(SelectB);

	FCommandTutorialStep Action;
	Action.Title = FText::FromString(TEXT("3/5 行动命令"));
	Action.Highlight = FText::FromString(TEXT("食指指向目标"));
	Action.Body = FText::FromString(TEXT("后续会根据射线命中的地面、门或敌人，切换移动、破门或集火。"));
	Action.FooterHint = FText::FromString(TEXT("下一阶段接入射线命令。"));
	Action.RequiredGesture = ECommandGesture::Action;
	Action.GestureDisplayIndex = 2;
	Steps.Add(Action);

	FCommandTutorialStep Watch;
	Watch.Title = FText::FromString(TEXT("4/5 警戒命令"));
	Watch.Highlight = FText::FromString(TEXT("手掌竖起"));
	Watch.Body = FText::FromString(TEXT("队友就地找掩体，进入高阶戒备并允许自主索敌。"));
	Watch.FooterHint = FText::FromString(TEXT("下一阶段接入警戒命令。"));
	Watch.RequiredGesture = ECommandGesture::Watch;
	Watch.GestureDisplayIndex = 3;
	Steps.Add(Watch);

	FCommandTutorialStep Recall;
	Recall.Title = FText::FromString(TEXT("5/5 集合召回"));
	Recall.Highlight = FText::FromString(TEXT("握拳拉回胸口"));
	Recall.Body = FText::FromString(TEXT("全队放弃当前任务，回到玩家两侧护卫。"));
	Recall.FooterHint = FText::FromString(TEXT("下一阶段接入召回路径。"));
	Recall.RequiredGesture = ECommandGesture::Recall;
	Recall.GestureDisplayIndex = 4;
	Steps.Add(Recall);
}

void ATutorialInstructionActor::InitializeWidget()
{
	if (!TutorialWidgetComponent)
	{
		return;
	}

	ApplyScreenLayout();
	TutorialWidgetComponent->SetWidgetClass(UTutorialCommandWidget::StaticClass());
	TutorialWidgetComponent->InitWidget();
	CachedWidget = Cast<UTutorialCommandWidget>(TutorialWidgetComponent->GetWidget());
}

void ATutorialInstructionActor::ApplyScreenLayout()
{
	if (TutorialWidgetComponent)
	{
		TutorialWidgetComponent->SetDrawSize(WidgetDrawSize);
	}

	SetActorScale3D(FVector(FMath::Max(0.01f, ScreenScale)));
}

void ATutorialInstructionActor::ApplyCurrentStepToWidget()
{
	if (!CachedWidget || !Steps.IsValidIndex(CurrentStepIndex))
	{
		return;
	}

	const FCommandTutorialStep& Step = Steps[CurrentStepIndex];
	CachedWidget->SetTitleText(Step.Title);
	CachedWidget->SetHighlightText(Step.Highlight);
	CachedWidget->SetBodyText(Step.Body);
	CachedWidget->SetFooterText(Step.FooterHint);
	CachedWidget->SetStepIndicator(CurrentStepIndex + 1, Steps.Num());

	ResolveGestureDisplayActor();
	if (GestureDisplayActor)
	{
		GestureDisplayActor->SetActorHiddenInGame(false);
		GestureDisplayActor->PlayGestureDemo(Step.GestureDisplayIndex);
		UpdateGestureDisplayTransform();
	}
}

void ATutorialInstructionActor::ResolveGestureDisplayActor()
{
	if (GestureDisplayActor || !GetWorld())
	{
		return;
	}

	if (!bAutoSpawnGestureDisplayActor)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GestureDisplayActor = GetWorld()->SpawnActor<ATutorialGestureDisplayActor>(
		ATutorialGestureDisplayActor::StaticClass(),
		GetActorTransform(),
		SpawnParams);
}

void ATutorialInstructionActor::UpdateTransform(float DeltaSeconds)
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
		UpdateGestureDisplayTransform();
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
	UpdateGestureDisplayTransform();
}

void ATutorialInstructionActor::UpdateGestureDisplayTransform()
{
	if (!GestureDisplayActor)
	{
		return;
	}

	const FVector GestureLocation = GetActorLocation() + GetActorQuat().RotateVector(GestureDisplayActor->WidgetOffset);
	const FQuat GestureRotation =
		GetActorQuat()
		* FQuat(FVector::UpVector, PI)
		* FQuat(FVector::UpVector, FMath::DegreesToRadians(GestureDisplayActor->ModelForwardCorrectionYawDegrees));

	GestureDisplayActor->SetTargetDisplayWidth(WidgetDrawSize.X * FMath::Max(0.01f, ScreenScale) * GestureDisplayActor->TargetWidthFractionOfScreen);
	GestureDisplayActor->SetActorLocationAndRotation(GestureLocation, GestureRotation.Rotator());
}

bool ATutorialInstructionActor::EnsureCamera()
{
	if (!PlayerCameraManager)
	{
		PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	}

	return PlayerCameraManager != nullptr;
}
