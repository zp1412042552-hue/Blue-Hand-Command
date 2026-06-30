#include "TutorialCommandWidget.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Misc/Paths.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace ActionSquadTutorialStyle
{
	static const FLinearColor Background = FLinearColor(0.015f, 0.032f, 0.045f, 0.94f);
	static const FLinearColor SurfaceFill = FLinearColor(0.025f, 0.060f, 0.078f, 0.94f);
	static const FLinearColor SurfaceOutline = FLinearColor(0.22f, 0.74f, 0.96f, 0.72f);
	static const FLinearColor TitleColor = FLinearColor(0.92f, 0.98f, 1.0f, 1.0f);
	static const FLinearColor HighlightColor = FLinearColor(0.08f, 0.82f, 1.0f, 1.0f);
	static const FLinearColor BodyColor = FLinearColor(0.88f, 0.94f, 0.97f, 0.97f);
	static const FLinearColor FooterColor = FLinearColor(0.60f, 0.77f, 0.86f, 0.95f);
	static const FLinearColor StepColor = FLinearColor(0.34f, 0.86f, 1.0f, 1.0f);
	static const FSlateRoundedBoxBrush SurfaceBrush(SurfaceFill, 24.0f, SurfaceOutline, 2.0f);

	inline FSlateFontInfo Font(int32 Size)
	{
		return FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/DroidSansFallback.ttf"), Size);
	}
}

class STutorialCommandPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STutorialCommandPanel) {}
		SLATE_ARGUMENT(FText, TitleText)
		SLATE_ARGUMENT(FText, HighlightText)
		SLATE_ARGUMENT(FText, BodyText)
		SLATE_ARGUMENT(FText, FooterText)
		SLATE_ARGUMENT(FText, StepIndicatorText)
		SLATE_ARGUMENT(FVector2D, PanelSize)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		PanelSize = InArgs._PanelSize;

		ChildSlot
		[
			SNew(SBorder)
			.BorderBackgroundColor(ActionSquadTutorialStyle::Background)
			.Padding(0.0f)
			[
				SAssignNew(RootBox, SBox)
				.WidthOverride(PanelSize.X)
				.HeightOverride(PanelSize.Y)
				[
					SNew(SBorder)
					.BorderImage(&ActionSquadTutorialStyle::SurfaceBrush)
					.Padding(FMargin(34.0f, 16.0f, 34.0f, 14.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(TitleTextBlock, STextBlock)
							.Text(InArgs._TitleText)
							.Font(ActionSquadTutorialStyle::Font(27))
							.ColorAndOpacity(ActionSquadTutorialStyle::TitleColor)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 2.0f, 0.0f, 8.0f))
						[
							SAssignNew(StepTextBlock, STextBlock)
							.Text(InArgs._StepIndicatorText)
							.Font(ActionSquadTutorialStyle::Font(13))
							.ColorAndOpacity(ActionSquadTutorialStyle::StepColor)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(20.0f, 0.0f, 20.0f, 8.0f))
						[
							SAssignNew(HighlightTextBlock, STextBlock)
							.Text(InArgs._HighlightText)
							.Font(ActionSquadTutorialStyle::Font(23))
							.ColorAndOpacity(ActionSquadTutorialStyle::HighlightColor)
							.AutoWrapText(true)
							.Justification(ETextJustify::Center)
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.VAlign(VAlign_Center)
						.Padding(FMargin(18.0f, 0.0f))
						[
							SAssignNew(BodyTextBlock, STextBlock)
							.Text(InArgs._BodyText)
							.Font(ActionSquadTutorialStyle::Font(20))
							.ColorAndOpacity(ActionSquadTutorialStyle::BodyColor)
							.AutoWrapText(true)
							.Justification(ETextJustify::Center)
							.LineHeightPercentage(1.12f)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(16.0f, 8.0f, 16.0f, 0.0f))
						[
							SAssignNew(FooterTextBlock, STextBlock)
							.Text(InArgs._FooterText)
							.Font(ActionSquadTutorialStyle::Font(14))
							.ColorAndOpacity(ActionSquadTutorialStyle::FooterColor)
							.AutoWrapText(true)
							.Justification(ETextJustify::Center)
						]
					]
				]
			]
		];
	}

	void SetPanelSize(const FVector2D& InSize)
	{
		PanelSize = InSize;
		if (RootBox.IsValid())
		{
			RootBox->SetWidthOverride(PanelSize.X);
			RootBox->SetHeightOverride(PanelSize.Y);
		}
	}

	void SetTitleText(const FText& InText) { if (TitleTextBlock.IsValid()) TitleTextBlock->SetText(InText); }
	void SetHighlightText(const FText& InText) { if (HighlightTextBlock.IsValid()) HighlightTextBlock->SetText(InText); }
	void SetBodyText(const FText& InText) { if (BodyTextBlock.IsValid()) BodyTextBlock->SetText(InText); }
	void SetFooterText(const FText& InText) { if (FooterTextBlock.IsValid()) FooterTextBlock->SetText(InText); }
	void SetStepText(const FText& InText) { if (StepTextBlock.IsValid()) StepTextBlock->SetText(InText); }

