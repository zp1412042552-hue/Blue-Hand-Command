#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeamNameplateWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ACTIONSQUAD_API UTeamNameplateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTeamNameplateWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Nameplate")
	void SetTeamLabel(const FText& InLabel);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Nameplate")
	void SetSelected(bool bInSelected);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Nameplate")
	FText TeamLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Nameplate")
	bool bSelected = false;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SynchronizeProperties() override;

private:
	void PushState();

	TSharedPtr<class STeamNameplatePanel> SlatePanel;
};
