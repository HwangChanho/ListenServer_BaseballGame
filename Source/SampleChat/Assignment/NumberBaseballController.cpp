#include "NumberBaseballController.h"
#include "FNumberBaseballResult.h"
#include "DebugHelper.h"
#include "NumberBaseballGameMode.h"
#include "NumberBaseballMainWidget.h"
#include "GameFramework/PlayerState.h"

void ANumberBaseballController::BeginPlay()
{
	Super::BeginPlay();

	// 마우스 모드
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	// 위젯 등록
	if (IsLocalController())
	{
		if (!BaseballClass) return;

		if (!BaseballInstance && BaseballClass)
		{
			BaseballInstance = CreateWidget<UNumberBaseballMainWidget>(this, BaseballClass);
		}

		BaseballInstance->AddToViewport();
		BaseballInstance->SetOwningPlayer(this);
	}

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANumberBaseballController] Im Host: %d"), this->GetUniqueID());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANumberBaseballController] I'm Guest: %d"), this->GetUniqueID());
	}
}

void ANumberBaseballController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void ANumberBaseballController::SetCurrentPlayer() const
{
	const int32 PlayerID = this->GetUniqueID();
	BaseballInstance->SetCurrentPlayer(FString::Printf(TEXT("%d"), PlayerID));
}

void ANumberBaseballController::Server_RequestBeginPlay_Implementation(APlayerController* PlayerController)
{
	ANumberBaseballGameMode* GameMode = GetWorld()->GetAuthGameMode<ANumberBaseballGameMode>();
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[ANumberBaseballController] BeginPlay Controller GameMode is NULL!"));
		return;
	}
	
	GameMode->PlayGame();
}

void ANumberBaseballController::Server_RequestReady_Implementation(APlayerController* PlayerController)
{
	ANumberBaseballGameMode* GameMode = GetWorld()->GetAuthGameMode<ANumberBaseballGameMode>();
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[ANumberBaseballController] BeginPlay Controller GameMode is NULL!"));
		return;
	}
	
	GameMode->PlayerReady(PlayerController);
}

void ANumberBaseballController::Client_IsOut_Implementation(int32 PlayerID)
{
	const FString JoinLog = FString::Printf(TEXT("[Player %d] Out"), PlayerID);
	DebugHelper::PrintDebugMessage(JoinLog, WarningDisplayTime, ServerColor);

	// TODO::위젯 변경
	
}

void ANumberBaseballController::Client_IsWinner_Implementation(int32 PlayerID)
{
	const FString JoinLog = FString::Printf(TEXT("[Player %d] Won!"), PlayerID);
	DebugHelper::PrintDebugMessage(JoinLog, DisplayTime, ServerColor);
}

void ANumberBaseballController::Client_TurnStart_Implementation(int32 PlayerID, FNumberBaseballResult Result)
{
	BaseballInstance->SetTurn(FString::Printf(TEXT("%d"), PlayerID));
	BaseballInstance->SetDisplay(Result);
}

void ANumberBaseballController::Client_SendResult_Implementation(FNumberBaseballResult Result)
{
	if (!BaseballInstance) return;
	BaseballInstance->SetDisplay(Result);
}

void ANumberBaseballController::Server_SendMessage_Implementation(const FString& Message)
{
	if (ANumberBaseballGameMode* GameMode = Cast<ANumberBaseballGameMode>(GetWorld()->GetAuthGameMode()))
	{
		FString FormattedMessage = FString::Printf(TEXT("[Player%d]: %s"), this->GetUniqueID(), *Message);
		GameMode->BroadcastMessageToAllControllers(FormattedMessage);
	}
}

void ANumberBaseballController::Client_ReceiveMessage_Implementation(const FString& Message)
{
	if (!BaseballInstance) return;
	BaseballInstance->AddMessage(FText::FromString(Message));
}

void ANumberBaseballController::SendMessageToServer(const FString& Message)
{
	if (Message[0] == TEXT('/'))
	{
		FString Text = Message.Mid(1);

		if (Text == TEXT("ready"))
		{
			Server_RequestReady(this);
			SetCurrentPlayer();
			return;
		}

		if (Text == TEXT("start")) // Host 만 시작 가능
		{
			if (!HasAuthority()) return;
			UE_LOG(LogTemp, Warning, TEXT("[Server] Start Game!"));
			Server_RequestBeginPlay(this);
			return;
		}
		
		TSet<TCHAR> UniqueChars;

		for (TCHAR Char : Text)
		{
			if (!FChar::IsDigit(Char))
			{
				const FString FormattedMessage = FString::Printf(TEXT("유효하지 않은 문자 포함: [%s]"), *Text);
				DebugHelper::PrintDebugMessage(FormattedMessage, WarningDisplayTime, WarningColor);
			}
			UniqueChars.Add(Char);
		}

		if (UniqueChars.Num() == 3)
		{
			UE_LOG(LogTemp, Log, TEXT("유효한 입력: %s"), *Text);
			DebugHelper::PrintDebugMessage(Text, DisplayTime, ServerColor);
			Server_SendNumber(this, Message);
		}

		const FString FormattedMessage = FString::Printf(TEXT("중복 숫자 포함: [%s]"), *Text);
		DebugHelper::PrintDebugMessage(FormattedMessage, WarningDisplayTime, WarningColor);
	}
	else
	{
		Server_SendMessage(Message);
	}
}

void ANumberBaseballController::Server_SendNumber_Implementation(APlayerController* PlayerController, const FString& NumString)
{
	FString NumberPart = NumString.Mid(1);
		
	ANumberBaseballGameMode* GameMode = GetWorld()->GetAuthGameMode<ANumberBaseballGameMode>();
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] GameMode is NULL!"));
		return;
	}

	if (GameMode->CheckTurn(PlayerController))
	{
		GameMode->CheckNumberString(this, NumString);
	}
	else
	{
		int32 PlayerID = PlayerController->GetUniqueID();
		const FString FormattedMessage = FString::Printf(TEXT("[Player %d] It's not your turn"), PlayerID);
		DebugHelper::PrintDebugMessage(FormattedMessage, WarningDisplayTime, WarningColor);
	}
}