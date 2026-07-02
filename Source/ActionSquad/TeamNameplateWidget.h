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

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Nameplate")
	void SetEnemy(bool bInEnemy);

	UFUNCTION(BlueprintCallable, Category = "Action Squad|Nameplate")
	void SetHealthPercent(float InHealthPercent, bool bInShowHealthBar);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Nameplate")
	FText TeamLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Nameplate")
	bool bSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Nameplate")
	bool bEnemy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Nameplate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Squad|Nameplate")
	bool bShowHealthBar = false;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SynchronizeProperties() override;

private:
	void PushState();

	TSharedPtr<class STeamNameplatePanel> SlatePanel;
};
