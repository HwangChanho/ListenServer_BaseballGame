#include "NumberBaseballMainWidget.h"
#include "FNumberBaseballResult.h"
#include "NumberBaseballController.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UNumberBaseballMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(NumberTextField))
	{
		NumberTextField->OnTextCommitted.AddDynamic(this, &UNumberBaseballMainWidget::OnTextCommittedFunction);
	}
}

void UNumberBaseballMainWidget::AddMessage(const FText& Message)
{
	if (!IsValid(ChatScrollBox)) return;

	UTextBlock* NewMessage = NewObject<UTextBlock>(this);
	if (NewMessage)
	{
		NewMessage->SetText(Message);
		NewMessage->Font.Size = 20;
		NewMessage->SetColorAndOpacity(FSlateColor(FLinearColor::White));

		ChatScrollBox->AddChild(NewMessage);
		ChatScrollBox->ScrollToEnd();
	}
}

void UNumberBaseballMainWidget::AddScore(const FText& Message)
{
	if (!IsValid(ScoreBoardScrollBox)) return;

	UTextBlock* NewMessage = NewObject<UTextBlock>(this);
	if (NewMessage)
	{
		NewMessage->SetText(Message);
		NewMessage->Font.Size = 15;
		NewMessage->SetColorAndOpacity(FSlateColor(FLinearColor::White));

		ScoreBoardScrollBox->AddChild(NewMessage);
		ScoreBoardScrollBox->ScrollToEnd();
	}
}

void UNumberBaseballMainWidget::OnTextCommittedFunction(const FText& Text, const ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		if (Text.IsEmpty()) return;
		
		ANumberBaseballController* Controller = Cast<ANumberBaseballController>(GetOwningPlayer());
		if (!Controller)
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerController is NULL!"));
			return;
		}
		
		Controller->SendMessageToServer(Text.ToString());
		NumberTextField->SetText(FText::FromString(""));
	}
}

void UNumberBaseballMainWidget::SetDisplay(FNumberBaseballResult& Result)
{
	if (!IsValid(BallTextBlock) || !IsValid(StrikeTextBlock) || !IsValid(TurnTextBlock)) return;

	BallTextBlock->SetText(FText::FromString(FString::Printf(TEXT("B: %d"), Result.BallCount)));
	StrikeTextBlock->SetText(FText::FromString(FString::Printf(TEXT("S: %d"), Result.StrikeCount)));
	TurnTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Turn: %d"), Result.TurnLeft)));
}

void UNumberBaseballMainWidget::SetTurn(const FString& Player)
{
	if (!PlayerTextBlock) return;

	PlayerTextBlock->SetText(FText::FromString(Player));
}

void UNumberBaseballMainWidget::SetTimerText(const FString& Time)
{
	if (!TimerTextBlock) return;

	TimerTextBlock->SetText(FText::FromString(FString(TEXT("Time: ")) + Time));
}

void UNumberBaseballMainWidget::SetCurrentPlayer(const FString& Player)
{
	if (!CurrentPlayerTextBlock) return;

	CurrentPlayerTextBlock->SetText(FText::FromString(Player + FString(TEXT(" Turn"))));
}
