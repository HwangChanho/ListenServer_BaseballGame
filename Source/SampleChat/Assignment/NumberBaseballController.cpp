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
		UE_LOG(LogTemp, Warning, TEXT("[ANumberBaseballController] Im Host: %p"), this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANumberBaseballController] I'm Guest: %p"), this);
	}
}

void ANumberBaseballController::Server_RequestBeginPlay_Implementation(APlayerController* PlayerController)
{
	// 플레이어 등록
	ANumberBaseballGameMode* GameMode = GetWorld()->GetAuthGameMode<ANumberBaseballGameMode>();
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[ANumberBaseballController] BeginPlay Controller GameMode is NULL!"));
		return;
	}
	
	GameMode->PlayGame();
}

void ANumberBaseballController::Client_IsOut_Implementation(APlayerController* PlayerController)
{
	const int32 PlayerID = PlayerController->PlayerState ? PlayerController->PlayerState->GetPlayerId() : -1;
	const FString JoinLog = FString::Printf(TEXT("[Player%d] Out"), PlayerID);
	DebugHelper::PrintDebugMessage(JoinLog, WarningDisplayTime, ServerColor);

	// TODO::위젯 변경
	
}

void ANumberBaseballController::Client_IsWinner_Implementation(APlayerController* PlayerController)
{
	const int32 PlayerID = PlayerController->PlayerState ? PlayerController->PlayerState->GetPlayerId() : -1;
	const FString JoinLog = FString::Printf(TEXT("[Player%d] Won!"), PlayerID);
	DebugHelper::PrintDebugMessage(JoinLog, DisplayTime, ServerColor);
}

void ANumberBaseballController::Client_TurnStart_Implementation(APlayerController* PlayerController)
{
	UE_LOG(LogTemp, Error, TEXT("[Client] First Player: %p"), PlayerController);
	int32 PlayerID = PlayerController->PlayerState ? PlayerController->PlayerState->GetPlayerId() : -1;
	const FString FormattedMessage = FString::Printf(TEXT("[Player%d] Turn"), PlayerID);
	DebugHelper::PrintDebugMessage(FormattedMessage, DisplayTime, ServerColor);

	BaseballInstance->SetTurn(FString::Printf(TEXT("%d"), PlayerID));
}

void ANumberBaseballController::Client_SendResult_Implementation(FNumberBaseballResult Result)
{
	if (!BaseballInstance) return;
	BaseballInstance->SetDisplay(Result);
}

void ANumberBaseballController::Server_SendNumber_Implementation(const FString& NumString)
{
	ANumberBaseballGameMode* GameMode = GetWorld()->GetAuthGameMode<ANumberBaseballGameMode>();
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] GameMode is NULL!"));
		return;
	}

	FString NumberPart = NumString.Mid(1);

	if (NumberPart == TEXT("start"))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] Start Game!"));
		Server_RequestBeginPlay(this);
		return;
	}

	GameMode->CheckNumberString(this, NumString);
}

bool ANumberBaseballController::Server_SendNumber_Validate(const FString& NumString)
{
	if (NumString[0] == TEXT('/'))
	{
		FString NumberPart = NumString.Mid(1); // 시작위치 변경
		TSet<TCHAR> UniqueChars;

		if (NumberPart == TEXT("start")) // start 일 경우 게임시작
		{
			return true;
		}

		for (TCHAR Char : NumberPart)
		{
			if (!FChar::IsDigit(Char))
			{
				const FString FormattedMessage = FString::Printf(TEXT("유효하지 않은 문자 포함: [%s]"), *NumString);
				DebugHelper::PrintDebugMessage(FormattedMessage, WarningDisplayTime, WarningColor);
				return false;
			}
			UniqueChars.Add(Char);
		}

		if (UniqueChars.Num() == 3)
		{
			UE_LOG(LogTemp, Log, TEXT("유효한 입력: %s"), *NumberPart);
			DebugHelper::PrintDebugMessage(NumberPart, DisplayTime, ServerColor);
			return true;
		}

		const FString FormattedMessage = FString::Printf(TEXT("중복 숫자 포함: [%s]"), *NumString);
		DebugHelper::PrintDebugMessage(FormattedMessage, WarningDisplayTime, WarningColor);
		return false;
	}
	else
	{
		// TODO::추가 개발 가능 채팅으로 분기
		FString FormattedMessage = FString::Printf(TEXT("[Player%d]: [%s]"), this->PlayerState->GetPlayerId(), *NumString);
		DebugHelper::PrintDebugMessage(FormattedMessage, ChatDisplayTime, ChatColor);
		return false;
	}
}