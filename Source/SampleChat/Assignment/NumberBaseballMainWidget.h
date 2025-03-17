#pragma once

#include "CoreMinimal.h"
#include "FNumberBaseballResult.h"
#include "Blueprint/UserWidget.h"
#include "NumberBaseballMainWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UEditableText;

// 구조체 전방선언 안됨

UCLASS()
class SAMPLECHAT_API UNumberBaseballMainWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UEditableText* NumberTextField;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BallTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StrikeTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TurnTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimerTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentPlayerTextBlock;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScoreBoardScrollBox;
	
	UFUNCTION(BlueprintCallable)
	void OnTextCommittedFunction(const FText& Text, const ETextCommit::Type CommitMethod);

	UFUNCTION(BlueprintCallable)
	void SetDisplay(FNumberBaseballResult& Result);

	UFUNCTION(BlueprintCallable)
	void SetTurn(const FString& Player);

	UFUNCTION(BlueprintCallable)
	void SetCurrentPlayer(const FString& Player);

	UFUNCTION(BlueprintCallable)
	void AddMessage(const FText& Message);

	UFUNCTION(BlueprintCallable)
	void AddScore(const FText& Message);

	UFUNCTION(BlueprintCallable)
	void SetTimerText(const FString& Time);

protected:
	virtual void NativeConstruct() override;
};
