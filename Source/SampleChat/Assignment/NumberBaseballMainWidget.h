#pragma once

#include "CoreMinimal.h"
#include "FNumberBaseballResult.h"
#include "Blueprint/UserWidget.h"
#include "NumberBaseballMainWidget.generated.h"

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
	
	UFUNCTION()
	void OnTextCommittedFunction(const FText& Text, const ETextCommit::Type CommitMethod);

	UFUNCTION()
	void SetDisplay(FNumberBaseballResult& Result);

	UFUNCTION()
	void SetTurn(const FString& Player);

protected:
	virtual void NativeConstruct() override;
};
