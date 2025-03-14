#include "NumberBaseballMainWidget.h"
#include "FNumberBaseballResult.h"
#include "NumberBaseballController.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"

void UNumberBaseballMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (NumberTextField)
	{
		NumberTextField->OnTextCommitted.AddDynamic(this, &UNumberBaseballMainWidget::OnTextCommittedFunction);
	}
}

void UNumberBaseballMainWidget::OnTextCommittedFunction(const FText& Text, const ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		ANumberBaseballController* Controller = Cast<ANumberBaseballController>(GetOwningPlayer());
		if (!Controller)
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerController is NULL!"));
			return;
		}
		
		Controller->Server_SendNumber(Text.ToString());
		NumberTextField->SetText(FText::FromString(""));
	}
}

void UNumberBaseballMainWidget::SetDisplay(FNumberBaseballResult& Result)
{
	if (!BallTextBlock || !StrikeTextBlock || !TurnTextBlock) return;

	BallTextBlock->SetText(FText::FromString(FString::Printf(TEXT("B: %d"), Result.BallCount)));
	StrikeTextBlock->SetText(FText::FromString(FString::Printf(TEXT("S: %d"), Result.StrikeCount)));
	TurnTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Turn: %d"), Result.TurnLeft)));
}

void UNumberBaseballMainWidget::SetTurn(const FString& Player)
{
	if (!PlayerTextBlock) return;

	PlayerTextBlock->SetText(FText::FromString(Player + " Turn"));
}


