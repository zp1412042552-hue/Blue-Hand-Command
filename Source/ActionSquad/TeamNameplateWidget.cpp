#include "TeamNameplateWidget.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Misc/Paths.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace ActionSquadNameplateStyle
{
	static const FLinearColor InactiveFill = FLinearColor(0.03f, 0.04f, 0.045f, 0.88f);
	static const FLinearColor InactiveOutline = FLinearColor(0.20f, 0.28f, 0.32f, 0.75f);
	static const FLinearColor ActiveFill = FLinearColor(0.02f, 0.22f, 0.35f, 0.96f);
	static const FLinearColor ActiveOutline = FLinearColor(0.18f, 0.82f, 1.0f, 1.0f);
	static const FLinearColor TextInactive = FLinearColor(0.70f, 0.78f, 0.82f, 0.95f);
	static const FLinearColor TextActive = FLinearColor(0.86f, 0.98f, 1.0f, 1.0f);

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
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		TeamLabel = InArgs._TeamLabel;
		bSelected = InArgs._bSelected;

		ChildSlot
		[
			SAssignNew(Border, SBorder)
			.Padding(FMargin(22.0f, 8.0f))
			[
				SAssignNew(LabelText, STextBlock)
				.Text(TeamLabel)
				.Font(ActionSquadNameplateStyle::Font(54))
				.Justification(ETextJustify::Center)
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

private:
	void ApplyState()
	{
		ActiveBrush = FSlateRoundedBoxBrush(
			bSelected ? ActionSquadNameplateStyle::ActiveFill : ActionSquadNameplateStyle::InactiveFill,
			14.0f,
			bSelected ? ActionSquadNameplateStyle::ActiveOutline : ActionSquadNameplateStyle::InactiveOutline,
			bSelected ? 4.0f : 2.0f);

		if (Border.IsValid())
		{
			Border->SetBorderImage(&ActiveBrush);
		}
		if (LabelText.IsValid())
		{
			LabelText->SetColorAndOpacity(bSelected ? ActionSquadNameplateStyle::TextActive : ActionSquadNameplateStyle::TextInactive);
		}
	}

	FText TeamLabel;
	bool bSelected = false;
	FSlateRoundedBoxBrush ActiveBrush = FSlateRoundedBoxBrush(FLinearColor::Black, 14.0f);
	TSharedPtr<SBorder> Border;
	TSharedPtr<STextBlock> LabelText;
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
		.bSelected(bSelected);

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

void UTeamNameplateWidget::PushState()
{
	if (!SlatePanel.IsValid())
	{
		return;
	}

	SlatePanel->SetTeamLabel(TeamLabel);
	SlatePanel->SetSelected(bSelected);
}