private:
	FVector2D PanelSize = FVector2D(920.0f, 340.0f);
	TSharedPtr<SBox> RootBox;
	TSharedPtr<STextBlock> TitleTextBlock;
	TSharedPtr<STextBlock> HighlightTextBlock;
	TSharedPtr<STextBlock> BodyTextBlock;
	TSharedPtr<STextBlock> FooterTextBlock;
	TSharedPtr<STextBlock> StepTextBlock;
};

UTutorialCommandWidget::UTutorialCommandWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TitleText = FText::FromString(TEXT("1/5 选择队友"));
	HighlightText = FText::FromString(TEXT("伸出 1 根或 2 根手指"));
	BodyText = FText::FromString(TEXT("1 根手指选择队友 A，2 根手指选择队友 B。被选中的队友会点亮头顶牌并进入警戒姿态。"));
	FooterText = FText::FromString(TEXT("测试键：键盘 1 / 2"));
	StepIndicatorText = FText::FromString(TEXT("●  ○  ○  ○  ○"));
}

TSharedRef<SWidget> UTutorialCommandWidget::RebuildWidget()
{
	SAssignNew(SlatePanel, STutorialCommandPanel)
		.TitleText(TitleText)
		.HighlightText(HighlightText)
		.BodyText(BodyText)
		.FooterText(FooterText)
		.StepIndicatorText(StepIndicatorText)
		.PanelSize(PanelSize);

	return SlatePanel.ToSharedRef();
}

void UTutorialCommandWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	SlatePanel.Reset();
}

void UTutorialCommandWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	PushContent();
}

void UTutorialCommandWidget::SetTitleText(const FText& InText)
{
	TitleText = InText;
	PushContent();
}

void UTutorialCommandWidget::SetBodyText(const FText& InText)
{
	BodyText = InText;
	PushContent();
}

void UTutorialCommandWidget::SetHighlightText(const FText& InText)
{
	HighlightText = InText;
	PushContent();
}

void UTutorialCommandWidget::SetFooterText(const FText& InText)
{
	FooterText = InText;
	PushContent();
}

void UTutorialCommandWidget::SetStepIndicator(int32 CurrentStep, int32 TotalSteps)
{
	FString Indicator;
	for (int32 i = 0; i < TotalSteps; ++i)
	{
		if (i > 0)
		{
			Indicator += TEXT("  ");
		}
		Indicator += i < CurrentStep ? TEXT("●") : TEXT("○");
	}
	StepIndicatorText = FText::FromString(Indicator);
	PushContent();
}

void UTutorialCommandWidget::PushContent()
{
	if (!SlatePanel.IsValid())
	{
		return;
	}

	SlatePanel->SetPanelSize(PanelSize);
	SlatePanel->SetTitleText(TitleText);
	SlatePanel->SetHighlightText(HighlightText);
	SlatePanel->SetBodyText(BodyText);
	SlatePanel->SetFooterText(FooterText);
	SlatePanel->SetStepText(StepIndicatorText);
}
