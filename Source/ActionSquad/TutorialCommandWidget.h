#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialCommandWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ACTIONSQUAD_API UTutorialCommandWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTutorialCommandWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	void SetTitleText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	void SetBodyText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	void SetHighlightText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	void SetFooterText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Tutorial")
	void SetStepIndicator(int32 CurrentStep, int32 TotalSteps);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial")
	FText TitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial")
	FText BodyText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial")
	FText HighlightText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial")
	FText FooterText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial")
	FText StepIndicatorText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Tutorial")
	FVector2D PanelSize = FVector2D(920.0f, 340.0f);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SynchronizeProperties() override;

private:
	void PushContent();

	TSharedPtr<class STutorialCommandPanel> SlatePanel;
};
