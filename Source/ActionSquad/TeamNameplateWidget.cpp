#include "TeamNameplateWidget.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Misc/Paths.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"

namespace ActionSquadNameplateStyle
{
	static const FLinearColor InactiveFill = FLinearColor(0.03f, 0.04f, 0.045f, 0.88f);
	static const FLinearColor InactiveOutline = FLinearColor(0.20f, 0.28f, 0.32f, 0.75f);
	static const FLinearColor ActiveFill = FLinearColor(0.02f, 0.22f, 0.35f, 0.96f);
	static const FLinearColor ActiveOutline = FLinearColor(0.18f, 0.82f, 1.0f, 1.0f);
	static const FLinearColor EnemyFill = FLinearColor(0.30f, 0.04f, 0.035f, 0.96f);
	static const FLinearColor EnemyOutline = FLinearColor(1.0f, 0.25f, 0.10f, 1.0f);
	static const FLinearColor TextInactive = FLinearColor(0.70f, 0.78f, 0.82f, 0.95f);
	static const FLinearColor TextActive = FLinearColor(0.86f, 0.98f, 1.0f, 1.0f);
	static const FLinearColor TextEnemy = FLinearColor(1.0f, 0.86f, 0.76f, 1.0f);
	static const FLinearColor HealthFill = FLinearColor(1.0f, 0.22f, 0.08f, 1.0f);

	inline FSlateFontInfo Font(int32 Size)
	{
		return FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/DroidSansFallback.ttf"), Size);
	}
}

class STeamNameplatePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STeamNameplatePanel) {}
		SLATE_ARGUMENT(FText, TeamLabel)
		SLATE_ARGUMENT(bool, bSelected)
		SLATE_ARGUMENT(bool, bEnemy)
		SLATE_ARGUMENT(float, HealthPercent)
		SLATE_ARGUMENT(bool, bShowHealthBar)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		TeamLabel = InArgs._TeamLabel;
		bSelected = InArgs._bSelected;
		bEnemy = InArgs._bEnemy;
		HealthPercent = FMath::Clamp(InArgs._HealthPercent, 0.0f, 1.0f);
		bShowHealthBar = InArgs._bShowHealthBar;

		ChildSlot
		[
			SAssignNew(Border, SBorder)
			.Padding(FMargin(18.0f, 8.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SAssignNew(LabelText, STextBlock)
					.Text(TeamLabel)
					.Font(ActionSquadNameplateStyle::Font(54))
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
				[
					SAssignNew(HealthBox, SBox)
					.WidthOverride(150.0f)
					.HeightOverride(12.0f)
					[
						SAssignNew(HealthBar, SProgressBar)
						.Percent(TOptional<float>(HealthPercent))
						.FillColorAndOpacity(ActionSquadNameplateStyle::HealthFill)
					]
				]
			]
		];

		ApplyState();
	}

	void SetTeamLabel(const FText& InLabel)
	{
		TeamLabel = InLabel;
		if (LabelText.IsValid())
		{
			LabelText->SetText(TeamLabel);
		}
	}

	void SetSelected(bool bInSelected)
	{
		bSelected = bInSelected;
		ApplyState();
	}

	void SetEnemy(bool bInEnemy)
	{
		bEnemy = bInEnemy;
		ApplyState();
	}

	void SetHealthPercent(float InHealthPercent, bool bInShowHealthBar)
	{
		HealthPercent = FMath::Clamp(InHealthPercent, 0.0f, 1.0f);
		bShowHealthBar = bInShowHealthBar;
		if (HealthBar.IsValid())
		{
			HealthBar->SetPercent(TOptional<float>(HealthPercent));
		}
		ApplyState();
	}

private:
	void ApplyState()
	{
		const FLinearColor FillColor = bEnemy
			? ActionSquadNameplateStyle::EnemyFill
			: (bSelected ? ActionSquadNameplateStyle::ActiveFill : ActionSquadNameplateStyle::InactiveFill);
		const FLinearColor OutlineColor = bEnemy
			? ActionSquadNameplateStyle::EnemyOutline
			: (bSelected ? ActionSquadNameplateStyle::ActiveOutline : ActionSquadNameplateStyle::InactiveOutline);

		ActiveBrush = FSlateRoundedBoxBrush(
			FillColor,
			14.0f,
			OutlineColor,
			(bSelected || bEnemy) ? 4.0f : 2.0f);

		if (Border.IsValid())
		{
			Border->SetBorderImage(&ActiveBrush);
		}
		if (LabelText.IsValid())
		{
			LabelText->SetColorAndOpacity(
				bEnemy ? ActionSquadNameplateStyle::TextEnemy : (bSelected ? ActionSquadNameplateStyle::TextActive : ActionSquadNameplateStyle::TextInactive));
		}
		if (HealthBox.IsValid())
		{
			HealthBox->SetVisibility(bShowHealthBar ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
		}
		if (HealthBar.IsValid())
		{
			HealthBar->SetPercent(TOptional<float>(HealthPercent));
		}
	}

	FText TeamLabel;
	bool bSelected = false;
	bool bEnemy = false;
	float HealthPercent = 1.0f;
	bool bShowHealthBar = false;
	FSlateRoundedBoxBrush ActiveBrush = FSlateRoundedBoxBrush(FLinearColor::Black, 14.0f);
	TSharedPtr<SBorder> Border;
	TSharedPtr<STextBlock> LabelText;
	TSharedPtr<SBox> HealthBox;
	TSharedPtr<SProgressBar> HealthBar;
};

UTeamNameplateWidget::UTeamNameplateWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TeamLabel = FText::FromString(TEXT("A"));
}

TSharedRef<SWidget> UTeamNameplateWidget::RebuildWidget()
{
	SAssignNew(SlatePanel, STeamNameplatePanel)
		.TeamLabel(TeamLabel)
		.bSelected(bSelected)
		.bEnemy(bEnemy)
		.HealthPercent(HealthPercent)
		.bShowHealthBar(bShowHealthBar);

	return SlatePanel.ToSharedRef();
}

void UTeamNameplateWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	SlatePanel.Reset();
}

void UTeamNameplateWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	PushState();
}

void UTeamNameplateWidget::SetTeamLabel(const FText& InLabel)
{
	TeamLabel = InLabel;
	PushState();
}

void UTeamNameplateWidget::SetSelected(bool bInSelected)
{
	bSelected = bInSelected;
	PushState();
}

void UTeamNameplateWidget::SetEnemy(bool bInEnemy)
{
	bEnemy = bInEnemy;
	PushState();
}

void UTeamNameplateWidget::SetHealthPercent(float InHealthPercent, bool bInShowHealthBar)
{
	HealthPercent = FMath::Clamp(InHealthPercent, 0.0f, 1.0f);
	bShowHealthBar = bInShowHealthBar;
	PushState();
}

void UTeamNameplateWidget::PushState()
{
	if (!SlatePanel.IsValid())
	{
		return;
	}

	SlatePanel->SetTeamLabel(TeamLabel);
	SlatePanel->SetSelected(bSelected);
	SlatePanel->SetEnemy(bEnemy);
	SlatePanel->SetHealthPercent(HealthPercent, bShowHealthBar);
}
